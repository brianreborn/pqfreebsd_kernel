#!/bin/sh
# Run FreeBSD style(9) / style.mdoc(5) convention tools on this tree.
# Usage: sh tools/checkstyle.sh
set -eu
cd "$(dirname "$0")/.."
SRC=${SRCTOP:-/usr/src}
CHECK=${SRC}/tools/build/checkstyle9.pl
st=0

echo "==> checkstyle9.pl (style(9))"
if [ -f "$CHECK" ]; then
	perl "$CHECK" -f --color=never \
	    sys/modules/pqfreebsd/pqfreebsd.c \
	    sys/modules/pqfreebsd_compat_zfs_multilabel/pqfreebsd_compat_zfs_multilabel.c \
	    sys/modules/pqfreebsd_compat_zfs_multilabel/pqfreebsd_zfs_prop.h \
	    || st=1
else
	echo "skip: $CHECK not found" >&2
fi

echo "==> git diff --check (whitespace)"
git diff --check HEAD -- . || st=1

echo "==> lines > 80 columns (src)"
if awk 'length > 80 { print FILENAME ":" NR ":" length; found=1 }
    END { exit found+0 }' \
    sys/modules/pqfreebsd/pqfreebsd.c \
    sys/modules/pqfreebsd_compat_zfs_multilabel/pqfreebsd_compat_zfs_multilabel.c \
    sys/modules/pqfreebsd_compat_zfs_multilabel/pqfreebsd_zfs_prop.h
then
	:
else
	st=1
fi

echo "==> mandoc -T lint (style.mdoc(5))"
if command -v mandoc >/dev/null 2>&1; then
	# Sibling Xr pages are not in the system mandoc db until installed;
	# strip that STYLE so local trees can lint clean.
	lintout=$(mandoc -T lint man/man4/*.4 2>&1) || true
	lintout=$(printf '%s\n' "$lintout" |
	    grep -v 'referenced manual not found: Xr pqfreebsd' || true)
	if [ -n "$lintout" ]; then
		printf '%s\n' "$lintout"
		st=1
	fi
else
	echo "skip: mandoc not found" >&2
fi

exit "$st"
