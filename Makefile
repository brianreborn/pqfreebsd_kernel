# Copyright (c) 2026 Brian Fundakowski Feldman. Light-ware License — see LICENSE.
#
# Top-level build for pqfreebsd_kernel.
#
#   make
#   make -C sys/modules/pqk_zfs_ea
#
# SYSDIR defaults to /usr/src/sys. Override if your FreeBSD source is elsewhere:
#
#   make SYSDIR=/path/to/sys

.if defined(SYSDIR)
.MAKEFLAGS: SYSDIR=${SYSDIR}
.endif

SUBDIR=	sys/modules

.include <bsd.subdir.mk>
