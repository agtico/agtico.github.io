---
layout: report
title: "Pearl PoUW | Useful Work vs. Consensus"
date: "2026-05-23 18:00:00 +0000"
summary: "AGTI analysis of Pearl's Proof-of-Useful-Work whitepaper and open-source miner: where dual-use AI mining ends and bare matmul lottery begins."
category: AGTI Research
pearl_report: true
report_css_version: 20260524
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
  <p>Pearl is only non-nonsensical as <em>inference-mining co-location</em>. At the protocol layer it is PoW with matmul puzzles and ZK receipts.</p>
</div>

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

## 6. Adoption & useful-work reality check

Pearl's pitch treats "matmul" as if the AI industry already runs it. **It doesn't.** Pearl uses a proprietary int7 / 7-bit noisy stack that only works inside their miner plugin — not standard BF16/FP8 inference.

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
    <text x="124" y="270" fill="#8aa898" text-anchor="middle" font-size="9">25% off via PRL subsidy</text>

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
| **Strongest dual-use case** | [Together × Pearl](https://www.together.ai/blog/together-ai-partners-with-pearl-research-labs) — volume not public |

**AGTI read:** Useful work is **weakly evidenced** and **narrowly compatible**. The chain proves matmul lottery tickets; external AI utility is essentially **one discounted Together endpoint** plus unverified miner-side inference.

## 7. Whitepaper virtuous loop vs reality

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

## 8. Verdict & due diligence

| Question | Answer |
|----------|--------|
| **Technically serious?** | Yes — full node, zk-pow, CUDA, vLLM plugin |
| **ASIC feasible?** | Yes — GEMM + BLAKE3 + ZK are acceleratable |
| **ASIC useful?** | No — secures chain, produces no AI product |
| **Useful work evidenced?** | Thin — 1 Together endpoint; HF downloads ≠ demand |
| **Industry adopted matmul?** | No — bespoke int7/7-bit plugin, 3 models |

**Evaluators should track:**

1. Share of hashrate on vLLM vs bare clients
2. Whether subsidies dominate inference margins
3. ZK proving cost vs search cost at target difficulty
4. Whether Together endpoint traffic grows (only public dual-use off-ramp)
5. HF inference provider count on pearl-ai models (still zero on Llama)

Full source doc with code citations: `agti/docs/pearl_pouw_useful_work_asic_analysis_2026-05-23.md`

---

*Disclaimer: Independent AGTI research for informational purposes only. Not investment advice.*
