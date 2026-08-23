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

Compat workarounds for **trivial host bugs** that would otherwise block
PQFreeBSD (today: ZFS not advertising `MNT_MULTILABEL`). The proper fix is
upstream. These KLDs exist so PQFreeBSD can keep working **without** asking
operators to perform special steps or compromise suite functionality.

## Integration policy

Separate kernel deliverable — not vendored into create-skill trees, not
built on demand by skills. PQFreeBSD loads installed `.ko` files quietly
from `onestart`. Do not have unrelated projects integrate this tree yet.
