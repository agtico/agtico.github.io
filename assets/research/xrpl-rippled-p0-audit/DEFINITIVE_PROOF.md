# Definitive proof log — XRPL lending freeze audit

Date: 2026-05-26  
Original binary: `/home/pfrpc/repos/rippled/.build/xrpld` (branch `internal/bug-hunt-plan`, built with `-Dtests=ON`)

2026-05-27 portability update: the public repro kit is self-locating and defaults to `/home/postfiat/repos/rippled` when a local upstream checkout exists. The Python model is portable. The jtx proof requires a local rippled test build with `OpenP0Repro_test.cpp` copied into `src/test/app/`.

2026-05-28 legacy-core update: `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` now has twelve current-live markers. The same cleared/default trustline reserve drift reproduced through offer crossing, transfer-rate offer crossing, `CheckCash`, TokenEscrow finish, NFToken seller proceeds, NFToken broker fees, AMM one-asset withdrawal, and AMMClawback paired-asset return; stronger controls show the drift persists when the receiver already owns two ticket objects, and reserve-boundary controls for offer crossing and `CheckCash` succeed while the receiver remains below the reserve needed for the missing third owner object. The full `OpenP0Repro` suite now reports 70 cases / 16752 tests / 0 failures.

2026-05-27 P0-hunt update: the expanded suite also reproduces nine pre-`fixCleanup3_1_3` historical/replay-era root causes with fixed-path negative controls:

- `PermissionedDomainSet` ticket-sequence collision: two ticket-paid creates from the same account collide on `keylet::permissionedDomain(account, 0)` and the second transaction returns `tefEXCEPTION` / `dirInsert: double insertion`; with `fixCleanup3_1_3` enabled, the same sequence creates two distinct domains keyed by ticket sequence.
- MPT multi-send aggregate `MaximumAmount` bypass: the upstream helper/accounting path `accountSendMulti` allows aggregate sends of 100+100 against `MaximumAmount=150`; with `fixCleanup3_1_3` enabled, the same helper call returns `tecPATH_DRY`.
- `VaultWithdraw` share-denominated trustline-limit bypass: asset-denominated withdraw to a limit-5 trustline fails, but the pre-fix share-denominated equivalent succeeds and increases the destination balance; with `fixCleanup3_1_3` enabled, the share path returns `tecNO_LINE`.
- Vault share-MPT locked-escrow deletion: after share MPT is escrowed and the spendable share balance is withdrawn, pre-fix `VaultWithdraw` deletes the MPToken despite `sfLockedAmount=500`; with `fixCleanup3_1_3` enabled, the MPToken remains with locked amount preserved.
- `VaultClawback` zero-amount unclamped asset clawback: with outstanding loan assets, pre-fix zero-amount clawback attempts to claw back the full share value rather than clamping to `sfAssetsAvailable`, reaches the negative-vault-balance guard, and returns `tefINTERNAL`; with `fixCleanup3_1_3` enabled, the same flow succeeds and clamps the clawback to available assets.
- `LoanPay` fee-cap DoS path: a high-amount `LoanPay` that the handler can process only up to `loanMaximumPaymentsPerTransaction` still required an uncapped pre-fix fee; with `fixCleanup3_1_3` enabled, the same transaction succeeds with the capped fee.
- Invariant bool overwrite: pre-fix invariant checkers used assignment in paths where violations must accumulate, so a later valid entry can clear an earlier bad XRP trustline, bad deep-freeze trustline, or bad MPT issuance observation; with `fixCleanup3_1_3` enabled, all three invariant checks fail closed.
- Expired credential cleanup delete failure: pre-fix credential cleanup could report expired/success to credential consumers after `deleteSLE` failed, leaving the expired credential in state; with `fixCleanup3_1_3` enabled, the same forced-delete-failure paths return `tecINTERNAL`.
- Permissioned-DEX empty `sfAdditionalBooks`: pre-fix hybrid-offer invariants treated missing or multiple `sfAdditionalBooks` as malformed but allowed an empty array; with `fixCleanup3_1_3` enabled, the empty-array hybrid offer fails the malformed-hybrid invariant.

2026-05-27 current-3.1.3 update: the same suite also reproduces thirty additional current `3.1.3` transaction/helper root causes from later upstream fix history and source review, now covering thirty-two current paths beyond the lending-freeze class, one feature-bound MPT lock-state root cause with `MPTokensV1` active and `SingleAssetVault` inactive, plus one current MPT protocol-wire serialization proof:

- `LoanBrokerCover` IOU precision drift: current `3.1.3` can credit broker cover by `2e-14` after the depositor submits `1.8e-14`, and can accept positive cover deposit, withdraw, and clawback amounts that round to zero at the broker cover scale.
- LoanPay minimum-cover scale inconsistency: current `3.1.3` can route the service fee for the same broker-level cover state differently depending on an individual loan's scale; later upstream made the broker-cover minimum use vault scale consistently.
- Vault share MPT transfer-restriction bypass: after the underlying MPT issuer clears `CanTransfer`, current `3.1.3` still lets peer-to-peer vault-share payments succeed; later upstream makes shares inherit the underlying transfer restriction.
- `LoanBrokerDelete` returns locked MPT first-loss cover from the broker pseudo-account to the owner and deletes the locked pseudo-account MPToken.
- Loan payment factor cancellation: current `3.1.3` computes a near-zero-rate `computePaymentFactor` through a direct `(1+r)^n - 1` denominator that diverges from an independent polynomial reference; later upstream switched to a stable power-minus-one calculation.
- `VaultWithdraw` IOU scale-boundary invariant: withdrawing 5 IOU from a vault with assets at the 1e16 scale boundary returns `tecINVARIANT_FAILED` through vault-balance and destination-balance invariant failures.
- `VaultDeposit` issuer IOU edge invariant: issuer deposit at a vault IOU edge returns `tecINVARIANT_FAILED`; later upstream rejects the same precision-loss class proactively.
- Vault sole-shareholder impaired exit: after a second shareholder exits an impaired vault, the sole remaining shareholder cannot withdraw available cash; current `3.1.3` hits the zero-sized-vault invariant instead of leaving residual shares backed by the impaired receivable.
- `VaultDeposit` can pass preclaim by counting the counterparty's opposite trustline limit, then return `tefINTERNAL` after the actual IOU send drives the depositor balance negative.
- Token `EscrowCancel` returns `tefEXCEPTION` when canceling an IOU escrow after the sender's trustline was deleted.
- AMM stale `AuthAccounts` persist after an empty pool is reinitialized with `tfTwoAssetIfEmpty`, leaving the previous auction slot's authorized accounts on the new slot.
- A delegated/authorized account can delete itself while the `Delegate` ledger entry and the delegator's owner reserve remain behind.
- A domain-bound MPT issuance can clear `RequireAuth` while retaining `DomainID`, leaving the issuance domain-bound in metadata but permissionless in authorization state.
- Number upward rounding can store a product below the exact value at the `maxRep` cusp, violating the directed-rounding contract used by consensus accounting helpers.
- Number upward division can store `2 / 1,000,000,000,000,000,007` below the exact quotient, violating the same directed-rounding contract in `Number::operator/=`.
- MPT transfer-rate scaling can overflow on a large valid integral MPT amount, because current `3.1.3` routes the operation through the legacy scaled-mantissa path rather than the later MPT/V2 `Number` arithmetic path.
- A delegated payment can fail even when the delegate can pay the fee because the pre-fix payment path couples the delegate-paid fee to the delegator's reserve requirement.
- A delegate can submit `VaultCreate` for another account; later upstream marked Single Asset Vault and Lending transactions `NotDelegable`.
- A delegated payment signed by the delegator as part of the delegatee's signer list is rejected even though the delegatee is the acting authority and fee payer; later upstream made delegated multisigning check the delegate account instead of the delegator.
- A non-canonical MPT amount can reach the ledger engine and return a fee-burning `tecPATH_PARTIAL` instead of failing preflight as `temBAD_AMOUNT`; later upstream rejects non-canonical MPT amounts before they enter transaction application.
- MPT `STIssue` serializes the sequence bytes in legacy swapped order; the internal codec round-trip hides the defect, while a canonical raw-MPTID payload parses to a different MPTID. Later upstream introduced the amended V2 `STIssue` format.
- A delegate with only `MPTokenIssuanceLock` authority can mutate issuance metadata and transfer fee through `MPTokenIssuanceSet`; current `MPTokenIssuanceSet::checkPermission` inspects lock/unlock flags but not mutation fields.
- A delegate with only unrelated `Payment` authority can submit an empty `AccountSet` for the principal; current `SetAccount::checkPermission` admits the no-field/no-flag transaction, advances the principal sequence, and charges the delegate fee.
- Batch signer outer-account replay: current `3.1.3` signs `BatchSigners` over flags and inner transaction IDs only, so signatures captured from one valid batch can be replayed under a different outer account with the same inner transaction IDs and flags; later upstream commit `7618b726b` binds the signature message to the outer account and sequence.
- `CheckCash` can auto-create an IOU trustline for a receiver even after the issuer has set `asfDisallowIncomingTrustline`; direct `TrustSet` is rejected, but CheckCash settlement succeeds and creates the trustline.
- `EscrowFinish` can auto-create an IOU trustline for a TokenEscrow destination even after the issuer has set `asfDisallowIncomingTrustline`; direct `TrustSet` is rejected, but escrow settlement succeeds and creates the trustline.
- `AMMWithdraw` can return an issuer IOU from an AMM pool to a withdrawing account even after the issuer has set `asfDisallowIncomingTrustline`; direct `TrustSet` is rejected, but AMM withdrawal succeeds and creates the trustline.
- `AMMCreate` can create a new AMM account trustline for an issuer IOU even after the issuer has set `asfDisallowIncomingTrustline`; direct `TrustSet` is rejected, but AMM pool creation succeeds and creates the AMM trustline.
- `AMMDeposit` with `tfTwoAssetIfEmpty` can reinitialize an empty AMM pool and recreate the AMM account trustline for an issuer IOU even after the issuer has set `asfDisallowIncomingTrustline`; direct `TrustSet` is rejected, but empty-pool reinitialization succeeds and creates the AMM trustline.
- MPT locked-holder unauthorize without SAV: with `MPTokensV1` active and `SingleAssetVault` inactive, a holder can `tfMPTUnauthorize` a locked zero-balance MPToken, delete the issuer's lock state, then re-authorize without `lsfMPTLocked`; current `MPTokenAuthorize::preclaim` gates the locked-token deletion check on `featureSingleAssetVault`.
- A partially-crossed permissioned-DEX hybrid offer can leave its open-book directory key at a different quality from `sfExchangeRate`.
- A valid permissioned-DEX `OfferCreate` that cancels the user's regular offer can fail with `tecINVARIANT_FAILED`.

## How to reproduce

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./run_definitive_proof.sh
```

No mainnet wallet. jtx standalone only.

---

## Results summary

| Finding | Proof method | Result | Log |
|---------|--------------|--------|-----|
| **F3.3** | jtx `OpenP0Repro` — regular-freeze-only dest receives cover | **PROVEN** — `tesSUCCESS`, balance +10 IOU | `proof_openp0repro.log` |
| **F3.3 control** | Same suite — deep-freeze dest | **PROVEN blocked** — `tecFROZEN` | same |
| **F3.5** | jtx `OpenP0Repro` — regular-freeze-only broker owner receives delete cover | **PROVEN** — `tesSUCCESS`, balance +10 IOU | same |
| **F3.6** | jtx `OpenP0Repro` — regular-freeze-only broker owner receives LoanPay service fee | **PROVEN** — `tesSUCCESS`, balance +100 IOU | same |
| **F3.7** | jtx `OpenP0Repro` — regular-freeze-only broker owner receives LoanSet origination fee | **PROVEN** — `tesSUCCESS`, balance +100 IOU | same |
| **F3.8** | jtx `OpenP0Repro` — regular-freeze-only vault pseudo receives LoanPay repayment | **PROVEN** — `tesSUCCESS`, balance increases | same |
| **F3.9** | jtx `OpenP0Repro` — regular-freeze-only broker pseudo receives cover deposit | **PROVEN** — `tesSUCCESS`, balance +10 IOU | same |
| **F3.10** | jtx `OpenP0Repro` — regular-freeze-only broker pseudo receives LoanPay fallback fee | **PROVEN** — `tesSUCCESS`, balance +100 IOU | same |
| **F6.1 control** | SetTrust missing issuer, SAV on | **PROVEN safe path** — `tecNO_DST` | same |
| **F6.1 crash claim** | `OpenP0ReproCrash` — AMM/SAV/DisallowIncoming off, missing issuer | **NOT REPRODUCED** — `tecNO_DST`, no crash | same |
| **Freeze logic** | `freeze_check_model.py` | **PROVEN** — regular-only row allows bug path | stdout |
| **PermissionedDomain ticket collision** | jtx `OpenP0Repro` — two pre-fix ticket-paid creates | **PROVEN historical/replay-era exception** — second tx returns `tefEXCEPTION`; post-fix control passes | same |
| **MPT multi-send aggregate limit** | helper/accounting path in `OpenP0Repro` — pre-fix `accountSendMulti` | **PROVEN historical/replay-era helper path** — aggregate 200 exceeds max 150; post-fix control returns `tecPATH_DRY` | same |
| **VaultWithdraw share limit bypass** | jtx `OpenP0Repro` — share-denominated withdraw to low-limit destination | **PROVEN historical/replay-era transaction path** — pre-fix balance increases; post-fix control returns `tecNO_LINE` | same |
| **Vault share-MPT locked escrow deletion** | jtx `OpenP0Repro` — withdraw spendable shares while shares remain escrow locked | **PROVEN historical/replay-era transaction path** — pre-fix MPToken deleted with locked amount; post-fix control preserves token | same |
| **VaultClawback zero-amount unclamped asset clawback** | jtx `OpenP0Repro` — outstanding loan leaves assets available below total assets | **PROVEN historical/replay-era transaction path** — pre-fix returns `tefINTERNAL` through negative-balance guard; post-fix clamps to available assets and succeeds | same |
| **LoanPay uncapped fee DoS** | jtx `OpenP0Repro` — high-amount loan payment with capped fee | **PROVEN historical/replay-era fee path** — pre-fix returns `telINSUF_FEE_P`; post-fix accepts capped fee and processes the maximum payment count | same |
| **Invariant bool overwrite** | direct invariant helper path in `OpenP0Repro` | **PROVEN historical/replay-era invariant path** — pre-fix passes bad XRP trustline, bad deep-freeze trustline, and bad MPT issuance sequences when followed by valid observations; post-fix controls fail closed | same |
| **Expired credential cleanup delete failure** | direct credential-consumer helper path in `OpenP0Repro` | **PROVEN historical/replay-era cleanup path** — pre-fix reports expired/success while the expired credential remains; post-fix returns `tecINTERNAL` | same |
| **Permissioned-DEX empty AdditionalBooks invariant bypass** | direct invariant helper path in `OpenP0Repro` | **PROVEN historical/replay-era invariant path** — pre-fix empty `sfAdditionalBooks` passes as a hybrid offer; post-fix fails malformed-hybrid invariant | same |
| **LoanBrokerCover IOU precision drift** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current paths** — deposit of `1.8e-14` credits cover by `2e-14`; positive zero-at-scale deposit, withdraw, and clawback succeed without changing cover or receiving balance | same |
| **LoanPay minimum-cover scale inconsistency** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — the same broker-level cover state routes service fees differently depending on loan scale | same |
| **Vault share MPT transfer-restriction bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — peer-to-peer vault-share payment succeeds after the underlying issuer clears `CanTransfer` | same |
| **LoanBrokerDelete locked MPT cover** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — locked broker pseudo-account MPT cover is returned and the locked MPToken is deleted | same |
| **Loan payment factor cancellation** | helper/accounting path in `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — near-zero-rate `computePaymentFactor` differs from an independent polynomial reference by more than `1e-12` | same |
| **VaultWithdraw IOU scale-boundary invariant** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — withdraw across IOU scale boundary returns `tecINVARIANT_FAILED` | same |
| **VaultDeposit issuer IOU edge invariant** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — issuer deposit at IOU edge returns `tecINVARIANT_FAILED` | same |
| **Vault sole-shareholder impaired exit** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — sole remaining shareholder in impaired vault hits zero-sized-vault invariant | same |
| **VaultDeposit opposite-limit internal failure** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — transaction returns `tefINTERNAL` after negative depositor asset balance guard | same |
| **EscrowCancel deleted IOU trustline exception** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — cancel returns `tefEXCEPTION` after sender trustline deletion | same |
| **AMM stale AuthAccounts after reinit** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — empty-pool reinitialization leaves stale `sfAuthAccounts` from the prior auction slot | same |
| **Delegatee account-delete stale delegation** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — delegatee account is deleted while the `Delegate` ledger entry and delegator reserve remain | same |
| **Domain-bound MPT `RequireAuth` clearing** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — `RequireAuth` is cleared while `DomainID` remains present on the issuance | same |
| **Number upward-rounding cusp violation** | helper/accounting path in `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — under upward rounding, a large `Number` product stores below the exact product at the `maxRep` cusp | same |
| **Number upward-division rounding violation** | helper/accounting path in `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — under upward rounding, `2 / 1,000,000,000,000,000,007` stores below the exact quotient | same |
| **MPT transfer-rate overflow** | helper/accounting path in `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — applying a transfer rate to a large integral MPT amount throws `overflow_error` in the legacy scaled-mantissa path | same |
| **Delegated-payment fee/reserve coupling** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — payment returns `tecUNFUNDED_PAYMENT` even though the delegate pays the high fee | same |
| **SAV transaction delegation** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — `VaultCreate` succeeds under `delegate::as(bob)` | same |
| **Delegated multisign self-check rejection** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — delegated payment with the delegator on the delegatee's signer list is rejected by RPC before ledger application | same |
| **MPT non-canonical amount ledger acceptance** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — malformed MPT amount reaches transaction application and returns fee-burning `tecPATH_PARTIAL` instead of `temBAD_AMOUNT` | same |
| **MPT STIssue legacy wire order** | protocol-wire path in `OpenP0Repro` on current `3.1.3` | **PROVEN current codec defect** — canonical sequence `de ad be ef` serializes as `ef be ad de`; canonical raw payload parses to a different MPTID | same |
| **Delegated MPT issuance metadata/fee mutation** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — a delegate with only `MPTokenIssuanceLock` mutates `sfMPTokenMetadata` and `sfTransferFee` | same |
| **Delegated empty AccountSet sequence consumption** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — a delegate with only unrelated `Payment` authority submits empty `AccountSet`, advances the principal sequence, and pays the fee | same |
| **Batch signer outer-account replay** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — captured `BatchSigners` from one valid batch authorize the same inner transaction IDs under a different outer account | same |
| **MPT locked-holder unauthorize without SAV** | jtx `OpenP0Repro` on current `3.1.3` with `MPTokensV1` active and `SingleAssetVault` inactive | **PROVEN feature-bound current path** — holder deletes a locked zero-balance MPToken with `tfMPTUnauthorize`, erasing the issuer lock state, then re-authorizes without `lsfMPTLocked` | same |
| **Trustline positive balance without reserve** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — offer crossing creates a positive IOU balance while receiver `OwnerCount` and reserve flag remain unset | same |
| **OfferCreate DisallowIncomingTrustline bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — direct `TrustSet` is rejected, but offer crossing creates the issuer IOU trustline | same |
| **NFTokenAcceptOffer DisallowIncomingTrustline bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — direct `TrustSet` is rejected, but NFT sell-offer IOU settlement creates the seller trustline | same |
| **CheckCash DisallowIncomingTrustline bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — direct `TrustSet` is rejected, but CheckCash auto-creates the receiver trustline | same |
| **TokenEscrow finish DisallowIncomingTrustline bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — direct `TrustSet` is rejected, but EscrowFinish auto-creates the destination trustline | same |
| **AMMWithdraw DisallowIncomingTrustline bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — direct `TrustSet` is rejected, but AMMWithdraw returns pool IOU assets and creates the receiver trustline | same |
| **AMMCreate DisallowIncomingTrustline bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — direct `TrustSet` is rejected, but AMMCreate creates the pool and AMM account issuer trustline | same |
| **AMMDeposit empty-pool DisallowIncomingTrustline bypass** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — direct `TrustSet` is rejected, but `tfTwoAssetIfEmpty` reinitialization creates the AMM account issuer trustline | same |
| **Permissioned DEX hybrid-offer quality mismatch** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — open-book directory key quality differs from `sfExchangeRate` after partial crossing | same |
| **Permissioned DEX regular-offer cancel invariant** | jtx `OpenP0Repro` on current `3.1.3` | **PROVEN current path** — valid domain `OfferCreate` with `OfferSequence` returns `tecINVARIANT_FAILED` | same |
| **DID dir-full partial mutation** | jtx `OpenP0Repro` — forced `tecDIR_FULL` | **NOT REPRODUCED** — no DID object remains | same |
| **Batch/Credential rollback leak** | jtx `OpenP0Repro` — expired `CredentialAccept` in `tfAllOrNothing` | **NOT REPRODUCED** — credential remains and payment rolls back | same |
| **F4.6 / B3-1** | jtx vault repro (draft) | **NOT COUNTED AS REPRODUCED** — `tecLOCKED` on draft setup | — |

---

## Lending freeze excerpt (definitive)

```
ripple.tx.OpenP0Repro F3.3 LoanBrokerCoverWithdraw — regular-freeze-only destination (P0)
ripple.tx.OpenP0Repro F3.3 control — deep-freeze blocks cover withdraw
ripple.tx.OpenP0Repro F3.5 LoanBrokerDelete — regular-freeze-only owner receives cover
ripple.tx.OpenP0Repro F3.9 LoanBrokerCoverDeposit — regular-freeze-only broker pseudo receives cover
ripple.tx.OpenP0Repro F3.7 LoanSet — regular-freeze-only broker owner receives origination fee
ripple.tx.OpenP0Repro F3.6 LoanPay — regular-freeze-only broker owner receives service fee
ripple.tx.OpenP0Repro F3.8 LoanPay — regular-freeze-only vault pseudo receives repayment
ripple.tx.OpenP0Repro F3.10 LoanPay — regular-freeze-only broker pseudo receives fallback fee
ripple.tx.OpenP0Repro F6.1 SetTrust missing issuer — tecNO_DST when SAV enabled (control)
ripple.tx.OpenP0Repro DIDSet tecDIR_FULL does not leave partial DID object
ripple.tx.OpenP0Repro Batch all-or-nothing rolls back expired CredentialAccept deletion
ripple.tx.OpenP0Repro PermissionedDomainSet pre-fix ticket sequence collision candidate
FTL:OpenLedger apply: dirInsert: double insertion
ripple.tx.OpenP0Repro Credentials pre-fix — expired cleanup ignores deleteSLE failure
ripple.tx.OpenP0Repro Permissioned DEX pre-fix — empty AdditionalBooks hides malformed hybrid offer
FTL:OpenP0Repro Invariant failed: hybrid offer is malformed
ripple.tx.OpenP0Repro MPT multi-send pre-fix aggregate MaximumAmount bypass
ripple.tx.OpenP0Repro VaultWithdraw pre-fix share-denominated trustline limit bypass
ripple.tx.OpenP0Repro VaultWithdraw pre-fix deletes share MPToken with locked escrow
ripple.tx.OpenP0Repro VaultClawback pre-fix zero-amount asset clawback is not clamped to assets available
ERR:OpenLedger FEDF49CF... VaultClawback: negative balance of vault assets.
ripple.tx.OpenP0Repro LoanPay pre-fix high-amount payment requires uncapped fee
ripple.tx.OpenP0Repro LoanBrokerCover current — IOU precision drift
ripple.tx.OpenP0Repro Vault share MPT current — underlying CanTransfer is not inherited
ripple.tx.OpenP0Repro LoanPay current — broker minimum cover scale inconsistency
ripple.tx.OpenP0Repro LoanBrokerDelete current — locked MPT cover is returned
ripple.tx.OpenP0Repro VaultDeposit current — opposite trustline limit causes tefINTERNAL
ERR:OpenLedger C3963ACE... VaultDeposit: negative balance of account assets.
ripple.tx.OpenP0Repro EscrowCancel current — deleted IOU trustline returns tefEXCEPTION
FTL:OpenLedger apply: Template field error 'OwnerCount'
ripple.tx.OpenP0Repro AMM current — stale AuthAccounts survive empty reinit
ripple.tx.OpenP0Repro Delegate current — delegatee account deletion leaves stale delegation
ripple.tx.OpenP0Repro MPT current — domain-bound RequireAuth can be cleared
ripple.tx.OpenP0Repro Number current — upward rounding falls below exact at maxRep cusp
ripple.tx.OpenP0Repro Number current - upward division falls below exact quotient
ripple.tx.OpenP0Repro MPT current — transfer-rate scaling overflows large integral amount
ripple.tx.OpenP0Repro Delegate current — delegated fee is coupled to delegator reserve
ripple.tx.OpenP0Repro Delegate current — SAV transaction can be delegated
ripple.tx.OpenP0Repro Permissioned DEX current — hybrid offer open-book quality mismatch
ripple.tx.OpenP0Repro Permissioned DEX current — cancel regular offer via domain offer invariant
ripple.tx.OpenP0Repro Loan current — near-zero payment factor cancellation
ripple.tx.OpenP0Repro Vault current — withdraw across IOU scale boundary invariant
ripple.tx.OpenP0Repro Vault current — issuer deposit at IOU edge invariant
ripple.tx.OpenP0Repro Vault current — sole shareholder impaired exit is stuck
ripple.tx.OpenP0Repro Delegate current — delegator multisign rejected for delegate
ripple.tx.OpenP0Repro MPT current — non-canonical amount reaches ledger engine
ripple.tx.OpenP0Repro MPT current — STIssue sequence bytes are legacy-swapped
ripple.tx.OpenP0Repro MPT current — locked holder can delete lock state without SAV
ripple.tx.OpenP0Repro TrustLine current — offer crossing creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — offer crossing leaves positive balance unowned with existing owner objects
ripple.tx.OpenP0Repro TrustLine current — offer crossing succeeds below missing owner reserve
ripple.tx.OpenP0Repro TrustLine current — offer crossing with transfer rate creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — CheckCash creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — CheckCash leaves positive balance unowned with existing owner objects
ripple.tx.OpenP0Repro TrustLine current — CheckCash succeeds below missing owner reserve
ripple.tx.OpenP0Repro TrustLine current — TokenEscrow creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — NFToken AcceptOffer creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — NFToken broker fee creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — AMMWithdraw creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — AMMClawback creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — OfferCreate bypasses DisallowIncomingTrustline
ripple.tx.OpenP0Repro NFToken current — AcceptOffer bypasses DisallowIncomingTrustline
ripple.tx.OpenP0Repro CheckCash current — bypasses DisallowIncomingTrustline
ripple.tx.OpenP0Repro TokenEscrow current — Finish bypasses DisallowIncomingTrustline
ripple.tx.OpenP0Repro Delegate current — MPT granular lock permission mutates issuance fields
ripple.tx.OpenP0Repro Delegate current — empty AccountSet with unrelated permission consumes principal sequence
ripple.tx.OpenP0Repro Batch current — batch signer signatures replay across outer account
ripple.tx.OpenP0Repro Invariant pre-fix — later good entries hide earlier bad entries
ripple.tx.OpenP0Repro had 0 failures.
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Test source: `src/test/app/OpenP0Repro_test.cpp`

Steps proven on ledger:
1. Issuer applies regular freeze only (`tfSetFreeze`, no `tfSetDeepFreeze`) to the relevant receiver.
2. The affected lending transaction returns **`tesSUCCESS`**.
3. The regular-frozen receiver's IOU balance increases.
4. Adding `tfSetDeepFreeze` blocks the F3.3 control path with **`tecFROZEN`**.

---

## F6.1 excerpt (negative control)

```
.build/rippled --unittest OpenP0ReproCrash
ripple.tx.OpenP0ReproCrash F6.1 negative control — missing issuer returns tecNO_DST
ripple.tx.OpenP0ReproCrash had 0 failures.
```

Clean upstream `3.1.3` did not reproduce the claimed SetTrust null deref. The public packet treats F6.1 as demoted unless a narrower clean-upstream crash reproducer is supplied.

---

## Build notes

```bash
cd rippled && mkdir -p .build && cd .build
conan remote add --index 0 xrplf https://conan.ripplex.io  # once
conan install .. --output-folder . --build missing --settings build_type=Release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake \
  -Dxrpld=ON -Dtests=ON ..
cmake --build . -j$(nproc)
./xrpld --unittest OpenP0Repro
./xrpld --unittest OpenP0ReproCrash   # negative control: expect tecNO_DST and no crash
```

---

## What this does not prove on jtx

- Vault F4.6 / B3-1 — draft jtx hit `tecLOCKED`; needs setup matching pseudo-account freeze semantics.
- Invariant gaps F2.1 / F3.1 — require second bug to show fund loss; code `// TBD` is definitive for F2.1.

Code audit + expanded jtx + F6.1 negative controls + logic model = **definitive for seven regular-freeze lending receive paths and for demoting the SetTrust crash claim in this public packet**.

The 2026-05-27 hunt adds twenty-seven current `3.1.3` transaction/helper root-cause repros, one feature-bound MPT lock-state repro, one current protocol-wire serialization proof, nine historical/replay-era root-cause repros, and two demoted false positives. The current root causes now cover thirty current paths beyond the lending-freeze class because the `LoanBrokerCover` precision root includes deposit, withdraw, and clawback sub-ULP paths, plus a separate `Number::operator/=` upward-division proof, a delegated MPT issuance-mutation proof, a delegated empty-`AccountSet` sequence-consumption proof, a batch signer outer-account replay proof, a CheckCash auto-trustline bypass of `DisallowIncomingTrustline`, a TokenEscrow finish auto-trustline bypass of `DisallowIncomingTrustline`, and a transfer-rate offer-crossing reserve-drift proof. The feature-bound MPT locked-holder proof requires `MPTokensV1` active and `SingleAssetVault` inactive. The MPT `STIssue` wire-order proof is counted separately from transaction-path P0s. The MPT aggregate `MaximumAmount`, invariant-overwrite, expired-credential cleanup, permissioned-DEX empty-`AdditionalBooks`, loan payment-factor cancellation, Number upward-rounding cusp, Number upward-division, and MPT transfer-rate overflow cases are helper/accounting, cleanup-consumer, or invariant-path repros following upstream regression style rather than standalone transaction-path repros in this suite. Follow-up artifacts:

- `runs/20260527-p0-hunt/candidate_matrix.md`
- `runs/20260527-p0-hunt/repro_results.md`
- `runs/20260527-p0-hunt/definitive_proof_number_division_20260527.log`
  SHA-256: `ff1d4f33654c2bc5b58eeba462d9ad04d18052a6fa386ab6e3b9b35b84cd6dba`
- `runs/20260527-p0-hunt/definitive_proof_mpt_locked_holder_unauthorize_20260527.log`
  SHA-256: `351bf51a04fcfbdec24bb1cce617c99f63045eb47918d7b34b215678b86c1e7a`
- `runs/20260527-p0-hunt/definitive_proof_delegate_mpt_granular_20260527.log`
  SHA-256: `288bf0d74b7a6d6d108e3e8f230c32508e72450efafa712b614b6ba62e58cc75`
- `runs/20260527-p0-hunt/definitive_proof_delegate_empty_accountset_20260527.log`
  SHA-256: `2b5b083e3c44d822b04bbb43210a726bff3840e9c87b64b3398b963aa30d918a`
- `runs/20260527-p0-hunt/definitive_proof_batch_signer_outer_replay_20260527.log`
  SHA-256: `a2d3a2f36ae8e2615bb002bef2c25eb047e0c7da8c029a7a1f1f2207ed24ff7c`
