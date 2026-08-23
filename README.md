# pqfreebsd_kernel

**Copyright © 2026 Brian Fundakowski Feldman.**  
**License: [Light-ware](LICENSE).** See **[NOTICE.md](NOTICE.md)**.

Tiny FreeBSD KLDs for PQFreeBSD. The **core** module holds suite state; **compat**
modules paper over host bugs or add focused critical features. Skills do not
build this tree on demand.

## Current status (blocker)

**`pqfreebsd_compat_zfs_multilabel` is a live mid-install blocker for
[pqfreebsd](https://github.com/brianreborn/pqfreebsd).** OpenZFS mounts do not
set `MNT_MULTILABEL` (UFS does), so MAC labels as EAs do not work until this
compat KLD (or an upstream fix) is in place. We are mid-install and checking
for bugs of this class.

**Also seen on the same box:** `su` (and similar) are broken. That is **not
directly** the multilabel bug. It may clear once labels can stick on ZFS; do
not fold it into the multilabel KLD’s job.

**Design rule:** do not require anything special of the operator wherever that
would not compromise PQFreeBSD. The userland suite loads installed `.ko` files
quietly from `onestart`. The **core module does not load compat KLDs**.

## Modules

| Module | Role |
| --- | --- |
| `pqfreebsd` | Core. Maintains suite **enforcement** and **audit** state (`security.pqfreebsd.*`). Logs to the kernel message buffer when those flags change. Does **not** load other `pqfreebsd_*` KLDs. Critical feature work lives in sibling modules. |
| `pqfreebsd_compat_zfs_multilabel` | Bug accommodation: set `MNT_MULTILABEL` on ZFS mounts with effective `xattr=on`/`dir` or `xattr=sa`. `MODULE_DEPEND`s on `pqfreebsd` + `zfsctrl`. Loaded by the suite, not by the core. |

```
pqfreebsd.ko          ← core state only; does not kldload children
 ├── (dependents MODULE_DEPEND on pqfreebsd)
│   pqfreebsd_compat_zfs_multilabel.ko
│   …
```

### Core sysctls

| Sysctl | Default | Effect today |
| --- | --- | --- |
| `security.pqfreebsd.enforcement` | `0` | On change: `pqfreebsd: enforcement enabled\|disabled` |
| `security.pqfreebsd.audit` | `0` | On change: `pqfreebsd: audit enabled\|disabled` |

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

`service pqfreebsd onestart` loads core then compat when present.

Pin: `git ls-remote https://github.com/brianreborn/pqfreebsd_kernel.git HEAD`
