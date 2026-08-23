# SPDX-License-Identifier: LicenseRef-Light-ware
#
# Copyright (c) 2026 Brian Fundakowski Feldman
#
# Out-of-tree analog of src/sys/modules: one subdirectory per KLD.

.if defined(SYSDIR)
.MAKEFLAGS: SYSDIR=${SYSDIR}
.endif

SUBDIR=	pqfreebsd \
	pqfreebsd_compat_zfs_multilabel
SUBDIR_PARALLEL=

.include <bsd.subdir.mk>
