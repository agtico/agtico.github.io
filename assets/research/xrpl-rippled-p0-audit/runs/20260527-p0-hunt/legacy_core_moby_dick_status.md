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

Direct live receipts were refreshed again during the current continuation:

```text
runtime_checked_utc: 2026-05-28T08:25:40Z
amendment_checked_utc: 2026-05-28T08:25:42Z
receipt files: direct_xrpl_mainnet_runtime_status_20260527.json, direct_xrpl_amendment_status_20260527.json
s1.ripple.com: rippled 3.1.3, ledger 104533710, hash 89E0D12AD48B9A5D976891706D3AF18FF0CAA87B2B4B5581E41AF8B28D6CBFB4
s2.ripple.com: rippled 3.1.3, ledger 104533710, hash 89E0D12AD48B9A5D976891706D3AF18FF0CAA87B2B4B5581E41AF8B28D6CBFB4
runtime receipt sha256: a0afde08c4e1e27984c178dfab5b9ff88c61e7b5394d0b9d6e0012be7efa0a19
amendment receipt sha256: c65b7f42d464fa6f003e118d0650f36eebaec74ce245f1254b869525c63d7241
fixCleanup3_1_3: enabled by raw Amendments hash 303ACB16CF8DBD3B5C34F131A9D19A7DE01AE05F480A8A682B869D1B4AAC8CFC
```

## Current Result

The strongest legacy-core candidate is
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`.

## Legacy-Core Queue Status

This continuation did not promote a new P0. It completed the planned
legacy-core queue behind the Moby Dick lane and left the packet classification
unchanged.

Completed and packet-bound work:

- current `3.1.3` repro for the trustline positive-balance reserve drift;
- old-tag repros for the same root on `2.5.0`, `2.0.0`, and `1.5.0`;
- trustline/offer reserve sibling probes;
- old object lifecycle and directory-state probes;
- raw sequence and ticket key-construction sweep;
- auth/freeze receive-path sibling sweep;
- deterministic exception and arithmetic sweep;
- source-signal clustering around reserve, owner-count, trustline, directory,
  and result-code fixes.
- regular-key/master-key authorization and signer cleanup sweep.
- payment sandbox deferred-credit focused pathing sweep.
- transaction-queue minimum-reserve potential-spend sweep.
- NFT brokered auth/freeze receive-path sweep.
- AMMDeposit LP-token reserve sibling scratch probe.
- AccountDelete positive-unowned-trustline lifecycle scratch probe.
- TrustSet legacy reserve-carveout source and suite sweep.
- legacy amount/quality arithmetic continuation sweep.
- AccountSet legacy flag and policy-setting sweep.
- legacy transaction envelope/signing/sequence source and suite sweep.
- legacy IOU zero-cross settlement source and suite sweep.

Current conclusion: `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` remains the best
old, simple, live, unfixed Moby Dick candidate. The later source-kill phases
did not isolate another clean legacy-core P0. Broader live-feature hunting can
continue under the whip's Step 6, but the legacy-core queue should not be
inflated with source-killed or speculative candidates.

It is a baseline IOU trustline and settlement accounting issue, not a new
disabled feature surface. The current repro markers are:

```text
TrustLine current - offer crossing creates positive balance without reserve
TrustLine current - offer crossing leaves positive balance unowned with existing owner objects
TrustLine current - offer crossing succeeds below missing owner reserve
TrustLine current - offer crossing with transfer rate creates positive balance without reserve
TrustLine current - CheckCash creates positive balance without reserve
TrustLine current - CheckCash leaves positive balance unowned with existing owner objects
TrustLine current - CheckCash succeeds below missing owner reserve
TrustLine current - TokenEscrow creates positive balance without reserve
TrustLine current - NFToken AcceptOffer creates positive balance without reserve
TrustLine current - NFToken broker fee creates positive balance without reserve
TrustLine current - AMMWithdraw creates positive balance without reserve
TrustLine current - AMMClawback creates positive balance without reserve
```

The packet wrapper reproduced the marker:

```bash
assets/research/xrpl-rippled-p0-audit/repros/TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.sh
```

Observed wrapper result:

```text
local wrapper log sha256: 2c0e58bd68648beac5ec05f82a7aea2b50a0114257a029f68b6128b189bb0a42
targeted finding TRUSTLINE-POSITIVE-BALANCE-RESERVE-001 reproduced by marker assertion.
ripple.tx.OpenP0Repro had 0 failures.
17.2s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

The full packet verifier also passed:

```text
packet-ok
records=19 markers=30 proof_sha256=dcbef70e76197ef923edaba32c9594a88f3ceb6a9c60c47587e3c4bc28772362
```

The manifest-only wrapper loop also passed for all 19 included IDs after the
shared runner footer was updated to the 70-case suite. A broader `repros/*.sh`
loop was intentionally not used as evidence because the directory still
contains non-manifest disabled-surface wrappers.

## Current sibling: transfer-rate offer crossing reserve drift

This continuation promoted one more marker under
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`.

Minimal behavior:

1. Gateway sets a non-default transfer rate.
2. Alice clears the gateway USD trustline back to zero balance, zero limit,
   `OwnerCount=0`, and no receiver reserve flag.
3. A market account posts a USD/XRP offer.
4. Alice crosses the offer and receives a positive gateway USD balance through
   the transfer-rate offer path.
5. Alice's trustline remains in the no-reserve state and `OwnerCount` remains
   `0`.

Result excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — offer crossing with transfer rate creates positive balance without reserve
17.3s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: the missing receiver-side owner-count/reserve transition is not
limited to the simplest par offer-crossing case. It survives an issuer
transfer-rate crossing, which is important because the affected path is still
normal IOU market settlement rather than an exotic disabled feature.

## Current source-killed siblings

This continuation tested two legacy-core hypotheses without promotion.

`PAYMENT-TRUSTLINE-RESERVE-SIBLING-001` asked whether direct issuer `Payment`
or XRP-to-IOU path `Payment` can recreate the same positive-balance/no-reserve
state after a holder clears its trustline. Both scratch probes returned
`tecPATH_DRY` in the cleared-trustline setup and did not create the broken
state. That narrows the current reserve finding to offer crossing, CheckCash,
TokenEscrow, NFToken settlement, and AMM one-asset withdrawal rather than every
`Payment` path.

`CHECK-LEGACY-DIRFULL-PARTIAL-001` asked whether `CreateCheck` leaves a partial
object or owner-directory entry when the source owner directory fails after the
destination-directory insertion point. A forced source-dir-full scratch probe
returned `tecDIR_FULL`, left no Check object, left source and destination owner
counts unchanged, and left no check entry in either owner directory.

`PAYCHAN-DIRFULL-PARTIAL-001` asked whether `PaymentChannelCreate` leaves a
partial source-side channel object when the destination owner-directory insert
fails under `fixPayChanRecipientOwnerDir`. A forced destination-dir-full scratch
probe returned `tecDIR_FULL`, left no PayChannel object, and left the source
`OwnerCount` unchanged. Scratch artifact
`paychan_dest_dirfull_source_kill_20260528.log` has sha256
`ff0ac270d50fc9424eed8d077be0b91e08d7f771ec4cbf1aed37f7425dd3f8cc`.

`PAYCHAN-LEGACY-CLOSE-OWNERDIR-001` asked whether close, claim, fund,
account-delete, or old/new recipient-owner-dir migration around
`fixPayChanRecipientOwnerDir` can strand a channel or leak owner-directory /
obligation state. Existing upstream `PayChan` and `AccountDelete` coverage
passed across old-style channels, new-style recipient backlinks, mixed
activation, destination deletion/resurrection, claim/fund after deletion,
close/refund cleanup, metadata ownership, tickets, DepositAuth, credentials,
and account-delete obligation checks. Source-kill artifact
`paychan_accountdelete_source_kill_20260528.log` has sha256
`a26cd88fb8f0746c44708dd951b2132d5f907ab6fd7f911bd4d3113c2368985f`.

```text
ripple.app.AccountDelete had 0 failures.
ripple.app.PayChan had 0 failures.
22.0s, 2 suites, 64 cases, 11535 tests total, 0 failures
```

`OFFER-LEGACY-RESERVE-CROSSED-001` asked whether the old offer-reserve rule can
leave an invalid remainder offer or owner/reserve drift when an under-reserved
offer partially crosses. The scratch control at the same reserve boundary
returned `tecINSUF_RESERVE_OFFER` when nothing crossed. The partial-cross path
returned `tesSUCCESS`, moved the crossed 50 XRP/USD leg, placed no Alice
remainder offer, removed the market offer, and left `OwnerCount=1`. That matches
the explicit `CreateOffer::applyGuts` behavior: crossed value can stand, but an
under-reserved remainder is not placed. Source-kill artifact
`offer_partial_cross_underreserve_source_kill_20260528.log` has sha256
`9cc82378a5e67ba5c59db7f4c34a1b4ab23df11b628b3ee3bae2e24b4b4b7db6`.

```text
ripple.tx.OpenP0Repro SCRATCH OfferCreate partial cross under reserve cancels remainder
16.0s, 1 suite, 71 cases, 16828 tests total, 0 failures
```

`ESCROW-LEGACY-XRP-DELETE-EDGE-001` asked whether legacy XRP Escrow finish,
cancel, tickets, owner-directory cleanup, or account-delete obligation checks
can strand obligations or corrupt owner-count/directory state. Existing
upstream `Escrow` and `AccountDelete` coverage passed across enablement,
finish/cancel timing, tags, DisallowXRP, implied finish time, failure cases,
lockup owner-count assertions, crypto-conditions, metadata, consequences,
tickets, credentials, and account-delete obligation checks. Source-kill artifact
`escrow_accountdelete_source_kill_20260528.log` has sha256
`b38d0438aa3af6bc9d7c20a9ffe789923fbd5537c94f5c26628555c51c68b05a`.

```text
ripple.app.AccountDelete had 0 failures.
ripple.app.Escrow had 0 failures.
19.4s, 2 suites, 53 cases, 11568 tests total, 0 failures
```

`RAW-SEQUENCE-SWEEP-001` asked whether another live core object-creation path
still uses raw `sfSequence` rather than the ticket-aware sequence proxy. The
focused scan found the historical `PermissionedDomainSet` pre-fix branch, but
the checked old live create paths use `getSeqValue()` or non-create sequence
semantics. Existing `Ticket`, `Escrow`, `PayChan`, `Check`, and `Offer` suites
then passed. Static artifact `ticket_sequence_static_sweep_20260528.log` has
sha256 `8e7ec705187f93ea45e48957296df5953828afa8728b9b761c80ed272fcee8bd`.
Suite artifact `ticket_sequence_source_kill_20260528.log` has sha256
`55022461bafc5c2865833de278b06140a608614dfc6adaf3b3463c07b8fc6413`.

```text
ripple.app.Check had 0 failures.
ripple.app.Escrow had 0 failures.
ripple.app.PayChan had 0 failures.
ripple.app.Ticket had 0 failures.
94.1s, 11 suites, 489 cases, 63983 tests total, 0 failures
```

`AUTH-FREEZE-RECEIVE-SWEEP-001` asked whether old live IOU receive paths hide a
stronger `RequireAuth`, local-freeze, global-freeze, deep-freeze, or DepositAuth
sibling beyond the already-promoted DisallowIncomingTrustline, AMM DepositAuth,
and lending regular-freeze surfaces. The static sweep covered
`checkAcceptAsset`, `requireAuth`, freeze/deep-freeze helpers, `trustCreate`,
`trustDelete`, `rippleCredit`, `accountSend`, `issueIOU`, and
`verifyDepositPreauth`. No new old-core receive-policy witness was isolated.
Existing `Freeze`, `SetTrust`, `Flow`, `Offer`, `Check`, `Escrow`,
`EscrowToken`, `TrustAndBalance`, and `PayStrand` suites then passed. Static
artifact `auth_freeze_receive_static_sweep_20260528.log` has sha256
`3ce4e848635f707855f468ee7a94bd87f24295a0be2088ed4793c3961aaf30fc`. Suite
artifact `auth_freeze_receive_source_kill_20260528.log` has sha256
`79c5255082209f8b2ae90d13738bf3b21368dd0a4c8e0c7e400e802804f4d584`.

```text
ripple.app.Freeze had 0 failures.
ripple.app.SetTrust had 0 failures.
ripple.app.TrustAndBalance had 0 failures.
144.1s, 15 suites, 716 cases, 96462 tests total, 0 failures
```

`DETERMINISTIC-EXCEPTION-ARITHMETIC-SWEEP-001` asked whether old live
IOU/path/offer arithmetic hides another normal-input `tefINTERNAL`,
`tefEXCEPTION`, overflow, assertion, or invariant-failure witness beyond the
current packet. The static sweep covered exception, invariant, overflow, and
rounding call sites across transaction, path, misc, protocol, and basics code.
The history sweep rediscovered already-promoted or already-demoted signals:
Number upward rounding and division, MPT transfer-rate overflow, invariant
bool-overwrite, LoanBrokerCover precision, vault invariant edges, and
permissioned-DEX invariant cases. No new old-core transaction witness was
isolated. Static artifact
`deterministic_exception_arithmetic_static_sweep_20260528.log` has sha256
`16207ac6ee3de1c5c82c6205351d28d00c526ea042db2daf83bced518640ae48`. History
artifact `deterministic_exception_arithmetic_history_sweep_20260528.log` has
sha256 `1341fc2cbd5d3c8c308b794a7814e8850c24096c7dca7aa73990cc8ea8cc4e67`.
Suite artifact `deterministic_exception_arithmetic_source_kill_20260528.log`
has sha256 `d6385bd19f70f12a50445601d0095c2821c691505f21f1bcc79d63282b45b284`.

```text
ripple.basics.Number had 0 failures.
ripple.protocol.STAmount had 0 failures.
ripple.protocol.Quality had 0 failures.
114.9s, 18 suites, 626 cases, 175219 tests total, 0 failures
```

This continuation ran a narrower legacy amount/quality pass across STAmount,
Number, Quality, Offer, Flow, Path, PayStrand, and TrustAndBalance after
refreshing direct live receipts. The pass again rediscovered the existing
Number, MPT transfer-rate, permissioned-DEX quality, and trustline reserve
signals but did not isolate a new old-core transaction witness. Static artifact
`legacy_amount_quality_static_sweep_20260528.log` has sha256
`e9acb64c13dfeea4adbc6a8cc9e99d60158054fd2852bc931a88a88b368b9802`.
History artifact `legacy_amount_quality_history_sweep_20260528.log` has sha256
`6e7024e72514d9edadc523bec130edbde1322e557877c42e2c64e4222b2e7542`.
Suite artifact `legacy_amount_quality_source_kill_20260528.log` has sha256
`e1d693d42ac1db37f178d41b0c07a0f25de3f7835dcf2cd4af2fcedbe69c037d`.

```text
ripple.app.Flow had 0 failures.
ripple.app.OfferBaseUtil had 0 failures.
ripple.app.PayStrand had 0 failures.
ripple.app.TrustAndBalance had 0 failures.
ripple.basics.Number had 0 failures.
ripple.protocol.Quality had 0 failures.
ripple.protocol.STAmount had 0 failures.
89.9s, 14 suites, 540 cases, 166840 tests total, 0 failures
```

`SOURCE-SIGNAL-CORE-ACCOUNTING-001` closed the planned source-signal clustering
phase around reserve, owner-count, trustline, directory, and result-code
history. The sweep covered `rippleCreditIOU`, `accountSend`, `trustCreate`,
`trustDelete`, `adjustOwnerCount`, owner-directory helpers, and related history.
The actionable signals mapped to existing packet or triage entries:
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`, `DELEGATE-FEE-RESERVE-001`,
`TXQ-MIN-RESERVE-POTENTIAL-SPEND-001`,
`PAYMENT-SANDBOX-DEFERRED-CREDITS-001`, `VAULT-WITHDRAW` reserve branches,
future AMMWithdraw double-owner-count, and directory-limit history. No new
old-core transaction witness was isolated. Static artifact
`source_signal_core_accounting_static_sweep_20260528.log` has sha256
`8965a760cec40a403aba0173e36a60c3a98a8eaa0d844cbee7d1e80f850f2b21`. History
artifacts `source_signal_core_accounting_history_grep_20260528.log` and
`source_signal_core_accounting_history_pickaxe_20260528.log` have sha256
`b13b62e85e5c3ee2591967a132cb5b35fd36cd20b53e60222dd03011e0c22b1d` and
`f31d5ece55a6498dd680af5a2aaac7dab80bfa971f1b5be4dcfc8cf5d9a2893d`.
Suite artifact `source_signal_core_accounting_source_kill_20260528.log` has
sha256 `d22eee8cc2697c8d6446324ee442c6ebffeac798667ec011018880b048c69f71`.

```text
ripple.app.SetTrust had 0 failures.
ripple.app.TrustAndBalance had 0 failures.
ripple.ledger.PaymentSandbox had 0 failures.
157.9s, 19 suites, 724 cases, 102253 tests total, 0 failures
```

`AMM-PSEUDO-ACCOUNT-RESERVE-SIBLING-001` asked whether `AMMCreate` or
empty-pool `AMMDeposit` creates a trustline/owner-count reserve drift analogous
to the user-account reserve bug when IOU assets are sent into an AMM
pseudo-account. The source review found that this is deliberately special AMM
pool state, not a normal receiver trustline: `createPseudoAccount` marks the
account with `sfAMMID`, `AMMCreate::sendAndTrustSet` marks pool trustlines with
`lsfAMMNode`, `deleteAMMTrustLine` has AMM-specific deletion and owner-count
logic, and invariant checks treat `sfAMMID` account roots and `lsfAMMNode`
trustlines as AMM pool mutations. Existing upstream AMM coverage then passed.
Static artifact `amm_pseudo_account_reserve_static_sweep_20260528.log` has
sha256 `3648f82757e66b392a8eaa6ad06f85c12150da222f7a3bc4eac5eced37a28a25`.
Suite artifact `amm_pseudo_account_reserve_source_kill_20260528.log` has
sha256 `bcf4b6972a5181954c73f77eb416430f662252e0333bc770c5b424f2c008fadc`.

```text
ripple.app.AMM had 0 failures.
90.5s, 1 suite, 90 cases, 77270 tests total, 0 failures
```

`AMMDEPOSIT-LPTOKEN-RESERVE-SIBLING-001` asked whether an AMM depositor can
receive newly minted LP-token balance without carrying the missing owner reserve
for the LP-token trustline. This is the direct AMMDeposit sibling of the
positive-balance/no-reserve trustline bug. The source review found a specific
fail-closed guard in `AMMDeposit::preclaim`: when `ammLPHolds(...)` is zero,
the transaction must have liquid XRP for one additional owner reserve via
`xrpLiquid(ctx.view, accountID, 1)`, otherwise it returns
`tecINSUF_RESERVE_LINE`. A rebuilt scratch transaction then funded Alice at the
base reserve plus fees, left her with no LP-token line and `OwnerCount=0`, and
attempted a one-sided XRP AMM deposit. The transaction returned
`tecINSUF_RESERVE_LINE`, created no LP-token trustline, and left
`OwnerCount=0`. Static artifact
`ammdeposit_lptoken_reserve_static_sweep_20260528.log` has sha256
`bc16b8ce452a4afcc9885f1e1203294e329d0571e4bf609de8bb81ae386ab5e1`.
History artifact `ammdeposit_lptoken_reserve_history_sweep_20260528.log` has
sha256 `e2968b5f04104baee12e24f54bd865e76f37c60c0723edf56af4a6b3fe3e1426`.
Scratch source-kill artifact
`ammdeposit_lptoken_reserve_source_kill_20260528.log` has sha256
`97e3a0f0549c56d8bb47050417734d96e9a757e6fa9c97f20eed43eb1b0f289a`.

```text
ripple.tx.OpenP0Repro SCRATCH AMMDeposit LP token reserve sibling fails closed
ripple.tx.OpenP0Repro had 0 failures.
16.0s, 1 suite, 71 cases, 16795 tests total, 0 failures
```

`SIGNERLIST-LEGACY-OWNERCOUNT-SWEEP-001` asked whether the old signer-list
reserve model, the `featureMultiSignReserve` transition, signer-list
replacement/deletion, or `AccountDelete` cleanup can leave owner-count,
reserve, auth, or invariant drift. The source review covered
`SetSignerList::signerCountBasedOwnerCountDelta`, `lsfOneOwnerCount`,
`SignerEntries`, `SetSignerList::removeFromLedger`, multi-sign auth, and
`DeleteAccount` cleanup. The key risk shape was old-style signer-list reserve
accounting crossing into modern one-owner-count semantics. Existing upstream
coverage passed across pre/post amendment signer-list objects, signer-list
flags, amendment transition behavior, multi-sign auth failures, signer tags,
delegated multi-sign, Oracle multi-sign, AccountDelete cleanup, and invariant
owner-count checks. Static artifact
`signerlist_ownercount_static_sweep_20260528.log` has sha256
`30e4f26dcdf0ccf531d4bd54182eb40ad3c07922d2bfbdafe015ffe7451478f7`. Suite
artifact `signerlist_ownercount_source_kill_20260528.log` has sha256
`5837fdd6e0a82511719a4e38e13aaf28f634a085ae23e2f8400f447cea984ad0`.

```text
ripple.app.AccountDelete had 0 failures.
ripple.app.Delegate had 0 failures.
ripple.app.Invariants had 0 failures.
ripple.app.MultiSign had 0 failures.
ripple.app.Oracle had 0 failures.
67.1s, 5 suites, 170 cases, 25224 tests total, 0 failures
```

`DEPOSITPREAUTH-OWNERDIR-SWEEP-001` asked whether legacy DepositPreauth,
DepositAuth, credential-bound preauthorization, or related `AccountDelete`
cleanup can leave owner-count, reserve, directory, or receive-policy drift.
The source review covered `DepositPreauth`, `Credentials`, credential helper
cleanup, `verifyDepositPreauth`, `lsfDepositAuth`, owner-directory insertion
and deletion, reserve checks, and account deletion. Existing upstream coverage
passed across direct DepositAuth receive policy, IOU/XRP payment behavior,
credential-bound preauthorization, expired and invalid credentials,
preauthorization creation/deletion, directory-full and reserve failures,
AccountDelete constraints, credential deletion during issuer/subject deletion,
and invariant owner-count checks. Static artifact
`depositpreauth_ownerdir_static_sweep_20260528.log` has sha256
`4ccad122a5ff55264604863d077f9b59cfb12c2496437f72eb40be3fbd25ed5f`. Suite
artifact `depositpreauth_ownerdir_source_kill_20260528.log` has sha256
`42294a55728c813a9dff9cef2e73e6e283f47726271082d334ff66d63d27b950`.

```text
ripple.app.AccountDelete had 0 failures.
ripple.app.Credentials had 0 failures.
ripple.app.DepositAuth had 0 failures.
ripple.app.DepositPreauth had 0 failures.
ripple.app.Invariants had 0 failures.
46.2s, 5 suites, 155 cases, 19207 tests total, 0 failures
```

`REGULARKEY-MASTERKEY-AUTH-SWEEP-001` asked whether old regular-key,
disabled-master, signer-list, ticket-paid key rotation, or delegated-signature
behavior can leave an account with broken authorization, unremovable signing
state, or transaction-visible auth bypass. The source review covered
`SetRegularKey::preflight`, `SetRegularKey::doApply`, `SetAccount`'s
`asfDisableMaster` handling, `Transactor::checkSingleSign`,
`Transactor::checkMultiSign`, signer-list cleanup, delegated signing, ticket
usage, and account deletion. The history sweep found the old 2013 regular-key
fixes, the 2015 master-key requirement, the 2019 `fixMasterKeyAsRegularKey`
amendment, and the 2025 retirement of that amendment, but no current live
branch with a minimized transaction-visible witness. Existing upstream coverage
then passed across regular-key setting/revocation, master-key disable/reenable,
master-as-regular-key before and after the fix, zero-fee password-spent
behavior, ticket-paid regular-key changes, multisign master/regular-key checks,
signer-list transitions, delegation, account-delete cleanup, and invariants.
Static artifact `regularkey_masterkey_auth_static_sweep_20260528.log` has
sha256 `f776a22896a78511d430d6b2ef2e160cba5afccd0613932256254256cc85b3e2`.
History artifact `regularkey_masterkey_auth_history_sweep_20260528.log` has
sha256 `3f8beaa3bb6147f7560eda5e0dfade9ad0bd57d9e48a5ad58f205204fbfc82bc`.
Suite artifact `regularkey_masterkey_auth_source_kill_20260528.log` has
sha256 `cd57379575d1eb28747cf3f686e6f28c423d8cf855f2079b757b4c7f246b3a23`.

```text
ripple.app.AccountDelete had 0 failures.
ripple.rpc.AccountSet had 0 failures.
ripple.app.Delegate had 0 failures.
ripple.app.Invariants had 0 failures.
ripple.app.MultiSign had 0 failures.
ripple.app.SetRegularKey had 0 failures.
ripple.app.Ticket had 0 failures.
71.7s, 7 suites, 194 cases, 28923 tests total, 0 failures
```

After removing scratch-only probes, the upstream `OpenP0Repro` suite returned:

```text
16.8s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

## Current sibling: NFToken settlement positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal seller-proceeds behavior:

1. The seller clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. The seller mints an NFT and creates a sell offer priced in gateway USD.
3. The buyer accepts the sell offer.
4. The seller receives 40 USD, the NFT/offer objects are gone, `OwnerCount`
   is still `0`, and the receiver reserve flag remains unset.

Minimal broker-fee behavior:

1. The broker clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. A brokered NFT sale pays a 10 USD broker fee.
3. The broker receives 10 USD, `OwnerCount` is still `0`, and the receiver
   reserve flag remains unset.

Proof excerpts:

```text
ripple.tx.OpenP0Repro TrustLine current — NFToken AcceptOffer creates positive balance without reserve
ripple.tx.OpenP0Repro TrustLine current — NFToken broker fee creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: these are fourth and fifth current-live settlement witnesses
for the same old receiver-side reserve transition. They do not add root-cause
count; they show NFT seller proceeds and broker fees hit the same shared IOU
credit behavior.

## Current sibling: AMMWithdraw positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice clears the gateway USD trustline back to zero balance, zero limit,
   `OwnerCount=0`, and no receiver reserve flag.
2. Alice makes an XRP-only AMM deposit into an XRP/USD pool and receives LP
   tokens.
3. Alice withdraws her LP position as a one-asset USD withdrawal.
4. Alice receives gateway USD while the trustline receiver reserve flag remains
   unset and `OwnerCount=0` after the LP token object is burned.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — AMMWithdraw creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is a sixth current-live settlement witness for the same
old receiver-side reserve transition. It is useful because AMM is a separate
ledger-effect family, but it still resolves to the same reserve-accounting
root cause rather than a new finding count.

## Current sibling: AMMClawback positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Alice creates a two-issuer AMM and deposits all of her gateway2 EUR into the
   pool.
2. Gateway2 clears default ripple and Alice clears her EUR trust limit, leaving
   the EUR trustline SLE present with no receiver reserve flag.
3. Gateway1 performs `AMMClawback` against Alice's gateway1 USD in the pool.
4. The paired gateway2 EUR asset is returned to Alice.
5. Alice's EUR balance becomes positive while the EUR receiver reserve flag
   remains unset and Alice's `OwnerCount` does not increase.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — AMMClawback creates positive balance without reserve
17.3s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is a seventh current-live settlement witness for the same
old receiver-side reserve transition. It matters because AMMClawback reaches
the shared IOU credit behavior through a paired-asset return, not a normal user
withdrawal.

## Current sibling: TokenEscrow positive-balance reserve drift

Status: reproduced on current `3.1.3` under the same
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` finding.

Minimal behavior:

1. Bob opens a gateway USD trustline, receives 100 USD, clears the limit, and
   pays the 100 USD back.
2. The line remains with `OwnerCount=0` and no receiver reserve flag.
3. Alice escrows 40 USD to Bob through TokenEscrow.
4. Bob finishes the escrow and receives 40 USD.
5. Bob still has `OwnerCount=0` and no receiver reserve flag.

Proof excerpt:

```text
ripple.tx.OpenP0Repro TrustLine current — TokenEscrow creates positive balance without reserve
15.1s, 1 suite, 70 cases, 16752 tests total, 0 failures
```

Interpretation: this is a third current-live settlement path for the same
reserve/owner-count root cause. It is not counted as a new finding; it confirms
that the missing transition sits in the shared IOU credit behavior reached by
multiple ledger-effect paths.

## Offer Missing-Owner-Reserve Boundary

This slice added a sixth marker under the same finding. It is not a new finding
count.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2`.
3. A market account posts an offer selling gateway USD for XRP.
4. Alice is drained to exactly the two-owner reserve plus the offer-crossing
   XRP spend and fee, leaving no reserve capacity for a third owner object
   after the cross.
5. Alice crosses the offer and receives 50 USD.
6. Alice remains below the three-owner reserve, `OwnerCount` remains `2`, and
   the receiver reserve flag remains unset.

The packet proof marker is:

```text
ripple.tx.OpenP0Repro TrustLine current — offer crossing succeeds below missing owner reserve
```

This adds the offer-side reserve-boundary companion to the prior CheckCash
boundary proof: both live settlement paths accept a positive IOU balance even
when the receiver lacks the XRP reserve that the missing owner-count transition
would require.

## CheckCash Missing-Owner-Reserve Boundary

The prior slice added a fifth marker under the same finding. It is not a new
finding count.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero balance, zero
   limit, `OwnerCount=0`, and no receiver reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2`.
3. The gateway writes a USD check to Alice.
4. Alice is drained to exactly the two-owner reserve plus the CheckCash fee,
   leaving no reserve capacity for a third owner object.
5. Alice cashes the check and receives 50 USD.
6. Alice remains below the three-owner reserve, `OwnerCount` remains `2`, and
   the receiver reserve flag remains unset.

The packet proof marker is:

```text
ripple.tx.OpenP0Repro TrustLine current — CheckCash succeeds below missing owner reserve
```

## Offer Existing-Owner Control

The prior slice added a fourth marker under the same finding. It is not a new
finding count.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero and no receiver
   reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2` before crossing the
   offer.
3. A market account posts an offer selling gateway USD for XRP.
4. Alice crosses the offer and receives 50 USD.
5. Alice ends with a positive 50 USD balance while `OwnerCount` remains `2`
   and the receiver reserve flag remains unset.

The packet proof marker is:

```text
ripple.tx.OpenP0Repro TrustLine current — offer crossing leaves positive balance unowned with existing owner objects
```

This matters for the same reason as the CheckCash existing-owner control: it
rules out the narrow explanation that the behavior is only the historical
`OwnerCount < 2` reserve carveout. The offer-crossing witness also fails to
apply the positive-trustline owner-count/reserve transition after the holder
already owns two objects.

## CheckCash Sibling Repro

An earlier slice added a second current-live marker under the same finding, not
a new finding count.

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

## CheckCash Existing-Owner Control

An earlier slice added a third marker under the same finding. It is not a new
finding count.

Minimal behavior:

1. Alice clears the same gateway USD trustline back to zero and no receiver
   reserve flag.
2. Alice creates two ticket objects, so `OwnerCount=2` before cashing the
   check.
3. The gateway writes a USD check to Alice.
4. Alice cashes the check.
5. Alice ends with a positive 50 USD balance while `OwnerCount` remains `2`
   and the receiver reserve flag remains unset.

The packet proof marker is:

```text
ripple.tx.OpenP0Repro TrustLine current — CheckCash leaves positive balance unowned with existing owner objects
```

This matters because it rules out the narrow explanation that the behavior is
only the historical `OwnerCount < 2` reserve carveout. The holder already has
two owned objects before the trustline crosses positive; the missing transition
is the receiver-side owner-count/reserve update for the positive trustline.

## Why This Is The Moby Dick Candidate

The broken behavior is simple: after a holder clears its trust limit and
balance, old live settlement paths can give that holder a positive IOU balance
while `OwnerCount` and the trustline reserve flag remain wrong. This is now
reproduced through offer crossing with zero existing owned objects, offer
crossing with two existing owned objects, CheckCash with zero existing owned
objects, and CheckCash with two existing owned objects.

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
script_sha256: 085f6aaf26b4969ad01edd38d551c9404794bb64fa1f706032763cfd7a8f74e2
report: runs/20260527-p0-hunt/trustline_positive_balance_lineage_20260527.json
report_sha256: 6c5b694085fd6aff9583e5b9c7ca1f88dad2417edb61d49ecc0a4d83240083e5
result: passed=True
sampled refs: 0.12.0, 0.20.0, 0.30.0, 0.50.0, 0.80.0, 1.0.0, 1.5.0, 2.0.0, 2.5.0, 3.1.3
fix evidence: `fixTrustLineOwnerCount`, receiver-side reserve transition in `View.cpp`,
`TrustAndBalance::testOwnerCountOnBalanceChange`, and `Offer_test` owner-count
expectations conditional on the fix.
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

`PAYCHAN-DIRFULL-PARTIAL-001` and `ESCROW-DIRFULL-PARTIAL-001` were also
probed and demoted in the same two-directory-write class.

The scratch PayChannel probe filled the destination owner directory before
`PaymentChannelCreate`. The transaction returned `tecDIR_FULL`, the computed
channel key did not exist afterward, and the source account owner count stayed
at zero. The scratch Escrow probe filled the destination owner directory before
`EscrowCreate` with a valid `FinishAfter`. It also returned `tecDIR_FULL`,
left no escrow object, and left the source owner count at zero. Both temporary
scratch tests passed locally in `OpenP0Repro` and were removed from the suite
before this triage note was written.

These are not promoted findings.

## Current continuation: payment and reserve source-signal sweep

This slice inspected the remaining high-signal old payment/reserve commits
before returning to broader hunting. No new packet finding was promoted.

`PAYMENT-SANDBOX-DEFERRED-CREDITS-001` was source-reviewed from upstream commit
`7cfa5d461` / `b8792e242`, "fix: Make assorted Payments fixes (#6585)." The
commit changes `PaymentSandbox` deferred-credit accounting names, removes a
debug balance-delta helper, plus initializes `sfBalance` when `Payment` creates
a destination account. The account-creation branch is already represented in
the candidate matrix as `PAYMENT-BALANCE-001` and was scratch-tested/demoted:
exact-reserve XRP account creation succeeds on the current `3.1.3` target. This
continuation closed the remaining focused path-payment/MPT review target with
static and history sweeps plus upstream `PaymentSandbox`, `Flow`, `PayStrand`,
`Path`, `Offer`, `TrustAndBalance`, `SetTrust`, and `Invariants` coverage. No
current-release transaction witness showed unauthorized movement, reserve
drift, invariant failure, or consensus-visible state corruption. Static
artifact `payment_sandbox_deferred_credit_static_sweep_20260528.log` has
sha256 `2ae19ec357328ebb5a2f81b819cee65e9cf9c00b0c16f664de363c220d348b71`.
History artifact `payment_sandbox_deferred_credit_history_sweep_20260528.log`
has sha256 `7a5433dcd0ea726214ae837369845fdd8db85a6efeb22e11887fb459cd1bdbce`.
Suite artifact `payment_sandbox_deferred_credit_source_kill_20260528.log` has
sha256 `a3e702bbcad88f9584178a6fba48f7e7f0ea677fc5cbdeabc79d7e0c7c841d82`.

```text
ripple.app.Flow had 0 failures.
ripple.app.Path had 0 failures.
ripple.app.PayStrand had 0 failures.
ripple.app.SetTrust had 0 failures.
ripple.app.TrustAndBalance had 0 failures.
ripple.ledger.PaymentSandbox had 0 failures.
116.9s, 14 suites, 570 cases, 75191 tests total, 0 failures
```

`TXQ-MIN-RESERVE-POTENTIAL-SPEND-001` was source-reviewed from historical
commit `e7a69cce6`, "Account for minimum reserve in potential spend." That fix
is already an ancestor of the current `3.1.3` proof target and `origin/develop`.
It resolved an old queued-transaction fee/reserve accounting issue and also
changed legacy Escrow/PayChan owner-count writes to `adjustOwnerCount()`. This
continuation closed the source signal with a static sweep, the historical patch
record, and upstream TxQ plus owner-object coverage. Since the current target
already contains the fix and the current suite passed, it is not a live Moby
Dick candidate. Static artifact
`txq_min_reserve_potential_spend_static_sweep_20260528.log` has sha256
`c6f59dc786d9314f8b1688d1dd3f2fb7edc12a8de254ce1f668fcba6086d900e`.
History artifact `txq_min_reserve_potential_spend_history_sweep_20260528.log`
has sha256 `01f957eaf54d92a31f5044e04b6fee3c187eebc448736c97cf6789d3de7ab5fa`.
Suite artifact `txq_min_reserve_potential_spend_source_kill_20260528.log` has
sha256 `11fe6edc6b98a17e4b24ea9e90f56b705ecd502b1c7c963310669dc180853cf8`.

```text
ripple.app.AccountDelete had 0 failures.
ripple.app.Escrow had 0 failures.
ripple.app.PayChan had 0 failures.
ripple.app.Ticket had 0 failures.
101.9s, 13 suites, 502 cases, 71678 tests total, 0 failures
```

`NFT-BROKER-AUTH-001` asked whether `NFTokenAcceptOffer` brokered mode still
has a current buyer/seller/broker auth or freeze bypass under the live
`fixEnforceNFTokenTrustlineV2` path. This continuation source-killed the
candidate. Current source checks broker fee receipt with
`checkTrustlineAuthorized` and `checkTrustlineDeepFrozen`, checks buyer/seller
authorization on brokered and direct paths, and upstream tests cover
unauthorized broker, buyer, seller, minter, NFT-offer freeze, and deep-freeze
cases across feature sets. No clean current auth/freeze bypass was isolated.
Static artifact `nft_broker_auth_static_sweep_20260528.log` has sha256
`3355a8182950795ee840414293b904168ce86413d6f897ff5f9bded5240d1846`.
History artifact `nft_broker_auth_history_sweep_20260528.log` has sha256
`bba37fb0115e5e5e2313ebfbd0ad48ca337f372e088dca45b351ddb87ced245b`.
Auth/NFT suite artifact `nft_broker_auth_source_kill_20260528.log` has sha256
`61c019274a704d1caefc4fc8d1aff0a7250353f5045da47e2a93259ec4789535`.
Freeze suite artifact `nft_broker_auth_freeze_source_kill_20260528.log` has
sha256 `feeed2fc096c72facb1e4313dbc824d03a37a61e08a31cc4ea6e2e908caeadbe`.

```text
ripple.app.NFTokenAuth had 0 failures.
ripple.app.NFTokenDir had 0 failures.
289.7s, 15 suites, 343 cases, 365695 tests total, 0 failures

ripple.app.Freeze had 0 failures.
14.4s, 1 suite, 91 cases, 10632 tests total, 0 failures
```

`PAYMENT-HOLDER-TO-HOLDER-RESERVE-SIBLING-001` was scratch-tested as the next
plain-payment sibling for `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`. The setup
cleared Alice's gateway USD trustline back to zero balance, zero limit,
`OwnerCount=0`, and no receiver reserve flag, then attempted a holder-to-holder
USD payment from Bob into Alice. The compiled scratch case returned
`tecPATH_DRY`, left Alice's USD balance at zero, and left `OwnerCount=0`:

```text
ripple.tx.OpenP0Repro SCRATCH holder payment into cleared trustline
16.5s, 1 suite, 68 cases, 16619 tests total, 0 failures
```

The scratch probe was removed before packet commit. This further narrows the
current reserve finding: plain `Payment` variants fail closed in the cleared
trustline setup, while offer crossing, `CheckCash`, TokenEscrow finish, and
NFToken settlement remain the reproduced live paths that move the receiver
positive without the owner-count/reserve transition.

## PayChan Legacy Probe

`PAYCHAN-LEGACY-CLOSE-OWNERDIR-001` was source-reviewed in this slice but not
promoted. Existing upstream tests already cover old-style payment-channel
recipient owner-directory behavior and resurrection behavior around the
`fixPayChanRecipientOwnerDir` amendment. No new live P0 repro was produced for
this candidate in this slice.

## Offer And Payment Probes

`OFFER-LEGACY-RESERVE-CROSSED-001` was reviewed, scratch-tested, and not
promoted. The current `Offer_test::testPartialCross` table already covers
reserve-boundary behavior with `tecINSUF_RESERVE_OFFER` and `tecUNFUNDED_OFFER`
controls. The added scratch probe confirmed the live `OfferCreate` path: an
under-reserved no-cross offer rejects, while an under-reserved partial cross
lets crossed value stand but places no remainder offer. No new ledger corruption
or object-lifetime failure was shown.

`PAYMENT-LEGACY-TEFEXCEPTION-PATH-001` was reviewed and not promoted. Current
`Payment.cpp` contains explicit guards that route internal exceptions to
`tefEXCEPTION`/`tefINTERNAL` handling, but no clean current-release transaction
repro surfaced that turned a normal live-enabled payment into a new Moby Dick
P0. Keep it as a lower-priority source-review target unless a sharper input
shape appears.

The continuation also scratch-tested three reserve-drift variants after Alice
cleared the same gateway USD trustline back to zero: direct issuer `Payment`,
holder-to-holder IOU `Payment`, and an order-book path `Payment` with XRP
`SendMax`. Each returned `tecPATH_DRY` before creating positive balance, so
those plain payment variants are source-killed. That makes the promoted
witnesses sharper: offer crossing and `CheckCash` are the live paths that move
the cleared trustline positive without the receiver-side owner-count/reserve
transition.

The next reserve sibling tested the AMM auction refund path because `AMMBid`
already has a promoted holder-`DepositAuth` refund bypass. The scratch case
gave Alice an AMM auction slot, burned her LP-token balance to zero, cleared
her LP-token trustline, and let Bob outbid her. The refund did recreate
Alice's LP-token trustline, but the ledger charged the owner reserve: Alice's
`OwnerCount` increased and the correct reserve flag was set. That kills
`AMMBID-TRUSTLINE-RESERVE-SIBLING-001` as a positive-balance/no-reserve
witness while leaving the separate `AMMBID-DEPOSITAUTH-REFUND-001` finding
unchanged.

`ACCOUNTDELETE-POSITIVE-UNOWNED-TRUSTLINE-001` asked whether the current
reserve bug can be extended into account deletion: after offer crossing creates
a positive gateway USD balance while Alice's `OwnerCount` remains `0`, can
Alice delete her account and leave a stranded positive trustline behind? The
scratch repro advanced enough ledgers for AccountDelete, paid the owner-reserve
delete fee, and attempted to delete Alice into a funded destination. The
transaction returned `tecHAS_OBLIGATIONS`, Alice's account root remained, and
the positive trustline remained. This source-kills the account-deletion
extension even though the trustline is not counted in `OwnerCount`. Static
artifact `accountdelete_positive_unowned_trustline_static_sweep_20260528.log`
has sha256
`2c3233a85651099a33e21ff05ba41ed9c519432f1df5aaf100b3bdadcf4c83df`.
History artifact
`accountdelete_positive_unowned_trustline_history_sweep_20260528.log` has
sha256 `69141a8961bce1bf01016aead3e732a81021bbf6a5908f34e523cf84dafc4a2c`.
Scratch source-kill artifact
`accountdelete_positive_unowned_trustline_source_kill_20260528.log` has sha256
`6c4ddb4a2f230dc817ded3b02c1d7005796fd706cfca560e4982a14300d60871`.

```text
ripple.tx.OpenP0Repro SCRATCH AccountDelete with positive unowned trustline
ripple.tx.OpenP0Repro had 0 failures.
17.0s, 1 suite, 71 cases, 16830 tests total, 0 failures
```

## Legacy-Core Source-Kill Sweep

The remaining old-core queue was reviewed against current source and current
tests, and the following candidates were source-killed rather than promoted:

- `TRUSTSET-LEGACY-RESERVE-CARVEOUT-001`: the reserve carveout is an explicit
  gateway bootstrap rule in `SetTrust::doApply`, not an accidental leak. It
  skips reserve enforcement only while `OwnerCount < 2`, then reverts to normal
  reserve checks. A focused source/history sweep and the upstream
  `SetTrust,TrustAndBalance,Freeze` suites were packet-bound in this slice.
  Static artifact `trustset_reserve_carveout_static_sweep_20260528.log` has
  sha256 `3ec97306684ca8e4072a76c7058f285dcae06cff1aa5823a03dea69a8976f65b`.
  History artifact `trustset_reserve_carveout_history_sweep_20260528.log` has
  sha256 `89daae83eec3df6338b5855ffce51f945a1821829ce816c5f9abe32dd5c91352`.
  Suite artifact `trustset_reserve_carveout_source_kill_20260528.log` has
  sha256 `593e5958864764b00791eb39d5da57c5ccf6a9a49c1dc56a838b4b87c7112b31`.
  The suite passed with 145 cases and 13,435 tests, including explicit
  coverage for two free trustlines, dynamic trustline reserve enforcement,
  DisallowIncoming, trustline deletion/reset, no-ripple, authorization, freeze,
  and deep-freeze behavior. The current evidence does not show live-state
  corruption or a stranded object from the carveout itself.
- `ACCOUNTSET-LEGACY-FLAG-SWEEP-001`: old AccountSet policy flags and
  transfer-rate/tick-size fields were reviewed as a possible source of
  irreversible account-policy drift. The sweep covered `SetAccount`,
  `SetRegularKey`, `SetTrust`, RequireAuth, NoFreeze, GlobalFreeze,
  DefaultRipple, DisallowXRP, DisableMaster, AllowTrustLineClawback,
  TransferRate, TickSize, Domain, EmailHash, and MessageKey source and history.
  Static artifact `accountset_legacy_flag_static_sweep_20260528.log` has
  sha256 `3301768d245f4f8b883fd79e1bf9e3c90f1cf75b1f540ae8c3a873e88c04ab5e`.
  History artifact `accountset_legacy_flag_history_sweep_20260528.log` has
  sha256 `3c20b7cf7445e52bb283be15e4b0135a3d79a550bfc17b66dad641f382791137`.
  Suite artifact `accountset_legacy_flag_source_kill_20260528.log` has sha256
  `8027ce231849a0ec589c0dd1a254a598c8b14345b5a4f5e4075fe7870977a5ba`.
  The effect suites passed with 263 cases and 32,279 tests across Freeze,
  SetTrust, SetRegularKey, TrustAndBalance, AccountDelete, Invariants,
  Clawback, and DepositAuth. No clean legacy-core transaction witness showed
  forbidden flag-combination mutation, broken alternate-key state, freeze/auth
  policy drift, reserve/object drift, or invariant failure from AccountSet
  itself.
- `LEGACY-TX-ENVELOPE-SWEEP-001`: old transaction envelope, signing,
  multi-sign, ticket/sequence, canonical serialization, pseudo-transaction,
  transaction-ordering, and apply-boundary code was reviewed as a possible
  source of a replay, auth bypass, sequence collision, malformed canonical
  object, or deterministic ordering witness below individual transaction
  families. The sweep covered `STTx`, `STObject`, `Serializer`, `SeqProxy`,
  `Transactor`, `ApplyContext`, `CreateTicket`, `SetRegularKey`, multisign,
  pseudo transactions, and transaction ordering. Static artifact
  `legacy_tx_envelope_static_sweep_20260528.log` has sha256
  `5d27bc9d2065dba98556c9fd9a029de33305ac4c95a6f414bbe15d5e0ddbe8c3`.
  History artifact `legacy_tx_envelope_history_sweep_20260528.log` has sha256
  `ca4d8acc13cecef9687515db3b2a1ef12b6314779b56ee9050954840d665a882`.
  Suite artifact `legacy_tx_envelope_source_kill_20260528.log` has sha256
  `b490584f0860342399e30e3220f39a633afbb2bb2a8f46d8c9f020bc33f3eb7a`.
  The envelope suites passed with 10 suites, 92 cases, and 6,782 tests across
  `STTx`, `STObject`, `Serializer`, `SeqProxy`, `Ticket`, `MultiSign`,
  `SetRegularKey`, `Transaction_ordering`, `PseudoTx`, and `Apply`. No clean
  transaction-visible replay, signature/auth, sequence, serialization, or
  canonical-ordering P0 was isolated.
- `LEGACY-IOU-ZERO-CROSS-SWEEP-001`: old IOU settlement paths were reviewed
  again around the specific reserve/owner-count failure mode: moving a
  trustline from non-positive to positive balance without the receiver-side
  reserve flag or `OwnerCount` transition. The sweep covered shared
  `rippleCredit`, `accountSend`, `trustCreate`, `trustDelete`,
  `adjustOwnerCount`, `lsfLowReserve`, and `lsfHighReserve` call sites across
  `Payment`, path steps, `Offer`, `CheckCash`, `SetTrust`,
  `TrustAndBalance`, and `PaymentSandbox`. Static artifact
  `legacy_iou_zero_cross_static_sweep_20260528.log` has sha256
  `e7a48bf5faf8a95ee052abe2c9d99b814488e33a97a48e6ced2d6a87a9f7c155`.
  History artifact `legacy_iou_zero_cross_history_sweep_20260528.log` has
  sha256 `92d7e9c4fc52b68808d525be312a53285495800ad9f28a5d183470d2188a5b66`.
  Suite artifact `legacy_iou_zero_cross_source_kill_20260528.log` has sha256
  `b1148f687e36bc488913c25eac67b57197641ce75d98fab2a296c65add0f31c5`.
  The focused suite run passed with 14 suites, 545 cases, and 74,558 tests
  across `Payment`, `PaymentSandbox`, `Flow`, `PayStrand`, `Path`, `Offer`,
  `SetTrust`, `TrustAndBalance`, and `Check`. The sweep reinforced the
  existing `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` root and its packet-bound
  markers, but did not isolate a separate old-core P0 outside that root.
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
- `AUTO-CREATE-RESERVE-PARTIAL-CROSS-001`: upstream commit `d3e22a125`
  adds `OfferMPT_test::testAutoCreateReserve` and explicitly documents that a
  partially crossed book offer may auto-create an IOU trustline or MPToken for
  the offer owner without a reserve sufficiency check. That is a useful source
  signal, but not a new promoted finding by itself. The packet keeps the
  promoted reserve finding narrower: after the holder already has the
  trustline and moves from non-positive to positive balance, the receiver-side
  owner-count/reserve transition remains missing across offer crossing and
  `CheckCash`.
- `AMMWITHDRAW-MPT-DOUBLE-OWNERCOUNT-001`: upstream commit `0a896b795`
  fixes a double `adjustOwnerCount()` in a future MPT-aware `AMMWithdraw`
  implementation. It is not promoted in this legacy-core lane because current
  `3.1.3` `AMMWithdraw.cpp` handles `Issue` assets, not MPT `Asset`/`MPTIssue`
  withdrawals, and the fix commit is not an ancestor of `3.1.3`, `3.2.0-b7`,
  or `origin/develop`.
- `BOOK-DIR-INVARIANT-DELETE-001`: upstream commit `e34c2667d` fixes
  `ValidBookDirectory` invariant handling for deleted or ordinary modified
  book directories. It remains a future-branch quality signal for the
  `fixCleanup3_2_0` invariant lane, not a current `3.1.3` legacy-core P0,
  because the checked `3.1.3` proof target does not contain
  `ValidBookDirectory`.

That leaves the current legacy-core live candidate set unchanged: the best
remaining old/simple/current target is still
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`; the current slice moved it from
source-lineage plus current repro to source-lineage plus current, `2.5.0`,
`2.0.0`, and `1.5.0` binary repro, then added CheckCash as a second
current-live settlement-path marker, CheckCash-with-existing-owner-objects as a
third marker, and offer-crossing-with-existing-owner-objects as a fourth marker
for the same root cause. Later slices added fifth and sixth markers proving
CheckCash and offer crossing still succeed when the receiver lacks reserve
capacity for the missing third owner object.

## Next Step

Keep drilling trustline/settlement reserve siblings unless an even older and
cleaner source signal appears:

1. source-review sibling crossings that can move IOU balances across zero
   without the shared receiver-side owner-count transition;
2. provision the oldest practical pre-`1.5.0` toolchain only if it is cheap;
3. reproduce the same reserve/owner-count drift there if buildable before
   promoting any specific pre-2020 binary-span statement.
