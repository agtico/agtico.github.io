---
layout: report
title: "Pearl PoUW | Useful Work vs. Consensus"
date: "2026-05-23 18:00:00 +0000"
summary: "AGTI analysis of Pearl's Proof-of-Useful-Work whitepaper and open-source miner: where dual-use AI mining ends and bare matmul lottery begins."
category: AGTI Research
pearl_report: true
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

## 6. Whitepaper virtuous loop vs reality

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

## 7. Verdict & due diligence

| Question | Answer |
|----------|--------|
| **Technically serious?** | Yes — full node, zk-pow, CUDA, vLLM plugin |
| **ASIC feasible?** | Yes — GEMM + BLAKE3 + ZK are acceleratable |
| **ASIC useful?** | No — secures chain, produces no AI product |
| **Whitepaper ASIC defense** | Economic only; assumes inference revenue beats subsidy |

**Evaluators should track:**

1. Share of hashrate on vLLM vs bare clients
2. Whether subsidies dominate inference margins
3. ZK proving cost vs search cost at target difficulty

Full source doc with code citations: `agti/docs/pearl_pouw_useful_work_asic_analysis_2026-05-23.md`

---

*Disclaimer: Independent AGTI research for informational purposes only. Not investment advice.*
