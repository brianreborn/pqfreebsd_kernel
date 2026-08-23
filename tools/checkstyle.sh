#!/bin/sh
# Run FreeBSD style(9) / style.mdoc(5) convention tools on this tree.
# Usage: sh tools/checkstyle.sh
set -eu
cd "$(dirname "$0")/.."
SRC=${SRCTOP:-/usr/src}
CHECK=${SRC}/tools/build/checkstyle9.pl
st=0
CSRC="sys/pqfreebsd/pqfreebsd.c \
	sys/pqfreebsd/pqfreebsd_compat_zfs_multilabel.c \
	sys/pqfreebsd/pqfreebsd_zfs_prop.h"

echo "==> checkstyle9.pl (style(9))"
if [ -f "$CHECK" ]; then
	# shellcheck disable=SC2086
	perl "$CHECK" -f --color=never $CSRC || st=1
else
	echo "skip: $CHECK not found" >&2
fi

echo "==> git diff --check (whitespace)"
git diff --check HEAD -- . || st=1

echo "==> lines > 80 columns (src)"
# shellcheck disable=SC2086
if awk 'length > 80 { print FILENAME ":" NR ":" length; found=1 }
    END { exit found+0 }' $CSRC
then
	:
else
	st=1
fi

echo "==> mandoc -T lint (style.mdoc(5))"
if command -v mandoc >/dev/null 2>&1; then
	lintout=$(mandoc -T lint share/man/man4/*.4 2>&1) || true
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
