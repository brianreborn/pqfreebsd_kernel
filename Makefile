# SPDX-License-Identifier: LicenseRef-Light-ware
#
# Copyright (c) 2026 Brian Fundakowski Feldman
#
# Top-level build for pqfreebsd_kernel (pqk).
#
#	make
#	make SYSDIR=/path/to/sys
#
# Test-build with make clean all before check-in.
# Style: sh tools/checkstyle.sh
# Local load test (you run it): sh tools/kldtest.sh

.if defined(SYSDIR)
.MAKEFLAGS: SYSDIR=${SYSDIR}
.endif

SUBDIR=	sys/modules \
	share/man
SUBDIR_PARALLEL=

.include <bsd.subdir.mk>
