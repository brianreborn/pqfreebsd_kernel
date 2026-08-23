# pqfreebsd_kernel

**Copyright © 2026 Brian Fundakowski Feldman.**  
**License: [Light-ware](LICENSE).** See **[NOTICE.md](NOTICE.md)**.

Tiny FreeBSD KLDs that **accommodate a trivial bug** blocking PQFreeBSD on
ZFS: OpenZFS mounts do not set `MNT_MULTILABEL` the way UFS does, so TrustedBSD
MAC cannot store labels as EAs. The **proper fix** is upstream. Until then,
these modules paper over it.

**Design rule:** do not require anything special of the operator wherever that
would not compromise PQFreeBSD. PQFreeBSD `onestart` loads these quietly when
the `.ko` files are on the module path. This tree is **not** built on demand by
create-skill trees; it is a separate kernel deliverable.

## Modules

| Module | Role |
| --- | --- |
| `pqfreebsd` | Core KLD. Compat modules depend on this. |
| `pqfreebsd_compat_zfs_multilabel` | On load and on each new mount, set `MNT_MULTILABEL` on ZFS mounts whose effective `xattr` is `on`/`dir` or `sa` (local or inherited). |

## Build / install

Ordinary out-of-tree kmod (needs `SYSDIR` / `/usr/src/sys`):

```sh
git clone https://github.com/brianreborn/pqfreebsd_kernel.git
cd pqfreebsd_kernel
make                          # or: make SYSDIR=/path/to/sys
make install                  # puts .ko on the module path
```

Once installed, PQFreeBSD loads them from `service pqfreebsd onestart` — no
hand `kldload` ritual for the operator. Optional `loader.conf(5)` if you want
them earlier than that service:

```
pqfreebsd_load="YES"
pqfreebsd_compat_zfs_multilabel_load="YES"
```

Pin: `git ls-remote https://github.com/brianreborn/pqfreebsd_kernel.git HEAD`
