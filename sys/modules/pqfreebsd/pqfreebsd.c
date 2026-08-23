/*-
 * Copyright (c) 2026 Brian Fundakowski Feldman. All rights reserved.
 * Light-ware License — see LICENSE at the repository root.
 *
 * Skeletal core PQFreeBSD KLD — the only module always required.
 * Owns suite enforcement/audit state for transparency and an audit trail.
 * Does not implement feature policy and does not load sibling KLDs.
 * Compat / feature modules MODULE_DEPEND on this; the suite may load any
 * number of them to enable a given path.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/sysctl.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#define	PQFREEBSD_STATE_ENFORCEMENT	1
#define	PQFREEBSD_STATE_AUDIT		2

/*
 * Suite-level switches. Default off — same spirit as MAC policy modules
 * that stay loaded with enabled=0 until a deliberate window. Changing a
 * flag only logs for now (complete trail / transparency).
 */
static int pqfreebsd_enforcement;
static int pqfreebsd_audit;

static struct mtx pqfreebsd_state_mtx;
MTX_SYSINIT(pqfreebsd_state, &pqfreebsd_state_mtx, "pqfreebsd state", MTX_DEF);

static int
pqfreebsd_sysctl_state(SYSCTL_HANDLER_ARGS)
{
	int *flagp = arg1;
	int which = arg2;
	const char *name;
	int error, val;

	name = (which == PQFREEBSD_STATE_ENFORCEMENT) ? "enforcement" : "audit";

	mtx_lock(&pqfreebsd_state_mtx);
	val = *flagp;
	mtx_unlock(&pqfreebsd_state_mtx);

	error = sysctl_handle_int(oidp, &val, 0, req);
	if (error != 0 || req->newptr == NULL)
		return (error);

	val = val ? 1 : 0;
	mtx_lock(&pqfreebsd_state_mtx);
	if (val != *flagp) {
		*flagp = val;
		mtx_unlock(&pqfreebsd_state_mtx);
		printf("pqfreebsd: %s %s\n", name,
		    val ? "enabled" : "disabled");
	} else
		mtx_unlock(&pqfreebsd_state_mtx);

	return (0);
}

SYSCTL_NODE(_security, OID_AUTO, pqfreebsd, CTLFLAG_RW | CTLFLAG_MPSAFE, 0,
    "PQFreeBSD core state");
SYSCTL_PROC(_security_pqfreebsd, OID_AUTO, enforcement,
    CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_MPSAFE, &pqfreebsd_enforcement,
    PQFREEBSD_STATE_ENFORCEMENT, pqfreebsd_sysctl_state, "I",
    "PQFreeBSD enforcement (0=off, 1=on); kernel message on change");
SYSCTL_PROC(_security_pqfreebsd, OID_AUTO, audit,
    CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_MPSAFE, &pqfreebsd_audit,
    PQFREEBSD_STATE_AUDIT, pqfreebsd_sysctl_state, "I",
    "PQFreeBSD audit (0=off, 1=on); kernel message on change");

static int
pqfreebsd_modevent(module_t mod __unused, int type, void *data __unused)
{

	switch (type) {
	case MOD_LOAD:
		pqfreebsd_enforcement = 0;
		pqfreebsd_audit = 0;
		printf("pqfreebsd: loaded (enforcement=disabled audit=disabled)\n");
		return (0);
	case MOD_UNLOAD:
		printf("pqfreebsd: unloaded (enforcement was %s audit was %s)\n",
		    pqfreebsd_enforcement ? "enabled" : "disabled",
		    pqfreebsd_audit ? "enabled" : "disabled");
		return (0);
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
