/*-
 * Copyright (c) 2026 Brian Fundakowski Feldman. All rights reserved.
 * Light-ware License — see LICENSE at the repository root.
 *
 * Core PQFreeBSD KLD. Compat modules MODULE_DEPEND on this.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>

static int
pqfreebsd_modevent(module_t mod __unused, int type, void *data __unused)
{

	switch (type) {
	case MOD_LOAD:
		printf("pqfreebsd: loaded\n");
		return (0);
	case MOD_UNLOAD:
	case MOD_SHUTDOWN:
		return (0);
	default:
		return (EOPNOTSUPP);
	}
}

static moduledata_t pqfreebsd_mod = {
	"pqfreebsd",
	pqfreebsd_modevent,
	NULL
};

DECLARE_MODULE(pqfreebsd, pqfreebsd_mod, SI_SUB_VFS, SI_ORDER_FIRST);
MODULE_VERSION(pqfreebsd, 1);
