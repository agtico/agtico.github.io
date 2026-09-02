---
layout: report
title: "Fluidstack at $18B | Series B Evaluation"
date: "2026-09-02 16:00:00 +0000"
summary: "AGTI evaluation of the reported $18B Fluidstack preferred round: expanded public comps, credit-market read, landlord earnings-call evidence, per-GW re-underwriting, scenarios, and a verdict."
category: AGTI Research
pearl_report: true
report_css_version: 20260902a
copy_article: true
tags:
  - AGTI
  - Fluidstack
  - AI Infrastructure
  - Data Centers
  - Private Markets
---

<div class="pearl-hero-grid">
  <div class="pearl-scorecard good">
    <span class="label">Verdict</span>
    <span class="value">Participate, senior pref, venture size</span>
    <span class="hint">Real Google-guaranteed leases plus a 1x preference cap the loss case; upside if Indiana and Harlingen land is 4-10x.</span>
  </div>
  <div class="pearl-scorecard warn">
    <span class="label">Price vs fair value</span>
    <span class="value">$18B ask vs $10-14B fair</span>
    <span class="hint">10-12x schedule-adjusted 2027E EBITDA. The premium buys the Anthropic spread and the unsigned pipeline.</span>
  </div>
  <div class="pearl-scorecard bad">
    <span class="label">Walk triggers</span>
    <span class="value">Junior pref, >$20B post, Harlingen unsigned</span>
    <span class="hint">Any one of these breaks the risk-reward; the bust case needs the pref to return capital.</span>
  </div>
</div>

<div class="pearl-verdict-banner">
  <strong>AGTI bottom line</strong>
  <p>Fluidstack is a credit-tenant operator with a small owned asset base, priced as a landlord. Its cost-of-capital edge is real and visible in the bond market (Google-guaranteed leases fund at 6.7-7.6%; CoreWeave's balance-sheet model at 12-13%). The March 2026 round materials overstate per-GW equity by 2-4x, the anchor Indiana site has slipped 6-12 months, Anthropic now signs landlords directly, and Google has opened a second TPU channel. At $18B with a senior 1x preference the probability-weighted return is still ~2.9x. Participate at a size you can lose.</p>
</div>

## 0. Start here

<div class="pearl-primer-box">
  <p><strong>One-liner:</strong> Fluidstack builds and operates AI data centers for Anthropic on Google TPUs, with Google guaranteeing the leases. It is raising ~$1B at a reported $18B, up from $7.5B in January 2026. The question is whether the step-up reflects a business that changed or a market that did.</p>
  <p style="margin-top:12px"><strong>Read next:</strong> <a href="#3-what-fluidstack-operates-vs-what-it-owns">Asset map</a> · <a href="#4-the-credit-market-already-priced-this">Credit ladder</a> · <a href="#6-re-underwriting-the-per-gw-math">Per-GW re-underwriting</a> · <a href="#7-scenarios">Scenarios</a> · <a href="#appendix-expanded-public-comps">Full comp tables</a></p>
</div>

## 1. Where the valuation actually is

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Valuation walk, January to September 2026</h3>
    <span class="tag">Timeline</span>
  </div>
  <svg viewBox="0 0 920 230" role="img" aria-label="Fluidstack valuation timeline from $7.5B Series A to $18B target">
    <line x1="60" y1="150" x2="880" y2="150" stroke="#3a4a44" stroke-width="2"/>
    <g>
      <circle cx="110" cy="150" r="8" fill="#6ee58f"/>
      <text x="110" y="120" fill="#9dffc8" text-anchor="middle" font-size="18" font-weight="800">$7.5B</text>
      <text x="110" y="180" fill="#8aa898" text-anchor="middle" font-size="11">Jan 2026</text>
      <text x="110" y="196" fill="#8aa898" text-anchor="middle" font-size="11">Series A, $830M</text>
      <text x="110" y="212" fill="#8aa898" text-anchor="middle" font-size="11">Situational Awareness</text>
    </g>
    <g>
      <circle cx="360" cy="150" r="8" fill="#ffb347"/>
      <text x="360" y="120" fill="#ffd38a" text-anchor="middle" font-size="18" font-weight="800">$17B pre</text>
      <text x="360" y="180" fill="#8aa898" text-anchor="middle" font-size="11">Mar 2026</text>
      <text x="360" y="196" fill="#8aa898" text-anchor="middle" font-size="11">Round materials</text>
      <text x="360" y="212" fill="#8aa898" text-anchor="middle" font-size="11">$20B asking</text>
    </g>
    <g>
      <circle cx="600" cy="150" r="8" fill="#ffb347"/>
      <text x="600" y="120" fill="#ffd38a" text-anchor="middle" font-size="18" font-weight="800">$18B target</text>
      <text x="600" y="180" fill="#8aa898" text-anchor="middle" font-size="11">Apr 14, 2026</text>
      <text x="600" y="196" fill="#8aa898" text-anchor="middle" font-size="11">~$1B, Jane Street +</text>
      <text x="600" y="212" fill="#8aa898" text-anchor="middle" font-size="11">Situational Awareness</text>
    </g>
    <g>
      <circle cx="830" cy="150" r="8" fill="#ff5a42"/>
      <text x="830" y="120" fill="#ffb4a8" text-anchor="middle" font-size="18" font-weight="800">$18B pref</text>
      <text x="830" y="180" fill="#8aa898" text-anchor="middle" font-size="11">Sep 2026</text>
      <text x="830" y="196" fill="#8aa898" text-anchor="middle" font-size="11">Series B preferred</text>
      <text x="830" y="212" fill="#8aa898" text-anchor="middle" font-size="11">placing via SPVs</text>
    </g>
    <text x="235" y="60" fill="#6ee58f" text-anchor="middle" font-family="monospace" font-size="11">+127% in 2 months</text>
    <text x="715" y="60" fill="#ff5a42" text-anchor="middle" font-family="monospace" font-size="11">+140% in 8 months</text>
  </svg>
  <p class="pearl-figure-caption">A Series B preferred class exists (a listed fund disclosed a $15M SPV purchase dated July 16, 2026). The $18B print itself has not been publicly confirmed.</p>
</div>

## 2. Multiples: premium to neoclouds, discount to its own landlords

Consensus fiscal-year estimates for 22 public peers were pulled September 2, 2026 (full tables in the <a href="#appendix-expanded-public-comps">appendix</a>). Fluidstack is shown on both the management plan from the round materials and a schedule-adjusted case that cuts 2027 by a third for the slippage documented in <a href="#5-what-the-landlords-said-on-their-earnings-calls">section 5</a>.

| Bucket | EV/Sales 2027E | EV/EBITDA 2027E | EV/EBITDA 2028E | Revenue growth 2027E |
|---|---|---|---|---|
| Neocloud (CRWV, NBIS, IREN, DOCN, ORCL) | 4.2x | 7.4x | 4.0x | 106% |
| Powered land / HPC developers (APLD, WULF, CIFR, CORZ, HUT, GLXY) | 9.0x | 20.0x | 11.9x | 121% |
| Data center REITs / operators (EQIX, DLR, IRM, GDS, NXT, KDCREIT, DBRG) | 9.1x | 19.2x | 17.8x | 10% |
| Infra software (DDOG, NET, NOW, DT) | 11.0x | 38.6x | 31.1x | 21% |
| **Fluidstack @ $19B post, management plan** | **7.4x** | **12.3x** | **4.2x** | **286%** |
| **Fluidstack @ $19B post, schedule-adjusted** | **~11x** | **~18x** | **~6x** | **~160%** |

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>EV / 2027E EBITDA, bucket medians vs Fluidstack</h3>
    <span class="tag">Bar chart</span>
  </div>
  <div class="pearl-bar-chart">
    <div class="pearl-bar-row">
      <span class="name">Infra software</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:96%"></div></div>
      <span class="pct">38.6x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Powered land devs</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:50%"></div></div>
      <span class="pct" style="color:#6ee58f">20.0x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">DC REITs</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:48%"></div></div>
      <span class="pct" style="color:#6ee58f">19.2x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Fluidstack adjusted</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill amber" style="width:45%"></div></div>
      <span class="pct" style="color:#ffb347">~18x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Fluidstack plan</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill amber" style="width:31%"></div></div>
      <span class="pct" style="color:#ffb347">12.3x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Neoclouds</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:19%"></div></div>
      <span class="pct">7.4x</span>
    </div>
  </div>
  <p class="pearl-figure-caption">On the plan, $18B sits between the model Fluidstack is leaving (neoclouds) and the model it is becoming (developers, REITs). Schedule-adjusted, it lands on top of TeraWulf (19.6x) and Cipher (20.4x), the companies that own the buildings Fluidstack rents.</p>
</div>

Private cross-section: Crusoe is reported in talks near $30B on ~4.9 GW contracted, about $6.1B per contracted GW. Fluidstack at $18B on ~1.41 GW-IT operated is ~$12.8B per contracted GW, roughly twice Crusoe, while owning only ~0.3 GW-IT of it.

## 3. What Fluidstack operates vs what it owns

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Contracted IT capacity by site and by ownership</h3>
    <span class="tag">Asset map</span>
  </div>
  <svg viewBox="0 0 920 330" role="img" aria-label="Fluidstack operated versus owned IT capacity by site">
    <text x="40" y="30" fill="#6ee58f" font-family="monospace" font-size="11" font-weight="700">OPERATED AS TENANT OR OWNER: ~1.41 GW-IT</text>
    <rect x="40" y="42" width="243" height="44" fill="#0d2818" stroke="#6ee58f" rx="3"/>
    <text x="161" y="61" fill="#9dffc8" text-anchor="middle" font-size="12" font-weight="700">Meridian IN 430 MW</text>
    <text x="161" y="77" fill="#8aa898" text-anchor="middle" font-size="10">49.9% owned · Jul 2027</text>
    <rect x="283" y="42" width="204" height="44" fill="#14201a" stroke="#6ee58f" rx="3"/>
    <text x="385" y="61" fill="#9dffc8" text-anchor="middle" font-size="12" font-weight="700">Lake Mariner NY ~360</text>
    <text x="385" y="77" fill="#8aa898" text-anchor="middle" font-size="10">tenant · CB-3 live Jul 2026</text>
    <rect x="487" y="42" width="139" height="44" fill="#14201a" stroke="#6ee58f" rx="3"/>
    <text x="556" y="61" fill="#9dffc8" text-anchor="middle" font-size="12" font-weight="700">River Bend LA 245</text>
    <text x="556" y="77" fill="#8aa898" text-anchor="middle" font-size="10">tenant · Q2 2027</text>
    <rect x="626" y="42" width="117" height="44" fill="#14201a" stroke="#6ee58f" rx="3"/>
    <text x="684" y="61" fill="#9dffc8" text-anchor="middle" font-size="12" font-weight="700">Barber Lake 207</text>
    <text x="684" y="77" fill="#8aa898" text-anchor="middle" font-size="10">tenant · Q4 2026</text>
    <rect x="743" y="42" width="95" height="44" fill="#0d2818" stroke="#6ee58f" rx="3"/>
    <text x="790" y="61" fill="#9dffc8" text-anchor="middle" font-size="12" font-weight="700">Abernathy 168</text>
    <text x="790" y="77" fill="#8aa898" text-anchor="middle" font-size="10">majority · H2 2026</text>

    <text x="40" y="130" fill="#ffb347" font-family="monospace" font-size="11" font-weight="700">EQUITY-OWNED: ~0.30 GW-IT</text>
    <rect x="40" y="142" width="121" height="44" fill="#2a1e0c" stroke="#ffb347" rx="3"/>
    <text x="100" y="161" fill="#ffd38a" text-anchor="middle" font-size="12" font-weight="700">IN 215 MW</text>
    <text x="100" y="177" fill="#a89468" text-anchor="middle" font-size="10">49.9% of 430</text>
    <rect x="161" y="142" width="50" height="44" fill="#2a1e0c" stroke="#ffb347" rx="3"/>
    <text x="186" y="161" fill="#ffd38a" text-anchor="middle" font-size="11" font-weight="700">TX ~85</text>
    <text x="186" y="177" fill="#a89468" text-anchor="middle" font-size="9">Abernathy</text>
    <rect x="211" y="142" width="627" height="44" fill="none" stroke="#3a4a44" stroke-dasharray="4 4" rx="3"/>
    <text x="524" y="169" fill="#8aa898" text-anchor="middle" font-size="11">tenant / operator spread on the remaining ~1.1 GW-IT</text>

    <text x="40" y="230" fill="#ff5a42" font-family="monospace" font-size="11" font-weight="700">GUIDED, UNSIGNED ON THE PUBLIC RECORD</text>
    <rect x="40" y="242" width="500" height="44" fill="#2a1010" stroke="#ff5a42" stroke-dasharray="5 4" rx="3"/>
    <text x="290" y="261" fill="#ffb4a8" text-anchor="middle" font-size="12" font-weight="700">Harlingen / Cameron County TX ~1.1 GW-IT (to 4.5 GW utility)</text>
    <text x="290" y="277" fill="#a87878" text-anchor="middle" font-size="10">no executed lease located · drainage dispute with the city · construction "in the fall"</text>
    <rect x="540" y="242" width="298" height="44" fill="#2a1010" stroke="#ff5a42" stroke-dasharray="5 4" rx="3"/>
    <text x="689" y="261" fill="#ffb4a8" text-anchor="middle" font-size="12" font-weight="700">Indiana remainder ~1.17 GW-IT</text>
    <text x="689" y="277" fill="#a87878" text-anchor="middle" font-size="10">50/50 JV guided · unfinanced</text>
    <text x="40" y="318" fill="#8aa898" font-size="11">Scale: 1 px = 1.75 MW-IT across all three rows</text>
  </svg>
  <p class="pearl-figure-caption">About 80% of contracted capacity is a tenant/operator spread on Anthropic volume, guaranteed by Google. About 20% is owned development equity. The round materials value the whole as a landlord.</p>
</div>

| Site / landlord | MW-IT | Fluidstack role | Status | Google support |
|---|---|---|---|---|
| Meridian, New Lebanon IN | 430 | Tenant + 49.9% owner | First hall Jul 2027 | Full lease guarantee (basis for BB) |
| TeraWulf Lake Mariner NY (CB-3/4/5) | ~360 | Tenant | CB-3 online Jul 2026; CB-4 commissioning; CB-5 Jan 2027 | $3.2B backstop; ~14% TeraWulf equity to Google |
| Cipher Barber Lake TX | 207 | Tenant | Phase 1 Sep/Oct 2026 | $1.73B backstop; 5.4% Cipher warrants |
| Abernathy TX | 168 (to ~500) | Majority owner + tenant | H2 2026, sequencing open | $1.3B backstop |
| Hut 8 River Bend LA | 245 | Tenant; ROFO on +1 GW | First hall Q2 2027 | Full 15-year base-term backstop |
| Harlingen / Cameron County TX | guided 1.1 GW-IT | Developer | LOI / permitting | none located |

### What changed since the March round materials

<div class="pearl-flow">
  <div class="pearl-flow-step"><span class="num">1</span><strong>Indiana financed, not sold</strong><span>$5.7B 6.25% 2031 notes; 49.9% Fluidstack / 50.1% Coatue Next Frontier; Google-guaranteed 15-year NNN. Replaces the Blue Owl sale in the materials.</span></div>
  <div class="pearl-flow-step"><span class="num">2</span><strong>Phase 1 slipped 6-12 months</strong><span>Guided Q4 2026 / Q1 2027. County and rating-agency timetable: first 65 MW hall July 2027, full Phase 1 end-2027 / early 2028.</span></div>
  <div class="pearl-flow-step"><span class="num">3</span><strong>Harlingen unsigned</strong><span>No executed lease, JV percentage, tenant or guarantee on the public record. City drainage approval described as critical.</span></div>
  <div class="pearl-flow-step"><span class="num">4</span><strong>Anthropic goes direct</strong><span>TeraWulf 401 MW / 20-year / ~$19B; Nscale ~$45B; Hut 8 1 GW diligence; CoreWeave added. Anthropic is seeking Google guarantees directly.</span></div>
  <div class="pearl-flow-step"><span class="num">5</span><strong>TPU exclusivity refuted</strong><span>Google-Blackstone "TPU Co." ($5B, 500 MW in 2027); Google now ships TPU systems into customers' own data centers.</span></div>
</div>

## 4. The credit market already priced this

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Yield to maturity, dealer mid quotes, September 2, 2026</h3>
    <span class="tag">Credit ladder</span>
  </div>
  <div class="pearl-bar-chart">
    <div class="pearl-bar-row">
      <span class="name">CoreWeave 9.75% 2031 unsec</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:96%"></div></div>
      <span class="pct" style="color:#ff5a42">12.83%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">CoreWeave 9.25% 2030 unsec</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:89%"></div></div>
      <span class="pct" style="color:#ff5a42">11.89%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Applied Digital 9.25% 2030 sec</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill amber" style="width:58%"></div></div>
      <span class="pct" style="color:#ffb347">7.80%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Meridian Arc 6.25% 2031 sec</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill amber" style="width:57%"></div></div>
      <span class="pct" style="color:#ffb347">7.55%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Cipher Compute 7.125% 2030</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:50%"></div></div>
      <span class="pct" style="color:#6ee58f">6.73%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">WULF Compute 7.75% 2030</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:50%"></div></div>
      <span class="pct" style="color:#6ee58f">6.68%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Oracle 6.25% 2032</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:48%"></div></div>
      <span class="pct" style="color:#6ee58f">6.39%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Equinix 3.9% 2032</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:41%"></div></div>
      <span class="pct" style="color:#6ee58f">5.49%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Alphabet 4.5% 2035</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:40%"></div></div>
      <span class="pct" style="color:#6ee58f">5.29%</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">UST 5Y</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:34%"></div></div>
      <span class="pct">4.53%</span>
    </div>
  </div>
  <p class="pearl-figure-caption">Three tiers: Google-guaranteed Fluidstack leases (6.7-7.6%), CoreWeave-tenant secured paper (7.8%), CoreWeave's own unsecured balance sheet (12-13%, 5Y CDS ~848bp). The pivot from chips to buildings is validated by the debt market. The 6% cap rate in the round materials sits below the project's own cost of debt.</p>
</div>

| Bond | YTM | Spread vs 5Y UST | Rating |
|---|---|---|---|
| Meridian Arc 6.25% 2031 (Fluidstack's Indiana Phase 1; Google-guaranteed lease) | 7.55% (px 95.0; par at issue Apr 24) | 301bp | BB- / Ba2 / BB |
| WULF Compute 7.75% 2030 (Fluidstack tenant, Google backstop) | 6.68% | 214bp | BB / Ba2 |
| Cipher Compute 7.125% 2030 (Fluidstack tenant, Google backstop) | 6.73% | 220bp | Ba3 |
| Applied Digital 9.25% 2030 (CoreWeave tenant) | 7.80% | 327bp | BB- |
| CoreWeave 9.25% 2030 / 9.75% 2031 unsecured | 11.9% / 12.8% | 736 / 830bp | B / B1 |
| Oracle 6.25% 2032 (5Y CDS 212bp) | 6.39% | 185bp | BBB- |
| Equinix 3.9% 2032 / Digital Realty 5.55% 2028 | 5.49% / 4.85% | 96 / 32bp | BBB+ |
| Alphabet 4.5% 2035 (the guarantor) | 5.29% | 75bp | AA+ |

Two cautions. The gap is narrowing: CoreWeave cut its weighted average cost of debt ~300bp in a year, and Applied Digital refinanced CoreWeave-tenant paper at 7% behind an A3-rated SPV. And the window is tightening: AI-infrastructure high-yield issuance reached ~$32B by early July, the year's largest prints (Meridian Arc, Core Scientific, Tract) trade below par, and data-center HY spreads have re-widened since June. Meridian's 5-point drop in four months is the cleanest signal.

## 5. What the landlords said on their earnings calls

Thirty-one transcripts, latest two quarters, for TeraWulf, Cipher, Applied Digital, CoreWeave, Nebius, IREN, Core Scientific, Hut 8, Galaxy, Equinix, Digital Realty, Oracle, Alphabet and Vertiv.

<div class="pearl-source-grid">
  <div class="pearl-source-card">
    <span class="src-label">TeraWulf Q1 vs Q2 2026</span>
    <span class="src-stat">Every Fluidstack building slipped 1-3 quarters</span>
    <span class="src-note">May: CB-3 end of May, CB-4 Q3, CB-5 Q4, timelines "unchanged." August: CB-3 live early July, leases amended hall by hall, CB-5 first hall "very early January." Cause: the reference design "has changed over time, particularly as our clients get more experience running the hardware," plus electrician shortages.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">TeraWulf Q2 2026</span>
    <span class="src-stat">Anthropic direct: 401 MW, 20 years, ~$19B</span>
    <span class="src-note">"We get to deal direct now with our customer in Anthropic, and it just makes things that much more efficient." TeraWulf also sold its 50.1% of Abernathy to Fluidstack for ~$530M, a 20% IRR in about nine months.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Applied Digital Q4 FY26</span>
    <span class="src-stat">Backstop leases "face uncertainty after the initial 5-year tenor"</span>
    <span class="src-note">Why Applied now prefers direct hyperscaler leases. Hut 8 says Google covers the full 15-year base term at River Bend; Core Scientific says AMD's support has no equity step-in "as you've seen included in some other deals." Tenor and step-in at Fluidstack's owned sites are the first diligence item.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Alphabet Q1 and Q2 2026</span>
    <span class="src-stat">TPU systems delivered into customers' own data centers</span>
    <span class="src-note">First deliveries in Q2, revenue mostly 2027, inside the $514B cloud backlog. Google is also "expanding the use of third-party capacity in Q3 as a bridging strategy," which is direct Google demand for Fluidstack-type sites.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Build cost, five landlords</span>
    <span class="src-stat">$9-13M per MW-IT and rising</span>
    <span class="src-note">TeraWulf $9.1M/MW; Cipher and Hut 8 $9-11M; Core Scientific $11-12M for AMD; Vertiv content alone $3.25-3.75M; Meridian implied $13.3M including land, substation and reserves. The round materials assume $10-11M.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Lease economics</span>
    <span class="src-stat">Mid-teens yield on cost, and Fluidstack pays it</span>
    <span class="src-note">TeraWulf targets mid-teens and achieved it with both Fluidstack and Anthropic. Digital Realty develops at 11.5% stabilized yield with 25%+ renewal spreads. The $130/kW-month rate in the materials holds for turnkey; on ~1.1 GW-IT Fluidstack is the one paying it.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">IREN Q4 FY26, Nebius Q2 2026</span>
    <span class="src-stat">GPU contracts at $20-25M per MW, 2-year payback</span>
    <span class="src-note">3-year contract pricing up ~125% since November; prepayments fund 45-60% of GPU capex. Fluidstack's retreat from chip ownership gives up the highest-return segment in the current market. The materials present it as a pure upgrade.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Hut 8 Q2, Core Scientific Q4</span>
    <span class="src-stat">"Credit committees do not finance ambition"</span>
    <span class="src-note">Project debt is deep only behind investment-grade support. Google has priced its support in equity (TeraWulf ~14%, Cipher 5.4%). Expect the same in Fluidstack's JVs.</span>
  </div>
</div>

## 6. Re-underwriting the per-GW math

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Equity value per owned GW-IT: round materials vs evidence</h3>
    <span class="tag">Waterfall</span>
  </div>
  <svg viewBox="0 0 920 300" role="img" aria-label="Waterfall from $12.2B per GW in the round materials to $3-5.5B per GW on evidence">
    <line x1="60" y1="250" x2="880" y2="250" stroke="#3a4a44"/>
    <rect x="80" y="55" width="90" height="195" fill="#2a1e0c" stroke="#ffb347"/>
    <text x="125" y="45" fill="#ffd38a" text-anchor="middle" font-size="13" font-weight="800">$12.2B</text>
    <text x="125" y="270" fill="#8aa898" text-anchor="middle" font-size="10">Materials</text>
    <rect x="210" y="55" width="90" height="83" fill="#2a1010" stroke="#ff5a42"/>
    <text x="255" y="45" fill="#ffb4a8" text-anchor="middle" font-size="12" font-weight="700">-$5.2B</text>
    <text x="255" y="270" fill="#8aa898" text-anchor="middle" font-size="10">Cap 6% to 8%</text>
    <rect x="340" y="138" width="90" height="16" fill="#2a1010" stroke="#ff5a42"/>
    <text x="385" y="128" fill="#ffb4a8" text-anchor="middle" font-size="12" font-weight="700">-$1.0B</text>
    <text x="385" y="270" fill="#8aa898" text-anchor="middle" font-size="10">Build $10M to $12M</text>
    <rect x="470" y="154" width="90" height="16" fill="#2a1010" stroke="#ff5a42"/>
    <text x="515" y="144" fill="#ffb4a8" text-anchor="middle" font-size="12" font-weight="700">-$1.0B</text>
    <text x="515" y="270" fill="#8aa898" text-anchor="middle" font-size="10">Debt on cost, DSCR</text>
    <rect x="600" y="170" width="90" height="80" fill="#2a1010" stroke="#ff5a42"/>
    <text x="645" y="160" fill="#ffb4a8" text-anchor="middle" font-size="12" font-weight="700">-$5.0B</text>
    <text x="645" y="270" fill="#8aa898" text-anchor="middle" font-size="10">Software 40x to 15x</text>
    <rect x="730" y="170" width="90" height="80" fill="#0d2818" stroke="#6ee58f"/>
    <rect x="730" y="130" width="90" height="40" fill="none" stroke="#6ee58f" stroke-dasharray="4 4"/>
    <text x="775" y="120" fill="#9dffc8" text-anchor="middle" font-size="13" font-weight="800">$3-5.5B</text>
    <text x="775" y="270" fill="#8aa898" text-anchor="middle" font-size="10">Evidence</text>
    <text x="470" y="292" fill="#8aa898" text-anchor="middle" font-size="10">Bars are indicative; the ordering of adjustments changes individual bar sizes, not the endpoint.</text>
  </svg>
</div>

| Item | Round materials | Evidence | Basis |
|---|---|---|---|
| Build cost per MW-IT | ~$10-11M | $11-13M | Five landlords; Meridian implied |
| NOI per GW ($130/kW-mo, ~80% margin) | $1.25B | $1.25B | TeraWulf mid-teens yield on cost |
| Cap rate | 6% | 8-9% | Meridian YTM 7.55%; NNN range mid-4% to ~8% |
| Gross asset value per GW | $20.8B | $14-15.6B | |
| Debt per GW | 80% LTV of value = $16.6B | ~$13B on cost | Meridian: $5.7B on 430 MW |
| Infra equity per GW | $4.2B | $1-2.6B | |
| Google equity step-in / warrants | none | expect dilution | TeraWulf, Cipher precedents |
| Software EBITDA multiple ($200M) | 40x = $8B | 10-15x = $2-3B | TPUs procured by Anthropic/Broadcom; Google selling direct |
| **Equity per owned GW** | **$12.2B** | **$3-5.5B** | |

Cross-check: ~$530M for 50.1% of Abernathy (168 MW-IT scaling toward ~500, pre-commencement) implies ~$1.06B of JV equity, or $2-6M per MW depending on the capacity basis, consistent with the evidence column.

Attributable today: ~0.3 GW-IT owned. The Indiana remainder (~1.17 GW at 50%) and Harlingen (1.1 GW at 80%) are neither financed nor leased. Owned capacity at $3-5.5B per GW plus the tenant spread on ~1.1 GW-IT capitalized at 9-13x gives ~$10-14B today. $18B requires the Indiana remainder and Harlingen to close at the terms in the materials.

## 7. Scenarios

Entry $19B post, exit 2029-30, 10x forward EBITDA for a landlord-heavy mix, net of ~$8B consolidated debt.

| Case | Probability | 2030E EBITDA | Exit equity | MOIC common | MOIC 1x pref |
|---|---|---|---|---|---|
| Plan delivered | 10% | $20B | ~$190B | 10x | 10x |
| Half of plan, one-year slip | 30% | $8B | ~$72B | 3.8x | 3.8x |
| Anchors only; Harlingen and pipeline stall | 40% | $3B | ~$25B | 1.3x | 1.3x |
| Anthropic shifts direct, Google support capped, JV equity impaired | 20% | <$1B | ~$3-5B | 0.2x | ~1.0x |
| **Probability-weighted** | | | | **2.7x** | **2.9x** |

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>MOIC on a 1x preferred, by scenario</h3>
    <span class="tag">Outcome map</span>
  </div>
  <div class="pearl-bar-chart">
    <div class="pearl-bar-row">
      <span class="name">Plan delivered (10%)</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:100%"></div></div>
      <span class="pct" style="color:#6ee58f">10x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Half plan, 1-yr slip (30%)</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill green" style="width:38%"></div></div>
      <span class="pct" style="color:#6ee58f">3.8x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Anchors only (40%)</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill amber" style="width:13%"></div></div>
      <span class="pct" style="color:#ffb347">1.3x</span>
    </div>
    <div class="pearl-bar-row">
      <span class="name">Bust (20%)</span>
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:10%"></div></div>
      <span class="pct">~1.0x</span>
    </div>
  </div>
  <p class="pearl-figure-caption">The preference is doing real work: it lifts the bust case from 0.2x to roughly 1.0x, provided the total preference stack stays below any realistic distressed value of the JV interests and contracts.</p>
</div>

## 8. Verdict and conditions

<div class="pearl-split">
  <div class="pearl-panel good">
    <h4>Participate because</h4>
    <ul>
      <li>Cost-of-capital moat is real and priced in the bond market (6.7-7.6% vs 12-13%).</li>
      <li>Google is a guarantor on Indiana, not a handshake; Fitch rated the bond on that basis.</li>
      <li>Anthropic's compute deficit is real; 15-year NNN leases turn it into bond-like cash flow.</li>
      <li>Its own landlords trade at 12-20x 2027 EBITDA with a fraction of the growth.</li>
      <li>A senior 1x pref converts the bust case into roughly a return of capital.</li>
    </ul>
  </div>
  <div class="pearl-panel bad">
    <h4>Walk if</h4>
    <ul>
      <li>The pref is junior to the Series A, or pari passu without pro-rata.</li>
      <li>Post-money above ~$20B.</li>
      <li>Harlingen is still an LOI at close.</li>
      <li>The Google guarantee at Indiana is capped, 5-year, or carries step-in rights the materials omit.</li>
      <li>Indiana first-hall energization slips past Q1 2028.</li>
    </ul>
  </div>
</div>

### Diligence before wiring

1. Executed Harlingen lease, JV percentage, tenant, Google guarantee, utility-approved MW and energization date.
2. Tenor, caps and termination triggers of the Google guarantee at Indiana (Phase 1 vs full campus); any Google warrants or step-in rights in Meridian or Abernathy.
3. Anthropic contract form at each site: take-or-pay compute (5-year) vs NNN colo (15-year); termination for convenience; what happens when Anthropic signs the next campus direct.
4. Completion guarantees and equity-contribution schedule on Meridian, Abernathy and future JVs; parent GPU facility drawn balance and covenants.
5. Revised 2027 and 2028 revenue and EBITDA reflecting July 2027 Meridian, Q2 2027 River Bend and the TeraWulf CB-4/CB-5 amendments; attested margin on the Anthropic spread.
6. Written terms of Google TPU allocation to Fluidstack vs the Blackstone TPU Co. and direct TPU-system sales programs.
7. Preference terms: seniority, participation, anti-dilution, pro-rata, information rights; final round size, lead and pre/post.

## Appendix: expanded public comps

Consensus fiscal-year estimates and trailing fundamentals for 22 public peers, pulled September 2, 2026. USD millions unless noted; non-USD reporters converted at spot for size columns only; ratios are currency-neutral. FY1/FY2/FY3 are consensus fiscal years and fiscal year ends differ (Applied Digital May, IREN June, most others December).

### A. Size, growth, multiples

| Ticker | Name | Mkt cap | EV | LTM sales | Rev g FY1 | Rev g FY2 | Rev g FY3 | EV/S FY1 | EV/S FY2 | EV/EBITDA FY1 | EV/EBITDA FY2 | EV/EBITDA FY3 | P/E FY2 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **Neocloud / GPU compute (FS 2025 model)** | | | | | | | | | | | | | |
| CRWV US | CoreWeave | 45.7B | 91.8B | 7.6B | 152% | 106% | 60% | 7.1x | 3.5x | 12.1x | 5.5x | 3.3x | na |
| NBIS US | Nebius | 57.4B | 59.4B | 1.4B | 521% | 245% | 100% | 18.1x | 5.2x | 44.3x | 9.4x | 4.0x | na |
| IREN US | IREN | 14.5B | 16.5B | 707M | 300% | 147% | 69% | 5.8x | 2.4x | 9.3x | 3.2x | 1.8x | 27.2x |
| DOCN US | DigitalOcean | 12.8B | 13.4B | 1.0B | 31% | 54% | 48% | 11.4x | 7.4x | 29.2x | 18.4x | 12.0x | 61.0x |
| ORCL US | Oracle | 407.1B | 548.2B | 67.4B | 33% | 45% | 41% | 6.1x | 4.2x | 10.6x | 7.4x | 5.2x | 13.3x |
| *median* | | | | | 152% | 106% | 60% | 7.1x | 4.2x | 12.1x | 7.4x | 4.0x | 27.2x |
| **Powered land / HPC developers (FS landlords + 2026 model peers)** | | | | | | | | | | | | | |
| APLD US | Applied Digital | 7.1B | 12.6B | 611M | 34% | 148% | 49% | 15.4x | 6.2x | 34.5x | 11.2x | 6.7x | na |
| WULF US | TeraWulf | 7.3B | 9.9B | 165M | 48% | 252% | 81% | 39.8x | 11.3x | 167.3x | 19.6x | 9.8x | na |
| CIFR US | Cipher Mining | 6.1B | 10.8B | 191M | -4% | 273% | 17% | 50.3x | 13.5x | na | 20.4x | 14.1x | na |
| CORZ US | Core Scientific | 5.2B | 7.8B | 440M | 116% | 70% | 50% | 11.3x | 6.7x | 36.2x | 12.0x | 6.9x | 108.6x |
| HUT US | Hut 8 | 9.6B | 17.3B | 318M | 24% | 93% | 144% | 59.2x | 30.6x | na | 55.2x | 16.8x | na |
| GLXY US | Galaxy Digital | 9.2B | 15.2B | 58.4B | -39% | 18% | 14% | 0.4x | 0.3x | na | 35.1x | 17.9x | na |
| *median* | | | | | 29% | 121% | 50% | 27.6x | 9.0x | 36.2x | 20.0x | 11.9x | 108.6x |
| **Data center REITs / operators (FS 2026 identity)** | | | | | | | | | | | | | |
| EQIX US | Equinix | 101.4B | 122.5B | 9.8B | 11% | 10% | 11% | 11.9x | 10.8x | 23.4x | 20.8x | 18.5x | 52.5x |
| DLR US | Digital Realty | 69.0B | 87.1B | 6.8B | 16% | 11% | 14% | 12.2x | 11.0x | 22.6x | 20.2x | 17.8x | 57.9x |
| IRM US | Iron Mountain | 34.0B | 54.1B | 7.6B | 16% | 9% | 8% | 6.8x | 6.2x | 18.3x | 16.6x | 15.3x | 41.5x |
| GDS US | GDS Holdings | 6.3B | 11.7B | 12.3B | 11% | 10% | 22% | 6.2x | 5.6x | 13.2x | 12.7x | 10.5x | 181.3x |
| NXT AU | NEXTDC | 6.9B | 8.7B | 496M | 60% | 67% | 29% | 15.3x | 9.1x | 30.4x | 16.1x | 12.3x | na |
| KDCREIT SP | Keppel DC REIT | 4.2B | 5.8B | 472M | 9% | 7% | 4% | 15.4x | 14.4x | 20.3x | 19.2x | 18.6x | 17.2x |
| DBRG US | DigitalBridge | 3.0B | 3.9B | 633M | 294% | 15% | 16% | 10.5x | 9.1x | 32.1x | 25.3x | 18.6x | 24.9x |
| *median* | | | | | 16% | 10% | 14% | 11.9x | 9.1x | 22.6x | 19.2x | 17.8x | 47.0x |
| **Infra software (memo's software bucket)** | | | | | | | | | | | | | |
| DDOG US | Datadog | 80.6B | 76.9B | 4.0B | 30% | 23% | 22% | 17.2x | 14.0x | 71.5x | 56.9x | 45.4x | 72.9x |
| NET US | Cloudflare | 101.7B | 101.1B | 2.5B | 33% | 29% | 28% | 35.2x | 27.3x | 143.0x | 103.2x | 78.8x | 167.1x |
| NOW US | ServiceNow | 147.8B | 151.5B | 14.7B | 22% | 19% | 18% | 9.3x | 7.9x | 24.4x | 20.3x | 16.7x | 28.4x |
| DT US | Dynatrace | 15.3B | 14.3B | 2.1B | 15% | 15% | 15% | 6.2x | 5.4x | 20.5x | 17.5x | 14.6x | 23.3x |
| *median* | | | | | 26% | 21% | 20% | 13.3x | 11.0x | 47.9x | 38.6x | 31.1x | 50.7x |

### B. EBITDA, net income, capex, FCF (reporting currency)

| Ticker | Ccy | LTM EBITDA | EBITDA FY1 | EBITDA FY2 | EBITDA mgn LTM | EBITDA mgn FY2 | LTM NI | NI FY1 | NI FY2 | LTM capex | Capex FY1 | Capex FY2 | Capex/Sales FY1 | LTM FCF |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **Neocloud / GPU compute (FS 2025 model)** | | | | | | | | | | | | | | |
| CRWV US | USD | 4.9B | 7.6B | 16.7B | 64% | 63% | -1.9B | -2.2B | -1.1B | -21.5B | -35.5B | -48.0B | 275% | -14.5B |
| NBIS US | USD | 83M | 1.3B | 6.3B | 6% | 56% | 62M | -686M | -848M | -11.1B | -22.7B | -29.0B | 690% | -5.9B |
| IREN US | USD | -629M | 1.8B | 5.1B | -89% | 73% | -703M | -417M | 345M | -3.0B | -15.3B | -15.0B | 543% | -908M |
| DOCN US | USD | 322M | 460M | 730M | 32% | 40% | 235M | 177M | 236M | -270M | -216M | -323M | 18% | 40M |
| ORCL US | USD | 32.7B | 51.7B | 73.9B | 49% | 57% | 17.1B | 24.1B | 33.0B | -55.7B | -91.7B | -104.0B | 103% | -23.7B |
| **Powered land / HPC developers (FS landlords + 2026 model peers)** | | | | | | | | | | | | | | |
| APLD US | USD | -161M | 366M | 1.1B | -26% | 55% | -244M | -319M | -164M | -2.9B | -5.9B | -5.2B | 714% | -2.8B |
| WULF US | USD | -301M | 59M | 507M | -182% | 58% | -1.9B | -1.4B | -121M | -2.2B | -3.5B | -3.5B | 1422% | -2.5B |
| CIFR US | USD | -362M | -193M | 532M | -190% | 66% | -1.1B | -458M | -2M | -1.2B | -3.5B | -1.4B | 1628% | -1.5B |
| CORZ US | USD | -473M | 216M | 651M | -108% | 56% | -1.4B | -1.5B | 93M | -1.5B | -2.1B | -4.2B | 309% | -958M |
| HUT US | USD | -792M | -213M | 313M | -249% | 55% | -600M | -616M | -554M | -707M | -2.2B | -3.8B | 736% | -796M |
| GLXY US | USD | -274M | -138M | 432M | -0% | 1% | -209M | -248M | -79M | -1.4B | -1.6B | -2.2B | 4% | -2.2B |
| **Data center REITs / operators (FS 2026 identity)** | | | | | | | | | | | | | | |
| EQIX US | USD | 4.5B | 5.2B | 5.9B | 46% | 52% | 1.5B | 1.7B | 1.9B | -6.5B | -5.7B | -6.0B | 56% | -2.6B |
| DLR US | USD | 3.0B | 3.8B | 4.3B | 45% | 54% | 799M | 1.0B | 1.0B | -5.7B | -4.1B | -4.6B | 58% | -2.8B |
| IRM US | USD | 3.0B | 3.0B | 3.3B | 40% | 37% | 419M | 613M | 717M | -2.1B | -2.2B | -2.1B | 27% | -491M |
| GDS US | CNY | 3.9B | 6.0B | 6.2B | 32% | 44% | 3.7B | 2.5B | 1.2B | -4.4B | -9.5B | -11.7B | 75% | -853M |
| NXT AU | AUD | 373M | 399M | 756M | 75% | 57% | 82M | -164M | -154M | -2.1B | -5.4B | -3.2B | 684% | -2.0B |
| KDCREIT SP | SGD | 362M | 365M | 387M | 77% | 75% | 445M | 285M | 304M | -1.2B | -41M | -36M | 9% | -919M |
| DBRG US | USD | 180M | 121M | 154M | 28% | 36% | 344M | na | na | -1M | na | na | na | 255M |
| **Infra software (memo's software bucket)** | | | | | | | | | | | | | | |
| DDOG US | USD | 142M | 1.1B | 1.4B | 4% | 25% | 178M | 956M | 1.2B | -156M | -67M | -95M | 1% | 1.1B |
| NET US | USD | -51M | 707M | 980M | -2% | 26% | -206M | 472M | 645M | -319M | -403M | -521M | 14% | 315M |
| NOW US | USD | 2.9B | 6.2B | 7.5B | 20% | 39% | 1.7B | 4.2B | 5.2B | -728M | -711M | -945M | 4% | 4.6B |
| DT US | USD | 298M | 700M | 817M | 14% | 31% | 151M | 582M | 686M | -28M | -27M | -35M | 1% | 571M |

### C. Leverage and credit

| Ticker | Net debt | Net debt / EBITDA FY1 | Tot debt / LTM EBITDA | LTM interest | Cost of debt | S&P | Moody's | 5Y CDS (bp) | Model 1y default prob |
|---|---|---|---|---|---|---|---|---|---|
| **Neocloud / GPU compute (FS 2025 model)** | | | | | | | | | |
| CRWV US | 26.7B | 3.5x | 10.6x | 1.2B | 10.48% | B+ | Ba3 | 848 | 6.07% |
| NBIS US | 1.3B | 1.0x | 121.5x | 62M | 4.09% | NR | nan | na | 1.66% |
| IREN US | 1.9B | 1.1x | na | 59M | 7.20% | nan | nan | na | 3.35% |
| DOCN US | 1.3B | 2.9x | 4.4x | 18M | 5.32% | nan | nan | na | 0.25% |
| ORCL US | 135.5B | 2.6x | 5.1x | 4.6B | 6.54% | BBB- | Baa2 | 212 | 0.41% |
| **Powered land / HPC developers (FS landlords + 2026 model peers)** | | | | | | | | | |
| APLD US | 3.5B | 9.6x | na | 30M | 7.94% | B+ | nan | na | 6.10% |
| WULF US | 1.4B | 24.3x | na | 80M | 6.71% | BB- | nan | na | 5.89% |
| CIFR US | 2.1B | na | na | 37M | 5.71% | nan | nan | na | 8.21% |
| CORZ US | 851M | 3.9x | na | na | 8.35% | B+ | nan | na | 3.02% |
| HUT US | 384M | na | na | 30M | 6.58% | nan | nan | na | 7.11% |
| GLXY US | 5.2B | na | 711.9x | 59M | 5.18% | nan | nan | na | 3.17% |
| **Data center REITs / operators (FS 2026 identity)** | | | | | | | | | |
| EQIX US | 19.5B | 3.7x | 5.1x | 527M | 5.21% | BBB+ | Baa1 | 118 | 0.00% |
| DLR US | 16.2B | 4.2x | 6.6x | 438M | 4.50% | BBB+ | Baa2 | na | 0.01% |
| IRM US | 18.9B | 6.4x | 6.6x | 846M | 5.66% | BB- | Ba3 | 124 | 0.05% |
| GDS US | 33.2B | 5.6x | 12.0x | 1.8B | 2.03% | nan | nan | na | 1.99% |
| NXT AU | 2.5B | 6.2x | 9.0x | 178M | 6.28% | nan | nan | na | 0.13% |
| KDCREIT SP | 2.0B | 5.5x | 6.3x | 51M | 3.05% | nan | nan | na | 0.00% |
| DBRG US | -84M | -0.7x | 1.6x | na | 5.60% | nan | nan | na | 0.02% |
| **Infra software (memo's software bucket)** | | | | | | | | | |
| DDOG US | -3.2B | -3.0x | 9.0x | 11M | 5.06% | nan | nan | na | 0.00% |
| NET US | -583M | -0.8x | 57.4x | 9M | 5.22% | nan | nan | na | 0.02% |
| NOW US | -3.9B | -0.6x | 2.9x | 23M | 3.84% | A | A2 | na | 0.03% |
| DT US | -1.0B | -1.4x | 0.5x | 0M | 2.81% | NR | nan | 100 | 0.00% |

### D. Reference bonds

UST 5Y 4.53%, 10Y 4.77% on the pull date.

| Bond | Rank | Maturity | Size | Px | YTM | Spread vs 5Y UST | S&P | Moody's |
|---|---|---|---|---|---|---|---|---|
| Meridian Arc HoldCo 6.25% 2031 sr sec (Fluidstack Indiana Ph1, 49.9% FS / 50.1% Coatue NF, Google-guaranteed lease) | Secured | 2031-04-30 | 5.7B | 95.0 | 7.55% | 301bp | BB- | Ba2 |
| WULF Compute 7.75% 2030 sr sec (Fluidstack lease, Google backstop) | Secured | 2030-10-15 | 3.2B | 103.8 | 6.68% | 214bp | BB | Ba2 |
| Cipher Compute 7.125% 2030 1st lien (Fluidstack lease, Google backstop) | 1st lien | 2030-11-15 | 1.7B | 101.4 | 6.73% | 220bp | nan | Ba3 |
| Applied Digital 9.25% 2030 sr sec (CoreWeave leases) | Secured | 2030-12-15 | 2.4B | 105.2 | 7.80% | 327bp | BB- | nan |
| CoreWeave 9.25% 2030 sr unsec | Sr Unsecured | 2030-06-01 | 2.0B | 92.2 | 11.89% | 736bp | B | B1 |
| CoreWeave 9.75% 2031 sr unsec | Sr Unsecured | 2031-10-01 | 2.8B | 88.7 | 12.83% | 830bp | B | B1 |
| Oracle 6.25% 2032 sr unsec | Sr Unsecured | 2032-11-09 | 2.2B | 99.3 | 6.39% | 185bp | BBB- | Baa2 |
| Iron Mountain 5.25% 2030 sr unsec | Sr Unsecured | 2030-07-15 | 1.3B | 97.9 | 5.85% | 132bp | BB- | Ba3 |
| Equinix 3.9% 2032 sr unsec | Sr Unsecured | 2032-04-15 | 1.2B | 92.4 | 5.49% | 96bp | BBB+ | Baa1 |
| Digital Realty 5.55% 2028 sr unsec | Sr Unsecured | 2028-01-15 | 900M | 100.9 | 4.85% | 32bp | BBB+ | Baa2 |
| Alphabet 4.5% 2035 sr unsec (credit-tenant reference) | Sr Unsecured | 2035-05-15 | 1.2B | 94.6 | 5.29% | 75bp | AA+ | Aa2 |

## Sources

Public filings, press and data:

- Fluidstack, "Fluidstack raised $830M Series A," July 20, 2026.
- Bloomberg News, "Fluidstack Seeks $1 Billion in New Funding at $18 Billion Valuation," April 14, 2026.
- Latham & Watkins, Cahill, Cooley deal notices on the Meridian Arc HoldCo $5.7B 6.25% senior secured notes due 2031, April 16-17, 2026.
- LCD, "Meridian Arc inks $5.7B of high-yield bonds for data center build at 6.25%," April 17, 2026.
- New Project Media, "Data center start-up Potentia behind 2.1 GW Indiana project now backed by Fluidstack and Google," May 12, 2026.
- Sullivan County, Indiana board records and Fitch construction timetable via New Project Media; DCD and WTHI coverage of the August 5, 2026 open house.
- MyRGV, "Data center developer requesting Harlingen drainage agreement," September 1, 2026.
- TeraWulf Form 8-K, July 6, 2026, Anthropic lease at Justified Data campus and sale of Abernathy JV interest to Fluidstack.
- Hut 8 press release December 17, 2025 and Q2 2026 results on River Bend; Cipher Mining and TeraWulf filings on Google backstops and warrants.
- Destiny Tech100 Form 424B3 disclosing a July 16, 2026 purchase of Fluidstack Series B preferred shares.
- Google-Blackstone TPU Co. announcement, May 18, 2026; The Information (June 2026) on Anthropic developer LOIs and direct Google guarantees.
- PitchBook, Morningstar and Penn Mutual Asset Management commentary on AI-infrastructure high-yield issuance and spreads, April to August 2026.
- Bond identifiers: Meridian Arc US58990CAA18; WULF Compute US982911AA70; Cipher Compute US17253NAA54; Applied Digital US00202DAA54; CoreWeave US21873SAB43 / US21873SAG30. Prices are dealer mid quotes as of September 2, 2026.
- Earnings-call transcripts: TeraWulf Q1/Q2 2026; Cipher Mining Q1/Q2 2026; Applied Digital Q3/Q4 FY26; CoreWeave Q1/Q2 2026; Nebius Q2 2026; IREN Q4 FY26; Core Scientific Q4 2025 and Q2 2026; Hut 8 Q1/Q2 2026; Digital Realty Q2 2026; Equinix Q2 2026; Alphabet Q1/Q2 2026; Oracle Q4 FY26; Vertiv Q2 2026.
- Management plan figures (2026E-2030E revenue, EBITDA, FCF) are from investor materials circulated with the round in March 2026.

*This is research, not investment advice. AGTI may hold positions in securities discussed.*
