# XRPL rippled P0 hunt repro results

Date: 2026-05-27
Command:

```bash
cd /home/postfiat/repos/rippled
cmake --build .build --target rippled -j 8
.build/rippled --unittest OpenP0Repro
```

Saved proof logs: `definitive_proof_20260527.log`,
`definitive_proof_subulp_20260527.log`,
`definitive_proof_stissue_20260527.log`, and
`definitive_proof_number_division_20260527.log`, plus
`definitive_proof_delegate_mpt_granular_20260527.log` for the delegated MPT
granular-mutation rerun and
`definitive_proof_delegate_empty_accountset_20260527.log` for the delegated
empty-`AccountSet` sequence-consumption rerun, plus
`definitive_proof_batch_signer_outer_replay_20260527.log` for the batch signer
outer-account replay rerun.

## Continuation pass: killed candidates

The continuation pass initially kept the promoted root-cause count unchanged. The proof
harness was restored to the committed hash after each scratch probe, then
rerun cleanly. A later sub-ULP cover pass expanded the existing
`LoanBrokerCover` precision root to include zero-at-scale withdraw and clawback
paths. A protocol-wire pass added the MPT `STIssue` legacy sequence-byte
proof. A final arithmetic-helper pass added the `Number::operator/=`
upward-rounding proof. Delegation passes added the MPT granular
issuance-mutation proof, the empty-`AccountSet` sequence-consumption proof, and
the batch signer outer-account replay proof. `OpenP0Repro` then passed with 47
cases and 9,119 tests.
`OpenP0ReproCrash` passed with 1 case and 12 tests.

## Current source kill: PaymentChannel destination directory full

Status: tested and not promoted.

Hypothesis: `PaymentChannelCreate` writes the channel object and source owner
directory before adding the recipient owner-directory entry under
`fixPayChanRecipientOwnerDir`. If the recipient owner directory is full, the
transaction might leave the channel object or source owner-count state behind.

Scratch behavior: a forced destination-dir-full setup returned `tecDIR_FULL`,
left no PayChannel object, and left the source `OwnerCount` unchanged.

Source-kill artifact:

```text
runs/20260527-p0-hunt/paychan_dest_dirfull_source_kill_20260528.log
sha256: ff0ac270d50fc9424eed8d077be0b91e08d7f771ec4cbf1aed37f7425dd3f8cc
```

Proof excerpt:

```text
ripple.tx.OpenP0Repro SCRATCH PayChanCreate tecDIR_FULL leaves no partial channel
16.9s, 1 suite, 71 cases, 16783 tests total, 0 failures
```

After the scratch-only case was removed, the upstream packet harness returned:

```text
16.0s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

## Current source kill: PaymentChannel close and owner-directory migration

Status: tested through existing upstream suites and not promoted.

Hypothesis: payment-channel close, claim, fund, account-delete, or recipient
owner-directory migration around `fixPayChanRecipientOwnerDir` might strand a
channel, leak an owner-directory entry, miss an obligation, or corrupt owner
counts.

Existing upstream coverage exercised old-style channels without recipient
backlinks, new-style channels with recipient backlinks, mixed amendment
activation, destination account deletion and resurrection, claim/fund behavior
after destination deletion, close/refund cleanup, tickets, metadata ownership,
deposit authorization, credentials, and account-delete obligation checks.

Source-kill artifact:

```text
runs/20260527-p0-hunt/paychan_accountdelete_source_kill_20260528.log
sha256: a26cd88fb8f0746c44708dd951b2132d5f907ab6fd7f911bd4d3113c2368985f
```

Proof excerpt:

```text
ripple.app.AccountDelete had 0 failures.
ripple.app.PayChan had 0 failures.
22.0s, 2 suites, 64 cases, 11535 tests total, 0 failures
```

## Current source kill: offer partial-cross under reserve

Status: scratch-tested and not promoted.

Hypothesis: old `OfferCreate` reserve handling might let an under-reserved
account partially cross an offer and then leave a remainder offer, owner-count,
or reserve state that should be impossible.

Scratch behavior: the no-cross control at the same reserve boundary returned
`tecINSUF_RESERVE_OFFER` and placed no offer. The partial-cross path returned
`tesSUCCESS`, transferred the crossed 50 XRP/USD leg, placed no Alice remainder
offer, removed the market offer, and left Alice's `OwnerCount=1`. This matches
the explicit `CreateOffer::applyGuts` rule: crossed value can stand, but if the
account lacked reserve before processing, the remainder is not placed.

Source-kill artifact:

```text
runs/20260527-p0-hunt/offer_partial_cross_underreserve_source_kill_20260528.log
sha256: 9cc82378a5e67ba5c59db7f4c34a1b4ab23df11b628b3ee3bae2e24b4b4b7db6
```

Proof excerpt:

```text
ripple.tx.OpenP0Repro SCRATCH OfferCreate partial cross under reserve cancels remainder
16.0s, 1 suite, 71 cases, 16828 tests total, 0 failures
```

After the scratch-only case was removed, the upstream packet harness returned:

```text
16.8s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

## Current source kill: XRP Escrow lifecycle and account deletion

Status: tested through existing upstream suites and not promoted.

Hypothesis: legacy XRP Escrow finish, cancel, ticket use, directory cleanup, or
account-delete obligation checks might strand an escrow, leak owner-directory
entries, miss an obligation, or corrupt owner counts.

Existing upstream coverage exercised escrow enablement, finish/cancel timing,
tags, DisallowXRP behavior, implied finish time, failure cases, lockup
owner-count assertions, crypto-conditions, metadata to self/other, transaction
consequences, ticket-created escrows, credential/deposit-preauth combinations,
and account-delete obligation checks.

Source-kill artifact:

```text
runs/20260527-p0-hunt/escrow_accountdelete_source_kill_20260528.log
sha256: b38d0438aa3af6bc9d7c20a9ffe789923fbd5537c94f5c26628555c51c68b05a
```

Proof excerpt:

```text
ripple.app.AccountDelete had 0 failures.
ripple.app.Escrow had 0 failures.
19.4s, 2 suites, 53 cases, 11568 tests total, 0 failures
```

## Current source kill: raw-sequence ticket-created object sweep

Status: source-reviewed and tested through existing upstream suites; not
promoted.

Hypothesis: after the historical `PermissionedDomainSet` ticket-sequence
collision, another live core transaction family might still create ledger keys
from raw `sfSequence` instead of the ticket-aware sequence proxy.

Static result: a focused scan of `src/xrpld/app/tx/detail` found the historical
`PermissionedDomainSet` raw-sequence branch gated by `fixCleanup3_1_3`, but the
old live create paths checked in this slice use `ctx_.tx.getSeqValue()` for
offer, escrow, payment-channel, check, vault, loan-broker, and MPT issuance
object identifiers, or use `sfSequence` only for non-create semantics.

Source-kill artifacts:

```text
runs/20260527-p0-hunt/ticket_sequence_static_sweep_20260528.log
sha256: 8e7ec705187f93ea45e48957296df5953828afa8728b9b761c80ed272fcee8bd

runs/20260527-p0-hunt/ticket_sequence_source_kill_20260528.log
sha256: 55022461bafc5c2865833de278b06140a608614dfc6adaf3b3463c07b8fc6413
```

Proof excerpt:

```text
ripple.app.Check had 0 failures.
ripple.app.Escrow had 0 failures.
ripple.app.PayChan had 0 failures.
ripple.app.Ticket had 0 failures.
94.1s, 11 suites, 489 cases, 63983 tests total, 0 failures
```

## Current source kill: legacy authorization/freeze receive-path siblings

Status: source-reviewed and tested through existing upstream suites; not
promoted.

Hypothesis: after the DisallowIncomingTrustline and AMM DepositAuth findings, a
stronger old live receive-path sibling might exist where direct IOU transfer or
trustline controls reject `RequireAuth`, local freeze, global freeze, deep
freeze, or DepositAuth, but an old indirect path succeeds and creates durable
value/state movement.

Static result: the sweep covered `checkAcceptAsset`, `requireAuth`,
freeze/deep-freeze helpers, `trustCreate`, `trustDelete`, `rippleCredit`,
`accountSend`, `issueIOU`, and `verifyDepositPreauth` call sites across the
transaction, path, and misc layers. It did not isolate a new old-core receive
policy witness beyond the already-promoted DisallowIncomingTrustline,
AMM DepositAuth, and lending regular-freeze surfaces.

Source-kill artifacts:

```text
runs/20260527-p0-hunt/auth_freeze_receive_static_sweep_20260528.log
sha256: 3ce4e848635f707855f468ee7a94bd87f24295a0be2088ed4793c3961aaf30fc

runs/20260527-p0-hunt/auth_freeze_receive_source_kill_20260528.log
sha256: 79c5255082209f8b2ae90d13738bf3b21368dd0a4c8e0c7e400e802804f4d584
```

Proof excerpt:

```text
ripple.app.Freeze had 0 failures.
ripple.app.SetTrust had 0 failures.
ripple.app.TrustAndBalance had 0 failures.
144.1s, 15 suites, 716 cases, 96462 tests total, 0 failures
```

## Current source kill: deterministic exception and arithmetic sweep

Status: source-reviewed, history-reviewed, and tested through existing upstream
suites; not promoted.

Hypothesis: a normal transaction-shaped input on an old live path might still
reach `tefINTERNAL`, `tefEXCEPTION`, `overflow_error`, assertion, or
`tecINVARIANT_FAILED` instead of deterministic rejection.

Static/history result: the sweep covered exception, invariant, overflow, and
rounding call sites in `src/xrpld/app/tx/detail`, `src/xrpld/app/paths`,
`src/xrpld/app/misc`, `src/libxrpl/protocol`, and `src/libxrpl/basics`, then
mined local history for fix-looking arithmetic/result-code signals. The history
rediscovered already-promoted or already-demoted items: `NUMBER-CUSP-UPWARD-001`,
`NUMBER-DIVISION-UPWARD-001`, `MPT-TRANSFER-RATE-OVERFLOW-001`,
`INVARIANT-BOOL-OVERWRITE-001`, LoanBrokerCover precision, vault invariant
edges, and permissioned-DEX invariant cases. It did not isolate a new old-core
transaction witness in this slice.

Source-kill artifacts:

```text
runs/20260527-p0-hunt/deterministic_exception_arithmetic_static_sweep_20260528.log
sha256: 16207ac6ee3de1c5c82c6205351d28d00c526ea042db2daf83bced518640ae48

runs/20260527-p0-hunt/deterministic_exception_arithmetic_history_sweep_20260528.log
sha256: 1341fc2cbd5d3c8c308b794a7814e8850c24096c7dca7aa73990cc8ea8cc4e67

runs/20260527-p0-hunt/deterministic_exception_arithmetic_source_kill_20260528.log
sha256: d6385bd19f70f12a50445601d0095c2821c691505f21f1bcc79d63282b45b284
```

Proof excerpt:

```text
ripple.basics.Number had 0 failures.
ripple.protocol.STAmount had 0 failures.
ripple.protocol.Quality had 0 failures.
114.9s, 18 suites, 626 cases, 175219 tests total, 0 failures
```

Continuation on the next whip pass narrowed this to legacy amount/quality
paths: `STAmount`, `Number`, `Quality`, `Offer`, `Flow`, `Path`, `PayStrand`,
and `TrustAndBalance`. It again found only already-promoted or already-demoted
signals and no new old-core transaction witness.

```text
runs/20260527-p0-hunt/legacy_amount_quality_static_sweep_20260528.log
sha256: e9acb64c13dfeea4adbc6a8cc9e99d60158054fd2852bc931a88a88b368b9802

runs/20260527-p0-hunt/legacy_amount_quality_history_sweep_20260528.log
sha256: 6e7024e72514d9edadc523bec130edbde1322e557877c42e2c64e4222b2e7542

runs/20260527-p0-hunt/legacy_amount_quality_source_kill_20260528.log
sha256: e1d693d42ac1db37f178d41b0c07a0f25de3f7835dcf2cd4af2fcedbe69c037d

89.9s, 14 suites, 540 cases, 166840 tests total, 0 failures
```

## Current source kill: source-signal core-accounting cluster

Status: source-reviewed, history-reviewed, and tested through existing upstream
suites; not promoted.

Hypothesis: the remaining reserve, owner-count, trustline, directory, and
result-code history might point to another old live Moby Dick candidate near
`rippleCreditIOU`, `accountSend`, `trustCreate`, `trustDelete`,
`adjustOwnerCount`, or owner-directory helpers.

Static/history result: the sweep clustered source signals around
`fix-positive-balance-trustline-pay-no-reserve`, reserve/owner-count commits,
directory helper changes, and result-code changes. The useful signals mapped to
already-promoted, already-demoted, historical, or future-branch entries:
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`, `DELEGATE-FEE-RESERVE-001`,
`TXQ-MIN-RESERVE-POTENTIAL-SPEND-001`,
`PAYMENT-SANDBOX-DEFERRED-CREDITS-001`, `VAULT-WITHDRAW` reserve branches,
future AMMWithdraw double-owner-count, and directory-limit history. No new
old-core transaction witness was isolated in this slice.

Source-kill artifacts:

```text
runs/20260527-p0-hunt/source_signal_core_accounting_static_sweep_20260528.log
sha256: 8965a760cec40a403aba0173e36a60c3a98a8eaa0d844cbee7d1e80f850f2b21

runs/20260527-p0-hunt/source_signal_core_accounting_history_grep_20260528.log
sha256: b13b62e85e5c3ee2591967a132cb5b35fd36cd20b53e60222dd03011e0c22b1d

runs/20260527-p0-hunt/source_signal_core_accounting_history_pickaxe_20260528.log
sha256: f31d5ece55a6498dd680af5a2aaac7dab80bfa971f1b5be4dcfc8cf5d9a2893d

runs/20260527-p0-hunt/source_signal_core_accounting_source_kill_20260528.log
sha256: d22eee8cc2697c8d6446324ee442c6ebffeac798667ec011018880b048c69f71
```

Proof excerpt:

```text
ripple.app.SetTrust had 0 failures.
ripple.app.TrustAndBalance had 0 failures.
ripple.ledger.PaymentSandbox had 0 failures.
157.9s, 19 suites, 724 cases, 102253 tests total, 0 failures
```

## Old-tag hardening: trustline positive-balance reserve drift

Status: reproduced on the buildable `2.5.0`, `2.0.0`, and `1.5.0` release tags
in addition to the current `3.1.3` packet target.

Minimal behavior:

1. Alice opens a gateway USD trustline, receives 100 USD, then clears the limit.
2. Alice pays the 100 USD back, leaving the line with `OwnerCount=0` and no
   receiver reserve flag.
3. A market account posts an XRP-for-USD offer.
4. Alice crosses that offer with USD-for-XRP.
5. Alice ends with a positive 50 USD balance while `OwnerCount` remains 0 and
   the receiver reserve flag remains unset.

2.5.0 proof excerpt:

```text
ripple.app.SetTrust Legacy 2.5.0 -- offer crossing creates positive balance without reserve
ripple.app.SetTrust Legacy 2.5.0 -- offer crossing creates positive balance without reserve
ripple.app.SetTrust had 0 failures.
4.9s, 1 suite, 32 cases, 1656 tests total, 0 failures
```

2.0.0 proof excerpt:

```text
ripple.app.SetTrust Legacy 2.0.0 -- offer crossing creates positive balance without reserve
ripple.app.SetTrust Legacy 2.0.0 -- offer crossing creates positive balance without reserve
5.4s, 1 suite, 22 cases, 700 tests total, 0 failures
```

1.5.0 proof excerpt:

```text
ripple.app.SetTrust Legacy 1.5.0 -- offer crossing creates positive balance without reserve
2.7s, 1 suite, 9 cases, 271 tests total, 0 failures
```

Artifacts:

```text
runs/20260527-p0-hunt/trustline_positive_balance_2_5_0_repro.patch
runs/20260527-p0-hunt/trustline_positive_balance_2_5_0_repro.log
runs/20260527-p0-hunt/trustline_positive_balance_2_0_0_repro.patch
runs/20260527-p0-hunt/trustline_positive_balance_2_0_0_repro.log
runs/20260527-p0-hunt/trustline_positive_balance_1_5_0_repro.patch
runs/20260527-p0-hunt/trustline_positive_balance_1_5_0_repro.log
```

Interpretation: this is now current-binary repro plus three older buildable-tag
binary repros plus source-lineage evidence. The `1.5.0` tag is anchored at
commit `f00f263852c472938bf8e993e26c7f96f435935c`, dated 2020-03-30, which
gives this candidate a reproduced release-tag binary span of more than six
years by May 2026. Claims earlier than `1.5.0` still require older toolchain
provisioning.

## Current sibling: CheckCash positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice opens a gateway USD trustline, receives 100 USD, clears the limit, and
   pays the 100 USD back.
2. The line remains with `OwnerCount=0` and no receiver reserve flag.
3. The gateway writes a USD check to Alice.
4. Alice cashes the check and receives 50 USD.
5. Alice still has `OwnerCount=0` and no receiver reserve flag.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — CheckCash creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is a second current-live settlement path for the same
reserve/owner-count root cause. It is not counted as a new finding; it expands
the existing finding from an offer-crossing witness into a shared IOU-credit
transition witness.

## Current sibling: CheckCash existing-owner reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2` before receiving the
   check funds.
3. The gateway writes a USD check to Alice.
4. Alice cashes the check and receives 50 USD.
5. Alice still has `OwnerCount=2` and no receiver reserve flag.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — CheckCash leaves positive balance unowned with existing owner objects
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this strengthens the same root cause because it rules out a
narrow explanation based on the old `OwnerCount < 2` reserve carveout. The
holder already has two owned objects; the missing transition is still the
positive-trustline owner-count/reserve update.

## Current sibling: offer crossing existing-owner reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2` before crossing the
   offer.
3. A market account posts an offer selling gateway USD for XRP.
4. Alice crosses the offer and receives 50 USD.
5. Alice still has `OwnerCount=2` and no receiver reserve flag.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — offer crossing leaves positive balance unowned with existing owner objects
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is the offer-side companion to the CheckCash
existing-owner control. Both live settlement paths skip the receiver-side
owner-count/reserve update even after the holder is beyond the historical
two-object reserve carveout.

## Current sibling: offer crossing missing-owner-reserve boundary

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2`.
3. A market account posts an offer selling gateway USD for XRP.
4. Alice is drained to exactly the two-owner reserve plus the offer-crossing
   XRP spend and fee.
5. Alice crosses the offer and receives 50 USD.
6. Alice remains below the three-owner reserve while `OwnerCount` remains `2`
   and the receiver reserve flag remains unset.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — offer crossing succeeds below missing owner reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is the offer-side reserve-boundary companion to the
CheckCash boundary control. The path succeeds even when the receiver lacks
reserve capacity for the owner-count increment that should accompany a positive
trustline balance.

## Current sibling: offer crossing with issuer transfer rate

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Gateway sets a non-default transfer rate.
2. Alice clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
3. A market account posts a USD/XRP offer.
4. Alice crosses the offer and receives a positive gateway USD balance through
   the transfer-rate offer path.
5. Alice's trustline remains in the no-reserve state while `OwnerCount`
   remains `0`.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — offer crossing with transfer rate creates positive balance without reserve
17.3s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is the transfer-rate companion to the base offer-crossing
witness. It confirms the missing receiver-side owner-count/reserve transition
survives normal issuer transfer-rate accounting.

## Current sibling: CheckCash missing-owner-reserve boundary

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2`.
3. The gateway writes a USD check to Alice.
4. Alice is drained to exactly the two-owner reserve plus the CheckCash fee.
5. Alice cashes the check and receives 50 USD.
6. Alice remains below the three-owner reserve while `OwnerCount` remains `2`
   and the receiver reserve flag remains unset.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — CheckCash succeeds below missing owner reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this proves the same root cause at the reserve boundary. The
path succeeds even when the receiver lacks reserve capacity for the owner-count
increment that should accompany a positive trustline balance.

## Current sibling: TokenEscrow positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Bob clears the same gateway USD trustline back to zero balance, zero limit,
   `OwnerCount=0`, and no receiver reserve flag.
2. Alice escrows 40 USD to Bob through TokenEscrow.
3. Bob finishes the escrow and receives 40 USD.
4. Bob still has `OwnerCount=0` and no receiver reserve flag.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — TokenEscrow creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is a third live settlement path for the same old
receiver-side reserve transition. TokenEscrow correctly checks reserve when it
must create a missing destination line, but if the line already exists in the
cleared/no-reserve state, finish can move it positive without charging the
receiver owner reserve.

## Current sibling: NFToken seller-proceeds reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. The seller clears a gateway USD trustline back to zero balance, zero limit,
   `OwnerCount=0`, and no receiver reserve flag.
2. The seller mints an NFT and lists it for gateway USD.
3. The buyer accepts the sell offer and pays the seller 40 USD.
4. The seller still has `OwnerCount=0` and no receiver reserve flag.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — NFToken AcceptOffer creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this shows NFT seller proceeds can hit the same old
positive-balance trustline transition. The NFT page and offer are gone after
settlement; the positive IOU line is still not charged as an owned object.

## Current sibling: NFToken broker-fee reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. The broker clears a gateway USD trustline back to zero balance, zero limit,
   `OwnerCount=0`, and no receiver reserve flag.
2. A brokered NFT sale pays a 10 USD broker fee.
3. The broker receives the fee while `OwnerCount=0` and the receiver reserve
   flag remains unset.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — NFToken broker fee creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this gives a second NFT-specific witness for the same shared
IOU credit behavior. It is not counted as a new root cause.

## Current sibling: AMMWithdraw positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice clears a gateway USD trustline back to zero balance, zero limit,
   `OwnerCount=0`, and no receiver reserve flag.
2. Alice contributes XRP to an XRP/USD AMM and receives LP tokens.
3. Alice withdraws the LP position as a one-asset USD withdrawal.
4. Alice receives gateway USD while `OwnerCount=0` and the receiver reserve
   flag remains unset.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — AMMWithdraw creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is another current-live settlement family reaching the
same missing receiver-side reserve transition. It expands path coverage without
changing the root-cause count.

## Current sibling: AMMClawback positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice creates a two-issuer AMM and deposits all of her gateway2 EUR into the
   pool.
2. Alice clears the gateway2 EUR limit after gateway2 clears default ripple,
   leaving the EUR line present with no receiver reserve flag.
3. Gateway1 performs AMMClawback against Alice's gateway1 USD in the pool.
4. The paired gateway2 EUR return moves Alice positive while her owner count
   and EUR receiver reserve flag remain unchanged.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — AMMClawback creates positive balance without reserve
17.3s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is the AMMClawback paired-asset companion to the
AMMWithdraw witness. It is a path expansion for the same shared reserve
transition, not a separate root cause.

Additional dispositions:

- `BATCH-INNER-001`: a direct non-batch `LoanSet` carrying `tfInnerBatchTxn`
  and no counterparty signature was rejected as malformed before ledger
  application.
- `LOAN-PRINCIPAL-ZERO-001`: the later PR #7050 integer-MPT principal-zeroing
  hypothesis did not reproduce on the current proof build.
- `LOAN-OVERPAY-UNROUNDED-001`: the later PR #7039 precise IOU overpayment
  transaction completed with `tesSUCCESS` in the current proof build and did
  not produce a transaction-visible P0.
- `AMM-CLAWBACK-001`: demoted to product semantics because upstream
  `AMMClawback_test` intentionally exercises frozen/global-frozen trustlines
  and expects AMM clawback success.

## Reproduced: lending regular-freeze receive bypass

Status: reproduced before this sprint and preserved by the expanded suite.

Result excerpt:

```text
ripple.tx.OpenP0Repro F3.3 LoanBrokerCoverWithdraw — regular-freeze-only destination (P0)
ripple.tx.OpenP0Repro F3.3 control — deep-freeze blocks cover withdraw
ripple.tx.OpenP0Repro F3.5 LoanBrokerDelete — regular-freeze-only owner receives cover
ripple.tx.OpenP0Repro F3.9 LoanBrokerCoverDeposit — regular-freeze-only broker pseudo receives cover
ripple.tx.OpenP0Repro F3.7 LoanSet — regular-freeze-only broker owner receives origination fee
ripple.tx.OpenP0Repro F3.6 LoanPay — regular-freeze-only broker owner receives service fee
ripple.tx.OpenP0Repro F3.8 LoanPay — regular-freeze-only vault pseudo receives repayment
ripple.tx.OpenP0Repro F3.10 LoanPay — regular-freeze-only broker pseudo receives fallback fee
```

Interpretation: one root cause class with seven clean upstream receive paths.

## Reproduced: PermissionedDomainSet pre-fix ticket sequence collision

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `featurePermissionedDomains` and `featureCredentials` enabled, but `fixCleanup3_1_3` disabled.
2. Create two tickets for the same account.
3. Submit `PermissionedDomainSet` using the first ticket.
4. The pre-fix code stores the domain at `keylet::permissionedDomain(account, 0)` because it reads raw `sfSequence`.
5. Submit a second `PermissionedDomainSet` using the second ticket.
6. The second create collides on `keylet::permissionedDomain(account, 0)` and returns `tefEXCEPTION` with `dirInsert: double insertion`.

Result excerpt:

```text
ripple.tx.OpenP0Repro PermissionedDomainSet pre-fix ticket sequence collision candidate
FTL:OpenLedger apply: dirInsert: double insertion
ripple.tx.OpenP0Repro had 0 failures.
```

Negative control:

The same two-ticket sequence with `fixCleanup3_1_3` enabled creates two distinct permissioned-domain objects keyed by ticket sequence and leaves no `(account, 0)` object.

Interpretation: historical/replay-era P0-class transaction-path exception fixed by `fixCleanup3_1_3`, but relevant to pre-activation ledgers and to release-history quality analysis.

## Reproduced: expired credential cleanup ignored delete failure before fixCleanup3_1_3

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `featureCredentials`, `featurePermissionedDomains`, and `fixCleanup3_1_3` disabled.
2. Create and accept a credential that later expires.
3. Create credential-dependent `DepositPreauth` and `PermissionedDomain` objects.
4. Force the credential issuer account out of the view so `deleteSLE` cannot remove the expired credential cleanly.
5. The pre-fix cleanup path returns a normal expired/success result to consumers even though the credential remains.

Result excerpt:

```text
ripple.tx.OpenP0Repro Credentials pre-fix — expired cleanup ignores deleteSLE failure
```

Negative control:

With `fixCleanup3_1_3` enabled, the same `verifyDepositPreauth` and `verifyValidDomain` calls fail closed with `tecINTERNAL`.

Interpretation: historical/replay-era cleanup-consumer defect. The source signal is upstream commit `af89854a4` / PR #6962, which stops transaction processing if expired credential deletion fails.

## Reproduced: permissioned-DEX empty AdditionalBooks invariant bypass before fixCleanup3_1_3

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `featurePermissionedDomains` and `fixCleanup3_1_3` disabled.
2. Build a hybrid permissioned-DEX offer with `sfDomainID`.
3. Set `sfAdditionalBooks` to an empty array.
4. The pre-fix `ValidPermissionedDEX` invariant treats only missing or more-than-one `sfAdditionalBooks` as malformed and lets the empty array pass.

Result excerpt:

```text
ripple.tx.OpenP0Repro Permissioned DEX pre-fix — empty AdditionalBooks hides malformed hybrid offer
```

Negative control:

With `fixCleanup3_1_3` enabled, the same object fails the invariant with `hybrid offer is malformed`.

Interpretation: historical/replay-era invariant coverage defect. The source signal is upstream commit `96643bb0f` / PR #6716, which makes empty `sfAdditionalBooks` invalid for hybrid offers.

## Reproduced: MPT multi-send aggregate MaximumAmount bypass

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `fixCleanup3_1_3` disabled.
2. Create an MPT issuance with `MaximumAmount=150` and `tfMPTCanTransfer`.
3. Call the upstream helper/accounting path `accountSendMulti` with two destinations of 100 each.
4. The pre-fix helper call returns `tesSUCCESS` and the issuance records `sfOutstandingAmount=200`.

Result excerpt:

```text
ripple.tx.OpenP0Repro MPT multi-send pre-fix aggregate MaximumAmount bypass
```

Negative control:

The same helper call with `fixCleanup3_1_3` enabled returns `tecPATH_DRY`.

Interpretation: historical/replay-era P0-class accounting-path repro following upstream regression style. This proof suite does not present it as a standalone transaction-path repro.

## Reproduced: VaultWithdraw pre-fix share-denominated trustline limit bypass

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `featureSingleAssetVault` enabled and `fixCleanup3_1_3` disabled.
2. Give destination Charlie an IOU trustline limit of 5.
3. Confirm the asset-denominated withdraw path of 10 IOU to Charlie returns `tecNO_LINE`.
4. Submit the economically equivalent share-denominated withdraw to Charlie.
5. The pre-fix share path returns `tesSUCCESS` and Charlie's balance increases.

Result excerpt:

```text
ripple.tx.OpenP0Repro VaultWithdraw pre-fix share-denominated trustline limit bypass
```

Negative control:

The same share-denominated withdraw with `fixCleanup3_1_3` enabled returns `tecNO_LINE` and Charlie's balance is unchanged.

Interpretation: historical/replay-era P0-class transaction-path trustline-limit bypass fixed by `fixCleanup3_1_3`.

## Reproduced: VaultWithdraw pre-fix deletes share MPToken with locked escrow

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `fixCleanup3_1_3` disabled.
2. Deposit an MPT asset into a vault, receiving vault share MPT.
3. Escrow 500 share MPT to another account, leaving `sfLockedAmount=500` and `sfMPTAmount=500`.
4. Withdraw the remaining spendable shares.
5. The pre-fix path deletes the depositor's share MPToken despite the remaining locked amount.

Result excerpt:

```text
ripple.tx.OpenP0Repro VaultWithdraw pre-fix deletes share MPToken with locked escrow
```

Negative control:

The same flow with `fixCleanup3_1_3` enabled preserves the MPToken with `sfLockedAmount=500` and `sfMPTAmount=0`.

Interpretation: historical/replay-era P0-class transaction-path object-lifetime bug fixed by `fixCleanup3_1_3`.

## Reproduced: VaultClawback pre-fix zero-amount asset clawback is not clamped

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `featureSingleAssetVault`, `featureLendingProtocol`, and `fixCleanup3_1_3` disabled.
2. Deposit 100 IOU assets into a vault.
3. Create a loan backed by that vault so `sfAssetsAvailable=60` while `sfAssetsTotal=100`.
4. Submit a zero-amount `VaultClawback` for the depositor's asset shares.
5. The pre-fix path attempts to claw back the full share value rather than clamping to the available 60 assets, hits the negative-vault-balance guard, and returns `tefINTERNAL`.

Result excerpt:

```text
ripple.tx.OpenP0Repro VaultClawback pre-fix zero-amount asset clawback is not clamped to assets available
ERR:OpenLedger FEDF49CF... VaultClawback: negative balance of vault assets.
```

Negative control:

The same flow with `fixCleanup3_1_3` enabled succeeds, clamps to available assets, and leaves the outstanding 40 loan assets tracked in the vault.

Interpretation: historical/replay-era P0-class transaction-path solvency/accounting bug fixed by `fixCleanup3_1_3`.

## Reproduced: LoanPay pre-fix high-amount payment requires uncapped fee

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `featureSingleAssetVault`, `featureLendingProtocol`, and `fixCleanup3_1_3` disabled.
2. Create a loan with millions of remaining payments.
3. Submit a very high-amount `LoanPay` that can only process `loanMaximumPaymentsPerTransaction` payments in one transaction.
4. Pay only the capped fee implied by the actual maximum work performed.
5. The pre-fix path returns `telINSUF_FEE_P` because fee calculation scales with the estimated number of payments rather than the handler cap.

Result excerpt:

```text
ripple.tx.OpenP0Repro LoanPay pre-fix high-amount payment requires uncapped fee
```

Negative control:

The same flow with `fixCleanup3_1_3` enabled succeeds with the capped fee and reduces `sfPaymentRemaining` by exactly `loanMaximumPaymentsPerTransaction`.

Interpretation: historical/replay-era P0-class DoS/fee-schedule bug fixed by `fixCleanup3_1_3`.

## Reproduced current 3.1.3: LoanBrokerCover IOU precision drift

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create an IOU vault and loan broker with `sfCoverAvailable = 10`.
3. Submit `LoanBrokerCoverDeposit` for `1.8e-14` IOU.
4. The current path succeeds and `sfCoverAvailable` increases by `2e-14`, over-crediting broker cover relative to the amount submitted.
5. Submit a positive `1e-16` IOU cover deposit.
6. The current path succeeds even though the amount rounds to zero at the broker cover scale and leaves cover unchanged.
7. Submit a positive `1e-16` IOU cover withdraw.
8. The current path succeeds while broker cover and owner balance remain unchanged.
9. Submit a positive `1e-16` IOU cover clawback from an issuer with trustline clawback enabled.
10. The current path succeeds while broker cover and issuer balance remain unchanged.

Result excerpt:

```text
ripple.tx.OpenP0Repro LoanBrokerCover current — IOU precision drift
```

Interpretation: current `3.1.3` transaction-path precision/accounting drift. The source signals are later upstream commit `7fdaa0a5e` / PR #7274, which rounds cover operations to broker cover scale under the next cleanup lane, and commit `c327fc1ee`, which rejects sub-ULP cover amounts with `tecPRECISION_LOSS` under `fixCleanup3_2_0`.

## Reproduced current 3.1.3: LoanPay broker-cover minimum scale inconsistency

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create one tiny loan and one larger loan under the same broker.
3. Use issuer clawback to reduce broker cover to the current clawback minimum.
4. Pay each loan in a fresh environment from the same broker state.
5. The big loan's service fee goes to the broker pseudo-account while the tiny loan's fee goes to the broker owner, solely because LoanPay computes the minimum-cover threshold at individual loan scale.

Result excerpt:

```text
ripple.tx.OpenP0Repro LoanPay current — broker minimum cover scale inconsistency
```

Interpretation: current `3.1.3` transaction-path accounting bug. The source signal is later upstream commit `a911f9089` / PR #7093, which makes `LoanPay`, `LoanSet`, `LoanBrokerCoverWithdraw`, and `LoanBrokerCoverClawback` use the same vault-scale broker-cover minimum.

## Reproduced current 3.1.3: Vault share MPT transfer-restriction bypass

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create a transferable, mutable MPT issuance and an MPT-backed vault.
3. Deposit from Alice and Bob so both hold vault-share MPT.
4. Clear `CanTransfer` on the underlying MPT.
5. Peer-to-peer vault-share payment from Alice to Bob still succeeds, even though the underlying issuer disabled transfer.

Result excerpt:

```text
ripple.tx.OpenP0Repro Vault share MPT current — underlying CanTransfer is not inherited
```

Interpretation: current `3.1.3` vault-share authorization-state bug. The source signal is later upstream commit `9cb049276` / PR #7077, which propagates underlying MPT flags/reference metadata to vault shares so the share payment returns `tecNO_AUTH`.

## Reproduced current 3.1.3: LoanBrokerDelete returns locked MPT cover

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create a loan broker with MPT first-loss cover held by the broker pseudo-account.
3. Lock the broker pseudo-account's MPT cover.
4. Submit `LoanBrokerDelete`.
5. The current path succeeds, returns the locked cover to the broker owner, and deletes the locked pseudo-account MPToken.

Result excerpt:

```text
ripple.tx.OpenP0Repro LoanBrokerDelete current — locked MPT cover is returned
```

Interpretation: current `3.1.3` transaction-path object-lifetime / locked-asset behavior. The source signal is later upstream commit `179e73594` / PR #7125, which blocks this path under the next cleanup lane.

## Reproduced current 3.1.3: Loan payment factor cancellation

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Compute the periodic loan rate for `TenthBips32{1}` and interval `600`.
3. Compare current `detail::computePaymentFactor(r, 3)` with the independent polynomial reference for the same expression:
   `F(r,3) = (1 + 3r + 3r^2 + r^3) / (3 + 3r + r^2)`.
4. The current direct `(1+r)^n - 1` denominator loses precision near zero and differs from the reference by more than `1e-12`.

Result excerpt:

```text
ripple.tx.OpenP0Repro Loan current — near-zero payment factor cancellation
```

Interpretation: current `3.1.3` core loan amortization helper/accounting precision defect. This proof binds the root cause used by `LoanSet`/`LoanPay`; it is not a standalone transaction-path claim. The source signal is later upstream commit `ad2195f12` / PR #7033, which switches to a stable power-minus-one calculation.

## Reproduced current 3.1.3: VaultWithdraw IOU scale-boundary invariant failure

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create an IOU vault at scale 0.
3. Fund and deposit exactly `1e16` IOU so vault assets sit at an IOU precision boundary.
4. Submit `VaultWithdraw` for `5` IOU.
5. The current path returns `tecINVARIANT_FAILED` with vault-balance and destination-balance invariant failures.

Result excerpt:

```text
ripple.tx.OpenP0Repro Vault current — withdraw across IOU scale boundary invariant
FTL:OpenLedger Invariant failed: withdrawal must decrease vault balance
FTL:OpenLedger Invariant failed: withdrawal must increase destination balance
```

Interpretation: current `3.1.3` transaction-path liveness bug from scale-coarsened vault invariant rounding. The source signal is later upstream commit `633ef4706` / PR #7272, which adds precision-boundary coverage under the next cleanup lane.

## Reproduced current 3.1.3: VaultDeposit issuer IOU edge invariant failure

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create an IOU vault at scale 0.
3. Deposit `9,999,999,999,999,999` IOU so the vault pseudo-account sits at the IOU edge.
4. Submit an issuer `VaultDeposit` for `2` IOU.
5. The current path applies far enough to return `tecINVARIANT_FAILED` at finalize instead of rejecting precision loss before state mutation.

Result excerpt:

```text
ripple.tx.OpenP0Repro Vault current — issuer deposit at IOU edge invariant
FTL:OpenLedger Invariant failed: deposit must increase vault balance
```

Interpretation: current `3.1.3` transaction-path precision/invariant bug. The source signal is later upstream commit `633ef4706` / PR #7272, which changes this class to proactive `tecPRECISION_LOSS` rejection under the next cleanup lane.

## Reproduced current 3.1.3: Vault sole-shareholder impaired exit is stuck

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create an IOU vault with two depositors and issue a loan from that vault.
3. Impair the loan so the vault has available cash plus an impaired receivable.
4. Let the non-sole shareholder withdraw shares successfully.
5. Have the sole remaining shareholder request the vault's available cash.
6. The current path returns `tecINVARIANT_FAILED` because it burns every remaining share while assets are still outstanding as an impaired receivable.

Result excerpt:

```text
ripple.tx.OpenP0Repro Vault current — sole shareholder impaired exit is stuck
FTL:OpenLedger Invariant failed: updated zero sized vault must have no assets outstanding
```

Interpretation: current `3.1.3` transaction-path liveness bug for an impaired vault depositor. The source signal is later upstream commit `49567e728` / PR #7139, which changes sole-shareholder impaired withdraw math so the depositor can withdraw cash while residual shares remain backed by the impaired receivable.

## Reproduced current 3.1.3: VaultDeposit opposite-limit tefINTERNAL

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Give a depositor 100 IOU and configure the counterparty-side trustline limit so full-balance semantics admit a larger apparent balance.
3. Submit `VaultDeposit` for 500 IOU.
4. Preclaim admits the deposit, but apply-time accounting drives the depositor asset balance negative and returns `tefINTERNAL`.

Result excerpt:

```text
ripple.tx.OpenP0Repro VaultDeposit current — opposite trustline limit causes tefINTERNAL
ERR:OpenLedger C3963ACE... VaultDeposit: negative balance of account assets.
```

Interpretation: current `3.1.3` transaction-path internal failure from mismatched preclaim/apply balance semantics. The source signal is later upstream commit `93ac1aa7a` / PR #7288.

## Reproduced current 3.1.3: EscrowCancel after IOU trustline deletion

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create an IOU escrow from a funded trustline.
3. Empty and delete the sender's remaining IOU trustline.
4. After cancel time, submit `EscrowCancel`.
5. The current path returns `tefEXCEPTION` with an `OwnerCount` template-field error.

Result excerpt:

```text
ripple.tx.OpenP0Repro EscrowCancel current — deleted IOU trustline returns tefEXCEPTION
FTL:OpenLedger apply: Template field error 'OwnerCount'
```

Interpretation: current `3.1.3` transaction-path exception when escrow cancellation relies on a deleted IOU trustline instead of the account ledger entry. The source signal is later upstream commit `ad3d172a1` / PR #6171.

## Reproduced current 3.1.3: AMM stale AuthAccounts after empty reinit

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create an AMM and make a second account win the auction slot with authorized accounts `bob` and `dan`.
3. Withdraw all AMM liquidity while excess LP trustlines keep the empty AMM object alive.
4. Reinitialize the pool with `tfTwoAssetIfEmpty`.
5. The current path resets the slot owner and fee but leaves stale `sfAuthAccounts` from the previous auction slot.

Result excerpt:

```text
ripple.tx.OpenP0Repro AMM current — stale AuthAccounts survive empty reinit
```

Interpretation: current `3.1.3` AMM state reinitialization bug. The source signal is later upstream commit `e1fe35993` / PR #6996, which clears stale `AuthAccounts` under the next cleanup lane.

## Reproduced current 3.1.3: Delegatee account deletion leaves stale delegation

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Have account `alice` delegate `Payment` authority to account `bob`.
3. Let `bob` delete his account after the account-delete age window.
4. The current path succeeds, deletes `bob`, and leaves the `Delegate` ledger entry plus `alice`'s owner reserve behind.

Result excerpt:

```text
ripple.tx.OpenP0Repro Delegate current — delegatee account deletion leaves stale delegation
```

Interpretation: current `3.1.3` authority-state cleanup bug. The source signal is later upstream commit `4da46d31` / PR #6681, which stores `Delegate` objects in both the delegator and authorized account owner directories so account deletion can clean both sides atomically.

## Reproduced current 3.1.3: Domain-bound MPT RequireAuth clearing

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create a permissioned domain.
3. Create an MPT issuance with both `DomainID` and `RequireAuth` set, plus mutable `RequireAuth`.
4. Submit `MPTokenIssuanceSet` with `MPTClearRequireAuth`.
5. The current path succeeds, clears `RequireAuth`, and leaves `DomainID` present.

Result excerpt:

```text
ripple.tx.OpenP0Repro MPT current — domain-bound RequireAuth can be cleared
```

Interpretation: current `3.1.3` authorization-state bug. The source signal is later upstream commit `366899d5` / PR #6712, which rejects this mutation with `tecNO_PERMISSION` when `DomainID` is set.

## Reproduced current 3.1.3: Number upward-rounding cusp violation

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Set `Number` to the large mantissa scale and upward rounding mode.
3. Multiply `1,000,000,000,000,049,863` by `9,223,372,036,854,315,903`.
4. Compare the stored `Number` value to an exact integer product computed with `cpp_int`.
5. The current path stores a value below exact even though upward rounding should not round downward.

Result excerpt:

```text
ripple.tx.OpenP0Repro Number current — upward rounding falls below exact at maxRep cusp
```

Interpretation: current `3.1.3` consensus arithmetic helper defect. The source signal is later upstream commit `4094f7f6c` / PR #7051, which fixes rounding at the `Number::maxRep` cusp.

## Reproduced current 3.1.3: Number upward-division rounding violation

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Set `Number` to the large mantissa scale and upward rounding mode.
3. Divide `2` by `1,000,000,000,000,000,007`.
4. Compare the stored `Number` quotient to an exact decimal reference computed with `cpp_dec_float_50`.
5. The current path stores a positive quotient below exact even though upward rounding should not round downward.

Result excerpt:

```text
ripple.tx.OpenP0Repro Number current - upward division falls below exact quotient
```

Interpretation: current `3.1.3` consensus arithmetic helper defect. The source signal is later upstream commit `48b1716e6`, which expands the `operator/=` correction precision and adds the same upward-division regression class.

## Reproduced current 3.1.3: MPT transfer-rate overflow

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Construct a valid integral MPT amount of `1,230,000,000,000,000,000`.
3. Apply a `1,500,000,000` transfer rate with upward rounding through `multiplyRound`.
4. The current path throws `overflow_error` from the legacy scaled-mantissa route.

Result excerpt:

```text
ripple.tx.OpenP0Repro MPT current — transfer-rate scaling overflows large integral amount
```

Interpretation: current `3.1.3` consensus arithmetic helper defect for large integral MPT transfer-fee scaling. The source signal is later upstream commit `22fbf4d06`, which routes MPT/V2 transfer-rate math through `Number` arithmetic.

## Reproduced current 3.1.3: Delegated-payment fee/reserve coupling

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Give `alice` exactly enough XRP to keep the owner reserve and send a 1 XRP payment after creating a delegation.
3. Give delegate `bob` enough XRP to pay a high fee.
4. Submit a delegated payment from `alice` to `carol` with `bob` as delegate and fee payer.
5. The current path returns `tecUNFUNDED_PAYMENT` because the payment check incorrectly couples `alice`'s reserve requirement to the delegate-paid fee.

Result excerpt:

```text
ripple.tx.OpenP0Repro Delegate current — delegated fee is coupled to delegator reserve
```

Interpretation: current `3.1.3` delegated-payment liveness/accounting bug. The source signal is later upstream commit `17f26ba97` / PR #6568, which decouples reserve from delegated fee payment.

## Reproduced current 3.1.3: SAV transaction delegation

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Have `owner` delegate `VaultCreate` permission to `bob`.
3. Submit `VaultCreate` with `Account=owner` and `Delegate=bob`.
4. The current path succeeds and creates the vault.

Result excerpt:

```text
ripple.tx.OpenP0Repro Delegate current — SAV transaction can be delegated
```

Interpretation: current `3.1.3` authority-surface bug. The source signal is later upstream commit `46d5c67a` / PR #6489, which marks Single Asset Vault and Lending transactions `NotDelegable` until their delegation semantics are fully tested and validated.

## Reproduced current 3.1.3: Delegated multisign self-check rejection

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Have Alice delegate `Payment` authority to Bob.
3. Configure Bob's signer list to include Alice and Daria.
4. Submit a delegated Alice payment with `Delegate=bob` and a multisignature from Alice and Daria.
5. The current RPC/signing path rejects the transaction before ledger application, treating Alice as an illegal self-signer even though Bob is the delegate authority and fee payer.

Result excerpt:

```text
ripple.tx.OpenP0Repro Delegate current — delegator multisign rejected for delegate
```

Interpretation: current `3.1.3` delegated-authorization liveness bug. The source signal is later upstream commit `9cb074067` / PR #7064, which makes delegated multisigning check the delegate account instead of the delegator.

## Reproduced current 3.1.3: MPT non-canonical amount ledger acceptance

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create an MPT issuance and fund two holders.
3. Construct a signed `Payment` whose `sfAmount` is an unchecked non-canonical MPT amount with mantissa `UINT64_MAX`.
4. Submit the transaction from the issuer to a holder.
5. The current path accepts the transaction into ledger application and returns fee-burning `tecPATH_PARTIAL` instead of rejecting the malformed amount as `temBAD_AMOUNT`.

Result excerpt:

```text
ripple.tx.OpenP0Repro MPT current — non-canonical amount reaches ledger engine
```

Interpretation: current `3.1.3` malformed-amount admission bug. The source signal is later upstream commit `dcd2ff0b5` / PR #7117, which rejects non-canonical MPT amounts before transaction application and adds invariant coverage for non-canonical MPT ledger entries.

## Reproduced current 3.1.3: MPT STIssue legacy wire order

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Build an MPT `STIssue` with sequence `0xDEADBEEF`, whose canonical MPTID sequence bytes are `de ad be ef`.
3. Serialize the issue through current `STIssue::add()`.
4. The current wire payload stores the sequence bytes as `ef be ad de`.
5. The internal `STIssue` codec round-trip still returns the original MPTID, which hides the defect inside a homogeneous rippled process.
6. A canonical raw-MPTID payload using the V1 marker parses back to a different MPTID.

Result excerpt:

```text
ripple.tx.OpenP0Repro MPT current — STIssue sequence bytes are legacy-swapped
```

Interpretation: current `3.1.3` protocol-wire serialization defect, not a standalone transaction-path root in this suite. The source signal is later upstream commit `4b2d7871f`, which introduces the amended V2 `STIssue` format and writes/reads the MPT sequence bytes canonically.

## Reproduced current 3.1.3: Delegated MPT granular permission mutates issuance fields

Status: newly reproduced in this sprint from later upstream delegation-hardening history.

Minimal behavior:

1. Create an MPT issuance with mutable metadata and transfer-fee flags.
2. Grant a delegate only the granular `MPTokenIssuanceLock` permission.
3. Submit `MPTokenIssuanceSet` as the delegate with no lock/unlock flag, but with `sfMPTokenMetadata` and `sfTransferFee`.
4. Current `MPTokenIssuanceSet::checkPermission` inspects lock/unlock flags only, so the transaction succeeds and mutates the issuance.

Result excerpt:

```text
ripple.tx.OpenP0Repro Delegate current — MPT granular lock permission mutates issuance fields
```

Interpretation: current `3.1.3` delegated-authority scope bug. The source signal is later upstream commit `87e951470` / PR #6831, which adds a granular field/flag sandbox so a granular permission cannot smuggle unrelated transaction fields.

## Reproduced current 3.1.3: Delegated empty AccountSet consumes principal sequence

Status: newly reproduced in this sprint from authorization-batch triage and source review of `SetAccount::checkPermission`.

Minimal behavior:

1. Have account `alice` delegate only `Payment` authority to account `bob`.
2. Submit an empty/no-op `AccountSet` for `alice` with `Delegate=bob`.
3. Current `SetAccount::checkPermission` rejects unauthorized fields and flags, but if the delegated `AccountSet` has no fields or flags it returns success even though no AccountSet granular permission was granted.
4. The transaction returns `tesSUCCESS`, advances Alice's sequence, leaves Alice's balance unchanged, and charges Bob's fee.

Result excerpt:

```text
ripple.tx.OpenP0Repro Delegate current — empty AccountSet with unrelated permission consumes principal sequence
```

Interpretation: current `3.1.3` delegated-authority scope bug. The path lets unrelated delegated authority consume the principal account's sequence without any AccountSet permission.

## Reproduced current 3.1.3: Batch signer outer-account replay

Status: newly reproduced in this sprint from later upstream batch-signature fix history and source review of `serializeBatch`.

Minimal behavior:

1. Create a valid `tfOnlyOne` batch whose outer account is `alice`, with inner transactions from `bob` and `carol`, and `BatchSigners` from both.
2. Capture the `BatchSigners` array from that valid batch.
3. Submit a second batch with outer account `eve`, the same inner transaction IDs and flags, and the captured `BatchSigners`.
4. Current `3.1.3` accepts the replayed signer array because `serializeBatch` binds only the flags and inner transaction IDs, not the outer account or sequence.
5. The still-valid inner transaction applies and consumes the signer account's sequence and balance under Eve's outer batch.

Result excerpt:

```text
ripple.tx.OpenP0Repro Batch current — batch signer signatures replay across outer account
```

Interpretation: current `3.1.3` batch-authorization replay bug. The source signal is later upstream commit `7618b726b`, which binds batch signatures to the outer account and sequence so this replay becomes `temBAD_SIGNATURE`.

## Reproduced current 3.1.3: MPT locked-holder unauthorize without SAV

Status: newly reproduced in this sprint from source review of the current locked-deletion gate.

Minimal behavior:

1. Run current upstream tag `3.1.3` with `MPTokensV1` active and `SingleAssetVault` inactive.
2. Create an MPT issuance with `tfMPTCanLock`.
3. Authorize a holder, then have the issuer lock that holder's MPToken.
4. Have the holder submit `tfMPTUnauthorize`.
5. The holder's locked zero-balance MPToken is deleted, which erases the issuer lock state.
6. The holder can re-authorize and recreate the MPToken without `lsfMPTLocked`.

Result excerpt:

```text
ripple.tx.OpenP0Repro MPT current — locked holder can delete lock state without SAV
```

Interpretation: current `3.1.3` feature-bound lock-state bypass. The source signal is `MPTokenAuthorize::preclaim`, where the locked-token deletion check is gated on `featureSingleAssetVault`; this is not an all-amendments current-mainnet claim.

## Reproduced current 3.1.3: Permissioned DEX hybrid offer quality mismatch

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Create a permissioned domain and funded domain participants.
3. Have Bob place a hybrid offer in the domain book.
4. Have Alice place an opposing hybrid offer that partially crosses Bob's offer and leaves a remainder.
5. The remaining offer's domain-book directory key records the pre-crossing quality, while the open-book directory key records the post-crossing quality.
6. The open-book directory's `sfExchangeRate` still records the pre-crossing quality, so its metadata does not match the quality encoded in its ledger key.

Result excerpt:

```text
ripple.tx.OpenP0Repro Permissioned DEX current — hybrid offer open-book quality mismatch
```

Interpretation: current `3.1.3` order-book metadata corruption / repair-class bug. The source signal is later upstream commit `28cc20c81` / PR #7087, which fixes the open-book placement rate and adds a `LedgerStateFix` repair path for bad book-directory `sfExchangeRate` metadata.

## Reproduced current 3.1.3: Permissioned DEX regular-offer cancel invariant

Status: newly reproduced in this sprint from later upstream fix history.

Minimal behavior:

1. Run current upstream tag `3.1.3`.
2. Have a domain participant create a regular, non-domain offer.
3. Submit a domain `OfferCreate` with `OfferSequence` set to the regular offer sequence, which should atomically cancel the regular offer and place the domain offer.
4. The current path returns `tecINVARIANT_FAILED` because the permissioned-DEX invariant treats the deleted regular offer as a forbidden regular-offer mutation.

Result excerpt:

```text
ripple.tx.OpenP0Repro Permissioned DEX current — cancel regular offer via domain offer invariant
FTL:OpenLedger Invariant failed: domain transaction affected regular offers
```

Interpretation: current `3.1.3` transaction-path liveness bug for a valid permissioned-DEX offer operation. The source signal is later upstream commit `8c0080020` / PR #7118, which updates the invariant to ignore deleted regular offers under the next cleanup lane.

## Reproduced: invariant bool overwrite before fixCleanup3_1_3

Status: newly reproduced in this sprint.

Minimal behavior:

1. Run with `fixCleanup3_1_3` disabled.
2. Feed an invariant checker a bad ledger entry followed by a valid ledger entry of the same inspected type.
3. The pre-fix checker uses assignment where it should accumulate violations, so the later valid entry overwrites the earlier bad state.
4. The repro covers three affected invariant paths: an XRP-denominated trustline, a deep-freeze-without-freeze trustline, and an MPT issuance with invalid outstanding amount.

Result excerpt:

```text
ripple.tx.OpenP0Repro Invariant pre-fix — later good entries hide earlier bad entries
```

Negative control:

With `fixCleanup3_1_3` enabled, the same three invariant paths fail closed.

Interpretation: historical/replay-era invariant false-negative class. The source signal is upstream commit `321b86797` / PR #6609, which changes the relevant checks to accumulate with `|=` instead of replacing prior violation state.

## Demoted: DIDSet directory-full partial mutation

Status: tested and not reproduced.

The repro forces owner-directory fullness before `DIDSet`. The transaction returns `tecDIR_FULL` and no DID object remains.

Interpretation: do not count as a finding.

## Demoted: Batch all-or-nothing expired CredentialAccept rollback

Status: tested and not reproduced.

The repro creates an expired credential, wraps `CredentialAccept` and a payment inside `tfAllOrNothing`, and confirms the credential remains and the payment does not apply.

Interpretation: Batch rollback works for this side-effect case; do not count as a finding.

## Demoted: dry-run / simulate TxQ queue mutation

Status: source-reviewed against upstream `3.1.3`; not a current-tag finding.

Later branch commit `411286c51` looked like a candidate because it prevents dry-run transactions from being queued after direct apply fails. Current tag `3.1.3` already contains the same behavior through commit `3004e0493`: `TxQ::apply` returns `{telCAN_NOT_QUEUE, false}` when `tapDRY_RUN` is set before normal queue admission.

Interpretation: do not count as a current P0. A dry-run can still return an application result, but the checked path does not mutate TxQ on this target tag.

## Demoted: later permission-delegation rewrite as separate P0

Status: source-reviewed against upstream `3.1.3`; not promoted as a new current-tag root cause.

Upstream commit `fa6991812` introduces `featurePermissionDelegationV1_1` and rewrites several delegation-denial paths from fee-burning `tecNO_DELEGATE_PERMISSION` to non-fee `terNO_DELEGATE_PERMISSION`. The current `3.1.3` transaction pipeline already checks signatures before permission failure, so the reviewed delta does not expose an unauthenticated fee burn or direct unauthorized transaction path on this tag.

Interpretation: existing reproduced delegate findings remain counted separately: stale delegate state after delegatee account deletion, delegated payment reserve/fee coupling, SAV delegation, and delegated multisign self-check rejection. The broader `featurePermissionDelegationV1_1` rewrite is not counted as another P0 without a clean additional repro.

## Demoted: MPT DEX future-branch fixes

Status: scratch source/jtx review against upstream `3.1.3`; not a current-tag finding.

Several later MPT/DEX commits looked high-value: `210b6e08b` removes overflowing MPT DEX offers, `305d784b2` fixes transfer-fee burn rounding during offer crossing, and `c6918f591` prevents zero-input MPT offer fills after clipping. On current tag `3.1.3`, the local transaction checks reject MPT `OfferCreate` before ledger application with `invalidTransaction` / `Amount can not be MPT`.

Interpretation: these are future-branch quality signals, not current `3.1.3` public-article P0s.

## Demoted: AMMBid LP-token refund reserve sibling

Status: scratch-tested on upstream `3.1.3`; not a finding.

The repro gave Alice an AMM auction slot, burned her LP-token balance to zero,
cleared her LP-token trustline, and then let Bob outbid her. The AMMBid refund
did recreate Alice's LP-token trustline, but it also increased Alice's
`OwnerCount` and set the correct reserve flag.

Interpretation: `AMMBid` remains relevant for the separate holder-`DepositAuth`
refund bypass, but this path does not reproduce the legacy
positive-balance/no-reserve trustline defect.

## Demoted: AMMDeposit LP-token reserve sibling

Status: scratch-tested on rebuilt upstream `3.1.3`; not a finding.

The repro funded a depositor at base reserve plus fees, with no AMM LP-token
trustline and `OwnerCount=0`, then attempted a one-sided AMM deposit. Current
`AMMDeposit::preclaim` checks `ammLPHolds(...)` and requires liquid XRP for one
additional owner reserve before LP-token creation. The transaction returned
`tecINSUF_RESERVE_LINE`, created no LP-token trustline, and left
`OwnerCount=0`.

Interpretation: this is a clean source-kill for the AMMDeposit LP-token sibling
of `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`. AMMDeposit still matters for the
separate empty-pool `DisallowIncomingTrustline` bypass, but the LP-token reserve
boundary fails closed.

## Demoted: AccountDelete with positive unowned trustline

Status: scratch-tested on rebuilt upstream `3.1.3`; not a finding.

The repro first created the current trustline reserve drift: Alice held a
positive gateway USD balance after offer crossing while her `OwnerCount`
remained `0` and the receiver reserve flag stayed unset. It then advanced the
ledger enough for `AccountDelete` and attempted to delete Alice into a funded
destination. The transaction returned `tecHAS_OBLIGATIONS`; Alice's account
root and the positive trustline both remained.

Interpretation: the reserve bug does not currently extend into account deletion
or a stranded positive trustline after account removal. The positive trustline
still blocks `AccountDelete` even though it is not reflected in Alice's owner
count.

## Demoted: TrustSet legacy reserve carveout

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a finding.

The candidate asked whether the old two-free-trustline carveout in
`SetTrust::doApply` could be the deeper Moby Dick behind the current
positive-balance/no-reserve bug. It is not. The carveout is explicit gateway
bootstrap semantics: `reserveCreate` is zero only while `OwnerCount < 2`; after
that, normal trustline reserve enforcement resumes.

The focused source/history sweep and upstream `SetTrust,TrustAndBalance,Freeze`
suites were packet-bound:

```text
trustset_reserve_carveout_static_sweep_20260528.log sha256 3ec97306684ca8e4072a76c7058f285dcae06cff1aa5823a03dea69a8976f65b
trustset_reserve_carveout_history_sweep_20260528.log sha256 89daae83eec3df6338b5855ffce51f945a1821829ce816c5f9abe32dd5c91352
trustset_reserve_carveout_source_kill_20260528.log sha256 593e5958864764b00791eb39d5da57c5ccf6a9a49c1dc56a838b4b87c7112b31
19.8s, 3 suites, 145 cases, 13435 tests total, 0 failures
```

Interpretation: the promoted trustline reserve bug stays narrow. The bug is
the missing receiver-side owner-count/reserve transition when settlement moves
a trustline from non-positive to positive balance; the explicit `TrustSet`
bootstrap carveout does not by itself reproduce state corruption or stranded
objects.

## Demoted: AccountSet legacy flag sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a finding.

The candidate asked whether old AccountSet policy flags or settings could be a
legacy-core source of forbidden flag combinations, broken alternate-key state,
freeze/auth policy drift, reserve/object drift, or invariant failure. The sweep
covered `SetAccount`, `SetRegularKey`, RequireAuth, NoFreeze, GlobalFreeze,
DefaultRipple, DisallowXRP, DisableMaster, AllowTrustLineClawback,
TransferRate, TickSize, Domain, EmailHash, and MessageKey source/history.

There is no standalone `SetAccount` suite in this tree, so the test pass
covered AccountSet effects through the suites that exercise those flags:
`Freeze,SetTrust,SetRegularKey,TrustAndBalance,AccountDelete,Invariants,Clawback,DepositAuth`.

```text
accountset_legacy_flag_static_sweep_20260528.log sha256 3301768d245f4f8b883fd79e1bf9e3c90f1cf75b1f540ae8c3a873e88c04ab5e
accountset_legacy_flag_history_sweep_20260528.log sha256 3c20b7cf7445e52bb283be15e4b0135a3d79a550bfc17b66dad641f382791137
accountset_legacy_flag_source_kill_20260528.log sha256 8027ce231849a0ec589c0dd1a254a598c8b14345b5a4f5e4075fe7870977a5ba
62.7s, 8 suites, 263 cases, 32279 tests total, 0 failures
```

Interpretation: this did not isolate a new Moby Dick P0. AccountSet remains a
high-leverage review surface, but this bounded pass did not produce a clean
current transaction witness.

## Demoted: legacy transaction envelope sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a finding.

The candidate asked whether old transaction envelope/signing/sequence code
could expose a normal-input replay, signature/auth bypass, ticket/sequence
collision, malformed canonical object, pseudo-transaction flag issue, or
deterministic ordering witness below individual transaction families. The sweep
covered `STTx`, `STObject`, `Serializer`, `SeqProxy`, `Transactor`,
`ApplyContext`, `CreateTicket`, `SetRegularKey`, multisign, pseudo
transactions, and transaction ordering.

The focused source/history sweep and upstream protocol/transaction suites were
packet-bound:

```text
legacy_tx_envelope_static_sweep_20260528.log sha256 5d27bc9d2065dba98556c9fd9a029de33305ac4c95a6f414bbe15d5e0ddbe8c3
legacy_tx_envelope_history_sweep_20260528.log sha256 ca4d8acc13cecef9687515db3b2a1ef12b6314779b56ee9050954840d665a882
legacy_tx_envelope_source_kill_20260528.log sha256 b490584f0860342399e30e3220f39a633afbb2bb2a8f46d8c9f020bc33f3eb7a
9.4s, 10 suites, 92 cases, 6782 tests total, 0 failures
```

Interpretation: this did not isolate a new Moby Dick P0. The old envelope
layer remains security-critical, but this bounded pass did not produce a clean
transaction-visible replay, signature/auth, sequence, serialization, or
canonical-ordering witness.

## Demoted: legacy IOU zero-cross settlement sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The candidate asked whether the current trustline reserve bug has another old
IOU settlement sibling: a route that moves a trustline from non-positive to
positive balance without the matching receiver reserve flag or `OwnerCount`
transition. The sweep covered the shared `rippleCredit`, `accountSend`,
`trustCreate`, `trustDelete`, `adjustOwnerCount`, `lsfLowReserve`, and
`lsfHighReserve` call sites across `Payment`, path steps, `Offer`, `CheckCash`,
`SetTrust`, `TrustAndBalance`, and `PaymentSandbox`.

The focused source/history sweep and upstream settlement suites were
packet-bound:

```text
legacy_iou_zero_cross_static_sweep_20260528.log sha256 e7a48bf5faf8a95ee052abe2c9d99b814488e33a97a48e6ced2d6a87a9f7c155
legacy_iou_zero_cross_history_sweep_20260528.log sha256 92d7e9c4fc52b68808d525be312a53285495800ad9f28a5d183470d2188a5b66
legacy_iou_zero_cross_source_kill_20260528.log sha256 b1148f687e36bc488913c25eac67b57197641ce75d98fab2a296c65add0f31c5
102.9s, 14 suites, 545 cases, 74558 tests total, 0 failures
```

Interpretation: this pass reinforced the already-promoted
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` root and its packet-bound markers, but
did not isolate a separate Moby Dick P0 outside that root.

## Demoted: legacy account-root owner-directory cleanup sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a finding.

The candidate asked whether old account-root deletion, owner-directory,
owner-count, or obligation cleanup could permit account deletion with live
state, stranded owner objects, missing `tecHAS_OBLIGATIONS`, or owner-directory
drift. The sweep covered `AccountDelete`, `DeleteAccount`, owner-directory
helpers, `OwnerCount`, `adjustOwnerCount`, `dirInsert`, `dirRemove`,
`deleteSLE`, `AccountRootsNotDeleted`, and account-root lookup patterns across
core transaction and invariant code.

The focused source/history sweep and upstream account-cleanup suites were
packet-bound:

```text
legacy_accountroot_ownerdir_static_sweep_20260528.log sha256 8022db74ebbbf03fe0bd4028f14cdba95ae6edea05c61cb11422b20c8ae8511e
legacy_accountroot_ownerdir_history_sweep_20260528.log sha256 878c00d1557f3a96c7e2585348fea0bf49201b545fa7f74e21b0e0b5c7b7bbf0
legacy_accountroot_ownerdir_source_kill_20260528.log sha256 77fa2c011c5601d083e776af4c302c17a2667bc50eb23202813c00f3168b0fb0
75.2s, 10 suites, 302 cases, 37723 tests total, 0 failures
```

Interpretation: this did not isolate a new Moby Dick P0. The checked core
cleanup paths did not show account deletion with live obligations, stranded
core owner objects, owner-count drift, or missing cleanup.

## Demoted: legacy governance and pseudo-transaction sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a finding.

The candidate asked whether old amendment voting, fee voting, validator-list,
negative-UNL, validator-site, validator-key, load-fee, or pseudo-transaction
machinery could expose consensus governance drift, bad pseudo-transaction
validation, malformed fee setting, validator-list quorum/trust error,
negative-UNL handling error, or unsigned governance state transition. The sweep
covered `AmendmentTable`, `FeeVote`, `PseudoTx`, `ValidatorList`,
`ValidatorSite`, `ValidatorKeys`, `LoadFeeTrack`, amendment majority/veto
logic, `SetFee`, negative-UNL handling, and trusted-validator list parsing.

The focused source/history sweep and upstream governance suites were
packet-bound:

```text
legacy_governance_pseudotx_static_sweep_20260528.log sha256 575cb9b65f3d4d308f695f030d8e2a78b8d7d588d741ce3b277f8af9b5e83ccd
legacy_governance_pseudotx_history_sweep_20260528.log sha256 bccaf0e1845876d1d4caa6069d621bc7a0b905f36d6ad4caedbb0a6f7da1d1d4
legacy_governance_pseudotx_source_kill_20260528.log sha256 7c0448b965667a5e765543a26302f6856bbf6f125f3293dbe56a76138bf6e46e
40.5s, 7 suites, 126 cases, 9939 tests total, 0 failures
```

Interpretation: this did not isolate a new Moby Dick P0. The checked governance
and pseudo-transaction paths did not show bad amendment majority/veto state,
malformed fee-vote pseudo transaction, validator-list trust/quorum drift,
negative-UNL state corruption, or unsigned pseudo-transaction acceptance.

## Demoted: Check directory-full partial-write sweep

Status: scratch-tested earlier, then source-reviewed and suite-tested on
upstream `3.1.3`; not a finding.

The candidate asked whether `CreateCheck` could insert the `Check` object and
destination owner-directory entry, fail when inserting the source owner
directory entry, and still leave a persistent object, owner-directory entry, or
owner-count drift. The earlier scratch probe forced the source directory full;
`CheckCreate` returned `tecDIR_FULL` and left no partial `Check`, no source or
destination directory residue, and no owner-count change.

The follow-up source/history sweep and upstream suites were packet-bound:

```text
check_legacy_dirfull_partial_static_sweep_20260528.log sha256 7d14fb34bd74bbaf3dbcd69abe53aacc6db9d1de5bee3484195e928f71615121
check_legacy_dirfull_partial_history_sweep_20260528.log sha256 5beef5ceee9f14f3ca77e87dfb181ae1802d69c870700ca4fa134c4065811fe0
check_legacy_dirfull_partial_source_kill_20260528.log sha256 95a44193bb61202532e82a8bddeaf1127c436e3ee6c73816459f4fd5ba4a5881
44.5s, 3 suites, 121 cases, 22604 tests total, 0 failures
```

Interpretation: this did not isolate a new Moby Dick P0. The checked
`CreateCheck`, `CancelCheck`, `CashCheck`, account-delete, and invariant paths
did not show a persistent partial object, directory entry, owner-count drift,
or normal-input invariant failure from the Check directory-full boundary.

## Demoted: DID and Credentials directory-full/object-lifecycle sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a finding.

The candidate asked whether DID or Credential object creation/deletion could
leave a partial ledger object, owner-directory entry, owner-count drift, or
invariant residue when a directory write or cleanup path fails. Direct feature
receipts bound `DID`, `fixEmptyDID`, and `Credentials` as enabled on validated
mainnet data for this slice.

The focused source/history sweep and upstream suites were packet-bound:

```text
direct_xrpl_did_feature_status_20260528.json sha256 e97e39ecd9ebf7e83a144887c65e330c664e70230b823ac2dbbe6e0ad8bace4c
did_credentials_dirfull_static_sweep_20260528.log sha256 49c050eacb4e91dcedaed288ff4e25717fdbc28c4408145633c87fe8e624a943
did_credentials_dirfull_history_sweep_20260528.log sha256 0558122317a5c1bfce589f880e3c99180330745802465b77fadbc7a80722a409
did_credentials_dirfull_source_kill_20260528.log sha256 aeab9d9c3b27a75513941f81144136cf62ba49d4c4f7ab9c4447b28762de98f1
41.1s, 4 suites, 147 cases, 16656 tests total, 0 failures
```

Interpretation: this did not isolate a new Moby Dick P0. The checked DID,
Credential, account-delete, and invariant paths did not show a persistent
partial object, directory entry, owner-count drift, or normal-input invariant
failure from the DID/Credentials lifecycle boundary.

## Demoted: legacy offer-book directory, quality, and cancel sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The candidate asked whether old `OfferCreate`, `OfferCancel`, offer-stream,
reduced-offer, book-directory, or invariant paths could leave stale offer
objects, wrong book-directory quality, owner-count drift, or normal-input
internal/invariant failures outside the already-promoted trustline reserve
root. The sweep covered `BookTip`, `BookDirs`, directory insert/remove paths,
taker quality, `sfExchangeRate`, dry-offer removal, reduced-offer fixes,
Fill-or-Kill/Immediate-or-Cancel, owner directories, deletion, and book
invariants.

The focused source/history sweep and upstream suites were packet-bound:

```text
legacy_offer_book_directory_static_sweep_20260528.log sha256 da406b598c0010a35f6c94f17bffa44dc399d30495a981df5973dc22bbf89253
legacy_offer_book_directory_history_sweep_20260528.log sha256 cd255407625b9623033375c75e9a3b121697a7c80c5e841977cb0c64611aacc8
legacy_offer_book_directory_source_kill_20260528.log sha256 42209d165bd4eb8dd5ff75362f437142c027a5da09470ee803fd296f6d665b90
112.6s, 11 suites, 435 cases, 83311 tests total, 0 failures
```

Interpretation: this pass did not isolate a new Moby Dick P0. The checked
offer-book and directory paths did not show stale offer objects, wrong
book-directory quality, owner-count drift, or normal-input invariant/internal
failure beyond the existing trustline positive-balance reserve finding.

## Demoted: legacy payment/path result-code and amount/issue sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The candidate asked whether old `Payment` and pathing inputs could route normal
transaction-shaped input to `tefEXCEPTION`, `tefINTERNAL`, fee-burning bad-input
behavior, amount/issue mismatch state, or another transaction-visible payment
P0. The sweep covered `Payment`, `Flow`, `PayStrand`, path steps,
`PaymentSandbox`, `STAmount`, `STNumber`, `Number`, `IOUAmount`, `Issue`,
`SetTrust`, and `TrustAndBalance`.

The focused source/history sweep and upstream suites were packet-bound:

```text
legacy_payment_path_resultcode_static_sweep_20260528.log sha256 2daf4925260ba78b6130256fa2f260879aece0d994750264f1fba9e919cec718
legacy_payment_path_resultcode_history_sweep_20260528.log sha256 35f878a1f76e052744bdbd1f8aef81f25a59eaccec8b0149d93732c41c4bd884
legacy_payment_path_resultcode_source_kill_20260528.log sha256 e8445786dca171ccc500810f8a4ec456316282d423e126c9def3b5387215e254
31.1s, 11 suites, 265 cases, 122354 tests total, 0 failures
```

Interpretation: this pass did not isolate a new Moby Dick P0. The checked
payment, pathing, amount, and trustline-control paths did not show a separate
normal-input internal result-code, fee-burning malformed-input, amount/issue
mismatch, or payment-path witness beyond the existing trustline positive-balance
reserve finding and already-demoted plain-payment reserve siblings.

## Demoted: native XRP payment account-create and tag sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The candidate asked whether native XRP `Payment` account creation,
destination-tag enforcement, `DisallowXRP`, `DepositAuth`, direct `accountSend`,
or account-delete destination handling could expose a separate old-core
transaction witness. The sweep covered destination `AccountRoot` creation,
`tecNO_DST`, `tecNO_DST_INSUF_XRP`, `tecDST_TAG_NEEDED`, direct XRP policy
checks, owner counts, and invariant boundaries.

The focused source/history sweep and upstream suites were packet-bound:

```text
xrp_payment_accountcreate_static_sweep_20260528.log sha256 8033c2dd46c73bfdf8b558443ea388d865e52baa54013c4a5525119b1e3455e3
xrp_payment_accountcreate_history_sweep_20260528.log sha256 a3302a7641dc965cbf24a862afa8fbf567ea1d6656bcb22223bd59280f182bae
xrp_payment_accountcreate_source_kill_20260528.log sha256 1f58085a0366563ee445160d5c18496fb3ba8ad1cc664fcb97d892b31293f463
68.9s, 10 suites, 234 cases, 39070 tests total, 0 failures
```

Interpretation: this pass did not isolate a new Moby Dick P0. The checked
native XRP account-create, destination-tag, `DisallowXRP`, `DepositAuth`, and
direct-send boundaries did not show account-object corruption, policy bypass,
fee-burning bad-input, owner-count drift, or a normal-input invariant/internal
failure.

## Demoted: MPT authorization, lock, and clawback sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

After the legacy-core queue was source-killed, the first broader live-feature
pass focused on MPT authorization, lock, RequireAuth, clawback, TokenEscrow,
credential, and domain-bound paths. The sweep covered `MPTokenIssuanceCreate`,
`MPTokenIssuanceSet`, `MPTokenIssuanceDestroy`, `MPTokenAuthorize`,
`MPTAmount`, `MPTIssue`, `accountSendMPT`, `rippleSendMPT`, `Clawback`,
`AMMClawback`, `EscrowToken`, `PermissionedDomains`, and `Credentials`.

The focused source/history sweep and upstream suites were packet-bound:

```text
mpt_auth_lock_clawback_static_sweep_20260528.log sha256 f8ba2b4821d3c7e0ae06173b3ab2faaf70ed311cf96035ebd76b4cdd94a16a18
mpt_auth_lock_clawback_history_sweep_20260528.log sha256 09dbed8b34b14cd3b993766b2b0ddfa2b21954f834ce5fa906cb43e55be35d59
mpt_auth_lock_clawback_source_kill_20260528.log sha256 02ccb8bdd0653a034e6f3ede57a92167e81a493138f6ea592346437e76181018
64.1s, 6 suites, 258 cases, 31582 tests total, 0 failures
```

Interpretation: this pass did not isolate a new live MPT P0. The checked paths
reinforce existing packet and triage entries around MPT transfer-rate overflow,
non-canonical amount handling, wire-format ordering, no-SAV lock-state product
semantics, and domain-auth/SAV dependency boundaries, but did not produce a new
authorization, lock, clawback, owner-count, credential, TokenEscrow, or
invariant witness.

## Demoted: AMM authorization, freeze, and invariant sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The next broader live-feature pass focused on AMM and AMMClawback
authorization, freeze, clawback, auction-slot, LP-token, offer-crossing,
pool-state, and invariant paths. The sweep covered `AMMCreate`, `AMMDeposit`,
`AMMWithdraw`, `AMMBid`, `AMMVote`, `AMMDelete`, `AMMClawback`, `Clawback`,
offer crossing, `SetTrust`, `Freeze`, `DepositAuth`, AMM helper code, AMM core
math, owner-count paths, `sfAuthAccounts`, LP-token paths, and invariant
checks.

The focused source/history sweep and upstream suites were packet-bound:

```text
amm_auth_freeze_invariant_static_sweep_20260528.log sha256 2f82bff1fb0a81f2dff834a1d0a66dcbac9cc7cbda7e0ff5704e3c954b1fffed
amm_auth_freeze_invariant_history_sweep_20260528.log sha256 131fb56e5f428e29e098976978305c27c27ad91aedf8921835e11ec890a69459
amm_auth_freeze_invariant_source_kill_20260528.log sha256 7ec02a6f30a672bdcf992ae663800a5506b3a5af44cdf8e6b83d05b9ae2d91e6
212.0s, 14 suites, 688 cases, 154192 tests total, 0 failures
```

Interpretation: this pass did not isolate a new live AMM P0. The checked paths
reinforce existing packet and triage entries around AMM `DisallowIncoming`,
AMMClawback paired-asset returns, AMM DepositAuth bypasses, AMMBid refunds,
stale `AuthAccounts`, and trustline reserve drift, but did not produce a new
authorization, freeze, clawback, pool-state, owner-count, offer-crossing, or
invariant witness.

## Demoted: PermissionedDEX domain, credential, and invariant sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The next broader live-feature pass focused on PermissionedDEX domain,
credential, hybrid-offer quality, cancellation, book-directory, and invariant
paths. The sweep covered `PermissionedDEXHelpers`,
`PermissionedDomainSet/Delete`, `Credentials`, credential helper cleanup,
`CreateOffer`, `CancelOffer`, `OfferStream`, `sfAdditionalBooks`,
`sfExchangeRate`, domain and credential lookup, owner-count paths, directory
paths, and invariant checks.

The focused source/history sweep and upstream suites were packet-bound:

```text
pdex_domain_credential_invariant_static_sweep_20260528.log sha256 7ffa888179fdde633a202e96ad00001a50416d7f79b37fd2209faa7940c3fb31
pdex_domain_credential_invariant_history_sweep_20260528.log sha256 a4d5db210f6bb2c18278b64d0945baf2f096311ed58c672765a1dabc0eeed89e
pdex_domain_credential_invariant_source_kill_20260528.log sha256 048f829278f8ab75d2f085eb3b3872d0575634341f673fa3bbe67f00a3ee4f8b
125.7s, 13 suites, 513 cases, 89613 tests total, 0 failures
```

Interpretation: this pass did not isolate a new live PermissionedDEX P0. The
checked paths reinforce existing packet and triage entries around
`PDEX-HYBRID-QUALITY-001`, `PDEX-CANCEL-INVARIANT-001`, the cleanup-era
empty-AdditionalBooks exclusion, and credential/domain lifecycle handling, but
did not produce a new domain, credential, hybrid-book, cancellation,
book-directory, owner-count, or invariant witness.

## Demoted: TokenEscrow result-code and invariant sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The next broader live-feature pass focused on TokenEscrow result-code,
owner-count, directory, amount, authorization, freeze, clawback, MPT, and
invariant paths. The sweep covered `EscrowCreate`, `EscrowFinish`,
`EscrowCancel`, `escrowUnlockApplyHelper`, `DeleteAccount`, `InvariantCheck`,
`MPTokenAuthorize`, `MPTokenIssuanceCreate/Set`, `Clawback`, `AMMClawback`,
STAmount/MPTAmount/IOUAmount arithmetic, owner directories, `DepositAuth`,
`RequireAuth`, freeze checks, and `fixTokenEscrowV1` boundaries.

The focused source/history sweep and upstream suites were packet-bound:

```text
tokenescrow_resultcode_invariant_static_sweep_20260528.log sha256 a38b3fc2f7b5fbfd7eb1ed94338513e1c96cb48b880a1fc4f4c288e67c3515c6
tokenescrow_resultcode_invariant_history_sweep_20260528.log sha256 031ddf73f8b1ea7acb68f4bc4c2ac346ae656d97539762130fa20bf3bd3bab9f
tokenescrow_resultcode_invariant_source_kill_20260528.log sha256 11cf95bd00a8e169a62df4968d679556fbc1a925efe2d5e9fab5f7b873fb94a5
122.4s, 13 suites, 447 cases, 169461 tests total, 0 failures
```

Interpretation: this pass did not isolate a new live TokenEscrow P0. The
checked paths reinforce existing packet and triage entries around
`TOKENESCROW-DISALLOW-INCOMING-FINISH-001`, TokenEscrow trustline-reserve
markers, MPT lock/authorization boundaries, escrow account-delete obligations,
and amount/invariant handling, but did not produce a new result-code,
owner-count, directory, amount, authorization, freeze, clawback, MPT, or
invariant witness.

## Demoted: Credentials current-tag transaction surface sweep

Status: source-reviewed and suite-tested on upstream `3.1.3`; not a separate
finding.

The next broader live-feature pass focused on Credentials create, accept,
delete, helper cleanup, DepositAuth, PermissionedDomains, DID, AccountDelete,
owner directories, owner counts, RPC visibility, and invariant paths. The
sweep covered `CredentialCreate`, `CredentialAccept`, `CredentialDelete`,
`deleteSLE`, `removeExpired`, `verifyDepositPreauth`, `verifyValidDomain`,
`PermissionedDomainSet/Delete`, DID cleanup, `AccountObjects`, `OwnerInfo`,
`DepositAuthorized`, owner-directory boundaries, and credential-visible
settlement surfaces.

The focused source/history sweep and upstream suites were packet-bound:

```text
credentials_current_tag_surface_static_sweep_20260528.log sha256 43b494f6da8ca4c8d9bca5278ca8592b2f8b936eecfadb5a33f391055488139c
credentials_current_tag_surface_history_sweep_20260528.log sha256 0ba4427881b5e9100b76ba4ac4a1f115370dd8ab59686171330baaa2bcd219be
credentials_current_tag_surface_source_kill_20260528.log sha256 de40b800e94e4a19b09e3e54d14ffd2731cad298e9cbd60924fb533ef3555b58
101.7s, 15 suites, 381 cases, 48135 tests total, 0 failures
```

Interpretation: this pass did not isolate a new live Credentials P0. The
checked paths reinforce existing packet and triage entries around
PermissionedDEX credential usage, DepositAuth credential authorization, DID and
Credentials lifecycle cleanup, owner-directory boundaries, and RPC visibility,
but did not produce a new credential create/accept/delete, expired-credential
cleanup, PermissionedDomain, DID, account-cleanup, owner-count, directory, or
invariant witness.

## Demoted: XChainBridge live-gate sweep

Status: direct-live-gated and suite-tested on upstream `3.1.3`; not a packet
finding.

The open bridge lane asked whether XChain claim/account-create counters,
attestations, bridge object lookup, bridge RPC visibility, or bridge
transaction result-code paths could produce a live-mainnet candidate.

Direct XRPL `feature` calls against `s1.ripple.com` and `s2.ripple.com` showed
both `XChainBridge` and `fixXChainRewardRounding` are supported but disabled on
validated ledger `104535656`, hash
`45489521F86EBEBE317210579A50D680802199BF6D912A737147EAFD9AC2E58D`.

The focused source/history sweep and upstream suites were packet-bound:

```text
xchain_bridge_feature_status_20260528.json sha256 86a50d1de9df2601e07be0e09d5ddefb77c22b11106c3eec58af9cd9672d8b9e
xchain_bridge_live_gate_static_sweep_20260528.log sha256 52c3491dc8719c9b1c7c5228d63c18336f5547aceb87bed75ac0fe861b31dd38
xchain_bridge_live_gate_history_sweep_20260528.log sha256 a8cb4a51b0cfe3ec74664abae9a043130dab239da4e3a2b37864472ccab47187
xchain_bridge_live_gate_source_kill_20260528.log sha256 e5c9c16fc01a604825af90d63340b94beaefb3c2abbbb708108ad3b46e3774de
42.8s, 4 suites, 42 cases, 56064 tests total, 0 failures
```

Interpretation: this pass did not isolate a live bridge P0. The live gate alone
excludes bridge candidates from the packet unless mainnet later enables the
amendment surface; the local suite run also passed across XChain transaction,
simulation, AccountObjects, and LedgerEntry coverage.

## Open candidates

The NFT, authorization, and remaining vault/loan candidates are
source-backed or model-triaged but not reproduced. They stay in
`candidate_matrix.md` until a clean jtx or unit-level repro exists.
