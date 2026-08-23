# pqfreebsd_kernel

**Copyright © 2026 Brian Fundakowski Feldman.**  
**License: [Light-ware](LICENSE).** See **[NOTICE.md](NOTICE.md)**.

Tiny FreeBSD KLDs for PQFreeBSD. Skills do not build this tree on demand.

## Required vs optional

| Requirement | Module |
| --- | --- |
| **Always required** | `pqfreebsd.ko` only — the skeletal core (enforcement / audit state). |
| **As needed to enable** | Any number of sibling `pqfreebsd_*` KLDs (compat, features). The suite may need zero or many of them for a given enablement path; none of them are required merely to have the core loaded. |

The core **does not** load siblings. Userland decides what else to load.
Enabling full PQFreeBSD functionality may pull in several KLDs; the only
hard baseline is the skeleton.

### Why the core matters (even while small)

`pqfreebsd.ko` is **trivial today** on purpose. It will become **extremely
critical** for PQFreeBSD to function properly: it is our **clean path into
locking down kernels without modifying FreeBSD source**. Policy, enablement
gates, and later critical controls hang off this loadable module rather than
out-of-tree kernel patches. Treat it as the permanent in-kernel foothold —
siblings come and go for bugs and features; the core stays.

## Current status (blocker)

**On ZFS hosts, `pqfreebsd_compat_zfs_multilabel` is a live mid-install blocker
for [pqfreebsd](https://github.com/brianreborn/pqfreebsd).** OpenZFS mounts do
not set `MNT_MULTILABEL` (UFS/FFS do — and did before ZFS introduced this
regression), so MAC labels as EAs on ZFS do not work until this compat KLD (or
a proper fix that will go upstream) is in place. A pure UFS/FFS system does
**not** need this module. We are mid-install on ZFS and checking for bugs of
this class.

**Also seen on the same box:** `su` (and similar) are broken. That is **not
directly** the multilabel bug. It may clear once labels can stick on ZFS; do
not fold it into the multilabel KLD’s job.

**Design rule:** do not require anything special of the operator wherever that
would not compromise PQFreeBSD. The userland suite loads installed `.ko` files
quietly from `onestart`. The **core module does not load compat KLDs**.

## Modules

| Module | Role |
| --- | --- |
| `pqfreebsd` | **Required** skeletal core. Suite **enforcement** / **audit** state (`security.pqfreebsd.*`); kernel messages on change. Does **not** load other `pqfreebsd_*` KLDs. |
| `pqfreebsd_compat_zfs_multilabel` | **ZFS-only.** Not needed on legacy **UFS/FFS** — those already set `MNT_MULTILABEL` (the store predated ZFS and never had this regression). On ZFS: set `MNT_MULTILABEL` when effective `xattr` is `on`/`dir` or `sa`. `MODULE_DEPEND`s on `pqfreebsd` + `zfsctrl`. Suite loads it only when ZFS is in play. |

```
pqfreebsd.ko                    ← ONLY module always required (UFS or ZFS)
 ├── optional dependents (MODULE_DEPEND on pqfreebsd; suite loads as needed)
│   pqfreebsd_compat_zfs_multilabel.ko   ← ZFS hosts only
│   pqfreebsd_* …
```

### Core sysctls

| Sysctl | Default | Effect today |
| --- | --- | --- |
| `security.pqfreebsd.enforcement` | `0` (RWTUN) | On change: `pqfreebsd: enforcement enabled\|disabled` |
| `security.pqfreebsd.audit` | `0` (RWTUN) | On change: `pqfreebsd: audit enabled\|disabled` |

### Hygiene (vs shipped KLDs)

Checked against patterns from `mac_seeotheruids` / `mac_ifoff` (sysctl flags),
`siftr` / `alq` (`MOD_QUIESCE` before teardown), and `g_journal` /
`suspend_all_fs` (mountlist + `vfs_busy`):

- No `MTX_SYSINIT` in the core for flag ints (plain ints + `SYSCTL_PROC` log).
- Compat drops `vfs_mounted` in `MOD_QUIESCE` / `MOD_SHUTDOWN`, not only unload.
- Core does not `kldload` siblings; `MODULE_DEPEND` orders dependents.
- Register the mount handler **before** the initial scan.

## Compatibility

Targets **recent FreeBSD -RELEASE** lines of the last several years (practically
13/14/15-era). The VFS / sysctl surface used here has been stable; these modules
are not expected to need per-release ifdefs for that API. Always build against
the host’s own `sys` tree.

## Build / install

```sh
git clone https://github.com/brianreborn/pqfreebsd_kernel.git
cd pqfreebsd_kernel
make                          # or: make SYSDIR=/path/to/sys
make install
```

**Before check-in:** `make clean all` (and a second `SYSDIR` when available),
then commit. Do not push untested `.c` changes.

Suite load (userland — not the core KLD):

```
# loader.conf optional
pqfreebsd_load="YES"
pqfreebsd_compat_zfs_multilabel_load="YES"
```

`service pqfreebsd onestart` loads the required core, then any sibling
`.ko` files that are installed and wanted for that host’s enablement.

Pin: `git ls-remote https://github.com/brianreborn/pqfreebsd_kernel.git HEAD`
