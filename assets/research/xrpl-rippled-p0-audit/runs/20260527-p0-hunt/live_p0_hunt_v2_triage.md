# Live XRPL P0 Hunt V2 Triage

Checked: `2026-05-28T10:28:36Z`

## Live Scope

Direct XRPL JSON-RPC showed the public servers checked for the packet reporting
`rippled_version=3.1.3`.

Enabled live surfaces for this packet:

- `AMM`
- `AMMClawback`
- `Checks`
- `CheckCashMakesTrustLine`
- `DepositAuth`
- `DisallowIncoming`
- `fixDisallowIncomingV1`
- `MPTokensV1`
- `NonFungibleTokensV1_1`
- `fixEnforceNFTokenTrustline`
- `fixEnforceNFTokenTrustlineV2`
- `fixRemoveNFTokenAutoTrustLine`
- `fixNFTokenReserve`
- `fixNFTokenRemint`
- `NFTokenMintOffer`
- `PermissionedDomains`
- `PermissionedDEX`
- `TokenEscrow`
- `Credentials`
- `fixMPTDeliveredAmount`
- `fixAMMv1_3`
- `fixTokenEscrowV1`
- `fixAMMClawbackRounding`
- `fixCleanup3_1_3`

Disabled surfaces that remain excluded:

- `LendingProtocol`
- `SingleAssetVault`
- `PermissionDelegation`
- `Batch`
- `XChainBridge`
- `fixDelegateV1_1`
- `fixDisallowIncomingV1_1`
- `fixXChainRewardRounding`

`fixCleanup3_1_3` is included by raw amendment hash
`303ACB16CF8DBD3B5C34F131A9D19A7DE01AE05F480A8A682B869D1B4AAC8CFC`.
The public `feature` name lookup did not expose that name, so the gate checks
the raw on-ledger `Amendments` object rather than relying only on feature-name
visibility.

## Live Dependency Correction

`MPT-DOMAIN-AUTH-001` is excluded from the live packet. The reproduced path uses
MPT `DomainID`, and current `MPTokenIssuanceCreate` / `MPTokenIssuanceSet`
feature gates require `PermissionedDomains && SingleAssetVault` for that field.
Direct XRPL mainnet status shows `SingleAssetVault=false`, so this finding does
not satisfy the live-mainnet-only constraint.

## Promotion Result

Five additional findings were promoted during the live-only continuation:

- `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`: standard IOU settlement can create
  a positive balance for a receiver while leaving the receiver reserve flag
  unset after that receiver previously cleared the trustline limit and balance.
  The current packet reproduces the same reserve drift through offer crossing
  and CheckCash with `OwnerCount=0`, and through both paths with two existing
  ticket objects, where `OwnerCount` remains `2` instead of increasing for the
  positive trustline. Further offer-crossing and CheckCash boundary controls
  succeed after the receiver is drained below the three-owner reserve threshold,
  proving the path does not merely forget an accounting flag; it accepts the
  settlement without the reserve capacity the missing owner object would
  require.
- `TRUSTLINE-DISALLOW-INCOMING-OFFER-001`: an issuer can set
  `asfDisallowIncomingTrustline`, but `OfferCreate` can still cross into that
  issuer's IOU for a taker with no existing trustline, creating the incoming
  trustline through the offer path.
- `NFTOKEN-DISALLOW-INCOMING-ACCEPT-001`: the same issuer opt-out is bypassed
  by `NFTokenAcceptOffer`; direct `TrustSet` is rejected, but NFT sell-offer
  settlement in the issuer's IOU creates the seller trustline anyway.
- `CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001`: the same issuer opt-out is
  bypassed by `CheckCash`; direct `TrustSet` is rejected, but cashing an IOU
  check auto-creates the receiver trustline anyway.
- `TOKENESCROW-DISALLOW-INCOMING-FINISH-001`: the same issuer opt-out is
  bypassed by `EscrowFinish`; direct `TrustSet` is rejected, but finishing an
  IOU TokenEscrow auto-creates the destination trustline anyway.

## Latest Demotion

- `MPT-ESCROW-SENDER-LOCK-001`: tested whether MPT TokenEscrow finish ignores
  a sender lock applied after escrow creation. The scratch `OpenP0Repro` case
  returned `tecNO_PERMISSION`, not `tesSUCCESS`, so no live-mainnet P0 was
  promoted and the scratch repro was removed.

A later AMMClawback slice promoted one more unresolved live finding:

- `AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001`: in a two-issuer AMM, issuer
  A can claw back its asset and force-return issuer B's paired IOU to a holder
  after issuer B has set `asfDisallowIncomingTrustline`, recreating issuer B's
  trustline through the AMMWithdraw/accountSend path even though direct
  `TrustSet` is rejected.

A later NFToken broker-fee slice promoted one more unresolved live finding:

- `NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001`: a brokered
  `NFTokenAcceptOffer` can pay the broker fee in an issuer's IOU to a broker
  with no existing trustline after that issuer has set
  `asfDisallowIncomingTrustline`, creating the broker trustline through
  NFTokenAcceptOffer::pay/accountSend even though direct `TrustSet` is rejected.

A later AMMClawback/DepositAuth slice promoted one more unresolved live finding:

- `AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001`: in a two-issuer AMM, a holder can
  set `asfDepositAuth` and reject direct issuer payment with `tecNO_PERMISSION`,
  but AMMClawback can still force-return the paired IOU to that holder and
  recreate the paired-asset trustline through the AMMWithdraw/accountSend path.

A later AMMBid/DepositAuth slice promoted one more unresolved live finding:

- `AMMBID-DEPOSITAUTH-REFUND-001`: a previous AMM auction-slot owner can set
  `asfDepositAuth` and reject direct LP-token payment with `tecNO_PERMISSION`,
  but a later `AMMBid` by another account can still refund LP tokens to that
  previous owner through the AMMBid/accountSend path.

The remaining high-volume candidate set in the existing matrix is blocked by
one of the WHIP gates:

- cleanup-era candidates are excluded because `fixCleanup3_1_3` is enabled by
  raw amendment hash;
- lending, vault, delegation, and batch candidates are excluded because their
  required amendment surfaces remain disabled;
- protocol-wire and arithmetic-helper candidates remain below the P0 severity
  bar unless a transaction-visible live surface is demonstrated;
- already-remediated findings remain release-line evidence, not novel unfixed
  P0s.
- `CRED-DIRFULL-001` was tested on the live `Credentials` surface after direct
  status refresh. The scratch proof forced the subject owner directory full and
  returned `tecDIR_FULL` without leaving a `Credential` object or issuer/subject
  owner-count drift, so it is source-killed and not packet eligible.
- `DID-CREDENTIAL-DIRFULL-SWEEP-001` followed with a direct DID/fixEmptyDID
  feature receipt and a bounded source/history/suite sweep across DID,
  Credentials, credential helper cleanup, AccountDelete, owner directories,
  `deleteSLE`, owner counts, and invariants. `DID`, `Credentials`,
  `AccountDelete`, and `Invariants` passed with 147 cases and 16,656 tests; no
  partial object, directory entry, owner-count drift, or invariant witness was
  isolated.
- `LEGACY-OFFER-BOOK-DIRECTORY-SWEEP-001` source-killed the remaining old-core
  offer-book directory/quality/cancel lane. Static/history review plus
  `Offer,ReducedOffer,OfferStream,BookDirs,Directory,Invariants` coverage
  passed with 435 cases and 83,311 tests. No stale offer object, wrong
  book-directory quality, owner-count drift, or normal-input invariant/internal
  failure was isolated outside the already-promoted trustline reserve root.
- `PAYMENT-LEGACY-TEFEXCEPTION-PATH-001` was source-killed with a focused
  payment/path result-code and amount/issue sweep. Static/history review plus
  `PaymentSandbox,Flow,PayStrand,Path,SetTrust,TrustAndBalance,STAmount,
  STNumber,Number,IOUAmount,Issue` passed with 265 cases and 122,354 tests. No
  normal-input `tefEXCEPTION`, `tefINTERNAL`, fee-burning bad-input, amount
  issue mismatch, or separate payment/path Moby Dick witness was isolated.
- `XRP-PAYMENT-ACCOUNTCREATE-TAG-SWEEP-001` source-killed the native XRP
  payment account-create, destination-tag, `DisallowXRP`, `DepositAuth`, and
  direct result-code lane. Static/history review plus
  `PaymentSandbox,Flow,PayStrand,Path,DepositAuth,AccountDelete,
  TrustAndBalance,Invariants,SetRegularKey,TxQPosNegFlows` passed with 234
  cases and 39,070 tests. No separate account-create, tag, XRP-policy,
  owner-count, fee-burning bad-input, or invariant witness was isolated.
- `PAYMENT-DISALLOW-INCOMING-TRUSTLINE-001` was tested as a direct
  holder-to-recipient IOU `Payment` variant of the live
  `DisallowIncomingTrustline` family. It returned `tecPATH_DRY` and left no
  recipient trustline, so the plain Payment path is source-killed and not packet
  eligible.
- `PAYMENT-TRUSTLINE-RESERVE-DRIFT-001` was tested as direct issuer Payment,
  holder-to-holder IOU Payment, and order-book path Payment variants of
  `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`. All three returned `tecPATH_DRY`
  after the receiver cleared the trustline back to zero, so they are
  source-killed. The current promoted reserve-drift witnesses remain offer
  crossing and `CheckCash`.
- `MPT-LOCK-UNAUTH-NOSAV-001` was also source-reviewed and treated as a
  source-kill in this pass. Current upstream `MPToken_test` intentionally
  allows locked-holder unauthorize/delete when `featureSingleAssetVault` is
  disabled, and `MPTokenAuthorize::preclaim` only blocks that path when SAV is
  enabled. That makes the no-SAV delete behavior a product rule, not a new
  Moby Dick P0.
- `MPT-AUTH-LOCK-CLAWBACK-SWEEP-001` followed the legacy-core queue under the
  broader live-feature pivot. Static/history review plus
  `MPToken,Clawback,AMMClawback,EscrowToken,PermissionedDomains,Credentials`
  passed with 258 cases and 31,582 tests. No new live MPT authorization, lock,
  clawback, TokenEscrow, credential, owner-count, or invariant witness was
  isolated beyond the already packeted MPT findings and already-demoted
  product-semantics lanes.
- `AMM-AUTH-FREEZE-INVARIANT-SWEEP-001` source-killed the next live-feature
  lane after the MPT pass. Static/history review plus
  `AMM,AMMClawback,Clawback,Freeze,DepositAuth,Invariants,Offer,SetTrust`
  passed with 688 cases and 154,192 tests. No new AMM authorization, freeze,
  clawback, auction-slot, LP-token, pool-state, owner-count, offer-crossing, or
  invariant witness was isolated beyond the already packeted AMM findings.
- `PDEX-DOMAIN-CREDENTIAL-INVARIANT-SWEEP-001` source-killed the next
  live-feature lane after the AMM pass. Static/history review plus
  `PermissionedDEX`, `PermissionedDomains`, `Credentials`, `Invariants`,
  `Offer`, `OfferStream`, `ReducedOffer`, and `Directory` passed with 513
  cases and 89,613 tests.
  No new PermissionedDEX domain, credential, hybrid-book, cancellation,
  owner-count, directory, or invariant witness was isolated beyond the already
  packeted PDEX findings and cleanup-era exclusions.
- `TOKENESCROW-RESULTCODE-INVARIANT-SWEEP-001` source-killed the next
  live-feature lane after the PDEX pass. Static/history review plus `Escrow`,
  `EscrowToken`, `Invariants`, `AccountDelete`, `DepositAuth`,
  `DepositPreauth`, `Freeze`, `Clawback`, `AMMClawback`, `MPToken`,
  `STAmount`, `IOUAmount`, and `Directory` passed with 447 cases and 169,461
  tests. No new TokenEscrow result-code, owner-count, directory, amount,
  authorization, freeze, clawback, MPT, or invariant witness was isolated beyond
  the already packeted TokenEscrow DisallowIncoming and trustline-reserve
  markers.
- `CREDENTIALS-CURRENT-TAG-SURFACE-SWEEP-001` source-killed the remaining
  current-tag Credentials lane. Static/history review plus `Credentials`,
  `DepositAuth`, `DepositPreauth`, `PermissionedDomains`, `DID`,
  `AccountDelete`, `Invariants`, `Escrow`, `EscrowToken`, `MPToken`,
  `DepositAuthorized`, `AccountObjects`, `OwnerInfo`, and `AccountTx` passed
  with 381 cases and 48,135 tests. No new credential create/accept/delete,
  expired-credential cleanup, DepositAuth, PermissionedDomain, DID,
  account-cleanup, RPC visibility, owner-count, directory, or invariant witness
  was isolated.
- `XCHAIN-BRIDGE-LIVE-GATE-SWEEP-001` closed the open bridge candidate lane.
  Direct `feature` calls against `s1.ripple.com` and `s2.ripple.com` showed
  `XChainBridge` and `fixXChainRewardRounding` are supported but disabled at
  validated ledger `104535656`, hash
  `45489521F86EBEBE317210579A50D680802199BF6D912A737147EAFD9AC2E58D`.
  Static/history review plus `XChain`, `XChainSim`, `AccountObjects`, and
  `LedgerEntry` passed with 42 cases and 56,064 tests. Bridge candidates remain
  excluded from the live packet unless direct mainnet status later shows the
  bridge amendment surface enabled.
- The remaining legacy-core queue was also source-reviewed in this pass:
  `TRUSTSET-LEGACY-RESERVE-CARVEOUT-001`, `TICKET-LEGACY-SEQUENCE-COLLISION-001`,
  and `ESCROW-LEGACY-XRP-DELETE-EDGE-001` did not produce a clean current-tag
  Moby Dick repro. They remain source-review / older-tag hardening candidates,
  not live packet promotions.
- The trustline reserve candidate remains the only live legacy-core target
  worth hardening. The earliest source hit in local git history for the reserve
  carveout shape is `e416ee72c` (`Rearrange sources (#4997)`), which is a
  lineage anchor rather than an exact introduction proof.
- The `rippled` `2.5.0` historical tag is now a buildable anchor in this
  environment, and the legacy `SetTrust` and `Offer` suites both pass on that
  binary (`30 cases / 1534 tests` and `8 suites / 415 cases / 55237 tests`,
  respectively). That strengthens the older-tag hardening path, but it is not
  itself a tag-specific bug repro for `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`.
- A temporary 0.12.0 worktree boot confirmed the historical binary-hardening
  path is blocked here by Python 2-era SCons assumptions (`commands`,
  `platform.linux_distribution`), missing `protoc`, and missing Boost headers.
  That is an environment limit, not a packet promotion.
- The OFAC XRP address milestone was refreshed from official OFAC SLS data and
  direct XRPL RPC. The current SDN advanced extraction still returns exactly one
  XRP address, `rnXyVQzgxZe7TR1EPzTkGj2jxH4LMJYh66`. Direct account-state
  checks still show `requireDestinationTag=true`, `disableMasterKey=true`,
  four owned objects, three account lines, no current offers, and no current
  payment channels. The direct `account_tx` scan found 18 own-signed and 37
  counterparty-signed transactions at/after `2021-11-08T00:00:00Z`, with the
  address visible in every returned tx or metadata object. No OFAC-specific P0
  was promoted.
- The concrete OFAC deep-freeze/metadata branch was source-killed by focused
  static/history review and upstream suites. The focused suite set was
  `Freeze`, `Clawback`, `AMMClawback`, `NFTokenAuth`, `AccountLines`,
  `AccountObjects`, `AccountTx`, and `AccountOffers`; it passed with 9 suites,
  185 cases, and 23,586 tests. No deep-freeze bypass, metadata-hiding witness,
  unauthorized clawback/AMMClawback mutation, NFT settlement bypass, or RPC
  visibility failure was isolated.
- `MPT-DELIVERED-AMOUNT-RPC-SWEEP-001` source-killed the live
  `fixMPTDeliveredAmount` metadata/RPC branch. Static/history review plus
  `DeliveredAmount`, `MPToken`, `Flow`, `PayStrand`, `Path`, `AccountTx`,
  `LedgerRPC`, `Transaction`, and `TransactionEntry` passed with 11 suites, 165
  cases, and 38,008 tests. No direct MPT transfer, partial-payment,
  transfer-rate, ledger JSON, `tx`, `transaction_entry`, or `account_tx`
  metadata witness was isolated.

## Still-Promoted Unfixed Set

The packet now has fourteen live-enabled findings with no confirmed upstream fix in
the checked `3.2.0-b7` or `origin/develop` refs:

- `MPT-TRANSFER-RATE-OVERFLOW-001`
- `MPT-LOCK-UNAUTH-001`
- `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`
- `TRUSTLINE-DISALLOW-INCOMING-OFFER-001`
- `NFTOKEN-DISALLOW-INCOMING-ACCEPT-001`
- `NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001`
- `CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001`
- `TOKENESCROW-DISALLOW-INCOMING-FINISH-001`
- `AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001`
- `AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001`
- `AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001`
- `AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001`
- `AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001`
- `AMMBID-DEPOSITAUTH-REFUND-001`

## Next Hunt Targets

No explicit remaining current-tag live-feature target is listed in this lane.
The next pass should pivot only if a new source signal appears, or return to
the whip's legacy-core source-signal queue instead of forcing another bridge or
Credentials variant without a new lead.
