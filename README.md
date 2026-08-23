# pqfreebsd_kernel

**Copyright © 2026 Brian Fundakowski Feldman.**  
**License: [Light-ware](LICENSE).** See **[NOTICE.md](NOTICE.md)**.

Tiny FreeBSD KLDs for PQFreeBSD. Requisite and generic — skills may load
the `.ko` later; do not vendor this tree into other projects yet.

## Modules

| Module | Role |
| --- | --- |
| `pqk_zfs_ea` | On load, set `MNT_MULTILABEL` on ZFS mounts with effective `xattr=on`/`dir` or `xattr=sa` (local or inherited). |

## Build / load

```sh
make                          # or: make SYSDIR=/path/to/sys
kldload zfs                   # if needed
kldload ./sys/modules/pqk_zfs_ea/pqk_zfs_ea.ko
```

```
# loader.conf
pqk_zfs_ea_load="YES"
```

Pin: `git ls-remote https://github.com/brianreborn/pqfreebsd_kernel.git HEAD`
