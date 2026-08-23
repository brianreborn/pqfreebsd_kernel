/*-
 * Copyright (c) 2026 Brian Fundakowski Feldman. All rights reserved.
 *
 * Light-ware License — see LICENSE at the repository root.
 *
 * pqk_zfs_ea: mark active ZFS mounts with MNT_MULTILABEL when the
 * effective xattr property is "on"/"dir" or "sa" (local or inherited),
 * so TrustedBSD MAC can store labels as extended attributes.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <sys/eventhandler.h>
#include <sys/queue.h>

/*
 * OpenZFS xattr property values (sys/fs/zfs.h). Declared locally so this
 * module stays BSD-licensed and does not pull CDDL headers.
 */
#define	PQK_ZFS_XATTR_OFF	0
#define	PQK_ZFS_XATTR_DIR	1	/* "on" / "dir" */
#define	PQK_ZFS_XATTR_SA	2	/* "sa" */

/*
 * dsl_prop_get_integer() is implemented by zfs.ko (module name "zfsctrl").
 * It returns the *effective* property value, whether set locally on the
 * dataset or inherited from a parent. setpoint may be NULL.
 *
 * Locking note (from OpenZFS FreeBSD vfsops): this helper acquires
 * spa_namespace_lock. Callers must not hold z_teardown_lock. Holding a
 * vfs_busy() reference on the mount is enough for our purposes.
 */
int dsl_prop_get_integer(const char *ddname, const char *propname,
    uint64_t *valuep, char *setpoint);

static eventhandler_tag pqk_zfs_ea_mounted_tag;
static int pqk_zfs_ea_verbose = 0;
static unsigned long pqk_zfs_ea_scanned;
static unsigned long pqk_zfs_ea_tagged;
static unsigned long pqk_zfs_ea_skipped;
static unsigned long pqk_zfs_ea_errors;

SYSCTL_NODE(_debug, OID_AUTO, pqk_zfs_ea, CTLFLAG_RW | CTLFLAG_MPSAFE, 0,
    "PQFreeBSD ZFS EA / MNT_MULTILABEL helper");
SYSCTL_INT(_debug_pqk_zfs_ea, OID_AUTO, verbose, CTLFLAG_RW,
    &pqk_zfs_ea_verbose, 0, "Log each mount considered");
SYSCTL_ULONG(_debug_pqk_zfs_ea, OID_AUTO, scanned, CTLFLAG_RD,
    &pqk_zfs_ea_scanned, 0, "ZFS mounts examined");
SYSCTL_ULONG(_debug_pqk_zfs_ea, OID_AUTO, tagged, CTLFLAG_RD,
    &pqk_zfs_ea_tagged, 0, "Mounts that received MNT_MULTILABEL");
SYSCTL_ULONG(_debug_pqk_zfs_ea, OID_AUTO, skipped, CTLFLAG_RD,
    &pqk_zfs_ea_skipped, 0, "Mounts left unchanged");
SYSCTL_ULONG(_debug_pqk_zfs_ea, OID_AUTO, errors, CTLFLAG_RD,
    &pqk_zfs_ea_errors, 0, "Property lookup failures");

static int
pqk_opt_isset(struct mount *mp, const char *name)
{

	if (mp->mnt_opt == NULL)
		return (0);
	return (vfs_getopt(mp->mnt_opt, name, NULL, NULL) == 0);
}

/*
 * Decide whether this ZFS mount should advertise multilabel support.
 *
 * Mount-time overrides (mnt_opt) win, matching zfs_register_callbacks():
 *   noxattr → off
 *   saxattr / dirxattr / xattr → on
 * Otherwise consult the effective dataset property (inherited or local).
 */
static int
pqk_zfs_xattr_enabled(struct mount *mp)
{
	uint64_t xattr;
	const char *from;
	int error;

	if (pqk_opt_isset(mp, "noxattr"))
		return (0);
	if (pqk_opt_isset(mp, "saxattr") ||
	    pqk_opt_isset(mp, "dirxattr") ||
	    pqk_opt_isset(mp, "xattr"))
		return (1);

	from = mp->mnt_stat.f_mntfromname;
	if (from[0] == '\0')
		return (0);

	error = dsl_prop_get_integer(from, "xattr", &xattr, NULL);
	if (error != 0) {
		pqk_zfs_ea_errors++;
		if (pqk_zfs_ea_verbose)
			printf("pqk_zfs_ea: %s: xattr prop lookup failed: %d\n",
			    from, error);
		return (0);
	}

	return (xattr == PQK_ZFS_XATTR_DIR || xattr == PQK_ZFS_XATTR_SA);
}

/*
 * Apply MNT_MULTILABEL under MNT_ILOCK. Caller must hold a vfs_busy()
 * reference (or be inside the vfs_mounted path, where the mount is already
 * busy) so the mount cannot disappear under us.
 */
static void
pqk_zfs_ea_tag_mount(struct mount *mp)
{
	int already;

	pqk_zfs_ea_scanned++;

	if (strcmp(mp->mnt_stat.f_fstypename, "zfs") != 0) {
		pqk_zfs_ea_skipped++;
		return;
	}

	MNT_ILOCK(mp);
	already = (mp->mnt_flag & MNT_MULTILABEL) != 0;
	MNT_IUNLOCK(mp);
	if (already) {
		pqk_zfs_ea_skipped++;
		if (pqk_zfs_ea_verbose)
			printf("pqk_zfs_ea: %s on %s: already multilabel\n",
			    mp->mnt_stat.f_mntfromname,
			    mp->mnt_stat.f_mntonname);
		return;
	}

	if (!pqk_zfs_xattr_enabled(mp)) {
		pqk_zfs_ea_skipped++;
		if (pqk_zfs_ea_verbose)
			printf("pqk_zfs_ea: %s on %s: xattr off, skip\n",
			    mp->mnt_stat.f_mntfromname,
			    mp->mnt_stat.f_mntonname);
		return;
	}

	MNT_ILOCK(mp);
	mp->mnt_flag |= MNT_MULTILABEL;
	MNT_IUNLOCK(mp);

	pqk_zfs_ea_tagged++;
	printf("pqk_zfs_ea: %s on %s: set MNT_MULTILABEL\n",
	    mp->mnt_stat.f_mntfromname, mp->mnt_stat.f_mntonname);
}

/*
 * Walk the global mount list using the established kernel pattern:
 *   mountlist_mtx + vfs_busy(MBF_NOWAIT | MBF_MNTLSTLOCK)
 * vfs_busy drops mountlist_mtx on success; we reacquire it before
 * vfs_unbusy so TAILQ_FOREACH's next hop stays consistent.
 *
 * Do not call dsl_prop_get_integer while holding mountlist_mtx — it may
 * sleep on spa_namespace_lock.
 */
static void
pqk_zfs_ea_scan_all(void)
{
	struct mount *mp;

	mtx_lock(&mountlist_mtx);
	TAILQ_FOREACH(mp, &mountlist, mnt_list) {
		if (strcmp(mp->mnt_stat.f_fstypename, "zfs") != 0)
			continue;
		if (vfs_busy(mp, MBF_NOWAIT | MBF_MNTLSTLOCK) != 0)
			continue;
		/* mountlist_mtx released by vfs_busy() */

		pqk_zfs_ea_tag_mount(mp);

		mtx_lock(&mountlist_mtx);
		vfs_unbusy(mp);
	}
	mtx_unlock(&mountlist_mtx);
}

/*
 * New mounts: vfs_mounted is invoked with the mount already busy and on
 * the mount list (see vfs_mount.c). Do not vfs_busy again.
 */
static void
pqk_zfs_ea_vfs_mounted(void *arg __unused, struct mount *mp,
    struct vnode *fsrootvp __unused, struct thread *td __unused)
{

	pqk_zfs_ea_tag_mount(mp);
}

static int
pqk_zfs_ea_scan_sysctl(SYSCTL_HANDLER_ARGS)
{
	int error, val;

	val = 0;
	error = sysctl_handle_int(oidp, &val, 0, req);
	if (error != 0 || req->newptr == NULL)
		return (error);
	if (val != 0)
		pqk_zfs_ea_scan_all();
	return (0);
}
SYSCTL_PROC(_debug_pqk_zfs_ea, OID_AUTO, scan,
    CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_MPSAFE, NULL, 0,
    pqk_zfs_ea_scan_sysctl, "I",
    "Write non-zero to rescan all ZFS mounts");

static int
pqk_zfs_ea_modevent(module_t mod __unused, int type, void *data __unused)
{
	int error;

	switch (type) {
	case MOD_LOAD:
		pqk_zfs_ea_mounted_tag = EVENTHANDLER_REGISTER(vfs_mounted,
		    pqk_zfs_ea_vfs_mounted, NULL, EVENTHANDLER_PRI_ANY);
		if (pqk_zfs_ea_mounted_tag == NULL) {
			printf("pqk_zfs_ea: failed to register vfs_mounted\n");
			return (ENOMEM);
		}
		pqk_zfs_ea_scan_all();
		printf("pqk_zfs_ea: loaded (tagged %lu, skipped %lu, "
		    "errors %lu)\n",
		    pqk_zfs_ea_tagged, pqk_zfs_ea_skipped, pqk_zfs_ea_errors);
		return (0);

	case MOD_UNLOAD:
		if (pqk_zfs_ea_mounted_tag != NULL) {
			EVENTHANDLER_DEREGISTER(vfs_mounted,
			    pqk_zfs_ea_mounted_tag);
			pqk_zfs_ea_mounted_tag = NULL;
		}
		/*
		 * Leave MNT_MULTILABEL set on mounts we tagged. Clearing it
		 * on unload would race with MAC and surprise operators.
		 */
		printf("pqk_zfs_ea: unloaded\n");
		return (0);

	case MOD_SHUTDOWN:
		return (0);

	default:
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}

static moduledata_t pqk_zfs_ea_mod = {
	"pqk_zfs_ea",
	pqk_zfs_ea_modevent,
	NULL
};

DECLARE_MODULE(pqk_zfs_ea, pqk_zfs_ea_mod, SI_SUB_VFS, SI_ORDER_ANY);
MODULE_VERSION(pqk_zfs_ea, 1);
MODULE_DEPEND(pqk_zfs_ea, zfsctrl, 1, 1, 1);
