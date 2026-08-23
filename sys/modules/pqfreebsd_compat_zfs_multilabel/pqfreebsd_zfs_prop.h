/*-
 * Copyright (c) 2026 Brian Fundakowski Feldman. All rights reserved.
 * Light-ware License — see LICENSE at the repository root.
 *
 * Thin declaration of an OpenZFS kernel symbol.  Avoid CDDL headers.
 * style(9): do not put this prototype in the .c file.
 */

#ifndef _PQFREEBSD_ZFS_PROP_H_
#define	_PQFREEBSD_ZFS_PROP_H_

int	dsl_prop_get_integer(const char *, const char *, uint64_t *, char *);

#endif /* !_PQFREEBSD_ZFS_PROP_H_ */
