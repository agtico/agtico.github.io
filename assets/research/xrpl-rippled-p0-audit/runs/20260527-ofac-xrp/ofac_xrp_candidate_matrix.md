# OFAC XRP Address Candidate Matrix

Date: 2026-05-27, refreshed 2026-05-28

Scope: live-mainnet XRPL surfaces that can touch the current OFAC XRP
address set. This matrix is triage only. No public article change is
authorized in this lane.

## Direct Scope Evidence

- Official source: OFAC SDN advanced XML,
  `https://www.treasury.gov/ofac/downloads/sanctions/1.0/sdn_advanced.xml`
- Retrieval UTC: `2026-05-27T16:51:41Z`
- XML SHA-256:
  `a03f91c7eae8dc14891e01cf0e161095d250cc778008aad45a3ba77c230bdf82`
- Extracted XRP addresses: `rnXyVQzgxZe7TR1EPzTkGj2jxH4LMJYh66`
- Extracted XRP subset SHA-256:
  `d0ea17d9a345cedade07bbe6e6159e19da4467e7c3686d7d5b467ca805697405`
- Live XRPL account-state snapshot SHA-256:
  `b1725a8204882a8de3f9eaef3d2fd53ccfd79dfea1760cf8000552b020f5dcbb`

2026-05-28 refresh:

- Official source: OFAC SLS `SDN_ADVANCED.ZIP`,
  `https://sanctionslistservice.ofac.treas.gov/api/download/sdn_advanced.zip`
- Retrieval UTC: `2026-05-28T10:49:48Z`
- Download SHA-256:
  `de372a10ff9d1c3c4b8689b7f0b51a8a4ae3bd42c7129a1274a2f822b3b486a2`
- XML member SHA-256:
  `802ca279d4ec173dfd86b8c4cbefc8067d5354cd58fb5fc1f0aab1cca2eda818`
- Extracted XRP addresses: `rnXyVQzgxZe7TR1EPzTkGj2jxH4LMJYh66`
- Extracted XRP subset SHA-256:
  `6e2bad2102facc738f5aa55aa2526a8addb17cb23d0b78d22c193379ef38d343`
- SDN snapshot file SHA-256:
  `6713a69f2f3de145e357e39e954136db3b13387e34dcef42fc8907591872721b`
- Live XRPL scope snapshot SHA-256:
  `9a04f5492d3bb99832172bc8173553f940fd887ef5f78acb381171f8178eb675`
- Activity scan SHA-256:
  `bab6cb82c1943a4b82f546921b7a45a18e0713dd7a9a8a5fff6f468368a7bb73`

The refresh found the same single XRP SDN address. Direct XRPL JSON-RPC against
`s1.ripple.com` and `s2.ripple.com` showed `rippled_version=3.1.3`, the account
still has `requireDestinationTag=true` and `disableMasterKey=true`, owns four
objects, has three account lines, and has no current offers or payment
channels. The refreshed `account_tx` scan still found 18 own-signed and 37
counterparty-signed transactions at/after `2021-11-08T00:00:00Z`; all returned
transactions visibly contain the address in tx or metadata, and counterparty
successful fee burn remains `5.086555` XRP.

Direct XRPL JSON-RPC against `s1.ripple.com` and `s2.ripple.com` showed:

- server runtime: `rippled_version=3.1.3`;
- account flags: `requireDestinationTag=true`, `disableMasterKey=true`;
- account flags not set: `depositAuth`, `disallowIncomingXRP`,
  `disallowIncomingNFTokenOffer`, `disallowIncomingCheck`,
  `disallowIncomingPayChan`, `disallowIncomingTrustline`,
  `allowTrustLineClawback`, `globalFreeze`, `noFreeze`;
- current owned objects: three `RippleState` objects and one `SignerList`;
- current lines: three zero-balance IOU trustlines, all `freeze_peer=true`,
  one also `deep_freeze_peer=true`;
- current offers/channels: none in the checked account objects.

## Candidate Matrix

| ID | Candidate path | Affected files/functions | Live surface | Repro status | Disposition |
|---|---|---|---|---|---|
| OFAC-PAYMENT-RTAG-001 | Direct `Payment` into the address without `DestinationTag` despite `lsfRequireDestTag` | `Payment::preclaim`, `src/xrpld/app/tx/detail/Payment.cpp` | Baseline live | Source-killed by current preclaim check | Demoted: current code rejects established destinations with `lsfRequireDestTag` and no `sfDestinationTag` as `tecDST_TAG_NEEDED`. |
| OFAC-CHECK-RTAG-001 | `CheckCreate` or `CheckCash` deposits into the address without the stored required destination tag | `CreateCheck::preclaim`, `CashCheck::preclaim` | `Checks` live | Source-killed | Demoted: create-time and cash-time checks enforce `lsfRequireDestTag`; `CashCheck` rejects checks whose ledger object lacks `sfDestinationTag`. |
| OFAC-ESCROW-RTAG-001 | `EscrowCreate` funds escrow to the address without destination tag | `Escrow::preclaim` | `Escrow` live | Source-killed | Demoted: `EscrowCreate` checks destination account flags and requires `sfDestinationTag` when `lsfRequireDestTag` is set. |
| OFAC-PAYCHAN-RTAG-001 | `PaymentChannelCreate` creates a channel to the address without destination tag | `PayChan::preclaim` | `PayChan` live | Source-killed | Demoted: channel creation checks `lsfRequireDestTag` and requires `sfDestinationTag`. |
| OFAC-ACCOUNTDELETE-RTAG-001 | `AccountDelete` sends remaining XRP to the address without destination tag | `DeleteAccount::preclaim`, `DeleteAccount::doApply` | Baseline live | Source-killed | Demoted: delete preclaim checks `lsfRequireDestTag`; doApply also runs deposit-auth handling. |
| OFAC-XCHAIN-RTAG-001 | XChain claim/account-create paths bypass destination tag | `XChainBridge.cpp` destination checks | Not live | Excluded | Disabled by direct live feature status: `XChainBridge=false`. Code still contains destination-tag checks, but the surface is not live-mainnet scope. |
| OFAC-VAULT-LOAN-RTAG-001 | Vault or lending withdrawal/cover paths bypass destination tag | `VaultWithdraw`, `LoanBrokerCoverWithdraw` | Not live | Excluded | Disabled by direct live feature status: `SingleAssetVault=false`, `LendingProtocol=false`. Historical destination-tag fixes in these branches are not live-mainnet candidates. |
| OFAC-NFT-DESTINATION-001 | NFT sell offer or mint-with-offer targets the address with `sfDestination` but no destination tag field | `NFTokenUtils::tokenOfferCreatePreclaim`, `NFTokenMint`, `NFTokenCreateOffer` | NFT features live | Source-demoted | Not a P0 under current severity bar: NFT offer `Destination` is an offer restriction, not a payment deposit. The live control is `asfDisallowIncomingNFTokenOffer`; the checked account has that flag unset. Existing tests cover rejection when that flag is set. |
| OFAC-NFT-IOU-AUTH-FREEZE-001 | NFT accept/broker paths pay IOU through frozen or unauthorized trustlines involving the address | `NFTokenAcceptOffer::preclaim`, `nft::checkTrustlineAuthorized`, `nft::checkTrustlineDeepFrozen` | `NonFungibleTokensV1_1`, `fixEnforceNFTokenTrustlineV2` live | Source-demoted | No promoted repro in this slice. Current code checks buyer/seller/broker/issuer trustline authorization and deep-freeze paths for IOU NFT settlement under `fixEnforceNFTokenTrustlineV2`. |
| OFAC-TRUSTLINE-DISALLOW-OFFER-001 | Issuer `asfDisallowIncomingTrustline` bypass through `OfferCreate` could create an incoming trustline for a blocked account | `CreateOffer::checkAcceptAsset` / `OfferCreate` crossing | `DisallowIncoming` and `fixDisallowIncomingV1` live; proposed `fixDisallowIncomingV1_1` not live | Already promoted as general packet finding | This is already represented by `TRUSTLINE-DISALLOW-INCOMING-OFFER-001`. It is not OFAC-address-specific because the checked OFAC account has `disallowIncomingTrustline=false`; the root cause applies to any issuer that sets the flag. |
| OFAC-TRUSTLINE-RESERVE-OFFER-001 | Offer crossing can create positive IOU balance without receiver reserve, including for a screened address | `OfferCreate` crossing / trustline owner-count accounting | Baseline live | Already promoted as general packet finding | Already represented by `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`. It is address-relevant only as a generic account-touching offer-crossing risk; it does not depend on OFAC status. |
| OFAC-FROZEN-LINES-001 | Frozen zero-balance lines to the address can be used by `Payment`, `OfferCreate`, AMM, or clawback paths to bypass issuer freeze/deep-freeze intent | `Payment`, `CreateOffer`, `AMM*`, `NFTokenAcceptOffer`, `Clawback`, `AMMClawback` | Payment/Offer/AMM/NFT/AMMClawback live | Source/upstream-test reviewed | Demoted as protocol semantics. Existing `Freeze_test` coverage intentionally allows a holder whose line is regular-frozen by the issuer to receive IOU payments and NFT-sale proceeds; deep freeze is the inbound-blocking state. The checked address has two regular peer-frozen zero-balance lines and one deep peer-frozen zero-balance line. No deterministic local repro showed a deep-freeze bypass, unauthorized transfer, or hidden exposure. |
| OFAC-AMMCLAWBACK-NOFREEZE-001 | AMM clawback allowed when issuer lacks `asfAllowTrustLineClawback` or has `asfNoFreeze` | `AMMClawback::preclaim` | `AMMClawback` live | Source-killed | Demoted: current 3.1.3 code returns `tecNO_PERMISSION` when `lsfAllowTrustLineClawback` is absent or `lsfNoFreeze` is set. |
| OFAC-METADATA-HIDING-001 | RPC metadata hides the sanctioned account behind owner directories, deleted nodes, destination tags, or signer lists | `account_info`, `account_objects`, `account_lines`, `account_channels`, `account_tx` RPC surfaces | Baseline live | Direct live RPC and activity scan checked | No promoted bug. Direct RPC exposed `AccountRoot`, `SignerList`, and `RippleState` objects for the sanctioned account; no offers/channels were present in the checked snapshot. `ofac_xrp_activity_scan_20260527.json` scanned `account_tx` at/after `2021-11-08T00:00:00Z` and every returned transaction visibly contained the address in tx or metadata. Continue only with a concrete transaction-metadata repro outside this observed account_tx set. |
| OFAC-FEE-BURN-001 | Counterparties can pay and burn XRPL fees for transactions that reference or mutate objects involving the sanctioned address | `account_tx` observation; normal transaction fee burn | Baseline live | Direct activity scan checked | Demoted from P0: this is real but expected XRPL mechanics. The scan saw 37 counterparty-signed successful transactions at/after `2021-11-08T00:00:00Z` involving the address, with `5.086555` XRP in counterparty-paid successful fees burned. Fee payer was the counterparty, not the sanctioned account, and the address was visible in the returned tx/meta. |

## Result

No new OFAC-specific finding was promoted in this slice.

The pass did confirm that the sanctioned account is live, requires destination
tags, has disabled its master key, owns a signer list, and has three frozen
zero-balance IOU trustlines. The two offer-crossing trustline findings already
in the packet remain the relevant live address-touching root causes, but neither
depends on the OFAC account specifically.

The activity scan also confirms a narrow fee-burn fact: after
`2021-11-08T00:00:00Z`, counterparties submitted successful transactions that
referenced or mutated ledger objects involving the sanctioned address, and those
counterparties' fees were burned by normal XRPL rules. That is not the same as
the sanctioned account paying fees.

The regular-freeze result is compliance-relevant: ordinary issuer freeze should
not be treated as equivalent to a full inbound block. A P0 promotion would need
a stronger claim, such as deep-freeze bypass, metadata hiding, or unauthorized
state mutation on a live-enabled path.

Next OFAC-specific work should focus only on concrete metadata or deep-freeze
path repros. Compliance-tooling observations are not enough for
`live-unfixed-p0` without unauthorized state mutation, consensus-visible
failure, or a clean policy-bypass repro on a live-enabled path.
