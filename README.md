# pqfreebsd_kernel

**Copyright © 2026 Brian Fundakowski Feldman.**  
**License: [Light-ware](LICENSE).** See **[NOTICE.md](NOTICE.md)**.

Tiny FreeBSD KLDs for PQFreeBSD. Requisite and generic — skills may load
the `.ko` later; do not vendor this tree into other projects yet.

## Modules

| Module | Role |
| --- | --- |
| `pqfreebsd` | Core KLD. Compat modules depend on this. |
| `pqfreebsd_compat_zfs_multilabel` | On load and on each new mount, set `MNT_MULTILABEL` on ZFS mounts with effective `xattr=on`/`dir` or `xattr=sa` (local or inherited). |

## Build / load

```sh
make                          # or: make SYSDIR=/path/to/sys
kldload zfs                   # if needed
kldload ./sys/modules/pqfreebsd/pqfreebsd.ko
kldload ./sys/modules/pqfreebsd_compat_zfs_multilabel/pqfreebsd_compat_zfs_multilabel.ko
```

```
# loader.conf
pqfreebsd_load="YES"
pqfreebsd_compat_zfs_multilabel_load="YES"
```

Pin: `git ls-remote https://github.com/brianreborn/pqfreebsd_kernel.git HEAD`
