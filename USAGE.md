# Usage / expenses — pqk sprint

Captured at sprint wrap-up from this Grok session’s `/usage`-equivalent
telemetry (`updates.jsonl` usage snapshots + `signals.json`). **Credits UI
(`/usage`) remains authoritative** if it disagrees; `costUsdTicks` is
converted here as **nanodollars** (`USD = costUsdTicks / 1e9`). Reconcile
against the TUI if your billing sheet uses a different tick scale.

## Session

| Field | Value |
| --- | --- |
| Session id | `01a02deb-19e7-7661-b61a-5ded24dbf6d3` |
| Title | PQFreeBSD ZFS multilabel kernel modules |
| CWD | `/usr/home/green` |
| Started (UTC) | 2026-08-23T09:19:37Z |
| Captured (UTC) | 2026-08-23T09:55:39Z |
| Duration | ~2083 s (~34.7 min wall) |
| Primary model | `grok-4.6` / `grok-4.5` (build variants in usage) |
| Agent | `grok-build-plan` |

## Work product (this sprint)

- Repo: [brianreborn/pqfreebsd_kernel](https://github.com/brianreborn/pqfreebsd_kernel) (**pqk**)
- Companion docs/load path: [brianreborn/pqfreebsd](https://github.com/brianreborn/pqfreebsd) (**pqf skills**)
- Deliverables: `pqfreebsd.ko` (core state), `pqfreebsd_compat_zfs_multilabel.ko` (ZFS-only), loader-first load, RELEASE_NOTES / half-install continuity

## Token usage (sum of 25 turn usage snapshots)

| Metric | Total |
| --- | --- |
| Input tokens | 29,393,397 |
| Output tokens | 101,748 |
| Reasoning tokens | 79,735 |
| Cached read tokens | 28,645,248 |
| Cache creation tokens | 0 |
| **Total tokens** | **29,495,145** |
| Model API calls | 149 |
| API duration | 1,768,191 ms (~29.5 min cumulative API) |

### By model

| Model | Total tokens | Calls | `costUsdTicks` | Est. USD (@1e-9) |
| --- | --- | --- | --- | --- |
| `grok-4.5-build` | 28,454,947 | 134 | 57,414,456,320 | $57.41 |
| `grok-4.6-build` | 1,040,198 | 15 | 1,634,671,600 | $1.63 |
| **All** | **29,495,145** | **149** | **59,049,127,920** | **~$59.05** |

## Estimated expense

| | |
| --- | --- |
| **Estimated USD (ticks/1e9)** | **~$59.05** |
| Raw `costUsdTicks` (sum) | 59,049,127,920 |
| Alternate if ticks were 1e-8 USD | ~$590.49 (unlikely given cache-heavy mix) |

> Verify against interactive **`/usage`** (alias `/cost`) before treating this
> as an invoice line. Peak single-snapshot block was ~3.07M tokens /
> 4,668,414,080 ticks (~$4.67 @1e-9) mid-session.

## Activity signals (same session)

| Signal | Count |
| --- | --- |
| User messages | 25 |
| Assistant messages | 149 |
| Tool calls | 409 |
| Git commits (agent) | 22 |
| Agent lines added | 1097 |
| Agent lines removed | 9 |
| Files touched | 20 |
| Context window peak | 304,548 / 500,000 (~60%) |
| Errors / tool failures | 6 / 6 |

## Repositories touched

| Repo | Role |
| --- | --- |
| `pqfreebsd_kernel` | pqk KLDs |
| `pqfreebsd` | pqf skills docs + `onestart`/`loader` load path |

---

*Append further sprint rows below; do not overwrite prior captures.*
