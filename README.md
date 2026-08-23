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
| `pqfreebsd` | **Required** skeletal core. Suite **enforcement** / **audit** state (`security.pqfreebsd.*`); kernel messages on change. Does **not** load other `pqfreebsd_*` KLDs. |
| `pqfreebsd_compat_zfs_multilabel` | **Optional until ZFS MAC labels are needed.** Bug accommodation: `MNT_MULTILABEL` on ZFS with effective `xattr=on`/`dir` or `sa`. `MODULE_DEPEND`s on `pqfreebsd` + `zfsctrl`. Suite loads it, not the core. |

```
pqfreebsd.ko                    ← ONLY module always required
 ├── optional dependents (MODULE_DEPEND on pqfreebsd; suite loads)
│   pqfreebsd_compat_zfs_multilabel.ko
│   pqfreebsd_* …               ← any number, as enablement needs
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

`service pqfreebsd onestart` loads the required core, then any sibling
`.ko` files that are installed and wanted for that host’s enablement.

Pin: `git ls-remote https://github.com/brianreborn/pqfreebsd_kernel.git HEAD`
