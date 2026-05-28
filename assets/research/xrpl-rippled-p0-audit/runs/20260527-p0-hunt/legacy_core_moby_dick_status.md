# Legacy-Core Moby Dick Status

Date: 2026-05-28

Scope: live-mainnet XRPL P0 hunt, legacy-core priority lane. Public article
edits are out of scope for this run and were not made.

Fresh live-state snapshot for this slice:

```text
live_state_snapshot_20260528_moby_dick.json
sha256: e4f756f7ae60087a90ff7e4eaf15292fe092caecfab69ff3c22116e6a546c972
checked_utc: 2026-05-28T01:33:19Z
s1.ripple.com: rippled 3.1.3, ledger 104527318, hash 561F36C3B8C6EF66D643DF6157B135102D21ED2FD043BE3B419A307FA1D623A1
s2.ripple.com: rippled 3.1.3, ledger 104527318, hash 561F36C3B8C6EF66D643DF6157B135102D21ED2FD043BE3B419A307FA1D623A1
```

## Current Result

The strongest legacy-core candidate is
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`.

It is a baseline IOU trustline and settlement accounting issue, not a new
disabled feature surface. The current repro markers are:

```text
TrustLine current - offer crossing creates positive balance without reserve
TrustLine current - CheckCash creates positive balance without reserve
```

The packet wrapper reproduced the marker:

```bash
assets/research/xrpl-rippled-p0-audit/repros/TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.sh
```

Observed wrapper result:

```text
local wrapper log sha256: d3bd7e3ffe437fe30b97159538fe6ec0247a0608417011385e3192a01e7bd946
targeted finding TRUSTLINE-POSITIVE-BALANCE-RESERVE-001 reproduced by marker assertion.
ripple.tx.OpenP0Repro had 0 failures.
17.5s, 1 suite, 60 cases, 16114 tests total, 0 failures
```

The full packet verifier also passed:

```text
packet-ok
records=19 markers=20 proof_sha256=6f2e7abb04f556299b79d180c778df6fce1a9e131f306fedeb26051c639bf191
```

## CheckCash Sibling Repro

This slice added a second current-live marker under the same finding, not a new
finding count.

Minimal behavior:

1. Alice opens a gateway USD trustline, receives 100 USD, clears the limit, and
   pays the 100 USD back.
2. The line remains in default/no-reserve state with `OwnerCount=0`.
3. The gateway writes a USD check to Alice.
4. Alice cashes the check.
5. Alice ends with a positive 50 USD balance while `OwnerCount=0` and the
   receiver reserve flag remains unset.

The packet proof marker is:

```text
ripple.tx.OpenP0Repro TrustLine current — CheckCash creates positive balance without reserve
```

This matters because `CheckCash` already has explicit reserve logic for
auto-created trustlines. The sibling proves the issue is not only an
`OfferCreate` edge case; it is a shared IOU-credit transition problem when an
existing default trustline crosses from non-positive receiver balance to
positive receiver balance.

## Why This Is The Moby Dick Candidate

The broken behavior is simple: after a holder clears its trust limit and
balance, old live settlement paths can give that holder a positive IOU balance
while `OwnerCount` remains zero and the trustline reserve flag remains unset.
This is now reproduced through both offer crossing and CheckCash.

The expected behavior is also simple: if an account's trustline balance moves
from non-positive to positive, the receiver should either be charged the owner
reserve and have owner-count state updated, or the transaction should fail for
insufficient reserve.

This sits in old core ledger accounting:

- `TrustSet` reserve and owner-count history reaches back to 2012-era code.
- `OfferCreate`, offer crossing, Checks, and CheckCash are long-running core
  IOU paths.
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

That source-lineage evidence supports the hypothesis that the missing
receiver-side reserve transition is old. This lane now adds clean binary
reproduction on the buildable `2.5.0`, `2.0.0`, and `1.5.0` release tags, so
the candidate is no longer only current-binary plus source-lineage evidence.

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
"old, obvious, core, current, unfixed" hunt.

The earliest source hit for the trustline reserve carveout shape in the local
git history is:

```text
e416ee72c Rearrange sources (#4997)
```

That is a source-lineage anchor, not proof that the behavior was introduced in
that exact refactor. It is enough to support the claim that the shape is
well-entrenched in the pre-`3.1.3` lineage.

I hardened the old-tag anchor further by building `rippled` `2.5.0`, `2.0.0`,
and `1.5.0` from source in temporary worktrees or Docker and adding a focused
`SetTrust` probe for the same reserve/owner-count drift. The probe asserts that
offer crossing gives Alice a positive gateway IOU balance while `OwnerCount`
remains zero and the receiver reserve flag remains unset.

```text
2.5.0 build: succeeded
./rippled --unittest=SetTrust --unittest-log -> 32 cases, 1656 tests total, 0 failures
probe marker: Legacy 2.5.0 -- offer crossing creates positive balance without reserve
2.0.0 build: succeeded after build-only Conan compatibility edits for the current toolchain
./rippled --unittest SetTrust --unittest-log -> 22 cases, 700 tests total, 0 failures
probe marker: Legacy 2.0.0 -- offer crossing creates positive balance without reserve
1.5.0 build: succeeded in Ubuntu 20.04 Docker with build-only compatibility edits
./rippled --unittest SetTrust --unittest-log -> 9 cases, 271 tests total, 0 failures
probe marker: Legacy 1.5.0 -- offer crossing creates positive balance without reserve
```

The old-tag repro artifacts are preserved in this packet:

```text
patch: runs/20260527-p0-hunt/trustline_positive_balance_2_5_0_repro.patch
patch_sha256: f2b8bb9a31890aa91c7389b9a79157b64dc3d9824d22d6876d3716c4fc28e48d
log: runs/20260527-p0-hunt/trustline_positive_balance_2_5_0_repro.log
log_sha256: 3e9968cf305851dbd2cdf9f8a50d55b3dab784649af1fb7d9993fdd0fcccb494
tag_commit: 1e01cd34f7a216092ed779f291b43324c167167a
tag_commit_date: 2025-06-23
patch: runs/20260527-p0-hunt/trustline_positive_balance_2_0_0_repro.patch
patch_sha256: 07651b66c8e5f0e3e39fac776d57933a767988b8e663efb072eae1804be08cd9
log: runs/20260527-p0-hunt/trustline_positive_balance_2_0_0_repro.log
log_sha256: 8fc64bef728cb84a1f38e4763877e530a6ce54716706ac34ffb3957b070794b2
tag_commit: 2b0313d60c4226cc98ad39fe8eb659deca48f32a
tag_commit_date: 2024-01-08
patch: runs/20260527-p0-hunt/trustline_positive_balance_1_5_0_repro.patch
patch_sha256: b30a319451845797e1f55416990018b7c73a6759d5d2b9b6c853a83c57f5b047
log: runs/20260527-p0-hunt/trustline_positive_balance_1_5_0_repro.log
log_sha256: cba00449acaaad475b6e3d8824ffb0ee0abb4b981bbb62e44ca4ae9777c7c12f
tag_commit: f00f263852c472938bf8e993e26c7f96f435935c
tag_commit_date: 2020-03-30
docker_image: ubuntu:20.04
build_profile: cmake Release, static=OFF, Boost 1.71/OpenSSL 1.1.1f, local libarchive/protobuf/grpc/rocksdb
```

This proves the specific bad post-state on the current `3.1.3` packet target
and on the older buildable `2.5.0`, `2.0.0`, and `1.5.0` tags. The reproduced
release-tag binary span now reaches back to March 2020, or more than six years
by May 2026. Earlier claims remain source-lineage until a pre-`1.5.0` toolchain
is provisioned and reproduced.

I also attempted to harden the claim by booting the oldest sampled release tag
(`0.12.0`) in a temporary worktree. That path is currently blocked in this
environment by historical toolchain gaps:

- `SConstruct` expects Python 2-era modules (`commands`, `platform.linux_distribution`);
- the old build wants `protoc`, which is not installed locally;
- the old build also expects Boost headers that are not available in this
  workspace.

So the packet now has current-binary repro, `2.5.0`, `2.0.0`, and `1.5.0`
binary repros, and older source-lineage evidence for this candidate. A
pre-`1.5.0` binary span still requires old toolchain provisioning.

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
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`; the current slice moved it from
source-lineage plus current repro to source-lineage plus current, `2.5.0`,
`2.0.0`, and `1.5.0` binary repro, then added CheckCash as a second
current-live settlement-path marker for the same root cause.

## Next Step

Keep drilling trustline/settlement reserve siblings unless an even older and
cleaner source signal appears:

1. source-review sibling crossings that can move IOU balances across zero
   without the shared receiver-side owner-count transition;
2. provision the oldest practical pre-`1.5.0` toolchain only if it is cheap;
3. reproduce the same reserve/owner-count drift there if buildable before
   promoting any specific pre-2020 binary-span statement.
