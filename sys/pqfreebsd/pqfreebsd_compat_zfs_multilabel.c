/*
 * Copyright (c) 2026 Brian Fundakowski Feldman. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-Light-ware
 *
 * Light-ware License — see LICENSE at the repository root.
 */

/*
 * ZFS-only compat.  Legacy UFS/FFS already sets MNT_MULTILABEL and never
 * had this regression.  OpenZFS mounts omit the flag; a proper fix will go
 * upstream.  On load and each new mount, set MNT_MULTILABEL unless the
 * mount has noxattr.  Do not call into zfs.ko: it exports no KPI for the
 * inherited xattr property (dsl_prop_get_integer is local).  OpenZFS
 * default is xattr on (dir or sa).  Suite/loader load this only when ZFS
 * is in play.
 *
 * Eventhandler teardown follows siftr(4)/alq(9): drop the handler in
 * MOD_QUIESCE before MOD_UNLOAD.  Mount walk matches g_journal /
 * suspend_all_fs locking.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/eventhandler.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/queue.h>
#include <sys/syslog.h>
#include <sys/vnode.h>

static eventhandler_tag pqfreebsd_vfs_mounted_tag = NULL;

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

	if (opt_isset(mp, "noxattr"))
		return (0);
	/*
	 * saxattr / dirxattr / xattr mean on.  OpenZFS does not copy the
	 * inherited property into mnt_opt, and zfs.ko does not export
	 * dsl_prop_get_integer.  Default is xattr on.
	 */
	return (1);
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

	if (set)
		log(LOG_INFO,
		    "pqfreebsd_compat_zfs_multilabel: multilabel on %s (%s)\n",
		    mp->mnt_stat.f_mntonname, mp->mnt_stat.f_mntfromname);
}

/*
 * mountlist_mtx + vfs_busy(MBF_NOWAIT|MBF_MNTLSTLOCK).
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
	int error = 0;

	switch (type) {
	case MOD_LOAD:
		/*
		 * Register before the scan so mounts that appear during the
		 * walk are still covered by the handler.
		 */
		pqfreebsd_vfs_mounted_tag = EVENTHANDLER_REGISTER(vfs_mounted,
		    on_vfs_mounted, NULL, EVENTHANDLER_PRI_ANY);
		if (pqfreebsd_vfs_mounted_tag == NULL) {
			error = ENOMEM;
			break;
		}
		scan_zfs_mounts();
		log(LOG_NOTICE, "pqfreebsd_compat_zfs_multilabel: loaded\n");
		break;
	case MOD_QUIESCE:
	case MOD_SHUTDOWN:
		pqfreebsd_compat_zfs_multilabel_stop();
		break;
	case MOD_UNLOAD:
		pqfreebsd_compat_zfs_multilabel_stop();
		break;
	default:
		error = EOPNOTSUPP;
		break;
	}
	return (error);
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
