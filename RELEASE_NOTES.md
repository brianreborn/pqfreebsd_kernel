# Release notes — pqfreebsd_kernel (pqk)

**pqk** = *Public Quantum Free Berkeley Software Distribution Kernel*. Module
`.ko` names stay in the `pqfreebsd` / `pqfreebsd_*` family.

**PQFreeBSD** = official non-fancy name. **Free(ly)B(le)S(se)D** = *Freely
Blessed* (evocative FreeBSD expansion, Unix-traditional wordplay).

## Current cut (compat foothold)

### Test host status (acknowledgement)

The machine used to develop and try this tree is **only half-installed** with
the **PQFreeBSD** skill (`brianreborn/pqfreebsd`, `/pqfreebsd`). Install was
paused on a **ZFS `MNT_MULTILABEL` blocker** — the reason
`pqfreebsd_compat_zfs_multilabel` exists. Parent MAC labeling / enforce and the
rest of the PQFreeBSD `one*` order **have not been finished** on that host.

**Once this blocker is cleared** (compat `.ko` built, installed, and loaded on
that ZFS box — or a proper fix that will go upstream), **continue the
half-finished PQFreeBSD skill install** from the interrupted step (typically
after `start`/boot/install has the KLDs / before or at parent `onelabel`, then the remainder of
`pqledger` / `pqdac` / checklabels / optional `oneenforce`). Do not restart the
interview from scratch unless `PREINSTALL` / snapshots say otherwise.

### Where the master project should put this

When a master / umbrella project **imbibes** these repos, keep roles separate:

| Repo | Place in the master layout | Do not |
| --- | --- | --- |
| `brianreborn/pqfreebsd_kernel` (**pqk**) | **Kernel deliverable** — not LLM-dependent. Build/install like other out-of-tree KLDs. Sibling to pqf skills, not inside them. | Do not vendor into `/create-skill` or `skills/`; pqf skills must not build this on demand. |
| `brianreborn/pqfreebsd` (**pqf skills**) | Skill suite + userland. Ensures **loader preload** of pqk; `kldload` only until reboot. Later: TPM e2e on that loader path (not this cut). | Do not compile KLDs from pqf skills. |

**Load rules the master must preserve:**

1. Only skeletal **`pqfreebsd.ko` is always required**.
2. **`pqfreebsd_compat_zfs_multilabel.ko` is ZFS-only** (UFS/FFS needs it not).
3. Core **does not** `kldload` siblings; userland does.
4. Proper ZFS multilabel fix **will go upstream**; this compat is accommodation.

### Modules in this cut

- `pqfreebsd` — core enforcement/audit state (`security.pqfreebsd.*`).
- `pqfreebsd_compat_zfs_multilabel` — ZFS `MNT_MULTILABEL` accommodation.
