# pqfreebsd_kernel

**Public repository.** **Copyright © 2026 Brian Fundakowski Feldman.**  
**License: [Light-ware](LICENSE)** (4-clause BSD + a non-binding ask to help keep the lights on).  
See **[NOTICE.md](NOTICE.md)**.

Small FreeBSD **kernel loadable modules (KLDs)** for PQFreeBSD. Generic and
requisite — not built on demand by Grok skills. Skills may load the `.ko`
files; do **not** vendor this tree into other projects yet.

Unsigned on purpose. Pin a commit if you need a frozen tree.

## Modules

| Module | Role |
| --- | --- |
| `pqk_zfs_ea` | On load (and on each new mount), find active ZFS mounts whose effective `xattr` is `on`/`dir` or `sa` — whether that property is local or inherited — and set `mp->mnt_flag \|= MNT_MULTILABEL` under `MNT_ILOCK`, so MAC can store labels as EAs. |

More tiny modules will land here over time.

## Build

Needs FreeBSD kernel source headers (same as any out-of-tree kmod):

```sh
git clone https://github.com/brianreborn/pqfreebsd_kernel.git
cd pqfreebsd_kernel

# If /usr/src/sys is missing, point at your tree:
#   make SYSDIR=/path/to/sys
make
# or just one module:
make -C sys/modules/pqk_zfs_ea
```

Artifacts: `sys/modules/pqk_zfs_ea/pqk_zfs_ea.ko`.

## Load

`zfs.ko` must already be present (module name `zfsctrl`):

```sh
kldload zfs          # if not already loaded
kldload ./sys/modules/pqk_zfs_ea/pqk_zfs_ea.ko
# or after install:
kldload pqk_zfs_ea
```

`loader.conf(5)`:

```
pqk_zfs_ea_load="YES"
```

## Runtime knobs

| Sysctl | Meaning |
| --- | --- |
| `debug.pqk_zfs_ea.verbose` | Log every mount considered |
| `debug.pqk_zfs_ea.scan` | Write non-zero to rescan all ZFS mounts |
| `debug.pqk_zfs_ea.tagged` / `skipped` / `errors` / `scanned` | Counters |

Unload leaves `MNT_MULTILABEL` set on mounts already tagged.

## Locking (summary)

- Mount list: `mountlist_mtx` + `vfs_busy(MBF_NOWAIT \| MBF_MNTLSTLOCK)` (same pattern as `suspend_all_fs` / `g_journal`).
- Flag update: `MNT_ILOCK` / `MNT_IUNLOCK` around `mnt_flag`.
- Property read: `dsl_prop_get_integer` **after** dropping `mountlist_mtx` (it may sleep on `spa_namespace_lock`; must not hold ZFS teardown lock).
- New mounts: `EVENTHANDLER_REGISTER(vfs_mounted, …)` while the mount is already busy.

## Relation to skills

PQFreeBSD / FreeBSD MAC Grok skills (`freebsd-mac-grok` and successors) will
**use** these modules. They will not build them as part of skill install.
Keep this repository separate from `/create-skill` trees.

## Signatures

Unsigned for now. Later: PGP-signed tags / release artifacts. Until then:

```sh
git ls-remote https://github.com/brianreborn/pqfreebsd_kernel.git HEAD
```
