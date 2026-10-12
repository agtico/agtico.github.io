---
layout: report
title: "Engaging with PRL | Pearl, Five Months In"
date: "2026-10-12 09:00:00 +0000"
summary: "AGTI follow-up for market makers: May's Pearl claims checked against code and chain, the FP8 hard fork that redraws which GPUs can mine, ASIC resistance then and now, and a GPU-by-GPU mining economics deep dive for GPU-backed and mining-yield products."
category: AGTI Research
pearl_report: true
report_css_version: 20260524m
tags:
  - AGTI
  - Pearl
  - Proof of Work
  - Mining Economics
  - Market Structure
---

<div class="pearl-hero-grid">
  <div class="pearl-scorecard good">
    <span class="label">Network</span>
    <span class="value">47.6 EH/s</span>
    <span class="hint">Implied hashrate, 4.6× May 23. Difficulty 34.5M (4.6M then). Block 126,659.</span>
  </div>
  <div class="pearl-scorecard warn">
    <span class="label">FP8 hard fork</span>
    <span class="value">Hopper + Blackwell only</span>
    <span class="hint">On the fp8-scheme branch, after activation only FP8 certificates are valid, and the verifier replays only Hopper and Blackwell arithmetic. RTX 30/40, A100, L40S and MI300X lose native mining.</span>
  </div>
  <div class="pearl-scorecard bad">
    <span class="label">Useful work</span>
    <span class="value">None measured</span>
    <span class="hint">The only mined API endpoint is offline. Pearl's own API sells upstream models at market prices, with no mining link.</span>
  </div>
</div>

<div class="pearl-verdict-banner">
  <strong>AGTI bottom line for a market maker</strong>
  <p>PRL is a GPU-mined asset worth <strong>$488M</strong> that adds <strong>89% to its supply over the next twelve months</strong>. It trades on two small exchanges with <strong>under $70K of bids within 2%</strong>. One pool mines <strong>52% of blocks</strong>, and a pending hard fork decides which GPUs earn. <strong>Under today's int7 economics, among the GPUs FP8 keeps, only the RTX 5090 clears its full cost as a new purchase</strong>: $0.56 on a static 36-month basis, against $1.43. A 24-month payback, once emission decay is counted, needs PRL at $1.07 even with flat hashrate. Hopper and Blackwell earn their keep only as <strong>capacity already owned</strong> or as dual-use inference fleets. A GPU-backed or mining-yield product should pay <strong>PRL over a fixed term</strong>, and should launch only after the <a href="#launch-gates">five gates in §9</a> are met.</p>
</div>

## 0. What this follow-up does

Our [May 23 report](/intelligence-reports/2026/05/23/pearl-pouw-useful-work-asic-analysis/) asked whether Pearl's proof of useful work is useful. This one takes the view of a market maker launching retail access to PRL mining. That means one of two products: a token backed by GPUs that mine PRL, or a token that strips out the mining yield alone. Section 1 updates every May claim against the code and the chain. Sections 2–7 cover the network, the code, FP8, ASIC resistance, useful work and the market. Sections 8–10 are the economics, the product design and the catalysts.

Every number here comes from a public source or from reproducible scripts in [`/assets/research/pearl-update/`](https://agtico.github.io/assets/research/pearl-update/README.md). Data was observed **12 October 2026**.

## 1. The May claims, updated

| # | May 23 claim | Status | October evidence |
|---|---|---|---|
| 1 | Consensus mines int7×int7→int32 noisy GEMM | **Holds on mainnet** | Mainnet still runs the integer scheme (V3 certificates since the August hardfork). Its FP8 replacement sits on a branch ([PR #311](https://github.com/pearl-research-labs/pearl/pull/311), open). |
| 2 | Mining needs Hopper GPUs (sm_90a) | **Reversed for bare mining** | Pearl's [own kernels](https://github.com/pearl-research-labs/pearl/blob/master/miner/pearl-gemm/setup.py) still build only for `sm_90a`. Since June the open-source [akoya miner](https://github.com/akoyapool/akoya-miner) mines on every NVIDIA card from sm_70 up (RTX 20–50, T4, A100, L40S, H100/H200, B200) and on AMD MI300X. |
| 3 | Consensus never checks useful work | **Holds** | Five consensus changes since May (§3) close cheap proof shortcuts. None adds a usefulness check. The FP8 spec states that a miner who drops "the usability requirement" may gain a speedup. |
| 4 | Three public `pearl-ai` checkpoints | **Updated** | Four: [Qwen3-30B-A3B](https://huggingface.co/pearl-ai/Qwen3-30B-A3B-Instruct-2507-pearl) was added June 11. It is a mixture-of-experts model, and mainnet has rejected MoE certificates since July 23 ([#260](https://github.com/pearl-research-labs/pearl/pull/260)). |
| 5 | Together AI is the one commercial outlet | **Weaker** | [Together's page](https://www.together.ai/models/gemma-4-31b-it-pearl) now says the model "is not available on Together's Serverless API". [Pearl's network page](https://pearlresearch.ai/network) still calls it live. Hugging Face shows the provider mapping as `error`. |
| 6 | Pearl inference costs 2.3× market | **Updated** | Pearl now runs its own [seven-model API](https://pearlresearch.ai/pricing), priced at 0.95–3.16× the cheapest OpenRouter provider (§6). The models are upstream IDs, and nothing links them to mining. |
| 7 | ~59K blocks, difficulty ~4.9M | **Updated** | 126,659 blocks, difficulty 34.5M, 47.6 EH/s implied (§2). |
| 8 | Mining-layer threshold m, n, k ≥ 1024 | **Updated** | Since June 22, `min_n` is 256 ([#207](https://github.com/pearl-research-labs/pearl/pull/207)), so more layers qualify. |
| 9 | Bare and ASIC mining are structurally favoured | **Holds (GPU), open (ASIC)** | [Basu](https://arxiv.org/abs/2606.04819) measured **zero useful AI computation** in late May. No ASIC product exists. Consumer GPUs carry the economics (§8). |
| 10 | Absent inference demand, the useful share collapses | **Consistent** | Our May simulation predicted it, and Basu's census measured it. |

## 2. The network five months in

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Implied hashrate, genesis to today</h3>
    <span class="tag"><a href="https://agtico.github.io/assets/research/pearl-update/economics.py">economics.py</a></span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/hashrate.png?v=20261012" alt="Pearl implied network hashrate and block time from genesis to October 2026, with fork and release markers" loading="lazy" /></div>
  <p class="pearl-figure-caption">Pearl's difficulty is not in hash units. We imply hashrate as k × difficulty / block time, calibrated to the 43.8–51.3 EH/s that Kryptex and Hashrate.no published on 12 October. Calibrated only to today, the model returns 22.6 EH/s for 30 May, against the 24 EH/s Basu measured that week.</p>
</div>

Hashrate rose from **10.3 EH/s on May 23** to a **57.3 EH/s peak on July 21**. It then fell by half twice:

- **July 23 dense-only fork.** Hashrate dropped to about 23 EH/s.
- **August 11 salted-seed hardfork.** It dropped to about 20 EH/s.

Both forks removed proof modes that were cheaper than honest work (§3). A fall in difficulty-implied hashrate at those dates is what removing a shortcut looks like. Hashrate recovered with price through September, reached 53.3 EH/s on September 28, and stands at **47.6 EH/s**. Blocks average 198 seconds against the 194-second target.

**Mining is concentrated.** [Pearlchain's](https://pearlchain.live/mining) coinbase attribution for the last 24 hours gives:

| Pool | Share of blocks | Blocks |
|---|---|---|
| Kryptex | **51.9%** | 227 |
| PearlHash | 25.6% | 112 |
| PearlFortune | 5.7% | |
| HeroMiners | 4.8% | |
| LuckyPool | 4.1% | |
| F2Pool | 0.7% | |
| AlphaPool | 0.2% | |
| Unattributed | 6.9% | |

One pool above half of block production can reorder recent history at will. The top two hold 77.5%.

## 3. Code since May: the lottery got harder to game

<div class="pearl-figure" markdown="1">
  <div class="pearl-figure-head">
    <h3>Consensus and security changes, May → October</h3>
    <span class="tag">pearl-research-labs/pearl</span>
  </div>

| Date | Change | Type | What it closes |
|---|---|---|---|
| Jun 22 | [#207](https://github.com/pearl-research-labs/pearl/pull/207) Hadamard-quantized models | Miner | Mining-layer threshold drops to n ≥ 256 |
| Jul 23 | [#260](https://github.com/pearl-research-labs/pearl/pull/260) Dense-only softfork (mainnet ~91,600) | Consensus | Rejects MoE certificates |
| Aug 5 | [#275](https://github.com/pearl-research-labs/pearl/pull/275) Rank-penalty softfork | Consensus | Noise rank ≥ 128, with the jackpot bound scaled by 128/rank. Removes the large-rank discount. |
| Aug 11 | [#280](https://github.com/pearl-research-labs/pearl/pull/280) Salted noise-seed hardfork (98,900) | Consensus | V3 certificates. Domain-separated seeds bound to the declared dimensions. |
| Aug 13 | [#281](https://github.com/pearl-research-labs/pearl/pull/281) Merkle leaf-count pinning | Consensus | "Carousel" reuse of one committed tree under several dimensions |
| Aug 30 | [#299](https://github.com/pearl-research-labs/pearl/pull/299) Dimension-narrowing fix | Security | Wraparound that reopened work amplification (GHSA-vq6h-p843-m7c8) |
| Sep–Oct | Checkpoints at 100K and 120K, DNS seeds moved to pearlnetwork.net, bounty tiers | Ops | Faster syncing, less dependence on Pearl's own domain, paid disclosure |

</div>

The team ships consensus changes about every two weeks and pays for disclosures. Every fix targets a cheap shortcut through the lottery. None enforces usefulness: a bare `mine()` loop over random matrices still produces valid blocks.

<div class="pearl-code-ref"><span class="path"><a href="https://github.com/pearl-research-labs/pearl/blob/master/zk-pow/src/api/sanity_checks.rs">zk-pow/src/api/sanity_checks.rs</a> — rank-penalty rule (#275)</span>
PENALTY_BASE_RANK = 128      // noise rank must be ≥ 128
jackpot bound × 128 / rank   // mining at a larger rank no longer buys an easier target
</div>

## 4. FP8: what the update means

The [Pearl Floating Point Scheme Specification](https://pearlresearch.ai/Pearl_Whitepaper.pdf) (September 2026) replaces the integer lottery with **FP8 (E4M3) matrix multiplication reproduced bit-exactly as each NVIDIA architecture computes it**. The spec pins down how each chip accumulates:

- **Accumulator precision:** a 13-bit mantissa on Hopper, 25-bit on Blackwell.
- **Grouping:** summation in groups of 32.
- **Rounding:** toward zero.

Operands are committed in an FP10 format and perturbed with low-rank noise. A jackpot policy then rejects cheap inputs, using a noise-floor check, an unpredictable-summands check and a tamed-products check. Verification uses the [Hawkeye](https://arxiv.org/abs/2603.20421) method (MLSys 2026) for reproducing GPU arithmetic.

**Status.** The code lives on the `fp8-scheme` branch. About 25 merged changes since September 9 add:

- Hopper support ([#337](https://github.com/pearl-research-labs/pearl/pull/337)) and consumer Blackwell ([#342](https://github.com/pearl-research-labs/pearl/pull/342));
- MoE support ([#343](https://github.com/pearl-research-labs/pearl/pull/343));
- an end-to-end B200 test that mines a block on a local test chain ([#344](https://github.com/pearl-research-labs/pearl/pull/344)).

The merge into mainline ([#311](https://github.com/pearl-research-labs/pearl/pull/311)) is open. Activation is configured only for local test networks. Co-founder Omri Weinstein said on September 25 that Pearl-GEMM was in testnet and "expected within weeks". No public FP8 testnet is visible.

The founders' September 15 AMA tied activation to implementation and security confidence, and confirmed continued support for the RTX 5090 and RTX PRO 6000. On October 7, [Kryptex](https://pool.kryptex.com/ja/articles/pearl-fp8-fork-en), the largest pool, said no activation block is set. It backs the fork and told owners of incompatible cards to price exclusion into their payback. No grace period or migration programme has been announced.

<div class="pearl-code-ref"><span class="path"><a href="https://github.com/pearl-research-labs/pearl/blob/fp8-scheme/node/blockchain/validate.go">fp8-scheme: node/blockchain/validate.go</a> — certificate rules</span>
At and after SaltedSeedForkHeight (hardfork): only V3 accepted
At and after Fp8ForkHeight (hardfork):       only V4 accepted
</div>

<div class="pearl-code-ref"><span class="path"><a href="https://github.com/pearl-research-labs/pearl/blob/fp8-scheme/miner/miner-base/src/miner_base/devices.py">fp8-scheme: miner/miner-base/src/miner_base/devices.py</a> — committed mining devices</span>
SM90  (H100/H200)                -> Device.HOPPER
SM100 (B200/B300)                -> Device.BLACKWELL
SM120 (RTX 50, RTX PRO 6000)     -> Device.BLACKWELL
every other capability           -> ValueError("no committed mining Device")
</div>

**What FP8 means, in four consequences:**

1. **The hardware map is redrawn overnight.** The fork is a hard fork that replaces int7. The V4 verifier contains arithmetic circuits for only two devices, Hopper (`matmul_h100`) and Blackwell (`matmul_b200_stark`). A valid certificate must reproduce one of them bit for bit. RTX 50-series, H100/H200 and B200 do that natively. RTX 30 and 40-series, A100, L40S and MI300X could only emulate it, at a fraction of native speed, so in practice they leave. A separate open proposal, [#369](https://github.com/pearl-research-labs/pearl/pull/369), would add an A100 device through an FP16 scheme; its consensus activation is disabled.
2. **Eligible miners get a revenue step on day one.** Issuance is fixed per block, so stranded hashrate leaves the same PRL to fewer GPUs (chart below). The step could be large. In Basu's May census of 8,012 workers, datacenter Hopper and Blackwell were about 3.6% of workers and RTX 5090-class cards 6.1%. Most workers sat in the 40-series and mixed brackets that the fork strands. Worker counts are not hashrate, so the stranded share has not been measured. The step also splits unevenly: the RTX 5090's FP8 tensor rate with FP32 accumulation is half its INT8 rate (419 vs 838 per [NVIDIA](https://pausehardware.com/wp-content/uploads/2025/01/nvidia-rtx-blackwell-gpu-architecture.pdf)), a limit datacenter Hopper and Blackwell do not share. Consumer Blackwell may keep its seat at the table but lose half its relative speed.
3. **The useful-work story becomes credible.** Mining runs the same class of FP8 kernel as production inference. The miner recovers the useful product "at FP8-level accuracy" with 1 + o(1) overhead. Models still need Pearl's quantization path (`--quantization pearl`), and consensus still does not check usefulness.
4. **Inference fleets become competitors.** A dual-use H100 or B200 cluster mines at near-zero marginal cost. Once FP8 makes dual-use real, those fleets compress margins for pure miners.

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>FP8 activation, day one: eligible GPUs split the same issuance</h3>
    <span class="tag"><a href="https://agtico.github.io/assets/research/pearl-update/gpu_economics.py">gpu_economics.py</a></span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/fp8_fork_scenarios.png?v=20261012" alt="Gross daily revenue for RTX 5090, H100 and B200 as the eligible share of hashrate falls after the FP8 fork" loading="lazy" /></div>
  <p class="pearl-figure-caption">Price and issuance are held fixed, and FP8 per-GPU throughput is assumed equal to today's int7 benchmark. If half of today's hashrate is Hopper or Blackwell, an RTX 5090's gross revenue doubles from $12 to $24 a day, before any FP8 slowdown on consumer cards.</p>
</div>

## 5. ASIC resistance, then and now

The integer whitepaper claims that Pearl "plausibly achieves ASIC-resistance by an economic argument", because matrix multiplication "is the native language of all modern AI accelerators" ([§ASIC](https://pearlresearch.ai/research/int-whitepaper)). The FP8 specification makes a narrower claim. It defends against cheap specialised circuits and accepts efficient silicon:

> "Operands with so little local entropy invite specialized hardware: a circuit built for a reduced alphabet of values can multiply faster than a general FP8 kernel… The noise-floor check removes this option."
>
> "A miner with a faster bit-exact implementation of MatMul_Device, through better scheduling or better silicon, outpaces other miners, but this is the same advantage an honest miner gains and is not an attack on the protocol."

| Threat | int7 mainnet today | FP8 hard fork |
|---|---|---|
| A cheap integer ASIC (int7 MAC array + BLAKE3) | **Most exposed.** The alphabet is 129 integer values and the arithmetic is exact. | Removed: the arithmetic is device-exact floating point |
| Reduced-alphabet circuits | Inherent to int7 | Blocked by the noise-floor check |
| Skipping summands that round away | Not applicable | Capped by the unpredictable-summands check |
| Efficient general silicon | Allowed | Allowed. The spec calls it "not an attack". |
| Who can mine | Anyone (Hopper-only applies just to Pearl's own kernels) | Whoever has a consensus `Device` entry: today NVIDIA Hopper and Blackwell |

Five and a half months in, no Pearl ASIC or FPGA product has been announced. The integer lottery is the easier ASIC target, and consumer GPUs already mine it at a profit, which keeps the reward for building a chip low. FP8 makes the competing chip an NVIDIA-compatible FP8 tensor core, whose rival is the whole AI accelerator industry. That turns the economic argument from rhetoric into something credible.

The new lever is governance. The `Device` list decides which silicon is allowed to mine.

## 6. Useful work: the evidence

- **Measured usefulness: zero.** [Basu](https://arxiv.org/abs/2606.04819) studied the network in late May:
  - 8,012 workers in the largest pool;
  - the dominant mining software has **no inference code**;
  - the pool accepted 44 shares built from random matrices;
  - the 24 EH/s network (about 320,000 GPU-equivalents, about 112 MW) "produces zero useful AI computation";
  - budget GPU rental prices rose 38% after the miner was released.
- **The mined endpoint is offline.** Together's page says the Pearl Gemma model is unavailable, and Hugging Face's provider mapping reads `error`.
- **Download growth is slow.** Llama-3.3-70B-pearl has 151K all-time downloads, 5.2K in the last 30 days. Gemma-4-31B-pearl had 193 in the last 30 days.
- **Pearl's own API is a conventional inference business.** It sells seven upstream models ([pricing](https://pearlresearch.ai/pricing), [docs](https://platform.pearlresearch.ai/docs)) with no stated mining link. None of them is a `pearl-ai` mining checkpoint. DeepSeek V4 and GLM-5.3 are MoE models, which mainnet rejects. The API's Gemma serves 256K context, against 32K for the mined variant.

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Pearl Inference Platform vs OpenRouter's cheapest provider</h3>
    <span class="tag"><a href="https://agtico.github.io/assets/research/pearl-update/compare_api_pricing.py">compare_api_pricing.py</a></span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/api_pricing.png?v=20261012" alt="Ratio of Pearl API list price to the cheapest OpenRouter provider for seven models" loading="lazy" /></div>
  <p class="pearl-figure-caption">Pearl matches the market floor on Gemma 4 31B, comes in about 5% under on the nearest Qwen3.8 Flash listing, and runs 1.7–3.2× above it on the other five models. Blend: 75% input, 25% output tokens.</p>
</div>

For network value, this means PRL is priced today on speculation and mining economics. Inference revenue contributes nothing yet. FP8 activation, plus any verifiable link between Pearl's API traffic and mined blocks, is what would change that.

## 7. Supply, market and liquidity

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Supply: front-loaded, no halvings</h3>
    <span class="tag">supply(h) = 2.1B · h / (h + 650,226)</span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/emission.png?v=20261012" alt="Actual and projected PRL mined supply, with the on-schedule comparison" loading="lazy" /></div>
  <p class="pearl-figure-caption">The subsidy (<a href="https://github.com/pearl-research-labs/pearl/blob/master/node/blockchain/validate.go">CalcBlockSubsidy</a>) decays like 1/height². Half of the 2.1B is mined after four years of 194-second blocks.</p>
</div>

- **Front-loading.** Mainnet launched at minimal difficulty and ran blocks every few seconds while difficulty climbed. The chain is **51,829 blocks ahead of schedule**, and **126M PRL** (37% of today's supply) was mined early. The whitepaper says low initial difficulty was chosen to give early miners a fair opportunity and "to ensure the core team has minimal resources needed". There is no premine, and the team mined.
- **Inflation ahead.** Supply grows from 342M to **647M over the next twelve months (+89%)**. The block reward falls from 2,262 to 1,547 PRL.
- **Supply data disagrees.** CoinGecko's 342.4M circulating matches the chain exactly. CoinMarketCap shows a self-reported 225.8M.

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Price and volume since CoinGecko coverage</h3>
    <span class="tag">CoinGecko pearl-2</span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/market.png?v=20261012" alt="PRL daily price and volume from June to October 2026" loading="lazy" /></div>
  <p class="pearl-figure-caption">$1.43 today: market cap $488M, FDV $3.0B, 24-hour volume $3.9M. Intraday all-time low $0.14 (July 23, the day of the dense-only fork); all-time high $1.76 (September 23). Listed on BigONE (from June 25) and SafeTrade.</p>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Miner issuance vs traded volume</h3>
    <span class="tag">supply overhang</span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/issuance_vs_volume.png?v=20261012" alt="Daily new PRL issuance in dollars versus daily exchange volume" loading="lazy" /></div>
  <p class="pearl-figure-caption">Miners receive about 990,000 PRL ($1.41M) a day, equal to 35% of exchange volume. In July and August issuance exceeded volume on many days.</p>
</div>

**Liquidity and hedges** (CoinGecko depth snapshot):

| Venue | +2% asks | −2% bids |
|---|---|---|
| BigONE PRL/USDT | $852 | $6,855 |
| SafeTrade PRL/USDT | $148,439 | $56,023 |
| SafeTrade PRL/USDC | $84 | $540 |

Derivatives barely exist:
- **Margin Trade** lists a native-PRL 3× perpetual with $6K daily volume and $3K open interest ([margin.trade](https://margin.trade/trade/PRL)).
- **Lighter** has officially announced 3× PRL perpetuals on Lighter Core ([announcements](https://t.me/s/lighter_announcements)). A third-party snapshot showed $500K of open interest on October 3. Executable depth is untested.
- **Hyperliquid's** `io:PRL` market, deployed by Entropy (ticker bought for 500 HYPE on September 26), is marked delisted, with $0 volume and $0 open interest ([OAK](https://oakresearch.io/en/hyperliquid/hip-3/markets/io:PRL)).

CoinEx announced a PRL listing on June 24. CoinGecko's tickers today show only BigONE and SafeTrade.

OTC runs through [pearl-otc.com](https://pearl-otc.com/): 2-of-2 escrow, 2% fee, settled in Arbitrum USDC. Ticker collisions (MEXC's "PRL" is Perle) inflate apparent options.

## 8. Mining economics deep dive

Today's network pays **0.0208 PRL per TH/s per day** before pool fees. The table nets a 2% pool fee. Profitability depends on the cost basis, and a fair comparison needs all three:

- **Running cost only:** power, plus a hosted slot for datacenter cards. This is the marginal cost of a card that's already paid for and would otherwise sit idle.
- **Full owned cost:** the purchase price spread over 36 months, plus running cost.
- **Rent:** what the GPU costs to rent on the market.

Purchase prices are quoted used or refurbished prices (sources in the caption). Consumer rigs are hosted all-in at $0.09/kWh. Datacenter cards sit in a $95–240/month hosted slot, plus power at $0.07/kWh with 1.3× facility overhead.

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Breakeven PRL price by GPU, three cost bases</h3>
    <span class="tag"><a href="https://agtico.github.io/assets/research/pearl-update/gpu_economics.py">gpu_economics.py</a></span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/gpu_margins.png?v=20261012b" alt="Breakeven PRL price per GPU under running-cost, full owned cost and rental cost bases, with today's price" loading="lazy" /></div>
  <p class="pearl-figure-caption">Benchmarks: Oyster, Kryptex, Hashrate.no. Rents: cheapest published rate on 12 Oct (Vast floors, RunPod, TensorDock). Purchase prices: <a href="https://cresvament.com/used-gpu-prices">CresvaMent</a> and <a href="https://bestvaluegpu.com/history/new-and-used-rtx-5090-price-history-and-specs/">BestValueGPU</a> (October), <a href="https://compute.exchange/hardware-market/used-gpus">Compute Exchange</a> (Q3 2026 broker benchmarks), <a href="https://gatewellusa.com/ai-compute/p/amd-instinct-mi300x">Gatewell</a>. Hosting: <a href="https://www.endemine.com/hosting/">Endemine</a>, <a href="https://www.gpurenthub.com/colocation/">GPU Rent Hub</a>. [FP8] marks GPUs that keep mining after the fork.</p>
</div>

| GPU | TH/s | W | Price | PRL/day (net of 2% fee) | Gross $/day | Breakeven: running cost | Breakeven: full owned | Breakeven: rented | After FP8 (native) |
|---|---|---|---|---|---|---|---|---|---|
| RTX 3090 | 132 | 267 | $1,450 | 2.70 | 3.85 | $0.26 | $0.75 | $1.07 | stranded |
| RTX 4090 | 236 | 420 | $3,199 | 4.81 | 6.87 | $0.23 | $0.83 | $0.70 | stranded |
| **RTX 5090** | **412** | **475** | **$3,822** | **8.40** | **12.00** | **$0.15** | **$0.56** | **$1.06** | **mines** |
| L40S | 259 | 350 | $6,500 | 5.28 | 7.54 | $0.74 | $1.86 | $4.95 | stranded |
| A100 80GB | 187 | 300 | $8,250 | 3.81 | 5.45 | $0.99 | $2.96 | $9.44 | stranded (#369 would re-admit) |
| MI300X | 504 | 750 | $18,000 | 10.28 | 14.68 | $0.93 | $2.52 | $5.58 | stranded |
| H100 SXM | 610 | 610 | $24,500 | 12.44 | 17.77 | $0.74 | $2.54 | $4.34 | mines |
| H200 SXM | 742 | 680 | $32,500 | 15.13 | 21.61 | $0.62 | $2.58 | $8.39 | mines |
| B200 | 1,310 | 1,000 | $40,000 | 26.71 | 38.15 | $0.38 | $1.74 | $5.61 | mines |

Three conclusions follow:

1. **The marginal miner is a consumer GPU.** Rented RTX 3090/4090/5090 capacity breaks even at $0.70–1.07. When PRL rises above that, marketplace supply on Vast and Salad starts mining. Hashrate therefore tracks price, and USD yield per consumer GPU mean-reverts toward consumer rental rates.
2. **Datacenter GPUs don't mine at market.** An H100 earns about $18 a day mining, against $54–96 a day renting to AI. At full cost, every datacenter card needs PRL at $1.74–2.58. They mine when idle, or as dual-use capacity once FP8 lands.
3. **Hashrate is sticky on the way down.** Consumer running-cost breakevens of $0.15–0.26 mean owned rigs keep mining through an 80–90% price fall. Difficulty stays high when price falls, which compresses USD yield.

**Payback on a new RTX 5090** (bought at $3,822; 24 months; PRL held at $1.43; int7 rules; no FP8 step and no resale value):

| Network hashrate growth | PRL mined in 24 months | Net of running cost | Versus purchase price | PRL price needed to pay back |
|---|---|---|---|---|
| Flat | 4,408 | $5,396 | **+$1,574** | $1.07 |
| +50% a year | 3,163 | $3,619 | −$203 | $1.49 |
| +150% a year | 2,233 | $2,290 | −$1,532 | $2.11 |

Hashrate grew 4.6× in the five months to October. A new card is therefore a bet on PRL's price, on the size of the FP8 step, and on the card's resale value. Mining yield alone doesn't cover it.

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Forward PRL yield per RTX 5090</h3>
    <span class="tag">the cash flow a mining-yield token pays</span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/forward_yield_5090.png?v=20261012" alt="Projected PRL per RTX 5090 per day over 24 months under hashrate growth scenarios" loading="lazy" /></div>
  <p class="pearl-figure-caption">Emission decay alone halves the block reward over 24 months. Flat hashrate gives about 4,400 PRL per 5090 over two years. Hashrate growth of 150% a year cuts that to about 2,250 PRL. These are int7 rules, before the FP8 step.</p>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>H100 mining revenue history and margin map</h3>
    <span class="tag">datacenter GPUs</span>
  </div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/h100_revenue.png?v=20261012" alt="Gross daily PRL revenue per H100 since June versus power cost and rental cost bands" loading="lazy" /></div>
  <div class="pearl-sim-chart"><img src="/assets/research/pearl-update/charts/h100_margin_heatmap.png?v=20261012" alt="Rented H100 daily margin across PRL price and network hashrate" loading="lazy" /></div>
  <p class="pearl-figure-caption">Benchmark: Oyster's 610 TH/s at 610 W, net of a 2% pool fee. An H100 earned about $5 a day in August and $18 today. A rented H100 breaks even at PRL $4.34 at the cheapest rent ($2.25/hr) or $6.02 at mid-market ($3.12/hr). At today's price, network hashrate would have to fall below 11–16 EH/s.</p>
</div>

## 9. Designing the product: GPU-backed vs stripped mining yield

<div class="pearl-mermaid">
  <div class="mermaid">
flowchart LR
  subgraph ops ["Operator (market maker)"]
    G[FP8-eligible GPUs<br/>RTX 5090 / H100-class] --> P[Pools, split across 3+<br/>Kryptex is 52%]
    P --> R[PRL receipts]
  end
  R --> W[Published waterfall<br/>power, hosting, fees]
  W --> A["Token A: GPU-backed<br/>hardware residual + PRL"]
  W --> B["Token B: stripped yield<br/>PRL per TH/s-day, fixed term"]
  B --> H[Optional hedge sleeve<br/>perp / OTC, sized to depth]
  style G fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style P fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style R fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style W fill:#1a1408,stroke:#ff9f45,color:#e8eeeb
  style A fill:#101a14,stroke:#9dffc8,color:#e8eeeb
  style B fill:#101a14,stroke:#9dffc8,color:#e8eeeb
  style H fill:#1a0c0a,stroke:#ff5a42,color:#e8eeeb
  </div>
</div>

**Token A: GPU-backed.** Holders own a pro-rata share of specific GPUs plus their mining receipts. Value is the hardware's resale price plus PRL mined minus operating cost. This structure survives a PRL crash because the hardware keeps resale and AI-rental value. Its risk is obsolescence, so only FP8-eligible cards belong in it. The two cases differ sharply:

- **Buying new hardware.** Under today's int7 economics, only the **RTX 5090** clears full cost among FP8-eligible cards: breakeven $0.56, and it resells into a deep consumer market. The RTX 3090 and 4090 also clear today, but the FP8 fork strands them. H100, H200 and B200 break even at $1.74–2.58 on full cost, so they are not a new-purchase product at today's price. Even the 5090 pays back inside two years only with flat hashrate (§8).
- **Capacity already owned.** Idle Hopper or Blackwell capacity mines profitably at running-cost breakevens of $0.38–0.74. Dual-use inference fleets after FP8 belong here too. This is a sourcing strategy for an operator, not a retail token.

**Token B: stripped mining yield.** Holders receive the PRL mined by a defined hashrate (TH/s-days) over a fixed term, and the operator keeps the hardware. This is Pearl's version of a hashrate forward or a mining note:

- **Price it in PRL** off the forward curve above. Discount for hashrate growth and treat FP8 activation as a binary step.
- **Pay out in PRL**, not USD. The hedge market cannot support a USD-yield promise: $3K of open interest on the only live perpetual, and under $70K of bids within 2%.

**What the comparables teach:**
- [BTCST](https://www.binance.com/en/support/announcement/detail/35a4169422154c838f012331f5f4b425), a hashrate-backed token, died with its exchange listing.
- [Blockstream's BMN](https://staging.mining.blockstream.com/finance/bmn/) worked as a fixed-term note delivering coins at maturity.
- [Luxor's hashrate forwards](https://docs.luxor.tech/platform/derivatives/getting-started/order-book) cleared over $380M notional in 2025.
- [Compute Labs](https://help.computelabs.ai/en/docs/yield-operations/yield-generation-explained) runs a 90/10 investor/manager revenue waterfall on GPUs.

Fixed terms, coin-denominated payouts and published waterfalls survive. Perpetual, liquidity-dependent wrappers do not.

<div class="pearl-primer-box" id="launch-gates">
  <p><strong>Launch gates.</strong> A GPU-backed or mining-yield PRL product launches when all five hold:</p>
  <ol>
    <li><strong>The FP8 cutover is known.</strong> A mainnet <code>Fp8ForkHeight</code> is published, and the eligible share of hashrate is measured, so the day-one step in §4 is a number and not a scenario.</li>
    <li><strong>FP8 throughput is measured on the hardware held.</strong> RTX 5090 and H100 FP8 mining rates are benchmarked against today's int7 rates. This matters most for the 5090: its FP8 tensor rate with FP32 accumulation is half its INT8 rate (419 vs 838 TFLOPS/TOPS), a limit datacenter Hopper and Blackwell do not share.</li>
    <li><strong>A hedge venue exists.</strong> There is a PRL perpetual or OTC line with open interest that covers the product's monthly PRL flow, or the market maker commits its own book.</li>
    <li><strong>Exit capacity is tested.</strong> The daily redemption cap is set from executed fills, well below displayed depth, and tightened under stress.</li>
    <li><strong>Pool risk is diversified.</strong> Hashrate is split so that no pool carries more than a third of the product's PRL.</li>
  </ol>
</div>

**Operating rules for either token:**
- Split hashrate across at least three pools.
- Publish GPU inventory, miner versions, fees, measured power and wallet receipts.
- Reserve for the FP8 cutover and for depreciation.
- Set the daily redemption cap from executed fills, not aggregator depth snapshots.
- Get jurisdiction-specific legal review of the token's structure and distribution before launch.

## 10. Catalysts for network value and miner economics

| Catalyst | Timing | Network value | Miner economics | Watch |
|---|---|---|---|---|
| FP8 hard fork | "Within weeks" (Sep 25); no height set | ↑ useful-work credibility | ↑ step for eligible GPUs; ↓ for stranded hardware | `Fp8ForkHeight` in mainnet params; PR #311 merge |
| FP16 / A100 scheme | Open proposal | ↑ wider eligible base | A100 re-enters | PR #369 activation |
| Emission decay | Continuous | ↓ sell pressure over time | −32% block reward in 12 months | `CalcBlockSubsidy` |
| Hyperliquid or tier-1 listing | Ticker owned, no market | ↑ liquidity, hedging | Enables USD-hedged products | HL market activation, listing notices |
| Together relaunch, API mining link | None announced | ↑ useful-work evidence | Inference revenue for dual-use fleets | Together page, Pearl network page |
| Pool concentration | Now | ↓ security premium | Payout counterparty risk | Kryptex share above 50% |
| GPU rental floors | Now | — | Sets the marginal miner | Vast and Salad consumer GPU rates |
| ASIC entry | None seen | ↓ GPU decentralisation story | ↓ GPU yields | Vendor announcements, hashrate jumps |
| PRLCON | 2027 | Community signal | — | [prlcon.com](https://www.prlcon.com/press) |

## 11. What we're researching next

- The hashrate mix by GPU family. This determines the size of the FP8 step in §4.
- FP8 mining throughput per device, especially consumer Blackwell's FP8 rate with FP32 accumulation.
- Executable quotes for datacenter GPUs. The Q3 broker benchmarks in §8 are indicative.
- The FP8 activation height, and any grace period for int7 miners.
- Whether Pearl API traffic runs on the `pearl-ai` stack: block-level evidence tying served tokens to certificates.

## Reproduce

```bash
mkdir pearl-update && cd pearl-update
for f in README.md fetch_chain.py fetch_market.py economics.py gpu_economics.py compare_api_pricing.py requirements.txt; do
  curl -LO https://agtico.github.io/assets/research/pearl-update/$f; done
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
.venv/bin/python fetch_chain.py && .venv/bin/python fetch_market.py
.venv/bin/python economics.py && .venv/bin/python gpu_economics.py && .venv/bin/python compare_api_pricing.py
```

## Caveats

- Implied hashrate rests on two pool-published estimates. Benchmarks come from miner and pool operators with differing software and power methods.
- Purchase prices mix October consumer quotes with Q3 2026 broker benchmarks, and rental quotes are published floors.
- The FP8 day-one chart assumes FP8 per-GPU throughput equal to int7.
- Depth figures are aggregator snapshots. The Lighter and Hyperliquid market statuses are reported, not executed.
- Pool shares come from coinbase attribution over 24 hours.

## Sources

- **Pearl code and specs:**
  - [Pearl repository](https://github.com/pearl-research-labs/pearl);
  - [FP8 branch PR #311](https://github.com/pearl-research-labs/pearl/pull/311);
  - [FP16 PR #369](https://github.com/pearl-research-labs/pearl/pull/369);
  - [FP8 specification](https://pearlresearch.ai/Pearl_Whitepaper.pdf);
  - [integer whitepaper](https://pearlresearch.ai/research/int-whitepaper).
- **Chain and pools:**
  - [Blockbook API](https://blockbook.pearlresearch.ai/api/);
  - [Pearlchain pool attribution](https://pearlchain.live/mining);
  - [Kryptex](https://pool.kryptex.com/prl/about-coin);
  - [Hashrate.no](https://hashrate.no/coins/PRL).
- **GPU benchmarks:** [Oyster](https://www.oysterpool.com/), [Linkrra](https://pool.linkrra.com/mining-cost), [akoya miner](https://github.com/akoyapool/akoya-miner).
- **Purchase prices and hosting:** [CresvaMent](https://cresvament.com/used-gpu-prices), [BestValueGPU](https://bestvaluegpu.com/history/new-and-used-rtx-5090-price-history-and-specs/), [Compute Exchange](https://compute.exchange/hardware-market/used-gpus), [Gatewell](https://gatewellusa.com/ai-compute/p/amd-instinct-mi300x), [Endemine](https://www.endemine.com/hosting/), [GPU Rent Hub](https://www.gpurenthub.com/colocation/).
- **FP8 fork communications:** [Kryptex FP8 notice](https://pool.kryptex.com/ja/articles/pearl-fp8-fork-en), [NVIDIA RTX Blackwell architecture](https://pausehardware.com/wp-content/uploads/2025/01/nvidia-rtx-blackwell-gpu-architecture.pdf), [PRLScan](https://prlscan.com/).
- **Rental prices:** [RunPod](https://www.runpod.io/pricing), [Vast](https://vast.ai/), [Lambda](https://lambda.ai/pricing), [CoreWeave](https://www.coreweave.com/pricing), [TensorDock](https://marketplace.tensordock.com/cloud-gpus.html), [Salad](https://salad.com/pricing/).
- **Market:** [CoinGecko](https://www.coingecko.com/en/coins/pearl-research), [CoinMarketCap](https://coinmarketcap.com/currencies/pearlresearch/), [Margin Trade](https://margin.trade/trade/PRL), [Lighter](https://t.me/s/lighter_announcements), [OAK io:PRL](https://oakresearch.io/en/hyperliquid/hip-3/markets/io:PRL), [Pearl OTC](https://pearl-otc.com/).
- **Useful work and inference:** [Basu, The Usefulness Gap](https://arxiv.org/abs/2606.04819), [Pass, The Economics of PoUW](https://arxiv.org/abs/2606.06700), [Hawkeye](https://arxiv.org/abs/2603.20421), [Pearl pricing](https://pearlresearch.ai/pricing), [Pearl docs](https://platform.pearlresearch.ai/docs), [Together model page](https://www.together.ai/models/gemma-4-31b-it-pearl), [Hugging Face pearl-ai](https://huggingface.co/pearl-ai), [OpenRouter API](https://openrouter.ai/api/v1/models).
