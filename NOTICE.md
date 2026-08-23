# Copyright and license

**This GitHub repository is public.**

All files in this distribution — including kernel modules and this
notice — are:

**Copyright © 2026 Brian Fundakowski Feldman. All rights reserved.**

Licensed under the **Light-ware License** (the 4-clause BSD License plus
one non-binding invitation to help keep the lights on). The full text is
in [`LICENSE`](LICENSE) at the repository root.

- SPDX: this is **not** a stock SPDX identifier. GitHub may show the
  license as “Other”. That does not change the text in `LICENSE`.
- The lights-on paragraph is an **ethical ask**, not a fifth legal
  condition. Declining it does not affect rights under clauses 1–4.
- No third-party code is vendored here. The modules call into FreeBSD /
  OpenZFS kernel APIs that already ship with the system.

Required acknowledgement in advertising (clause 3):

> This product includes software developed by Brian Fundakowski Feldman.

## Purpose

- **`pqfreebsd.ko`** — **only always-required** KLD. Skeletal core: suite
  state (enforcement / audit), transparency, audit trail. Does not load
  other KLDs; does not implement feature policy.
- **`pqfreebsd_*` siblings** — any number may be required **to enable** a
  given path. None are required merely to load the skeleton. Today:
  `pqfreebsd_compat_zfs_multilabel` is **ZFS-only** (UFS/FFS already has
  `MNT_MULTILABEL`; proper fix for ZFS is upstream).

These KLDs exist so PQFreeBSD can keep working **without** asking operators
to perform special steps or compromise suite functionality.

**Now:** multilabel compat is a mid-install blocker while validating a
PQFreeBSD host. Related pain (e.g. `su` failing) is a separate symptom that
may clear once labels stick on ZFS — not a second responsibility of that
compat module.

## Integration policy

Separate kernel deliverable — not vendored into create-skill trees, not
built on demand by skills. The **userland suite** loads installed `.ko`
files quietly from `onestart`; the core module does **not** load compat
KLDs. Do not have unrelated projects integrate this tree yet.
