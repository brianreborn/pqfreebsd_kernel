# Manuals (GitHub view)

Canonical sources are [mandoc](https://mandoc.bsd.lv/) pages:

- [`man4/pqfreebsd.4`](man4/pqfreebsd.4) — `mandoc -T ascii man/man4/pqfreebsd.4`
- [`man4/pqfreebsd_compat_zfs_multilabel.4`](man4/pqfreebsd_compat_zfs_multilabel.4)

GitHub does not typeset mdoc. Readable copies follow.

---

## pqfreebsd(4)

**pqfreebsd** — PQFreeBSD skeletal core kernel module

### SYNOPSIS

Load at boot (`loader.conf(5)`):

```
pqfreebsd_load="YES"
```

Or at run time:

```
kldload pqfreebsd
```

### DESCRIPTION

**pqfreebsd** is the always-required **pqk** core module for the pqfreebsd
project (official name PQFreeBSD; evocative form Free(ly)B(le)S(se)D). It
holds suite *enforcement* and *audit* state and logs changes with
`log(9)`. It does not implement feature policy and does not load sibling
`pqfreebsd_*` modules; userland or `loader(8)` loads those separately.

Ideal steady state is `loader(8)` preload. `kldload(8)` is a same-session
fallback until reboot. Later ideal: full TPM e2e on that loader path (not
this cut).

### SYSCTL VARIABLES

Under `security.pqfreebsd`:

| Variable | Meaning |
| --- | --- |
| `enforcement` | 0 off, 1 on (`CTLFLAG_RWTUN`). `log(9)` `LOG_NOTICE` on change. |
| `audit` | Same conventions as `enforcement`. |

### SEE ALSO

[pqfreebsd_compat_zfs_multilabel(4)](#pqfreebsd_compat_zfs_multilabel4),
`mac(4)`, `loader.conf(5)`, `kldload(8)`, `sysctl(8)`, `log(9)`

---

## pqfreebsd_compat_zfs_multilabel(4)

**pqfreebsd_compat_zfs_multilabel** — set `MNT_MULTILABEL` on ZFS mounts
with xattr enabled

### SYNOPSIS

ZFS hosts only. Prefer `loader.conf(5)`:

```
pqfreebsd_load="YES"
pqfreebsd_compat_zfs_multilabel_load="YES"
```

Or at run time after [pqfreebsd(4)](#pqfreebsd4):

```
kldload pqfreebsd_compat_zfs_multilabel
```

### DESCRIPTION

ZFS-only compat for the pqfreebsd project (pqk). Legacy UFS/FFS already
sets `MNT_MULTILABEL` and never had this regression. OpenZFS mounts that
omit the flag receive it when the effective **xattr** property is **on**
(also **dir**) or **sa** (local or inherited). A proper fix will go
upstream.

On `MOD_LOAD` the module registers a `vfs_mounted` handler and scans
active ZFS mounts. Each mount newly marked emits a `log(9)` `LOG_INFO`
line. The handler is dropped in `MOD_QUIESCE`.

Depends on `pqfreebsd` and `zfsctrl` via `MODULE_DEPEND`. pqfreebsd(4)
does not load it; pqf skills or `loader(8)` do when ZFS is in play.

### SEE ALSO

[pqfreebsd(4)](#pqfreebsd4), `mac(4)`, `loader.conf(5)`, `kldload(8)`,
`mount(8)`, `zfs(8)`, `log(9)`

### BUGS

Does not clear `MNT_MULTILABEL` on unload. Remounts that change **xattr**
are not observed until reload or a new mount.

---

Copyright © 2026 Brian Fundakowski Feldman. Light-ware License.
