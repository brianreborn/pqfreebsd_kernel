# Copyright (c) 2026 Brian Fundakowski Feldman.
# Light-ware License — see LICENSE.
#
# Top-level build for pqfreebsd_kernel.
#
#   make
#   make -C sys/modules/pqfreebsd_compat_zfs_multilabel
#
# SYSDIR defaults to /usr/src/sys. Override if your FreeBSD source is elsewhere:
#
#   make SYSDIR=/path/to/sys
#
# Test-build with make clean all before check-in. Intended for recent
# FreeBSD -RELEASE lines without per-release ifdefs for this VFS surface.
# Style: sh tools/checkstyle.sh  (checkstyle9.pl, git diff --check, mandoc lint)

.if defined(SYSDIR)
.MAKEFLAGS: SYSDIR=${SYSDIR}
.endif

SUBDIR=	sys/modules

.include <bsd.subdir.mk>

# Manuals (optional): mandoc -T ascii man/man4/pqfreebsd.4
