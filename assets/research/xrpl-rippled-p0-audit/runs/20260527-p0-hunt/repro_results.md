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
14.9s, 1 suite, 63 cases, 16284 tests total, 0 failures
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
14.9s, 1 suite, 63 cases, 16284 tests total, 0 failures
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
14.9s, 1 suite, 63 cases, 16284 tests total, 0 failures
```

Interpretation: this is the offer-side companion to the CheckCash
existing-owner control. Both live settlement paths skip the receiver-side
owner-count/reserve update even after the holder is beyond the historical
two-object reserve carveout.

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
14.8s, 1 suite, 63 cases, 16284 tests total, 0 failures
```

Interpretation: this proves the same root cause at the reserve boundary. The
path succeeds even when the receiver lacks reserve capacity for the owner-count
increment that should accompany a positive trustline balance.

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

## Open candidates

The bridge, NFT, AMM, authorization, and remaining vault/loan candidates are source-backed or model-triaged but not reproduced. They stay in `candidate_matrix.md` until a clean jtx or unit-level repro exists.
