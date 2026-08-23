#!/bin/sh
# SPDX-License-Identifier: LicenseRef-Light-ware
#
# Copyright (c) 2026 Brian Fundakowski Feldman
#
# Local same-session load test.  You run this; it does not write
# loader.conf.  kldload(8) needs root.  Does not change xattr=.
#
#	sh tools/kldtest.sh
#
set -eu
cd "$(dirname "$0")/.."
ROOT=$(pwd)
SYSDIR=${SYSDIR:-/usr/src/sys}
CORE=$ROOT/sys/modules/pqfreebsd/pqfreebsd.ko
COMPATDIR=$ROOT/sys/modules/pqfreebsd_compat_zfs_multilabel
COMPAT=$COMPATDIR/pqfreebsd_compat_zfs_multilabel.ko
st=0

have_zfs() {
	kldstat -q -n zfs 2>/dev/null || mount -t zfs | grep -q .
}

zfs_counts() {
	# mount(8) prints "multilabel" when MNT_MULTILABEL is set.
	_nz=$(mount -t zfs 2>/dev/null | grep -c . || true)
	_nm=$(mount -t zfs 2>/dev/null | grep -c multilabel || true)
	echo "zfs_mounts=${_nz} multilabel=${_nm}"
}

echo "==> build (SYSDIR=$SYSDIR)"
make SYSDIR="$SYSDIR" all
if [ ! -f "$CORE" ] || [ ! -f "$COMPAT" ]; then
	echo "missing .ko under sys/modules" >&2
	exit 1
fi

echo "==> symbols (compat must not need dsl_prop_get_integer)"
if nm -u "$COMPAT" | grep -q dsl_prop_get_integer; then
	echo "FAIL: unresolved dsl_prop_get_integer" >&2
	st=1
else
	echo "ok: no dsl_prop_get_integer"
fi

echo "==> baseline"
zfs_counts
mount -t zfs 2>/dev/null || true

if [ "$(id -u)" -ne 0 ]; then
	echo "==> load skipped (not root)"
	echo "re-run as root: sh tools/kldtest.sh"
	exit "$st"
fi

echo "==> load pqfreebsd"
if kldstat -q -n pqfreebsd; then
	echo "already loaded"
else
	kldload "$CORE"
fi

if have_zfs; then
	echo "==> load pqfreebsd_compat_zfs_multilabel (ZFS)"
	if kldstat -q -n pqfreebsd_compat_zfs_multilabel; then
		echo "already loaded"
	else
		kldload "$COMPAT"
	fi
else
	echo "==> skip compat (no ZFS)"
fi

echo "==> kldstat"
kldstat | grep pqfreebsd || true

echo "==> sysctl security.pqfreebsd"
sysctl security.pqfreebsd || st=1

echo "==> dmesg"
dmesg | grep pqfreebsd || true

echo "==> after load"
zfs_counts
mount -t zfs 2>/dev/null || true

if have_zfs; then
	_nm=$(mount -t zfs 2>/dev/null | grep -c multilabel || true)
	if [ "$_nm" -eq 0 ]; then
		echo "FAIL: no ZFS mount shows multilabel" >&2
		st=1
	else
		echo "ok: multilabel on $_nm ZFS mount(s)"
	fi
fi

exit "$st"
