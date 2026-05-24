---
layout: report
title: "Pearl PoUW | Useful Work vs. Consensus"
date: "2026-05-23 18:00:00 +0000"
summary: "AGTI analysis of Pearl PoUW: int7 matmul is not commodity GPU inference; dual-use vs bare lottery; API pricing vs OpenRouter; fleet simulation."
category: AGTI Research
pearl_report: true
report_css_version: 20260524m
tags:
  - AGTI
  - Pearl
  - Proof of Work
  - AI Infrastructure
  - Crypto
---

<div class="pearl-hero-grid">
  <div class="pearl-scorecard good">
    <span class="label">Dual-use path</span>
    <span class="value">vLLM + NoisyGEMM</span>
    <span class="hint">Real inference + mining on the same H100 cycles. This is Pearl's coherent story.</span>
  </div>
  <div class="pearl-scorecard warn">
    <span class="label">Consensus checks</span>
    <span class="value">ZK matmul proof</span>
    <span class="hint">Nodes verify Plonky2 certificates — not whether anyone consumed tokens.</span>
  </div>
  <div class="pearl-scorecard bad">
    <span class="label">Bare / ASIC path</span>
    <span class="value">Matmul lottery</span>
    <span class="hint">Valid blocks, zero useful AI output. Bitcoin-shaped waste with different algebra.</span>
  </div>
</div>

<div class="pearl-verdict-banner">
  <strong>AGTI bottom line</strong>
  <p>Pearl is only non-nonsensical as <em>inference-mining co-location</em>. At the protocol layer it is PoW with matmul puzzles and ZK receipts. Pearl matmul is <strong>not</strong> the BF16/FP8 matmul the industry already runs — see <a href="#what-pearl-matmul-actually-means-three-stacks">§7 deep dive</a>.</p>
</div>

## 0. Start here — product, users, and mass adoption

<div class="pearl-primer-box">
  <p><strong>One-liner:</strong> Pearl is a <em>new proof-of-work cryptocurrency (PRL)</em> whose mining puzzle looks like matrix multiplication. The optional “useful” part is running a special LLM inference stack on the same GPU. <strong>Nobody buys matmul.</strong> They buy coins, or (rarely) discounted API tokens.</p>
  <p style="margin-top:12px"><strong>Read next:</strong> <a href="#what-pearl-matmul-actually-means-three-stacks">What Pearl matmul actually means</a> (not BF16/FP8 commodity inference) · <a href="#inference-api-pricing--subsidized-vs-market">API pricing vs OpenRouter</a> · <a href="#6-fleet-dynamics-simulation-agti-model">Fleet simulation</a></p>
</div>

### What is the product? (there are three, not one)

| # | Product | What you get | Who pays |
|---|---------|--------------|----------|
| **A** | **PRL coin** | Block rewards + speculative asset | Miners earn it; market buys/sells it |
| **B** | **Mining software** | `pearld` + vLLM Pearl plugin + gateway | Miners (to compete for PRL) |
| **C** | **Inference API** (thin) | Normal LLM chat completions; Pearl checkpoint only | Developers via [Together AI](https://www.together.ai/models/gemma-4-31b-it-pearl) — ~25% off **Together list**, not vs OpenRouter |

**Not a product today:** training compute, generic matmul-as-a-service, marketplace GPU rentals ([compute.pearlresearch.ai](https://compute.pearlresearch.ai/) is gated), industry-standard FP8/BF16 stacks.

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>What transacts vs what runs under the hood</h3>
    <span class="tag">Product map</span>
  </div>
  <svg class="pearl-matrix" viewBox="0 0 920 240" role="img" aria-label="Pearl product map showing what customers buy versus internal matmul machinery">
    <text x="40" y="36" fill="#6ee58f" font-family="monospace" font-size="11" font-weight="700">CUSTOMERS ACTUALLY BUY</text>
    <rect x="40" y="48" width="240" height="52" fill="#0d2818" stroke="#6ee58f" rx="4"/>
    <text x="160" y="72" fill="#9dffc8" text-anchor="middle" font-size="13" font-weight="700">PRL (coin)</text>
    <text x="160" y="90" fill="#8aa898" text-anchor="middle" font-size="10">speculation · miner sells</text>
    <rect x="340" y="48" width="240" height="52" fill="#0d2818" stroke="#6ee58f" rx="4"/>
    <text x="460" y="72" fill="#9dffc8" text-anchor="middle" font-size="13" font-weight="700">LLM tokens</text>
    <text x="460" y="90" fill="#8aa898" text-anchor="middle" font-size="10">Together Gemma API only, really</text>
    <rect x="640" y="48" width="240" height="52" fill="#2a1010" stroke="#ff5a42" rx="4"/>
    <text x="760" y="72" fill="#ffb4a8" text-anchor="middle" font-size="13" font-weight="700">NOT SOLD</text>
    <text x="760" y="90" fill="#a87878" text-anchor="middle" font-size="10">matmul · training · raw GPU</text>

    <text x="40" y="140" fill="#ff5a42" font-family="monospace" font-size="11" font-weight="700">INTERNAL (NOT PURCHASED DIRECTLY)</text>
    <rect x="40" y="152" width="840" height="64" fill="#1a1210" stroke="#ff5a42" rx="4"/>
    <text x="460" y="178" fill="#ffb4a8" text-anchor="middle" font-size="12">Proprietary int7 / 7-bit NoisyGEMM + Blake3 jackpot + Plonky2 ZK proof</text>
    <text x="460" y="198" fill="#a87878" text-anchor="middle" font-size="11">Runs on H100/H200 · pearl-ai models · Pearl vLLM fork only</text>
  </svg>
</div>

### User stories (four personas)

<div class="pearl-persona-grid">
  <div class="pearl-persona">
    <span class="who">Persona 1 — GPU miner</span>
    <span class="buys">Earns PRL, not AI output</span>
    <span class="story">Rents H200s → runs Pearl Docker stack → hunts PoW tickets while (maybe) serving Llama-pearl. Primary payoff is ~2,700 PRL/block. LLM is load for matrices, not necessarily customers.</span>
  </div>
  <div class="pearl-persona">
    <span class="who">Persona 2 — Together API customer</span>
    <span class="buys">Discounted inference tokens</span>
    <span class="story">Calls <code>pearl-ai/gemma-4-31b-it</code> like any LLM API. Doesn't know or care about matmul. Gets ~25% off because Together mines PRL on same GPUs.</span>
  </div>
  <div class="pearl-persona">
    <span class="who">Persona 3 — PRL holder</span>
    <span class="buys">Exposure to PoW chain</span>
    <span class="story">Bets Pearl becomes "Bitcoin for GPUs" — coin tied to mining hashrate. No inference involved.</span>
  </div>
  <div class="pearl-persona">
    <span class="who">Persona 4 — AI lab (vision)</span>
    <span class="buys">Cheaper GPU hours (future)</span>
    <span class="story">Whitepaper dream: run training/inference, subsidize capex with mining. <strong>Not live for training.</strong> Requires FP PoUW upgrade + marketplace.</span>
  </div>
</div>

### Training or inference?

| | **Today (mainnet)** | **Pearl marketing / future** |
|--|---------------------|------------------------------|
| **Training** | ❌ Not supported | Maybe after FP PoUW ([whitepaper §1.1](https://pearlresearch.ai/)) |
| **Inference** | ✅ Only via Pearl quant models + plugin | Same, plus cheaper if subsidies grow |
| **Bare mining (no AI)** | ✅ Valid consensus path | Competes with "useful" story |

### The world if *everyone* used Pearl

<div class="pearl-world-split">
  <div class="pearl-world-panel today">
    <h4>Today (~May 2026)</h4>
    <ul>
      <li>Small miner fleet on H200 pods</li>
      <li>~59k blocks, rewards dominate economics</li>
      <li>1 commercial inference partner (Together)</li>
      <li>3 pearl-ai models, 0 HF inference providers on Llama</li>
      <li>Mining works; AI marketplace doesn't exist yet</li>
    </ul>
  </div>
  <div class="pearl-world-panel vision">
    <h4>Pearl success world (their pitch)</h4>
    <ul>
      <li>Every GPU datacenter runs Pearl plugin by default</li>
      <li>Inference/training revenue + PRL subsidy on same watt-hour</li>
      <li>PRL valuable → bigger subsidy → cheaper AI APIs</li>
      <li>On-chain compute marketplace matches buyers/sellers</li>
      <li>AI industry adopts Pearl quant path at scale</li>
    </ul>
  </div>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Mass-adoption flywheel (why they'd say it happens)</h3>
    <span class="tag">Bull case</span>
  </div>
  <div class="pearl-mermaid">
    <div class="mermaid">
flowchart TB
  A[AI demand for inference] --> B[GPU providers run Pearl stack]
  B --> C[Mine PRL while serving tokens]
  C --> D[PRL price rises]
  D --> E[Mining subsidy grows]
  E --> F[AI APIs get cheaper]
  F --> A

  style A fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style B fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style C fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style D fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style E fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style F fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
    </div>
  </div>
  <p class="pearl-figure-caption">This only works if inference demand and PRL price move together. The chain does not enforce the left side of the loop.</p>
</div>

### Is mass adoption realistic?

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>AGTI realism check — what must go right</h3>
    <span class="tag">May 2026 assessment</span>
  </div>
  <div class="pearl-realism-meter">
    <div class="pearl-realism-row">
      <span class="label">Mining / chain works</span>
      <div class="track"><div class="fill green" style="width:88%"></div></div>
      <span class="score" style="color:#6ee58f">HIGH</span>
    </div>
    <div class="pearl-realism-row">
      <span class="label">PRL becomes valuable asset</span>
      <div class="track"><div class="fill" style="width:45%"></div></div>
      <span class="score">?</span>
    </div>
    <div class="pearl-realism-row">
      <span class="label">Inference demand at scale</span>
      <div class="track"><div class="fill" style="width:18%"></div></div>
      <span class="score">LOW</span>
    </div>
    <div class="pearl-realism-row">
      <span class="label">Industry adopts Pearl matmul</span>
      <div class="track"><div class="fill" style="width:8%"></div></div>
      <span class="score">LOW</span>
    </div>
    <p class="pearl-figure-caption" style="margin:-8px 0 12px">Pearl int7/8-bit plugin ≠ OpenRouter BF16/FP8 Gemma — different weights, Hopper-only, 3 models. <a href="#what-pearl-matmul-actually-means-three-stacks">Details §7</a></p>
    <div class="pearl-realism-row">
      <span class="label">Training on Pearl PoUW</span>
      <div class="track"><div class="fill" style="width:3%"></div></div>
      <span class="score">N/A</span>
    </div>
    <div class="pearl-realism-row">
      <span class="label">Dual-use beats bare mining farms</span>
      <div class="track"><div class="fill" style="width:22%"></div></div>
      <span class="score">WEAK</span>
    </div>
  </div>
</div>

**Why mass adoption might happen (bull case):**
1. PRL price pumps → mining subsidy exceeds inference margin → every GPU farm installs Pearl plugin "for free money."
2. More cloud APIs copy Together → discounted inference pulls developers into Pearl quant models.
3. FP PoUW ships → training clusters join → "useful work" expands beyond inference.

**Why it might not (bear case — AGTI default):**
1. **Protocol allows bare mining** — farms skip LLM entirely once subsidies are high enough (whitepaper admits this).
2. **Not commodity matmul** — industry runs BF16/FP8 `google/*` on stock vLLM; Pearl requires `pearl-ai/*-pearl`, int7 NoisyGEMM, sm90 Hopper, and most layers/dims never mine ([§7](#what-pearl-matmul-actually-means-three-stacks)).
3. **Circular economics** — Together discount depends on PRL emissions; if coin is weak, subsidy vanishes.
4. **Training never arrives** — without it, "AI-native PoW" is really "inference-miner PoW."
5. **Bitcoin dynamics repeat** — specialization wins; dual-use hobbyist GPUs lose to dedicated matmul lottery farms.

**AGTI plain answer:** The **product that exists today** is **PRL + miner software**, with **one subsidized inference API** as a proof-of-concept. Mass adoption is **not unrealistic for the coin/mining layer** (another PoW chain can absolutely attract hashrate). It **is unrealistic as stated** for "the AI industry runs on Pearl matmul" unless Pearl quant becomes a standard, PRL stays valuable, and inference demand genuinely co-locates with mining — none of which is proven yet.

## 1. The tension in one glance

Pearl's [whitepaper](https://pearlresearch.ai/) sells **2-for-1 GPU economics**: security subsidy plus useful AI output from the same watt-hours. The open-source **reference miner** grinds **random matrices** until a BLAKE3 jackpot clears difficulty — no LLM, no user, no tokens.

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Bitcoin PoW vs Pearl PoW — what silicon actually does</h3>
    <span class="tag">Stack comparison</span>
  </div>
  <svg class="pearl-matrix" viewBox="0 0 920 260" role="img" aria-label="Comparison of Bitcoin SHA256 mining versus Pearl matmul mining stacks">
    <defs>
      <linearGradient id="btcGrad" x1="0" x2="1"><stop offset="0%" stop-color="#ff2d20"/><stop offset="100%" stop-color="#ff9f45"/></linearGradient>
      <linearGradient id="pearlGrad" x1="0" x2="1"><stop offset="0%" stop-color="#6ee58f"/><stop offset="100%" stop-color="#ff9f45"/></linearGradient>
      <marker id="arrow" markerWidth="8" markerHeight="8" refX="6" refY="4" orient="auto"><path d="M0,0 L8,4 L0,8 Z" fill="#ff5a42"/></marker>
    </defs>
    <text x="24" y="34" fill="#9aa2a0" font-family="monospace" font-size="12" font-weight="700">BITCOIN MINER</text>
    <rect x="24" y="48" width="400" height="44" fill="url(#btcGrad)" opacity="0.92" rx="4"/>
    <text x="224" y="76" fill="#fff" text-anchor="middle" font-size="14" font-weight="700">SHA256 hash grind (artificial)</text>
    <rect x="24" y="104" width="400" height="44" fill="#3a3f3e" rx="4"/>
    <text x="224" y="132" fill="#c8d0cc" text-anchor="middle" font-size="13">Useful output: none</text>
    <text x="496" y="34" fill="#9aa2a0" font-family="monospace" font-size="12" font-weight="700">PEARL MINER (CONSENSUS)</text>
    <rect x="496" y="48" width="400" height="36" fill="url(#pearlGrad)" opacity="0.88" rx="4"/>
    <text x="696" y="70" fill="#081010" text-anchor="middle" font-size="13" font-weight="700">Noisy int7 GEMM tiles</text>
    <rect x="496" y="92" width="400" height="28" fill="#ff9f45" opacity="0.75" rx="4"/>
    <text x="696" y="110" fill="#081010" text-anchor="middle" font-size="12" font-weight="700">BLAKE3 jackpot + Merkle strips</text>
    <rect x="496" y="128" width="400" height="28" fill="#5b8cff" opacity="0.72" rx="4"/>
    <text x="696" y="146" fill="#fff" text-anchor="middle" font-size="12" font-weight="700">Plonky2 ZK proof</text>
    <rect x="496" y="164" width="400" height="44" fill="#3a3f3e" rx="4"/>
    <text x="696" y="184" fill="#c8d0cc" text-anchor="middle" font-size="12">Useful output on this path: none</text>
    <rect x="496" y="214" width="400" height="36" fill="#1f4d32" stroke="#6ee58f" stroke-width="1.2" rx="4"/>
    <text x="696" y="236" fill="#6ee58f" text-anchor="middle" font-size="12" font-weight="700">+ vLLM layer (optional): tokens for users</text>
    <path d="M424 120 H472" stroke="#ff5a42" stroke-width="2" marker-end="url(#arrow)"/>
    <text x="448" y="112" fill="#ff5a42" text-anchor="middle" font-size="11" font-family="monospace">SAME WASTE SHAPE</text>
  </svg>
  <p class="pearl-figure-caption">Pearl swaps SHA256 for matmul+ZK. Without the optional vLLM box, the waste structure mirrors Bitcoin — only the opcode changed.</p>
</div>

## 2. Whitepaper promise vs chain rules

<div class="pearl-split">
  <div class="pearl-panel good">
    <h4>Marketing layer</h4>
    <ul>
      <li>Mining native to AI MatMul</li>
      <li>2-for-1 GPU cycles (security + inference)</li>
      <li>Virtuous loop: demand → subsidy → compute → security</li>
      <li>Drop-in plugin, negligible overhead</li>
      <li>Future compute marketplace</li>
    </ul>
  </div>
  <div class="pearl-panel bad">
    <h4>Consensus layer</h4>
    <ul>
      <li>Valid ZK-PoW certificate required</li>
      <li>Random / committed matrices OK</li>
      <li>No inference API check</li>
      <li>No "tokens served" field</li>
      <li>Bare <code>mine()</code> loop is valid</li>
    </ul>
  </div>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Where the whitepaper forks from the protocol</h3>
    <span class="tag">Economic vs enforced</span>
  </div>
  <div class="pearl-mermaid">
    <div class="mermaid">
flowchart LR
  subgraph promise ["Whitepaper story"]
    A1[AI demand] --> A2[GPU runs inference]
    A2 --> A3[NoisyGEMM side effect]
    A3 --> A4[Block reward + tokens out]
  end

  subgraph protocol ["What pearld accepts"]
    B1[Block header σ] --> B2[Matrix commitments]
    B2 --> B3[Noisy tiled MatMul]
    B3 --> B4[Jackpot under difficulty]
    B4 --> B5[ZK proof valid]
    B5 --> B6[Block accepted]
  end

  A3 -. optional coupling .-> B3

  style promise fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style protocol fill:#1a1210,stroke:#ff5a42,color:#e8eeeb
    </div>
  </div>
  <p class="pearl-figure-caption">The dashed line is a deployment choice (vLLM). The protocol path works without ever serving a user.</p>
</div>

The whitepaper also admits: *"Pearl attracts compute that does not necessarily have useful work."*

## 3. How Pearl mining works (PoUW pipeline)

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>From matrices to block — the lottery inside the multiply</h3>
    <span class="tag">Protocol flow</span>
  </div>
  <div class="pearl-flow">
    <div class="pearl-flow-step"><span class="num">1</span><strong>Commit A, B</strong><span>Merkle roots + chain state σ</span></div>
    <div class="pearl-flow-step"><span class="num">2</span><strong>Add noise E, F</strong><span>Low-rank, rank r=32</span></div>
    <div class="pearl-flow-step"><span class="num">3</span><strong>Tile multiply</strong><span>int7×int7→int32 transcript</span></div>
    <div class="pearl-flow-step"><span class="num">4</span><strong>Jackpot hash</strong><span>BLAKE3(state) under target</span></div>
    <div class="pearl-flow-step"><span class="num">5</span><strong>ZK prove</strong><span>Plonky2 → block cert</span></div>
  </div>
  <div class="pearl-mermaid" style="margin-top:18px">
    <div class="mermaid">
sequenceDiagram
  participant M as Miner
  participant G as pearl-gateway
  participant N as pearld

  M->>M: Random or model matrices
  M->>M: NoisyGEMM + jackpot search
  M->>M: generate_proof (Plonky2)
  M->>G: PlainProof / ZK certificate
  G->>N: Submit block
  N->>N: verify_zk_proof()
  Note over N: Does NOT ask: inference served?
  N-->>G: Accept / reject
    </div>
  </div>
</div>

**Code anchors:** bare search in `zk-pow/src/ffi/mine.rs` · defaults in `node/zkpow/miner.go` (`k=1024`, `rank=32`) · verify in `node/zkpow/verify.go`.

## 4. Two miners, one protocol

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Coherent vs nonsensical — same consensus, different economics</h3>
    <span class="tag">Miner comparison</span>
  </div>
  <div class="pearl-split">
    <div class="pearl-panel good">
      <h4>Production: vLLM miner</h4>
      <ul>
        <li>Matrices = weights + activations</li>
        <li>7-bit layers → NoisyGEMM</li>
        <li>8-bit layers → vanilla GEMM</li>
        <li>Output: LLM tokens + block candidates</li>
        <li>Hardware: H100 / H200 (sm90)</li>
      </ul>
    </div>
    <div class="pearl-panel bad">
      <h4>Bare / ASIC: zk-pow only</h4>
      <ul>
        <li>Matrices = lottery inputs</li>
        <li>Same jackpot + ZK machinery</li>
        <li>No model, no API, no users</li>
        <li>Output: block rewards only</li>
        <li>Hardware: matmul+zk silicon</li>
      </ul>
    </div>
  </div>

  <div class="pearl-bar-chart" style="margin-top:22px">
    <div class="pearl-bar-row">
      <span class="name">Useful AI output</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:92%"></div></div>
      <span class="pct" style="color:#6ee58f">vLLM</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Useful AI output</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:3%"></div></div>
      <span class="pct">ASIC</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Consensus validity</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill amber" style="width:100%"></div></div>
      <span class="pct">both</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Mission alignment</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:88%"></div></div>
      <span class="pct" style="color:#6ee58f">vLLM</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Mission alignment</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill" style="width:8%"></div></div>
      <span class="pct">ASIC</span>
    </div>
  </div>
  <p class="pearl-figure-caption">Both paths can win blocks. Only the vLLM path delivers Pearl's stated utility.</p>
</div>

## 5. ASIC economics — when dual-use wins

The whitepaper's ASIC argument is **profitability**, not **impossibility**: a Pearl-only chip "will lose the ability to do useful work, and thereby be overall less profitable."

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Who wins the hardware game?</h3>
    <span class="tag">Equilibrium map</span>
  </div>
  <div class="pearl-quadrant-wrap">
    <div class="pearl-quadrant-y">Useful work revenue</div>
    <div class="pearl-quadrant">
      <div class="pearl-quadrant-cell">
        <h5>Low subsidy · High inference</h5>
        <p>GPU farms serve APIs. Mining is opportunistic side income. Pearl's happy path.</p>
      </div>
      <div class="pearl-quadrant-cell highlight">
        <h5>High subsidy · High inference</h5>
        <p>Best of both worlds — if overhead stays negligible. Whitepaper virtuous loop.</p>
      </div>
      <div class="pearl-quadrant-cell">
        <h5>Low subsidy · Low inference</h5>
        <p>Niche hobbyist mining. Chain stays weak. Uninteresting equilibrium.</p>
      </div>
      <div class="pearl-quadrant-cell highlight">
        <h5>High subsidy · Low inference</h5>
        <p><strong>ASIC / bare farm zone.</strong> Matmul lottery dominates. Mission collapses.</p>
      </div>
    </div>
    <div class="pearl-quadrant-x">Token subsidy / block reward</div>
  </div>
  <div class="pearl-legend">
    <span class="pearl-legend-item"><span class="pearl-legend-swatch" style="background:rgba(255,45,32,.55)"></span>Highlighted = structurally important equilibria</span>
  </div>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Compute budget — where the joules go</h3>
    <span class="tag">Conceptual energy split</span>
  </div>
  <div class="pearl-bar-chart">
    <div class="pearl-bar-row">
      <span class="name">vLLM dual-use</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:72%"></div></div>
      <span class="pct" style="color:#6ee58f">inference</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">vLLM dual-use</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill amber" style="width:18%"></div></div>
      <span class="pct">PoW</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">vLLM dual-use</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:10%"></div></div>
      <span class="pct">ZK</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Bare / ASIC farm</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill" style="width:78%"></div></div>
      <span class="pct">PoW</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Bare / ASIC farm</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:16%"></div></div>
      <span class="pct">ZK</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Bare / ASIC farm</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:6%"></div></div>
      <span class="pct">idle</span>
    </div>
  </div>
  <p class="pearl-figure-caption">Illustrative split, not measured mainnet telemetry. Shows why bare mining recenters waste in the PoW bucket.</p>
</div>

## 6. Fleet dynamics simulation (AGTI model)

The quadrant map above is qualitative. To stress-test **when dual-use wins vs ASIC/bare farms**, AGTI built a monthly **fleet dynamics** model in Python/numpy: three fleet types (dual-use GPU, bare GPU, ASIC) compete on hashrate share, revenue, and profit-driven capacity growth.

<p class="pearl-sim-intro"><strong>Question:</strong> If PRL subsidies rise, inference demand stays flat, and ASIC hash/$ improves, does the network stay on the useful-work path — or converge to matmul lottery farms? <strong>Source:</strong> <a href="https://agtico.github.io/assets/research/pearl-economics/">AGTI fleet simulation</a> (<a href="https://agtico.github.io/assets/research/pearl-economics/simulate.py"><code>simulate.py</code></a>). Parameters are <em>illustrative</em>, not calibrated to mainnet.</p>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Model mechanics (monthly loop)</h3>
    <span class="tag"><a href="https://agtico.github.io/assets/research/pearl-economics/simulate.py">simulate.py</a></span>
  </div>
  <div class="pearl-mermaid">
    <div class="mermaid">
flowchart TB
  subgraph fleets ["Three fleet types"]
    D[Dual-use GPU<br/>hash × 0.82 + inference margin]
    B[Bare GPU<br/>hash × 1.12, mining only]
    A[ASIC<br/>hash × 3.5–14×, mining only]
  end

  subgraph month ["Each month"]
    H[Total hashrate → share split]
    R[Mining revenue ∝ share × blocks × PRL]
    I[Inference revenue — dual only, demand-capped]
    C[Electricity + capex amort + opex]
    P[Profit margin → grow/shrink capacity]
    X[PRL mean-reverts to production cost]
  end

  D --> H
  B --> H
  A --> H
  H --> R --> P
  I --> P
  C --> P
  P --> D
  P --> B
  P --> A
  P --> X

  style fleets fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style month fill:#1a1210,stroke:#ff5a42,color:#e8eeeb
    </div>
  </div>
  <p class="pearl-figure-caption"><strong>Useful-work proxy:</strong> fraction of hashrate from dual-use fleets (serves inference + mining). Not enforced by Pearl consensus — this is the <em>mission</em> metric. <strong>Concentration:</strong> Herfindahl–Hirschman Index (HHI) on hashrate shares.</p>
</div>

| Scenario | Stress | Month 36: useful hash | ASIC share | Inference % revenue | HHI |
|----------|--------|----------------------:|-----------:|--------------------:|----:|
| `baseline` | Moderate PRL ($0.05), 400 GPU demand | 0.3% | 99.7% | 1.1% | 0.995 |
| `prl_pump` | PRL ramps to ~$0.75; flat demand | 0.7% | 95.6% | 0.5% | 0.916 |
| `inference_boom` | Demand → 4k GPUs; flat PRL | **11.5%** | 88.5% | **45.0%** | 0.796 |
| `asic_creep` | ASIC hash/$ → 4× | 0.1% | 99.9% | 0.2% | 0.999 |
| `together_only` | 50 GPU demand (today-shaped) | 0.2% | 99.6% | 1.3% | 0.992 |
| `bitcoinification` | PRL pump + ASIC creep + flat inference | 0.1% | 99.7% | 0.4% | 0.995 |

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Scenario charts — 36-month horizon</h3>
    <span class="tag">AGTI simulation</span>
  </div>

  <div class="pearl-sim-grid">
    <div class="pearl-sim-card">
      <div class="pearl-sim-card-head"><h4>baseline</h4><span class="tag">moderate PRL</span></div>
      <div class="pearl-sim-chart"><img src="/assets/images/pearl-economics/baseline.png?v=20260524d" alt="Baseline scenario: hashrate shares, PRL price, inference revenue fraction over 36 months" loading="lazy" /></div>
      <p class="pearl-sim-caption">Endogenous PRL mean-reversion cannot stop ASIC crowding once hash/$ advantage compounds.</p>
    </div>
    <div class="pearl-sim-card highlight">
      <div class="pearl-sim-card-head"><h4>inference_boom</h4><span class="tag">best case</span></div>
      <div class="pearl-sim-chart"><img src="/assets/images/pearl-economics/inference_boom.png?v=20260524d" alt="Inference boom scenario: dual-use retains meaningful hashrate when API demand scales" loading="lazy" /></div>
      <p class="pearl-sim-caption"><strong>Only scenario</strong> where dual-use hashrate stays double-digit and inference becomes ~half of network revenue.</p>
    </div>
    <div class="pearl-sim-card">
      <div class="pearl-sim-card-head"><h4>prl_pump</h4><span class="tag">subsidy shock</span></div>
      <div class="pearl-sim-chart"><img src="/assets/images/pearl-economics/prl_pump.png?v=20260524d" alt="PRL pump scenario: token price rise attracts ASIC capital" loading="lazy" /></div>
      <p class="pearl-sim-caption">Higher PRL alone attracts capital into best hash/$ — ASICs and bare GPUs, not vLLM farms.</p>
    </div>
    <div class="pearl-sim-card">
      <div class="pearl-sim-card-head"><h4>asic_creep</h4><span class="tag">hardware curve</span></div>
      <div class="pearl-sim-chart"><img src="/assets/images/pearl-economics/asic_creep.png?v=20260524d" alt="ASIC creep scenario: improving ASIC efficiency drives concentration" loading="lazy" /></div>
      <p class="pearl-sim-caption">4× ASIC efficiency gain → HHI ≈ 1.0. Matches whitepaper's "compute without useful work" branch.</p>
    </div>
    <div class="pearl-sim-card">
      <div class="pearl-sim-card-head"><h4>together_only</h4><span class="tag">May 2026 shaped</span></div>
      <div class="pearl-sim-chart"><img src="/assets/images/pearl-economics/together_only.png?v=20260524d" alt="Together-only scenario: tiny inference market, dual-use collapses" loading="lazy" /></div>
      <p class="pearl-sim-caption">Tiny API demand (~50 GPUs) + low PRL: dual-use starts weak and terminal useful hash → 0.2%.</p>
    </div>
    <div class="pearl-sim-card">
      <div class="pearl-sim-card-head"><h4>bitcoinification</h4><span class="tag">stress test</span></div>
      <div class="pearl-sim-chart"><img src="/assets/images/pearl-economics/bitcoinification.png?v=20260524d" alt="Bitcoinification scenario: combined PRL pump and ASIC creep with flat inference" loading="lazy" /></div>
      <p class="pearl-sim-caption">Combined pump + ASIC creep + flat inference — mission collapse. Structurally Bitcoin-shaped.</p>
    </div>
  </div>
</div>

**AGTI read from the sim:** PRL price alone does not preserve useful work. The whitepaper virtuous loop (§8 below) requires **inference demand scaling with subsidies** — the only lever that materially keeps dual-use competitive in this model. Today's adoption profile maps closest to `together_only`.

### Code anchors (reproducible)

<div class="pearl-code-ref"><span class="path"><a href="https://agtico.github.io/assets/research/pearl-economics/simulate.py">simulate.py</a> — fleet types & unit economics</span>
dual:  hash_units=1.0,  mining_overhead=18%, inference_margin=$1.2k/GPU-mo
bare:  hash_units=1.12 (no inference tax)
asic:  hash_units=3.5× baseline, lower power/capex (illustrative matmul+ZK ASIC)
</div>

<div class="pearl-code-ref"><span class="path"><a href="https://github.com/agtico/agtico.github.io/blob/main/assets/research/pearl-economics/simulate.py#L209-L271">simulate.py</a> — revenue & capacity adjustment (lines 209–271)</span>
rev_mine[k] = hashrate_share[k] × blocks × block_reward × PRL_price
rev_inf   = min(dual_capacity, inference_demand) × inference_margin  # dual only
cap[k]    *= 1 + clip(growth_rate × profit_margin, shrink..grow bounds)
</div>

<div class="pearl-code-ref"><span class="path">pearl/miner/vllm-miner/src/vllm_miner/config.yaml — dual-use overhead source</span>
Mining layers only when m,n,k ≥ 1024 — large layers only; overhead is real but localized.
</div>

<div class="pearl-code-ref"><span class="path">pearl/node/zkpow/miner.go — consensus does not check inference</span>
Default k=1024, rank=32 — nodes verify ZK matmul certificates, not API usage.
</div>

**Run locally** (scripts hosted on AGTI — no repo clone required):

```bash
mkdir pearl-economics && cd pearl-economics
curl -LO https://agtico.github.io/assets/research/pearl-economics/requirements.txt
curl -LO https://agtico.github.io/assets/research/pearl-economics/simulate.py
curl -LO https://agtico.github.io/assets/research/pearl-economics/compare_inference_pricing.py
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
.venv/bin/python simulate.py --all --plot
.venv/bin/python compare_inference_pricing.py
```

Full folder: [agtico.github.io/assets/research/pearl-economics/](https://agtico.github.io/assets/research/pearl-economics/)

**Limits:** Not calibrated to Pearl mainnet difficulty (~4.9M), block reward decay, pool variance, or token sell pressure. ASIC is modeled as zero inference utility by definition (protocol allows it). See the [research README](https://agtico.github.io/assets/research/pearl-economics/README.md).

## 7. Adoption & useful-work reality check

Pearl's pitch treats "matmul" as if the AI industry already runs it. **It doesn't.** When Google ships [Gemma 4 31B](https://huggingface.co/google/gemma-4-31b-it), the ecosystem runs **BF16/FP8** weights through **stock vLLM, TensorRT, llama.cpp, SGLang** — thousands of hosts, hundreds of billions of tokens/week on aggregators like OpenRouter. Pearl's stack is a **parallel, incompatible matmul pipeline**: proprietary **7-bit/8-bit integer** paths, **NoisyGEMM** crypto noise, **Blake3 + Plonky2** proofs, and a **Pearl-only vLLM plugin** — today on **three** `pearl-ai/*` checkpoints and **Hopper (sm90) GPUs only**.

**Plain English:** Pearl did not turn "GPU matmul" into a commodity. They built a **specialized lottery algebra** that *resembles* linear layers inside one forked inference stack. Most layers in a Pearl model **never mine**. Most matrix sizes **never mine**. Standard BF16 Gemma on normal vLLM **cannot produce valid Pearl blocks** even if you point it at the same GPU.

### What Pearl matmul actually means (three stacks)

Pearl conflates three different workloads under one brand. Only the first is what `pearld` validates; the second is optional dual-use inference; the third is inference-only inside the plugin.

| Stack | Precision / kernel | What it does | Required to mine? |
|-------|-------------------|--------------|-------------------|
| **① Consensus (`zk-pow`)** | **int7×int7→int32** tiles, values in **[-64, 64]**, low-rank noise, Blake3 jackpot, Plonky2 STARK | Block lottery — verifies a noisy matmul **transcript**, not token quality | Yes — this is all `pearld` checks |
| **② NoisyGEMM (vLLM plugin)** | **7-bit** quant on selected layers; CUTLASS Hopper kernels add commitment + noise + inner-hash PoW extraction | Runs **during** LLM forward pass on large layers | No — deployment choice; only if layer + dims qualify |
| **③ Vanilla Pearl GEMM** | **8-bit** quant, standard scaled int8 GEMM, **no** noise / hash | Normal inference layers + small 7-bit matmuls | No — **zero PoW** on chain |

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>From LLM forward pass to block — where mining actually happens</h3>
    <span class="tag">Plugin routing</span>
  </div>
  <div class="pearl-mermaid">
    <div class="mermaid">
flowchart TB
  subgraph vllm ["Pearl vLLM plugin"]
    L7["7-bit mining layer"]
    L8["8-bit non-mining layer"]
    TH{"m,n,k ≥ 1024<br/>and mining on?"}
    NOISY["NoisyGEMM<br/>commit + noise + inner hash"]
    VAN["Vanilla pearl GEMM<br/>inference only"]
    L7 --> TH
    TH -->|yes| NOISY
    TH -->|no| VAN
    L8 --> VAN
  end

  subgraph chain ["pearld consensus"]
    PP["PlainProof: int7 strips + Merkle"]
    ZK["Plonky2 verify + difficulty"]
    PP --> ZK
  end

  NOISY -.->|optional block submit| PP
  VAN --> OUT["bf16/fp16 tokens out<br/>no block"]

  style vllm fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style chain fill:#1a1210,stroke:#ff5a42,color:#e8eeeb
    </div>
  </div>
  <p class="pearl-figure-caption">Source: Pearl <code>vllm_kernels.py</code>, <code>config.yaml</code>, <code>zk-pow/verify</code>. Inference can succeed while producing <strong>no</strong> valid blocks.</p>
</div>

#### Mainstream AI matmul vs Pearl matmul

| Dimension | Industry default (Gemma 4 31B today) | Pearl stack |
|-----------|--------------------------------------|-------------|
| **Weights** | Google `google/gemma-4-31b-it` BF16/FP8 checkpoints | **`pearl-ai/*-pearl`** re-quantized artifacts ([HF org](https://huggingface.co/pearl-ai)) |
| **Runtime** | Stock vLLM, TensorRT, NIM, llama.cpp, OpenRouter backends | **Pearl vLLM plugin** + `pearl-gemm` CUDA + `pearl-gateway` + `pearld` |
| **Linear ops** | FP16/BF16/FP8/FP4 tensor cores as provider chooses | **int7 mining** + **int8 non-mining** only; output activations back to bf16/fp16 |
| **Mining** | None | NoisyGEMM on **subset** of layers **and** only when **m,n,k ≥ 1024** |
| **Chain proof** | N/A | int7 noisy transcript + jackpot hash — **not** "we served tokens" |
| **GPU gen** | A100–Blackwell, consumer GPUs with quant | **sm90 Hopper only** (H100/H200) per Pearl build |
| **Model count** | One base model → dozens of hosts | **3** public Pearl checkpoints (May 2026) |

[OpenJarvis model enablement docs](https://open-jarvis.github.io/OpenJarvis/development/pearl-model-enablement/) state explicitly: raw Hugging Face models like `google/gemma-4-31b-it` are **not mineable** — you need `pearl-ai/Gemma-4-31B-it-pearl` with `quantization_config.quant_method = "pearl"`, mining layers tagged for 7-bit NoisyGEMM and attention/MLP down-proj kept on 8-bit vanilla paths.

#### Layer selection rules (why most matmul never mines)

**Model config** — a layer is a "mining layer" only if weights **and** activations are **7-bit**, static channel/tensor weights, dynamic per-token activations, symmetric quant ([`vllm_config.py`](https://github.com/pearl-research-labs/pearl/blob/master/miner/vllm-miner/src/vllm_miner/vllm_config.py)):

<div class="pearl-code-ref"><span class="path">pearl/miner/vllm-miner/src/vllm_miner/vllm_config.py — mining vs non-mining layers</span>
Mining layer (7-bit):   int7 quant + noisy GEMM (when dims allow)
Non-mining layer (8-bit): int8 quant + vanilla GEMM only — never submits blocks
</div>

**Runtime thresholds** — even on 7-bit mining layers, NoisyGEMM (PoW path) runs only when **all** of m, n, k are ≥ **1024** ([`config.yaml`](https://github.com/pearl-research-labs/pearl/blob/master/miner/vllm-miner/src/vllm_miner/config.yaml)). Smaller matmuls (typical of many attention/MLP shapes in a single forward step) fall back to vanilla GEMM → **inference OK, no mining**.

<div class="pearl-code-ref"><span class="path">pearl/miner/vllm-miner/src/vllm_miner/vllm_kernels.py — routing</span>
if should_use_noisy_gemm(m,n,k) and not no_mining:
    pearl_gemm_noisy(...)   # commitments + noise + inner hash + optional block
else:
    pearl_gemm_vanilla(...) # normal linear — no PoW
</div>

**Consensus** — separately, `pearld` only accepts **Int7×Int7→Int32** MMA with strip values in **[-64, 64]**, independent of whether anyone ran an LLM ([`verify.rs` / `proof.rs`](https://github.com/pearl-research-labs/pearl/tree/master/zk-pow)). The whitepaper's "arbitrary matmul" is **this fixed lottery format**, not PyTorch `torch.matmul` generically.

#### What breaks if you use standard BF16/FP8 vLLM

| You try… | Result |
|----------|--------|
| Run `google/gemma-4-31b-it` on stock vLLM + mine | **No Pearl blocks** — no int7 strips, no noise transcript, no PlainProof |
| Run `pearl-ai/*-pearl` without Pearl plugin | Cutlass/default kernels — **no NoisyGEMM**, no gateway integration |
| Use Pearl plugin but `MINER_NO_MINING=true` / small dims | Inference works; **hashrate = 0** |
| Run on A100 / L40 / consumer GPU | **`pearl-gemm` builds for sm_90a only** — won't compile/run Pearl kernels |
| Assume Pearl matmul = commodity cloud matmul | **Wrong product category** — different weights, quant, context, behavior |

Pearl's [HF model card](https://huggingface.co/pearl-ai/Gemma-4-31B-it-pearl) notes plain vLLM works for **inference-only** (no mining). Mining requires the full Docker stack: `pearld` + gateway + plugin-enabled vLLM.

#### Hardware & precision moat (May 2026)

- **GPU:** Pearl `pearl-gemm` compiles with `arch=compute_90a,code=sm_90a` — **Hopper H100/H200** ([`setup.py`](https://github.com/pearl-research-labs/pearl/blob/master/miner/pearl-gemm/setup.py)).
- **Precision:** Pearl [whitepaper §1.1](https://pearlresearch.ai/) states today's protocol is exact **INT** matmul; FP/quantized PoUW for modern BF16/FP8 inference is a **future** upgrade — not what mainnet verifies today.
- **Ecosystem:** Google's Gemma 4 launch targets BF16 on 80GB H100 and **quantized variants on workstation GPUs** via standard runtimes ([Google announcement coverage](https://smbtech.au/news/google-deepmind-releases-gemma-4-its-most-capable-open-source-ai-models/)) — a completely different supply chain from Pearl's int7 plugin.

**AGTI read:** Pearl matmul is **not fungible** with industry matmul. Adopting Pearl means adopting **Pearl-quantized models**, **Hopper-only kernels**, and **accepting divergent behavior** vs the commodity `google/*` endpoints developers already use. That is why "the AI industry runs on Pearl matmul" is misleading — the industry runs BF16/FP8; Pearl runs a **sidecar int lottery** on a tiny model list.

### Inference API pricing — subsidized vs market?

Together markets [`pearl-ai/gemma-4-31b-it`](https://www.together.ai/models/gemma-4-31b-it-pearl) at **~25% off**, subsidized by Pearl mining. AGTI queried public catalogs **May 24, 2026** ([OpenRouter models API](https://openrouter.ai/api/v1/models), [OpenRouter endpoints](https://openrouter.ai/api/v1/models/google/gemma-4-31b-it/endpoints), [Together model pages](https://www.together.ai/models/gemma-4-31b-it-pearl)).

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Same name on the tin — not the same product</h3>
    <span class="tag">Not a commodity switch</span>
  </div>
  <div class="pearl-mermaid">
    <div class="mermaid">
flowchart LR
  subgraph market ["What developers buy at scale"]
    M1["google/gemma-4-31b-it<br/>OpenRouter $0.12 / $0.37 per 1M"]
    M2["Google weights · FP4/FP8<br/>262K context · multimodal"]
    M3["~295B tokens/week on OR"]
  end

  subgraph pearl_api ["Pearl Together endpoint"]
    P1["pearl-ai/gemma-4-31b-it<br/>$0.28 / $0.86 per 1M"]
    P2["Pearl re-quant checkpoint · INT8<br/>32K context · Pearl plugin stack"]
    P3["No pearl-ai slug on OpenRouter"]
  end

  M1 -. "not interchangeable" .- P1
  M2 -. different weights/q/cap .- P2

  style market fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style pearl_api fill:#1a1210,stroke:#ff5a42,color:#e8eeeb
    </div>
  </div>
  <p class="pearl-figure-caption">Switching from OpenRouter Gemma to Pearl Gemma is <strong>not</strong> a drop-in price arbitrage — different model artifact, quantization, context, and stack. Benchmarks and tool schemas may diverge.</p>
</div>

| Offering | API slug | Input $/M | Output $/M | Context | Notes |
|----------|----------|----------:|-----------:|--------:|-------|
| **OpenRouter (router default)** | `google/gemma-4-31b-it` | **$0.12** | **$0.37** | 262K | Usually routes to DeepInfra-class providers |
| **OpenRouter → Together (Pearl stack)** | `google/gemma-4-31b-it` | $0.28 | $0.86 | **32K** | Same $/M as Pearl endpoint; one of the **most expensive** OR backends |
| **Together standard Gemma** | `google/gemma-4-31B-it` | $0.39 | $0.97 | 256K | FP8, full context — Together's list price |
| **Together Pearl Gemma** | `pearl-ai/gemma-4-31b-it` | $0.28 | $0.86 | 32K | Pearl checkpoint + INT8; marketed ~25% off **Together list** |

**Blended example** (1M input + 200k output tokens): OpenRouter best ≈ **$0.19** · Together Pearl ≈ **$0.45** → Pearl is **~2.3× more expensive** than where Gemma 4 31B actually trades volume today.

**AGTI read:**

1. **The discount is internal.** Pearl is ~23% below Together's own $0.39/$0.97 SKU — that's the "~25% off" claim. It is **not** below OpenRouter's $0.12/$0.37 market.
2. **Subsidy isn't showing up as market undercut.** If PoUW + PRL emissions were funding cheap inference, Together's Pearl route should compete on **absolute** price. Instead OpenRouter routes **away** from Together to cheaper hosts.
3. **Not commodity Gemma.** `pearl-ai/gemma-4-31b-it` is a **Pearl re-quantized checkpoint** (HF: [`pearl-ai/Gemma-4-31B-it-pearl`](https://huggingface.co/pearl-ai/Gemma-4-31B-it-pearl)), not Google's base weights. INT8 vs FP8, **32K vs 262K** context, Pearl vLLM plugin required. Developers cannot treat it as a fungible swap for `google/gemma-4-31b-it` on OpenRouter — quality, latency, and behavior are **questionable to assume equivalent**.
4. **No separate OpenRouter listing.** There is no `pearl-ai/*` slug on OpenRouter. Pearl only appears as Together's **32K / $0.28 / $0.86** backend when the router hits that provider.

Reproduce pricing check: [`compare_inference_pricing.py`](https://agtico.github.io/assets/research/pearl-economics/compare_inference_pricing.py) (public APIs, no keys).

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Three different things called "matmul"</h3>
    <span class="tag">Compatibility map</span>
  </div>
  <svg class="pearl-matrix" viewBox="0 0 920 300" role="img" aria-label="Three Pearl matmul stacks versus mainstream AI matmul">
    <text x="24" y="28" fill="#9aa2a0" font-family="monospace" font-size="11" font-weight="700">MAINSTREAM AI</text>
    <rect x="24" y="40" width="260" height="36" fill="#3a3f3e" rx="4"/>
    <text x="154" y="62" fill="#c8d0cc" text-anchor="middle" font-size="12">BF16 / FP8 / FP4</text>
    <rect x="24" y="84" width="260" height="36" fill="#3a3f3e" rx="4"/>
    <text x="154" y="106" fill="#c8d0cc" text-anchor="middle" font-size="12">vanilla vLLM · TensorRT · PyTorch</text>
    <rect x="24" y="128" width="260" height="36" fill="#3a3f3e" rx="4"/>
    <text x="154" y="150" fill="#c8d0cc" text-anchor="middle" font-size="12">thousands of public models</text>

    <text x="330" y="98" fill="#ff5a42" font-family="monospace" font-size="22" font-weight="700">≠</text>

    <text x="380" y="28" fill="#ff5a42" font-family="monospace" font-size="11" font-weight="700">PEARL ONLY</text>
    <rect x="380" y="40" width="516" height="36" fill="#2a1010" stroke="#ff5a42" stroke-width="1" rx="4"/>
    <text x="638" y="62" fill="#ffb4a8" text-anchor="middle" font-size="12" font-weight="700">Consensus: int7×int7 → int32 + noise + ZK (lottery)</text>
    <rect x="380" y="84" width="516" height="36" fill="#142220" stroke="#6ee58f" stroke-width="1" rx="4"/>
    <text x="638" y="106" fill="#9dffc8" text-anchor="middle" font-size="12">NoisyGEMM: 7-bit layers, m/n/k ≥ 1024 only</text>
    <rect x="380" y="128" width="516" height="36" fill="#1a1210" stroke="#ff9f45" stroke-width="1" rx="4"/>
    <text x="638" y="150" fill="#ffd8a8" text-anchor="middle" font-size="12">3 pearl-ai checkpoints · sm90 H100/H200 · plugin required</text>

    <text x="24" y="200" fill="#9aa2a0" font-family="monospace" font-size="11" font-weight="700">WHO USES IT (MAY 2026)</text>
    <rect x="24" y="212" width="200" height="72" fill="#0d2818" stroke="#6ee58f" rx="4"/>
    <text x="124" y="236" fill="#6ee58f" text-anchor="middle" font-size="11" font-weight="700">Together AI</text>
    <text x="124" y="254" fill="#c8e8d4" text-anchor="middle" font-size="10">1 endpoint: Gemma-4-31B-it</text>
    <text x="124" y="270" fill="#8aa898" text-anchor="middle" font-size="9">not cheaper vs OpenRouter</text>

    <rect x="240" y="212" width="200" height="72" fill="#1a1210" stroke="#ff9f45" rx="4"/>
    <text x="340" y="236" fill="#ff9f45" text-anchor="middle" font-size="11" font-weight="700">GPU miners</text>
    <text x="340" y="254" fill="#e8d8c8" text-anchor="middle" font-size="10">H200 pods · ~59k blocks</text>
    <text x="340" y="270" fill="#a89888" text-anchor="middle" font-size="9">minepearl.org guides</text>

    <rect x="456" y="212" width="200" height="72" fill="#141820" stroke="#888" rx="4"/>
    <text x="556" y="236" fill="#aaa" text-anchor="middle" font-size="11" font-weight="700">Pearl Labs</text>
    <text x="556" y="254" fill="#ccc" text-anchor="middle" font-size="10">3 HF models · self-dogfood</text>
    <text x="556" y="270" fill="#888" text-anchor="middle" font-size="9">131k downloads ≠ users</text>

    <rect x="672" y="212" width="224" height="72" fill="#2a1010" stroke="#ff5a42" rx="4"/>
    <text x="784" y="236" fill="#ff5a42" text-anchor="middle" font-size="11" font-weight="700">Everyone else</text>
    <text x="784" y="254" fill="#ffb4a8" text-anchor="middle" font-size="10">0 HF inference providers</text>
    <text x="784" y="270" fill="#a87878" text-anchor="middle" font-size="9">on Llama-pearl models</text>
  </svg>
  <p class="pearl-figure-caption">Sources: <a href="https://pearlresearch.ai/">whitepaper §1.1</a>, <a href="https://huggingface.co/pearl-ai">pearl-ai HF org</a>, <a href="https://www.together.ai/models/gemma-4-31b-it-pearl">Together endpoint</a>, <a href="https://explorer.pearlresearch.ai/">explorer</a>.</p>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>On-chain signals vs inference demand</h3>
    <span class="tag">May 2026 snapshot</span>
  </div>
  <div class="pearl-source-grid">
    <div class="pearl-source-card">
      <span class="src-label">Mainnet blocks</span>
      <span class="src-stat">~59,000+</span>
      <span class="src-note">Live since Apr 27, 2026. ~1 block / 1–2 min recently. <a href="https://explorer.pearlresearch.ai/">explorer</a></span>
    </div>
    <div class="pearl-source-card">
      <span class="src-label">Block reward</span>
      <span class="src-stat">~2,711 PRL</span>
      <span class="src-note">Coinbase outputs visible per block — subsidy farming, not API revenue. <a href="https://blockbook.pearlresearch.ai/">blockbook</a></span>
    </div>
    <div class="pearl-source-card">
      <span class="src-label">HF Llama 70B Pearl</span>
      <span class="src-stat">131k ↓ · 2 ♥</span>
      <span class="src-note">"Not deployed by any Inference Provider." Downloads ≈ miner setup. <a href="https://huggingface.co/pearl-ai/Llama-3.3-70B-Instruct-pearl">HF</a></span>
    </div>
  </div>

  <div class="pearl-adoption-ladder" style="margin-top:20px">
    <div class="pearl-adoption-rung highlight">
      <strong>Protocol shipped</strong>
      <span>pearld + zk-pow + vLLM miner on mainnet</span>
      <em>confirmed</em>
    </div>
    <div class="pearl-adoption-rung highlight">
      <strong>Mining works</strong>
      <span>Fast block cadence, rising difficulty (~4.9M)</span>
      <em>confirmed</em>
    </div>
    <div class="pearl-adoption-rung">
      <strong>External inference</strong>
      <span>Together AI: <code>pearl-ai/gemma-4-31b-it</code> at 25% discount</span>
      <em>1 partner</em>
    </div>
    <div class="pearl-adoption-rung dim">
      <strong>Industry matmul adoption</strong>
      <span>BF16/FP8 stacks unchanged; int-only PoUW today</span>
      <em>none</em>
    </div>
    <div class="pearl-adoption-rung dim">
      <strong>Proven paid demand</strong>
      <span>No public marketplace metrics; compute portal gated</span>
      <em>unverified</em>
    </div>
  </div>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Marketing claim vs observed evidence</h3>
    <span class="tag">Useful work audit</span>
  </div>
  <div class="pearl-mermaid">
    <div class="mermaid">
flowchart TB
  subgraph claim ["What Pearl markets"]
    M1[GPU matmul is AI-native PoW]
    M2[Same cycles earn coins + serve AI]
    M3[Industry-compatible useful work]
  end

  subgraph reality ["What we can verify May 2026"]
    R1[Proprietary int7 / 7-bit NoisyGEMM]
    R2[3 pearl-ai models on sm90 only]
    R3[59k blocks · 2.7k PRL rewards visible]
    R4[Together: 1 subsidized endpoint]
    R5[Whitepaper: compute without useful work OK]
  end

  M1 --> R1
  M2 --> R3
  M3 --> R4
  M3 -.-> R5

  style claim fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style reality fill:#1a1210,stroke:#ff5a42,color:#e8eeeb
    </div>
  </div>
  <p class="pearl-figure-caption">Whitepaper quote: <em>"Pearl attracts compute that does not necessarily have useful work."</em> — <a href="https://pearlresearch.ai/">pearlresearch.ai §1</a>. "Useful MADs" = matmul ops hashed for mining (<a href="https://huggingface.co/pearl-ai/Llama-3.3-70B-Instruct-pearl">HF benchmark table</a>), not MADs sold downstream.</p>
</div>

| Question | Factual answer (sourced) |
|----------|------------------------|
| **Who uses Pearl matmul?** | Pearl Labs, GPU miners, Together AI (Gemma endpoint only) |
| **Do HF downloads = users?** | No — likely miner weight pulls; 0 inference providers on Llama-pearl |
| **Does mainnet = utility?** | No — proves mining; ~2.7k PRL/block is subsidy, not API revenue |
| **Is useful work enforced?** | No — whitepaper admits non-useful compute; bare `mine()` valid |
| **Is Pearl API cheaper than market Gemma?** | No — ~2.3× above OpenRouter $0.12/$0.37; discount is vs Together list only |
| **Is Pearl matmul = industry matmul?** | No — int7/8-bit plugin stack vs BF16/FP8; Hopper-only; 3 models |
| **Can I swap google/gemma for pearl-ai?** | No — different checkpoint, quant, context; model migration not price toggle |
| **Strongest dual-use case** | [Together × Pearl](https://www.together.ai/blog/together-ai-partners-with-pearl-research-labs) — volume not public |

**AGTI read:** Useful work is **weakly evidenced** and **narrowly compatible**. The chain proves matmul lottery tickets; the one external API is **not market-competitive** on price and **not interchangeable** with commodity Gemma 4 31B on OpenRouter.

## 8. Whitepaper virtuous loop vs reality

<div class="pearl-figure">
  <div class="pearl-mermaid">
    <div class="mermaid">
flowchart TB
  subgraph virtuous ["Whitepaper virtuous loop"]
    direction TB
    V1[Market demand] --> V2[Subsidy value up]
    V2 --> V3[More useful compute deployed]
    V3 --> V4[Stronger security]
    V4 --> V5[Useful inference / training out]
    V5 --> V1
  end

  subgraph leak ["Protocol leak — no enforcement"]
    L1[Subsidy value up] --> L2[Specialized matmul farms]
    L2 --> L3[Security up]
    L3 --> L4[No useful output]
    L4 -.-> L1
  end

  V2 -. can bypass V3 .-> L2

  style virtuous fill:#0d2818,stroke:#6ee58f,color:#e8eeeb
  style leak fill:#2a1010,stroke:#ff5a42,color:#e8eeeb
    </div>
  </div>
</div>

## 9. Verdict & due diligence

| Question | Answer |
|----------|--------|
| **Technically serious?** | Yes — full node, zk-pow, CUDA, vLLM plugin |
| **ASIC feasible?** | Yes — GEMM + BLAKE3 + ZK are acceleratable |
| **ASIC useful?** | No — secures chain, produces no AI product |
| **Useful work evidenced?** | Thin — 1 Together endpoint; HF downloads ≠ demand |
| **Industry adopted matmul?** | No — int7/8-bit Pearl plugin on Hopper only; not fungible with BF16/FP8 `google/gemma-4-31b-it` |
| **Pearl API competitive?** | No — ~2.3× above OpenRouter Gemma pricing; `pearl-ai/*` is a different checkpoint |
| **Mass-adoption matmul thesis?** | Weak — parallel supply chain (3 models), not industry standard |

**Evaluators should track:**

1. Share of hashrate on vLLM vs bare clients (see **§6 simulation**)
2. Whether Pearl API prices undercut OpenRouter commodity Gemma (today: no — **§7 pricing**)
3. Whether developers adopt `pearl-ai/*` vs staying on `google/*` (model migration, not swap)
4. ZK proving cost vs search cost at target difficulty
5. Together endpoint traffic + HF inference provider count on pearl-ai models

Full source doc with code citations: `agti/docs/pearl_pouw_useful_work_asic_analysis_2026-05-23.md`

---

*Disclaimer: Independent AGTI research for informational purposes only. Not investment advice.*
