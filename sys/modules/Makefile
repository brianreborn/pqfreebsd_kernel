# Copyright (c) 2026 Brian Fundakowski Feldman. Light-ware License — see LICENSE.
#
# Collection of small PQFreeBSD kernel modules. Add new modules here as
# SUBDIR entries; each builds with bsd.kmod.mk like the base system.

.if defined(SYSDIR)
.MAKEFLAGS: SYSDIR=${SYSDIR}
.endif

SUBDIR=	pqfreebsd \
	pqfreebsd_compat_zfs_multilabel

.include <bsd.subdir.mk>
