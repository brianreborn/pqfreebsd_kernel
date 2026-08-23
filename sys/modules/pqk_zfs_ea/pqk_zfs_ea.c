/*-
 * Copyright (c) 2026 Brian Fundakowski Feldman. All rights reserved.
 * Light-ware License — see LICENSE at the repository root.
 *
 * On load and on each new mount: for ZFS with effective xattr on/dir or
 * sa (local or inherited), set MNT_MULTILABEL so MAC can use EAs.
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
#define	PQK_XATTR_DIR	1
#define	PQK_XATTR_SA	2

/* From zfs.ko (zfsctrl). Effective prop; may sleep on spa_namespace_lock. */
int dsl_prop_get_integer(const char *, const char *, uint64_t *, char *);

static eventhandler_tag pqk_mounted_tag;

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
	return (xattr == PQK_XATTR_DIR || xattr == PQK_XATTR_SA);
}

static void
tag_mount(struct mount *mp)
{

	if (strcmp(mp->mnt_stat.f_fstypename, "zfs") != 0)
		return;
	if (!xattr_enabled(mp))
		return;
	MNT_ILOCK(mp);
	mp->mnt_flag |= MNT_MULTILABEL;
	MNT_IUNLOCK(mp);
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

static int
pqk_zfs_ea_modevent(module_t mod __unused, int type, void *data __unused)
{

	switch (type) {
	case MOD_LOAD:
		pqk_mounted_tag = EVENTHANDLER_REGISTER(vfs_mounted,
		    on_vfs_mounted, NULL, EVENTHANDLER_PRI_ANY);
		if (pqk_mounted_tag == NULL)
			return (ENOMEM);
		scan_zfs_mounts();
		printf("pqk_zfs_ea: loaded\n");
		return (0);
	case MOD_UNLOAD:
		if (pqk_mounted_tag != NULL) {
			EVENTHANDLER_DEREGISTER(vfs_mounted, pqk_mounted_tag);
			pqk_mounted_tag = NULL;
		}
		return (0);
	case MOD_SHUTDOWN:
		return (0);
	default:
		return (EOPNOTSUPP);
	}
}

static moduledata_t pqk_zfs_ea_mod = {
	"pqk_zfs_ea",
	pqk_zfs_ea_modevent,
	NULL
};

DECLARE_MODULE(pqk_zfs_ea, pqk_zfs_ea_mod, SI_SUB_VFS, SI_ORDER_ANY);
MODULE_VERSION(pqk_zfs_ea, 1);
MODULE_DEPEND(pqk_zfs_ea, zfsctrl, 1, 1, 1);
