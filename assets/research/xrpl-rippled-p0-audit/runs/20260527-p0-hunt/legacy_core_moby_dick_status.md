# Legacy-Core Moby Dick Status

Date: 2026-05-27

Scope: live-mainnet XRPL P0 hunt, legacy-core priority lane. Public article
edits are out of scope for this run and were not made.

Fresh live-state snapshot for this slice:

```text
live_state_snapshot_20260527_moby_dick.json
sha256: bf5f91401de37d39b79820d32ad9c46ac621e145c1295b9698647b2cb8290f47
checked_utc: 2026-05-27T21:04:03Z
s1.ripple.com: rippled 3.1.3, ledger 104523140, hash 9772A3C0D787CC963A696BE47F2BFC23806EF23B4E2D796D587DE9B87D32152D
s2.ripple.com: rippled 3.1.3, ledger 104523140, hash 9772A3C0D787CC963A696BE47F2BFC23806EF23B4E2D796D587DE9B87D32152D
```

## Current Result

The strongest legacy-core candidate is
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`.

It is a baseline IOU trustline and offer-crossing accounting issue, not a new
disabled feature surface. The current repro marker is:

```text
TrustLine current - offer crossing creates positive balance without reserve
```

The packet wrapper reproduced the marker:

```bash
assets/research/xrpl-rippled-p0-audit/repros/TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.sh
```

Observed wrapper result:

```text
local wrapper log sha256: e2385380b2c4c91ccf5a46472c46450acdfa9ea58468a46a6e2804a4e7ae88ab
targeted finding TRUSTLINE-POSITIVE-BALANCE-RESERVE-001 reproduced by marker assertion.
ripple.tx.OpenP0Repro had 0 failures.
14.9s, 1 suite, 59 cases, 16068 tests total, 0 failures
```

The full packet verifier also passed:

```text
packet-ok
records=19 markers=19 proof_sha256=302da1ccf25b3ab103cdccf231be443515e81561593c34912aca87849a22cfd6
```

## Why This Is The Moby Dick Candidate

The broken behavior is simple: after a holder clears its trust limit and
balance, offer crossing can give that holder a positive IOU balance while
`OwnerCount` remains zero and the trustline reserve flag remains unset.

The expected behavior is also simple: if an account's trustline balance moves
from non-positive to positive, the receiver should either be charged the owner
reserve and have owner-count state updated, or the transaction should fail for
insufficient reserve.

This sits in old core ledger accounting:

- `TrustSet` reserve and owner-count history reaches back to 2012-era code.
- `OfferCreate` and offer crossing are long-running core IOU paths.
- The fix-looking branch touches `rippleCreditIOU` in `src/libxrpl/ledger/View.cpp`,
  a shared ledger helper rather than a narrow new amendment module.

The source-lineage check found the sender-side reserve-clearing branch and
receiver-reserve-clear comment in every sampled release line from `0.12.0`
through `3.1.3`:

```text
0.12.0 modules/ripple_app/ledger/ripple_LedgerEntrySet.cpp:1489 Sender balance was positive.
0.12.0 modules/ripple_app/ledger/ripple_LedgerEntrySet.cpp:1504 Receiver reserve is clear.
3.1.3  src/libxrpl/ledger/View.cpp:2048 saBefore > beast::zero
3.1.3  src/libxrpl/ledger/View.cpp:2083 Receiver reserve is clear.
```

That is source-lineage evidence, not a substitute for old-binary reproduction.
It supports the hypothesis that the missing receiver-side reserve transition is
old, but the only clean binary repro currently recorded in the packet is still
the current `3.1.3` repro.

The source-lineage check is now script-bound:

```text
script: verify_trustline_positive_balance_lineage.py
script_sha256: c85f62a3003052cffa39a9c41cc79273dbc53029e8f29a6f995774983094022c
report: runs/20260527-p0-hunt/trustline_positive_balance_lineage_20260527.json
report_sha256: 9e09f480c2d0e7a4494b7a67dda4819f8c0076d11bf88bb882816922d9f8ab82
result: passed=True
sampled refs: 0.12.0, 0.20.0, 0.30.0, 0.50.0, 0.80.0, 1.0.0, 1.5.0, 2.0.0, 2.5.0, 3.1.3
```

The current source signal is upstream branch
`origin/vvysokikh1/fix-positive-balance-trustline-pay-no-reserve`, commit
`b4a45f1f0f49d3caf56d2c790960380b5e648a60`:

```text
fixed positive balance trustline not incrementing owners count in some cases
```

That commit adds `fixTrustLineOwnerCount` and receiver-side reserve accounting
when IOU balance crosses from non-positive to positive. It is not contained in
the checked latest refs:

```text
b4a45f1f0... ancestor of 3.2.0-b7: exit 1
b4a45f1f0... ancestor of origin/develop: exit 1
```

Classification: live-unfixed candidate, already present in the packet as
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`. It is the best current target for the
"old, obvious, core, current, unfixed" hunt. Older-tag binary reproduction is
still the next useful hardening step before making a precise years-live claim.

The earliest source hit for the trustline reserve carveout shape in the local
git history is:

```text
e416ee72c Rearrange sources (#4997)
```

That is a source-lineage anchor, not proof that the behavior was introduced in
that exact refactor. It is enough to support the claim that the shape is
well-entrenched in the pre-`3.1.3` lineage.

I also hardened the old-tag anchor further by building `rippled` `2.5.0` from
source in a temporary worktree and running the legacy trustline/offer suites on
that binary. Both suites pass cleanly on that tag:

```text
2.5.0 build: succeeded
./rippled --unittest=SetTrust --unittest-log -> 30 cases, 1534 tests total, 0 failures
./rippled --unittest=Offer --unittest-log -> 8 suites, 415 cases, 55237 tests total, 0 failures
```

This does not by itself prove `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` is
absent on `2.5.0`; it does prove the old tag is buildable here and that the
legacy trustline/offer paths are still live enough to be a meaningful older-tag
hardening target. The claim remains source-lineage plus current-binary repro
until a tag-specific bug repro is isolated.

I also attempted to harden the claim by booting the oldest sampled release tag
(`0.12.0`) in a temporary worktree. That path is currently blocked in this
environment by historical toolchain gaps:

- `SConstruct` expects Python 2-era modules (`commands`, `platform.linux_distribution`);
- the old build wants `protoc`, which is not installed locally;
- the old build also expects Boost headers that are not available in this
  workspace.

So the packet is intentionally staying at source-lineage + current-binary repro
for this candidate until the old toolchain is provisioned.

## Source-Killed Candidate

`CHECK-LEGACY-DIRFULL-PARTIAL-001` was probed and demoted.

The scratch probe filled the source and destination owner directories before
`CheckCreate`. With a genuinely full directory, `CheckCreate` returned
`tecDIR_FULL` and left no partial `Check`, owner-directory entry, or owner-count
residue. The temporary scratch test passed locally and was removed from the
suite before this triage note was written.

This is not a promoted finding.

## PayChan Legacy Probe

`PAYCHAN-LEGACY-CLOSE-OWNERDIR-001` was source-reviewed in this slice but not
promoted. Existing upstream tests already cover old-style payment-channel
recipient owner-directory behavior and resurrection behavior around the
`fixPayChanRecipientOwnerDir` amendment. No new live P0 repro was produced for
this candidate in this slice.

## Offer And Payment Probes

`OFFER-LEGACY-RESERVE-CROSSED-001` was reviewed and not promoted. The current
`Offer_test::testPartialCross` table already covers reserve-boundary behavior
with `tecINSUF_RESERVE_OFFER` and `tecUNFUNDED_OFFER` controls, and the live
`OfferCreate` path still intentionally returns those boundary results rather
than exposing a new ledger corruption or object-lifetime failure in this slice.

`PAYMENT-LEGACY-TEFEXCEPTION-PATH-001` was reviewed and not promoted. Current
`Payment.cpp` contains explicit guards that route internal exceptions to
`tefEXCEPTION`/`tefINTERNAL` handling, but no clean current-release transaction
repro surfaced that turned a normal live-enabled payment into a new Moby Dick
P0. Keep it as a lower-priority source-review target unless a sharper input
shape appears.

## Legacy-Core Source-Kill Sweep

The remaining old-core queue was reviewed against current source and current
tests, and the following candidates were source-killed rather than promoted:

- `TRUSTSET-LEGACY-RESERVE-CARVEOUT-001`: the reserve carveout is an explicit
  gateway bootstrap rule in `SetTrust::doApply`, not an accidental leak. It
  skips reserve enforcement only while `OwnerCount < 2`, then reverts to normal
  reserve checks. The current code and test shape do not show a live-state
  corruption or stranded object from the carveout itself.
- `TICKET-LEGACY-SEQUENCE-COLLISION-001`: `CreateTicket` derives ticket keys
  from the post-consume sequence, checks `tecDIR_FULL` before owner-count
  mutation, and has explicit `tefINTERNAL` guardrails for bad sequence state.
  The current implementation does not expose a duplicate-key or collision path
  from normal ticket-paid creation.
- `ESCROW-LEGACY-XRP-DELETE-EDGE-001`: legacy XRP escrow cancel/finish/delete
  paths already remove owner-directory entries, update owner count, and return
  `tecHAS_OBLIGATIONS` / `tefBAD_LEDGER` on failed teardown. The current source
  does not show an obvious delete-with-live-obligation P0 on the live-enabled
  path.
- `MPT-LOCK-UNAUTH-NOSAV-001`: the current `MPToken_test` matrix already
  expects locked-holder unauthorize/delete to succeed when
  `featureSingleAssetVault` is off, and `MPTokenAuthorize::preclaim` only
  blocks the delete path when SAV is enabled. That makes the no-SAV deletion
  behavior look intentional, not like a live Moby Dick defect.

That leaves the current legacy-core live candidate set unchanged: the best
remaining old/simple/current target is still
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`, and the next useful hardening step
is an older-tag binary reproduction if we want to turn it into a stronger
years-live claim.

## Next Step

Keep drilling `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` unless an even older and
cleaner source signal appears:

1. reproduce the same reserve/owner-count drift on the oldest buildable tag that
   still contains the relevant offer-crossing path;
2. add that oldest-tag result to the remediation/longevity notes if clean;
3. only then promote any specific "years live" statement.
