/*-
 * Copyright (c) 2026 Brian Fundakowski Feldman. All rights reserved.
 * Light-ware License — see LICENSE at the repository root.
 *
 * ZFS-only compat. Legacy UFS/FFS already sets MNT_MULTILABEL and never had
 * this regression. OpenZFS mounts omit the flag; proper fix will go upstream.
 * Mid-install blocker for pqfreebsd on ZFS hosts. On load and each new
 * mount, if effective xattr is on/dir or sa (local or inherited), set
 * MNT_MULTILABEL so MAC can store labels as EAs. Suite loads this only when
 * ZFS is in play — not on pure UFS/FFS. su/login oddities are not this
 * module's job; they may clear once labels can stick.
 *
 * Eventhandler teardown follows siftr(4)/alq(9): drop the handler in
 * MOD_QUIESCE before MOD_UNLOAD so callbacks cannot race module teardown.
 * Mount walk matches g_journal / suspend_all_fs locking.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/mount.h>
#include <sys/queue.h>
#include <sys/eventhandler.h>

/* OpenZFS xattr index values; avoid CDDL headers. */
#define	PQFREEBSD_XATTR_DIR	1
#define	PQFREEBSD_XATTR_SA	2

/* From zfs.ko (zfsctrl). Effective prop; may sleep on spa_namespace_lock. */
int dsl_prop_get_integer(const char *, const char *, uint64_t *, char *);

static eventhandler_tag pqfreebsd_vfs_mounted_tag;

static int
opt_isset(struct mount *mp, const char *name)
{

	if (mp->mnt_opt == NULL)
		return (0);
	return (vfs_getopt(mp->mnt_opt, name, NULL, NULL) == 0);
}

static int
xattr_enabled(struct mount *mp)
{
	uint64_t xattr;
	const char *from;

	if (opt_isset(mp, "noxattr"))
		return (0);
	if (opt_isset(mp, "saxattr") || opt_isset(mp, "dirxattr") ||
	    opt_isset(mp, "xattr"))
		return (1);

	from = mp->mnt_stat.f_mntfromname;
	if (from[0] == '\0')
		return (0);
	if (dsl_prop_get_integer(from, "xattr", &xattr, NULL) != 0)
		return (0);
	return (xattr == PQFREEBSD_XATTR_DIR || xattr == PQFREEBSD_XATTR_SA);
}

static void
tag_mount(struct mount *mp)
{
	int set;

	if (strcmp(mp->mnt_stat.f_fstypename, "zfs") != 0)
		return;
	if (!xattr_enabled(mp))
		return;

	MNT_ILOCK(mp);
	set = (mp->mnt_flag & MNT_MULTILABEL) == 0;
	if (set)
		mp->mnt_flag |= MNT_MULTILABEL;
	MNT_IUNLOCK(mp);

	/* One line per mount we actually modify — shows up in dmesg / messages. */
	if (set)
		printf("pqfreebsd_compat_zfs_multilabel: multilabel on %s (%s)\n",
		    mp->mnt_stat.f_mntonname, mp->mnt_stat.f_mntfromname);
}

/*
 * mountlist_mtx + vfs_busy(MBF_NOWAIT|MBF_MNTLSTLOCK).
 * Drop list lock before dsl_prop_get_integer (may sleep).
 */
static void
scan_zfs_mounts(void)
{
	struct mount *mp;

	mtx_lock(&mountlist_mtx);
	TAILQ_FOREACH(mp, &mountlist, mnt_list) {
		if (strcmp(mp->mnt_stat.f_fstypename, "zfs") != 0)
			continue;
		if (vfs_busy(mp, MBF_NOWAIT | MBF_MNTLSTLOCK) != 0)
			continue;
		tag_mount(mp);
		mtx_lock(&mountlist_mtx);
		vfs_unbusy(mp);
	}
	mtx_unlock(&mountlist_mtx);
}

/* Mount is already busy here; do not vfs_busy again. */
static void
on_vfs_mounted(void *arg __unused, struct mount *mp,
    struct vnode *root __unused, struct thread *td __unused)
{

	tag_mount(mp);
}

static void
pqfreebsd_compat_zfs_multilabel_stop(void)
{

	if (pqfreebsd_vfs_mounted_tag != NULL) {
		EVENTHANDLER_DEREGISTER(vfs_mounted, pqfreebsd_vfs_mounted_tag);
		pqfreebsd_vfs_mounted_tag = NULL;
	}
}

static int
pqfreebsd_compat_zfs_multilabel_modevent(module_t mod __unused, int type,
    void *data __unused)
{

	switch (type) {
	case MOD_LOAD:
		/*
		 * Register before the scan so mounts that appear during the
		 * walk are still covered by the handler.
		 */
		pqfreebsd_vfs_mounted_tag = EVENTHANDLER_REGISTER(vfs_mounted,
		    on_vfs_mounted, NULL, EVENTHANDLER_PRI_ANY);
		if (pqfreebsd_vfs_mounted_tag == NULL)
			return (ENOMEM);
		scan_zfs_mounts();
		printf("pqfreebsd_compat_zfs_multilabel: loaded\n");
		return (0);
	case MOD_QUIESCE:
	case MOD_SHUTDOWN:
		pqfreebsd_compat_zfs_multilabel_stop();
		return (0);
	case MOD_UNLOAD:
		/* Idempotent if QUIESCE already ran. */
		pqfreebsd_compat_zfs_multilabel_stop();
		return (0);
	default:
		return (EOPNOTSUPP);
	}
}

static moduledata_t pqfreebsd_compat_zfs_multilabel_mod = {
	"pqfreebsd_compat_zfs_multilabel",
	pqfreebsd_compat_zfs_multilabel_modevent,
	NULL
};

DECLARE_MODULE(pqfreebsd_compat_zfs_multilabel,
    pqfreebsd_compat_zfs_multilabel_mod, SI_SUB_VFS, SI_ORDER_ANY);
MODULE_VERSION(pqfreebsd_compat_zfs_multilabel, 1);
MODULE_DEPEND(pqfreebsd_compat_zfs_multilabel, pqfreebsd, 1, 1, 1);
MODULE_DEPEND(pqfreebsd_compat_zfs_multilabel, zfsctrl, 1, 1, 1);
