# Legacy-Core Moby Dick Status

Date: 2026-05-27

Scope: live-mainnet XRPL P0 hunt, legacy-core priority lane. Public article
edits are out of scope for this run and were not made.

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
targeted finding TRUSTLINE-POSITIVE-BALANCE-RESERVE-001 reproduced by marker assertion.
ripple.tx.OpenP0Repro had 0 failures.
15.3s, 1 suite, 59 cases, 16068 tests total, 0 failures
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

## Next Step

Keep drilling `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` unless an even older and
cleaner source signal appears:

1. reproduce the same reserve/owner-count drift on the oldest buildable tag that
   still contains the relevant offer-crossing path;
2. add that oldest-tag result to the remediation/longevity notes if clean;
3. only then promote any specific "years live" statement.
