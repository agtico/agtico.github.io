---
layout: report
title: "Fluidstack at $18B | Series B Evaluation"
date: "2026-09-02 12:00:00 +0000"
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

<style>
.fs-summary{border:1px solid rgba(110,229,143,0.35);border-left:4px solid #6ee58f;background:rgba(10,14,14,0.96);padding:14px 18px 12px;margin:0 0 22px}
.fs-summary-label{display:block;color:#6ee58f;font:900 10px var(--mono);letter-spacing:.18em;text-transform:uppercase;margin-bottom:8px}
.fs-summary ul{margin:0;padding-left:18px}
.fs-summary li{margin:0 0 6px;font-size:14px;line-height:1.5;color:rgba(220,229,224,0.9)}
.fs-tablewrap{overflow-x:auto;-webkit-overflow-scrolling:touch;margin:14px 0 22px;border:1px solid var(--line-strong)}
.fs-table{width:100%;min-width:100%;border-collapse:collapse;table-layout:auto;font-size:13px;display:table}
.fs-table thead{display:table-header-group}
.fs-table tbody{display:table-row-group}
.fs-table th{background:rgba(255,45,32,0.08);color:var(--red-2,#ff5a42);font:900 10px var(--mono);letter-spacing:.14em;text-transform:uppercase;text-align:left;padding:10px 12px;border-bottom:1px solid var(--line-strong);white-space:nowrap}
.fs-table td{padding:9px 12px;border-bottom:1px solid rgba(255,255,255,0.06);vertical-align:top;white-space:nowrap;color:rgba(220,229,224,0.92)}
.fs-table td:first-child{white-space:normal;min-width:180px}
.fs-table th:not(:first-child),.fs-table td:not(:first-child){text-align:right;font-variant-numeric:tabular-nums}
.fs-table td.txt{text-align:left}
.fs-table tr.fs-group td{text-align:left;white-space:normal}
.fs-tablewrap{padding:0}
.fs-tablewrap table{margin:0}
.fs-table tr.fs-group td{background:rgba(110,229,143,0.07);color:#9dffc8;font-weight:700;border-top:1px solid rgba(110,229,143,0.3)}
.fs-table tr.fs-median td{color:#ffb347;font-style:italic}
.fs-table tbody tr:hover td{background:rgba(255,255,255,0.03)}
</style>

<div class="pearl-hero-grid">
  <div class="pearl-scorecard good">
    <span class="label">Verdict</span>
    <span class="value">Participate, conditional, venture size</span>
    <span class="hint">Conditions: senior 1x preference and an executed Harlingen lease at or before close. Google-guaranteed leases plus the preference bound the loss case; upside if Indiana and Harlingen land is 4-10x.</span>
  </div>
  <div class="pearl-scorecard warn">
    <span class="label">Price vs fair value</span>
    <span class="value">$18B ask vs $10-14B fair</span>
    <span class="hint">Fair value is 10-12x schedule-adjusted 2027E EBITDA plus owned development equity (bridge in section 6). The $18B ask is ~18x the same EBITDA; the premium buys the Anthropic spread and the unsigned pipeline.</span>
  </div>
  <div class="pearl-scorecard bad">
    <span class="label">Walk triggers</span>
    <span class="value">Junior pref, >$20B post, Harlingen unsigned at close</span>
    <span class="hint">Any one of these breaks the risk-reward; the bust case depends on the preference recovering most of the capital.</span>
  </div>
</div>

<div class="pearl-verdict-banner">
  <strong>AGTI bottom line</strong>
  <p>Fluidstack is a credit-tenant operator with a small owned asset base, priced as a landlord. Its cost-of-capital edge is real and visible in the bond market (Google-guaranteed leases fund at 6.7-7.6%; CoreWeave's balance-sheet model at 12-13%). The March 2026 round materials overstate per-GW equity by 2-4x, the anchor Indiana site has slipped 6-12 months, Anthropic now signs landlords directly, and Google has opened a second TPU channel. At $18B with a senior 1x preference the probability-weighted return is ~2.8x. Participate, conditional on Harlingen executing by close, at a size you can lose. Valuation basis throughout: $18B pre-money, $19B post-money after the ~$1B primary, with post-money equity used as the enterprise-value proxy because consolidated net debt (a GPU-collateralized facility, drawn balance undisclosed) is small relative to it and project debt sits in non-recourse JVs [14].</p>
</div>

## 0. Start here

<div class="pearl-primer-box">
  <p><strong>One-liner:</strong> Fluidstack builds and operates AI data centers for Anthropic on Google TPUs, with Google guaranteeing the leases. It is raising ~$1B at a reported $18B, up from $7.5B in January 2026. The question is whether the step-up reflects a business that changed or a market that did.</p>
  <p style="margin-top:12px"><strong>Read next:</strong> <a href="#3-what-fluidstack-operates-vs-what-it-owns">Asset map</a> · <a href="#4-the-credit-market-already-priced-this">Credit ladder</a> · <a href="#6-re-underwriting-the-per-gw-math">Per-GW re-underwriting</a> · <a href="#7-scenarios">Scenarios</a> · <a href="#appendix-expanded-public-comps">Full comp tables</a></p>
</div>

## 1. Where the valuation actually is

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>Last confirmed price is $7.5B (January 2026). The $18B is a target first reported in April; a Series B preferred class exists and is placing through SPVs.</li>
    <li>That is a 140% step-up in eight months on a company whose anchor project has since slipped.</li>
    <li>Treat $18B as the asking price, not a cleared mark.</li>
  </ul>
</div>

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
  <p class="pearl-figure-caption">A Series B preferred class exists (a listed fund disclosed a $15M SPV purchase dated July 16, 2026) [14]. The $18B print itself has not been publicly confirmed [1][2].</p>
</div>

## 2. Multiples: premium to neoclouds, discount to its own landlords

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>On the plan, $19B post-money is 12x 2027E EBITDA: above neoclouds (7x), below developers and REITs (19-20x).</li>
    <li>Schedule-adjusted, it is ~18x, level with TeraWulf and Cipher, the companies that own the buildings Fluidstack rents.</li>
    <li>Per contracted GW, Fluidstack is priced at about twice Crusoe.</li>
  </ul>
</div>

Consensus fiscal-year estimates for 22 public peers were pulled September 2, 2026 [17] (full tables in the <a href="#appendix-expanded-public-comps">appendix</a>). Fluidstack is shown on both the management plan from the round materials and a schedule-adjusted case that cuts 2027 by a third for the slippage documented in <a href="#5-what-the-landlords-said-on-their-earnings-calls">section 5</a>.

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Bucket (median)</th><th>EV/Sales 2027E</th><th>EV/EBITDA 2027E</th><th>EV/EBITDA 2028E</th><th>Rev growth 2027E</th></tr></thead>
<tbody>
<tr><td>Neoclouds</td><td>4.2x</td><td>7.4x</td><td>4.0x</td><td>106%</td></tr>
<tr><td>Powered-land developers</td><td>9.0x</td><td>20.0x</td><td>11.9x</td><td>121%</td></tr>
<tr><td>DC REITs / operators</td><td>9.1x</td><td>19.2x</td><td>17.8x</td><td>10%</td></tr>
<tr><td>Infra software</td><td>11.0x</td><td>38.6x</td><td>31.1x</td><td>21%</td></tr>
<tr><td><strong>Fluidstack, plan</strong></td><td><strong>7.4x</strong></td><td><strong>12.3x</strong></td><td><strong>4.2x</strong></td><td><strong>286%</strong></td></tr>
<tr><td><strong>Fluidstack, adjusted</strong></td><td><strong>~11x</strong></td><td><strong>~18x</strong></td><td><strong>~6x</strong></td><td><strong>~160%</strong></td></tr>
</tbody>
</table></div>

Buckets: neoclouds CRWV, NBIS, IREN, DOCN, ORCL; powered-land developers APLD, WULF, CIFR, CORZ, HUT, GLXY; DC REITs / operators EQIX, DLR, IRM, GDS, NXT, KDCREIT, DBRG; infra software DDOG, NET, NOW, DT. Fluidstack rows use $19B post-money.

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

Private cross-section [18]: Crusoe is reported in talks near $30B on ~4.9 GW contracted, about $6.1B per contracted GW. Fluidstack at $18B on ~1.41 GW-IT operated is ~$12.8B per contracted GW, roughly twice Crusoe, while owning only ~0.3 GW-IT of it.

## 3. What Fluidstack operates vs what it owns

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>Fluidstack operates ~1.41 GW-IT but owns only ~0.30 GW-IT (half of Indiana Phase 1, majority of Abernathy).</li>
    <li>Roughly 80% of the business is a tenant/operator spread on Anthropic volume, guaranteed by Google.</li>
    <li>Harlingen, the largest capacity claim, has no executed lease on the public record.</li>
    <li>Since March: Indiana financed by a $5.7B bond instead of sold, Phase 1 slipped 6-12 months, Anthropic signing landlords direct, Google opened a second TPU channel.</li>
  </ul>
</div>

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

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Site / landlord</th><th>MW-IT</th><th>Fluidstack role</th><th>Status</th><th>Google support</th></tr></thead>
<tbody>
<tr><td>Meridian, New Lebanon IN [3][4]</td><td>430</td><td class="txt">Tenant + 49.9% owner</td><td class="txt">First hall Jul 2027</td><td class="txt">Full lease guarantee (basis for BB)</td></tr>
<tr><td>TeraWulf Lake Mariner NY (CB-3/4/5) [7]</td><td>~360</td><td class="txt">Tenant</td><td class="txt">CB-3 online Jul 2026; CB-4 commissioning; CB-5 Jan 2027</td><td class="txt">$3.2B backstop; ~14% TeraWulf equity to Google</td></tr>
<tr><td>Cipher Barber Lake TX [9]</td><td>207</td><td class="txt">Tenant</td><td class="txt">Phase 1 Sep/Oct 2026</td><td class="txt">$1.73B backstop; 5.4% Cipher warrants</td></tr>
<tr><td>Abernathy TX [7]</td><td class="txt">168 (to ~500)</td><td class="txt">Majority owner + tenant</td><td class="txt">H2 2026, sequencing open</td><td class="txt">$1.3B backstop</td></tr>
<tr><td>Hut 8 River Bend LA [8]</td><td>245</td><td class="txt">Tenant; ROFO on +1 GW</td><td class="txt">First hall Q2 2027</td><td class="txt">Full 15-year base-term backstop</td></tr>
<tr><td>Harlingen / Cameron County TX [6]</td><td class="txt">guided 1.1 GW-IT</td><td class="txt">Developer</td><td class="txt">LOI / permitting</td><td class="txt">none located</td></tr>
</tbody>
</table></div>

### What changed since the March round materials

<div class="pearl-flow">
  <div class="pearl-flow-step"><span class="num">1</span><strong>Indiana financed, not sold</strong><span>$5.7B 6.25% 2031 notes; 49.9% Fluidstack / 50.1% Coatue Next Frontier; Google-guaranteed 15-year NNN. Replaces the Blue Owl sale in the materials [3][4].</span></div>
  <div class="pearl-flow-step"><span class="num">2</span><strong>Phase 1 slipped 6-12 months</strong><span>Guided Q4 2026 / Q1 2027. County and rating-agency timetable: first 65 MW hall July 2027, full Phase 1 end-2027 / early 2028 [4][5].</span></div>
  <div class="pearl-flow-step"><span class="num">3</span><strong>Harlingen unsigned</strong><span>No executed lease, JV percentage, tenant or guarantee on the public record. City drainage approval described as critical [6].</span></div>
  <div class="pearl-flow-step"><span class="num">4</span><strong>Anthropic goes direct</strong><span>TeraWulf 401 MW / 20-year / ~$19B; Nscale ~$45B; Hut 8 1 GW diligence; CoreWeave added. Anthropic is seeking Google guarantees directly [7][8][13][16].</span></div>
  <div class="pearl-flow-step"><span class="num">5</span><strong>TPU exclusivity refuted</strong><span>Google-Blackstone "TPU Co." ($5B, 500 MW in 2027); Google now ships TPU systems into customers' own data centers [12].</span></div>
</div>

## 4. The credit market already priced this

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>Google-guaranteed Fluidstack leases fund at 6.7-7.6%; CoreWeave's balance sheet funds at 12-13%. That gap is the moat.</li>
    <li>The 6% cap rate in the round materials is below Fluidstack's own project debt cost (7.55%).</li>
    <li>Meridian's bond has dropped 5 points since April; the AI high-yield window is tightening.</li>
  </ul>
</div>

<div class="pearl-figure">
  <div class="pearl-figure-head">
    <h3>Yield to maturity, dealer mid quotes, September 2, 2026 [17]</h3>
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

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Bond</th><th>YTM</th><th>Spread vs 5Y UST</th><th>Rating</th></tr></thead>
<tbody>
<tr><td>Meridian Arc 6.25% 2031 (Fluidstack's Indiana Phase 1; Google-guaranteed lease) [3]</td><td class="txt">7.55% (px 95.0; par at issue Apr 24)</td><td>301bp</td><td class="txt">BB- / Ba2 / BB</td></tr>
<tr><td>WULF Compute 7.75% 2030 (Fluidstack tenant, Google backstop)</td><td>6.68%</td><td>214bp</td><td class="txt">BB / Ba2</td></tr>
<tr><td>Cipher Compute 7.125% 2030 (Fluidstack tenant, Google backstop)</td><td>6.73%</td><td>220bp</td><td class="txt">Ba3</td></tr>
<tr><td>Applied Digital 9.25% 2030 (CoreWeave tenant)</td><td>7.80%</td><td>327bp</td><td>BB-</td></tr>
<tr><td>CoreWeave 9.25% 2030 / 9.75% 2031 unsecured</td><td>11.9% / 12.8%</td><td>736 / 830bp</td><td>B / B1</td></tr>
<tr><td>Oracle 6.25% 2032 (5Y CDS 212bp)</td><td>6.39%</td><td>185bp</td><td>BBB-</td></tr>
<tr><td>Equinix 3.9% 2032 / Digital Realty 5.55% 2028</td><td>5.49% / 4.85%</td><td>96 / 32bp</td><td class="txt">BBB+</td></tr>
<tr><td>Alphabet 4.5% 2035 (the guarantor)</td><td>5.29%</td><td>75bp</td><td class="txt">AA+</td></tr>
</tbody>
</table></div>

Two cautions. The gap is narrowing: CoreWeave cut its weighted average cost of debt ~300bp in a year, and Applied Digital refinanced CoreWeave-tenant paper at 7% behind an A3-rated SPV. And the window is tightening: AI-infrastructure high-yield issuance reached ~$32B by early July, the year's largest prints (Meridian Arc, Core Scientific, Tract) trade below par, and data-center HY spreads have re-widened since June. Meridian's 5-point drop in four months is the cleanest signal [15][17].

## 5. What the landlords said on their earnings calls

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>Every Fluidstack building at TeraWulf slipped 1-3 quarters between May and August; design changes and electrician shortages.</li>
    <li>Anthropic signed TeraWulf directly for 401 MW / $19B, and the landlord prefers it that way.</li>
    <li>Build cost is $9-13M per MW and rising; landlords earn mid-teens on cost, which Fluidstack pays as tenant.</li>
    <li>Google now ships TPUs into customers' own data centers; the exclusivity claim is gone.</li>
  </ul>
</div>

Thirty-one transcripts, latest two quarters, for TeraWulf, Cipher, Applied Digital, CoreWeave, Nebius, IREN, Core Scientific, Hut 8, Galaxy, Equinix, Digital Realty, Oracle, Alphabet and Vertiv [7]-[12][16].

<div class="pearl-source-grid">
  <div class="pearl-source-card">
    <span class="src-label">TeraWulf Q1 vs Q2 2026 [7]</span>
    <span class="src-stat">Every Fluidstack building slipped 1-3 quarters</span>
    <span class="src-note">May: CB-3 end of May, CB-4 Q3, CB-5 Q4, timelines "unchanged." August: CB-3 live early July, leases amended hall by hall, CB-5 first hall "very early January." Cause: the reference design "has changed over time, particularly as our clients get more experience running the hardware," plus electrician shortages.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">TeraWulf Q2 2026 [7]</span>
    <span class="src-stat">Anthropic direct: 401 MW, 20 years, ~$19B</span>
    <span class="src-note">"We get to deal direct now with our customer in Anthropic, and it just makes things that much more efficient." TeraWulf also sold its 50.1% of Abernathy to Fluidstack for ~$530M, a 20% IRR in about nine months.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Applied Digital Q4 FY26 [10], Hut 8 [8], Core Scientific [11]</span>
    <span class="src-stat">Backstop leases "face uncertainty after the initial 5-year tenor"</span>
    <span class="src-note">Why Applied now prefers direct hyperscaler leases. Hut 8 says Google covers the full 15-year base term at River Bend; Core Scientific says AMD's support has no equity step-in "as you've seen included in some other deals." Tenor and step-in at Fluidstack's owned sites are the first diligence item.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Alphabet Q1 and Q2 2026 [12]</span>
    <span class="src-stat">TPU systems delivered into customers' own data centers</span>
    <span class="src-note">First deliveries in Q2, revenue mostly 2027, inside the $514B cloud backlog. Google is also "expanding the use of third-party capacity in Q3 as a bridging strategy," which is direct Google demand for Fluidstack-type sites.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Build cost, five landlords [3][7][8][9][11][16]</span>
    <span class="src-stat">$9-13M per MW-IT and rising</span>
    <span class="src-note">TeraWulf $9.1M/MW; Cipher and Hut 8 $9-11M; Core Scientific $11-12M for AMD; Vertiv content alone $3.25-3.75M; Meridian implied $13.3M including land, substation and reserves. The round materials assume $10-11M.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Lease economics [7][9][16]</span>
    <span class="src-stat">Mid-teens yield on cost, and Fluidstack pays it</span>
    <span class="src-note">TeraWulf targets mid-teens and achieved it with both Fluidstack and Anthropic. Digital Realty develops at 11.5% stabilized yield with 25%+ renewal spreads. The $130/kW-month rate in the materials holds for turnkey; on ~1.1 GW-IT Fluidstack is the one paying it.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">IREN Q4 FY26, Nebius Q2 2026 [16]</span>
    <span class="src-stat">GPU contracts at $20-25M per MW, 2-year payback</span>
    <span class="src-note">3-year contract pricing up ~125% since November; prepayments fund 45-60% of GPU capex. Fluidstack's retreat from chip ownership gives up the highest-return segment in the current market. The materials present it as a pure upgrade.</span>
  </div>
  <div class="pearl-source-card">
    <span class="src-label">Hut 8 Q2, Core Scientific Q4 [8][11]</span>
    <span class="src-stat">"Credit committees do not finance ambition"</span>
    <span class="src-note">Project debt is deep only behind investment-grade support. Google has priced its support in equity (TeraWulf ~14%, Cipher 5.4%). Expect the same in Fluidstack's JVs.</span>
  </div>
</div>

## 6. Re-underwriting the per-GW math

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>The materials claim $12.2B of equity per owned GW; the evidence supports $3-5.5B.</li>
    <li>The gap is the cap rate (6% vs 8-9%) and a 40x software multiple on a managed-service layer.</li>
    <li>Fair value today is ~$10-14B; $18B needs the unsigned Indiana remainder and Harlingen to close at the stated terms.</li>
  </ul>
</div>

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

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Item</th><th>Round materials</th><th>Evidence</th><th>Basis</th></tr></thead>
<tbody>
<tr><td>Build cost per MW-IT</td><td>~$10-11M</td><td>$11-13M</td><td class="txt">Five landlords; Meridian implied</td></tr>
<tr><td>NOI per GW ($130/kW-mo, ~80% margin)</td><td>$1.25B</td><td>$1.25B</td><td class="txt">TeraWulf mid-teens yield on cost</td></tr>
<tr><td>Cap rate</td><td>6%</td><td>8-9%</td><td class="txt">Meridian YTM 7.55%; NNN range mid-4% to ~8%</td></tr>
<tr><td>Gross asset value per GW</td><td>$20.8B</td><td>$14-15.6B</td><td></td></tr>
<tr><td>Debt per GW</td><td class="txt">80% LTV of value = $16.6B</td><td class="txt">~$13B on cost</td><td class="txt">Meridian: $5.7B on 430 MW</td></tr>
<tr><td>Infra equity per GW</td><td>$4.2B</td><td>$1-2.6B</td><td></td></tr>
<tr><td>Google equity step-in / warrants</td><td class="txt">none</td><td class="txt">expect dilution</td><td class="txt">TeraWulf, Cipher precedents</td></tr>
<tr><td>Software EBITDA multiple ($200M)</td><td class="txt">40x = $8B</td><td class="txt">10-15x = $2-3B</td><td class="txt">TPUs procured by Anthropic/Broadcom; Google selling direct</td></tr>
<tr><td><strong>Equity per owned GW</strong></td><td><strong>$12.2B</strong></td><td><strong>$3-5.5B</strong></td><td></td></tr>
</tbody>
</table></div>

Cross-check [7]: ~$530M for 50.1% of Abernathy (168 MW-IT scaling toward ~500, pre-commencement) implies ~$1.06B of JV equity, or $2-6M per MW depending on the capacity basis, consistent with the evidence column.

Yield reconciliation: $1.25B of NOI per GW on a $12-13M/MW build is a 10% yield on cost, which matches the 10-12% lease yield in the materials. TeraWulf's mid-teens yield is earned on a $9.1M/MW turnkey cost at Lake Mariner rates [7]; a greenfield at $12-13M/MW earns the same rent at 10-12%.

### Fair-value bridge

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Component</th><th>Basis</th><th>Value</th></tr></thead>
<tbody>
<tr><td>Operating business (tenant spread on ~1.1 GW-IT plus owned-site operations)</td><td class="txt">Schedule-adjusted 2027E EBITDA $0.9-1.2B x 10-12x</td><td class="txt">$9-14B enterprise value</td></tr>
<tr><td>Less consolidated net debt</td><td class="txt">GPU facility, drawn balance undisclosed; assume $1-2B</td><td class="txt">($1-2B)</td></tr>
<tr><td>Owned development equity</td><td class="txt">0.3 GW-IT x $3-5.5B per GW</td><td class="txt">$1-1.6B</td></tr>
<tr><td>Unfinanced pipeline (Indiana remainder, Harlingen)</td><td class="txt">Option value, zero until leased and financed</td><td class="txt">0</td></tr>
<tr><td><strong>Equity value today</strong></td><td class="txt"></td><td class="txt"><strong>~$10-14B</strong></td></tr>
</tbody>
</table></div>

The 10-12x range sits between neoclouds (7.4x) and its own landlords (20x) because the business is a mix of the two. $18B requires the Indiana remainder and Harlingen to close at the terms in the materials, which is why an executed Harlingen lease is a closing condition rather than a nice-to-have.

## 7. Scenarios

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>Probability-weighted ~2.8x on a senior 1x preferred, 2.7x on common, over 3-4 years.</li>
    <li>The most likely single outcome (40%) is anchors-only at ~1.3x.</li>
    <li>The preference lifts the bust case from 0.2x to 0.5-1.0x; the waterfall below shows why it is a range.</li>
  </ul>
</div>

Entry $19B post-money, exit 2029-30. Exit multiple 10x forward EBITDA, between neoclouds (4-7x on 2028) and REITs (18x), for a mix that is mostly long-dated leases by 2030. Exit equity is enterprise value less ~$8B of consolidated debt, the compute facility peak in the materials. 2030E EBITDA by case: plan as stated; half of plan with a one-year slip; anchors only equals stabilized Indiana Phase 1 and Abernathy at Fluidstack's share plus the tenant spread on ~1.1 GW-IT; bust assumes Anthropic renews direct with landlords and the JV equity is impaired.

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Case</th><th>Probability</th><th>2030E EBITDA</th><th>Exit equity</th><th>MOIC common</th><th>MOIC 1x pref</th></tr></thead>
<tbody>
<tr><td>Plan delivered</td><td>10%</td><td>$20B</td><td>~$190B</td><td>10x</td><td>10x</td></tr>
<tr><td>Half of plan, one-year slip</td><td>30%</td><td>$8B</td><td>~$72B</td><td>3.8x</td><td>3.8x</td></tr>
<tr><td>Anchors only; Harlingen and pipeline stall</td><td>40%</td><td>$3B</td><td>~$25B</td><td>1.3x</td><td>1.3x</td></tr>
<tr><td>Anthropic shifts direct, Google support capped, JV equity impaired</td><td>20%</td><td>under $1B</td><td>~$1-3B</td><td>0.1x</td><td>0.5-1.0x</td></tr>
<tr><td><strong>Probability-weighted</strong></td><td></td><td></td><td></td><td><strong>2.7x</strong></td><td><strong>~2.8x</strong></td></tr>
</tbody>
</table></div>

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
      <div class="pearl-bar-track"><div class="pearl-bar-fill muted" style="width:8%"></div></div>
      <span class="pct">0.5-1.0x</span>
    </div>
  </div>
  <p class="pearl-figure-caption">The preference lifts the bust case from 0.1x to 0.5-1.0x. The range comes from the waterfall below.</p>
</div>

### Liquidation waterfall in the bust case

Project debt (Meridian $5.7B, landlord bonds) is non-recourse to the parent and sits ahead of JV equity, so in a bust the parent's claim on Indiana and Abernathy is close to zero. What remains at the parent: cash from this round (~$1B less what is spent on JV buy-ins), the Solis Arx land bank, the ~$1.06B Abernathy equity at a distressed mark, tenant contracts (worthless or negative if Anthropic leaves), and GPU collateral pledged to the Macquarie facility [14]. Realizable parent value: ~$1-3B. Preference stack: roughly $2B of prior rounds plus this $1B. A senior Series B recovers 0.5-1.0x across that range; pari passu with the Series A recovers 0.3-0.7x; junior recovers near zero. Seniority is the whole difference between a venture loss and a return of most capital, which is why it is a condition.

## 8. Verdict and conditions

<div class="fs-summary">
  <span class="fs-summary-label">Management summary</span>
  <ul>
    <li>Participate with a senior 1x preference and an executed Harlingen lease at close, sized as venture.</li>
    <li>Walk on a junior pref, post-money above ~$20B, or Harlingen unsigned at close.</li>
    <li>First diligence item: tenor, caps and step-in rights of the Google guarantee at Indiana.</li>
  </ul>
</div>

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
      <li>The preference is junior to the Series A, or pari passu without pro-rata (waterfall in section 7).</li>
      <li>Post-money above ~$20B.</li>
      <li>Harlingen is still an LOI at close; participate only on a signed, Google-guaranteed lease or a price near the $10-14B fair range.</li>
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

<div class="fs-summary">
  <span class="fs-summary-label">How to read the appendix</span>
  <ul>
    <li>Table A: size, growth and forward multiples. Table B: EBITDA, net income, capex and free cash flow. Table C: leverage, ratings, CDS. Table D: reference bonds.</li>
    <li>Bucket medians are shown in italics. Galaxy's revenue line is gross trading revenue, so its EV/Sales is not meaningful.</li>
    <li>Non-USD reporters (NEXTDC, Keppel DC REIT, GDS) are converted at spot for size columns only.</li>
  </ul>
</div>

Consensus fiscal-year estimates and trailing fundamentals for 22 public peers, pulled September 2, 2026. USD millions unless noted; non-USD reporters converted at spot for size columns only; ratios are currency-neutral. FY1/FY2/FY3 are consensus fiscal years and fiscal year ends differ (Applied Digital May, IREN June, most others December).

### A. Size, growth, multiples

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Ticker</th><th>Name</th><th>Mkt cap</th><th>EV</th><th>LTM sales</th><th>Rev g FY1</th><th>Rev g FY2</th><th>Rev g FY3</th><th>EV/S FY1</th><th>EV/S FY2</th><th>EV/EBITDA FY1</th><th>EV/EBITDA FY2</th><th>EV/EBITDA FY3</th><th>P/E FY2</th></tr></thead>
<tbody>
<tr class="fs-group"><td colspan="14"><strong>Neocloud / GPU compute</strong></td></tr>
<tr><td>CRWV US</td><td class="txt">CoreWeave</td><td>45.7B</td><td>91.8B</td><td>7.6B</td><td>152%</td><td>106%</td><td>60%</td><td>7.1x</td><td>3.5x</td><td>12.1x</td><td>5.5x</td><td>3.3x</td><td>na</td></tr>
<tr><td>NBIS US</td><td class="txt">Nebius</td><td>57.4B</td><td>59.4B</td><td>1.4B</td><td>521%</td><td>245%</td><td>100%</td><td>18.1x</td><td>5.2x</td><td>44.3x</td><td>9.4x</td><td>4.0x</td><td>na</td></tr>
<tr><td>IREN US</td><td class="txt">IREN</td><td>14.5B</td><td>16.5B</td><td>707M</td><td>300%</td><td>147%</td><td>69%</td><td>5.8x</td><td>2.4x</td><td>9.3x</td><td>3.2x</td><td>1.8x</td><td>27.2x</td></tr>
<tr><td>DOCN US</td><td class="txt">DigitalOcean</td><td>12.8B</td><td>13.4B</td><td>1.0B</td><td>31%</td><td>54%</td><td>48%</td><td>11.4x</td><td>7.4x</td><td>29.2x</td><td>18.4x</td><td>12.0x</td><td>61.0x</td></tr>
<tr><td>ORCL US</td><td class="txt">Oracle</td><td>407.1B</td><td>548.2B</td><td>67.4B</td><td>33%</td><td>45%</td><td>41%</td><td>6.1x</td><td>4.2x</td><td>10.6x</td><td>7.4x</td><td>5.2x</td><td>13.3x</td></tr>
<tr class="fs-median"><td><em>median</em></td><td></td><td></td><td></td><td></td><td>152%</td><td>106%</td><td>60%</td><td>7.1x</td><td>4.2x</td><td>12.1x</td><td>7.4x</td><td>4.0x</td><td>27.2x</td></tr>
<tr class="fs-group"><td colspan="14"><strong>Powered land / HPC developers (Fluidstack landlords and peers)</strong></td></tr>
<tr><td>APLD US</td><td class="txt">Applied Digital</td><td>7.1B</td><td>12.6B</td><td>611M</td><td>34%</td><td>148%</td><td>49%</td><td>15.4x</td><td>6.2x</td><td>34.5x</td><td>11.2x</td><td>6.7x</td><td>na</td></tr>
<tr><td>WULF US</td><td class="txt">TeraWulf</td><td>7.3B</td><td>9.9B</td><td>165M</td><td>48%</td><td>252%</td><td>81%</td><td>39.8x</td><td>11.3x</td><td>167.3x</td><td>19.6x</td><td>9.8x</td><td>na</td></tr>
<tr><td>CIFR US</td><td class="txt">Cipher Mining</td><td>6.1B</td><td>10.8B</td><td>191M</td><td>-4%</td><td>273%</td><td>17%</td><td>50.3x</td><td>13.5x</td><td>na</td><td>20.4x</td><td>14.1x</td><td>na</td></tr>
<tr><td>CORZ US</td><td class="txt">Core Scientific</td><td>5.2B</td><td>7.8B</td><td>440M</td><td>116%</td><td>70%</td><td>50%</td><td>11.3x</td><td>6.7x</td><td>36.2x</td><td>12.0x</td><td>6.9x</td><td>108.6x</td></tr>
<tr><td>HUT US</td><td class="txt">Hut 8</td><td>9.6B</td><td>17.3B</td><td>318M</td><td>24%</td><td>93%</td><td>144%</td><td>59.2x</td><td>30.6x</td><td>na</td><td>55.2x</td><td>16.8x</td><td>na</td></tr>
<tr><td>GLXY US</td><td class="txt">Galaxy Digital</td><td>9.2B</td><td>15.2B</td><td>58.4B</td><td>-39%</td><td>18%</td><td>14%</td><td>0.4x</td><td>0.3x</td><td>na</td><td>35.1x</td><td>17.9x</td><td>na</td></tr>
<tr class="fs-median"><td><em>median</em></td><td></td><td></td><td></td><td></td><td>29%</td><td>121%</td><td>50%</td><td>27.6x</td><td>9.0x</td><td>36.2x</td><td>20.0x</td><td>11.9x</td><td>108.6x</td></tr>
<tr class="fs-group"><td colspan="14"><strong>Data center REITs / operators</strong></td></tr>
<tr><td>EQIX US</td><td class="txt">Equinix</td><td>101.4B</td><td>122.5B</td><td>9.8B</td><td>11%</td><td>10%</td><td>11%</td><td>11.9x</td><td>10.8x</td><td>23.4x</td><td>20.8x</td><td>18.5x</td><td>52.5x</td></tr>
<tr><td>DLR US</td><td class="txt">Digital Realty</td><td>69.0B</td><td>87.1B</td><td>6.8B</td><td>16%</td><td>11%</td><td>14%</td><td>12.2x</td><td>11.0x</td><td>22.6x</td><td>20.2x</td><td>17.8x</td><td>57.9x</td></tr>
<tr><td>IRM US</td><td class="txt">Iron Mountain</td><td>34.0B</td><td>54.1B</td><td>7.6B</td><td>16%</td><td>9%</td><td>8%</td><td>6.8x</td><td>6.2x</td><td>18.3x</td><td>16.6x</td><td>15.3x</td><td>41.5x</td></tr>
<tr><td>GDS US</td><td class="txt">GDS Holdings</td><td>6.3B</td><td>11.7B</td><td>12.3B</td><td>11%</td><td>10%</td><td>22%</td><td>6.2x</td><td>5.6x</td><td>13.2x</td><td>12.7x</td><td>10.5x</td><td>181.3x</td></tr>
<tr><td>NXT AU</td><td class="txt">NEXTDC</td><td>6.9B</td><td>8.7B</td><td>496M</td><td>60%</td><td>67%</td><td>29%</td><td>15.3x</td><td>9.1x</td><td>30.4x</td><td>16.1x</td><td>12.3x</td><td>na</td></tr>
<tr><td>KDCREIT SP</td><td class="txt">Keppel DC REIT</td><td>4.2B</td><td>5.8B</td><td>472M</td><td>9%</td><td>7%</td><td>4%</td><td>15.4x</td><td>14.4x</td><td>20.3x</td><td>19.2x</td><td>18.6x</td><td>17.2x</td></tr>
<tr><td>DBRG US</td><td class="txt">DigitalBridge</td><td>3.0B</td><td>3.9B</td><td>633M</td><td>294%</td><td>15%</td><td>16%</td><td>10.5x</td><td>9.1x</td><td>32.1x</td><td>25.3x</td><td>18.6x</td><td>24.9x</td></tr>
<tr class="fs-median"><td><em>median</em></td><td></td><td></td><td></td><td></td><td>16%</td><td>10%</td><td>14%</td><td>11.9x</td><td>9.1x</td><td>22.6x</td><td>19.2x</td><td>17.8x</td><td>47.0x</td></tr>
<tr class="fs-group"><td colspan="14"><strong>Infra software</strong></td></tr>
<tr><td>DDOG US</td><td class="txt">Datadog</td><td>80.6B</td><td>76.9B</td><td>4.0B</td><td>30%</td><td>23%</td><td>22%</td><td>17.2x</td><td>14.0x</td><td>71.5x</td><td>56.9x</td><td>45.4x</td><td>72.9x</td></tr>
<tr><td>NET US</td><td class="txt">Cloudflare</td><td>101.7B</td><td>101.1B</td><td>2.5B</td><td>33%</td><td>29%</td><td>28%</td><td>35.2x</td><td>27.3x</td><td>143.0x</td><td>103.2x</td><td>78.8x</td><td>167.1x</td></tr>
<tr><td>NOW US</td><td class="txt">ServiceNow</td><td>147.8B</td><td>151.5B</td><td>14.7B</td><td>22%</td><td>19%</td><td>18%</td><td>9.3x</td><td>7.9x</td><td>24.4x</td><td>20.3x</td><td>16.7x</td><td>28.4x</td></tr>
<tr><td>DT US</td><td class="txt">Dynatrace</td><td>15.3B</td><td>14.3B</td><td>2.1B</td><td>15%</td><td>15%</td><td>15%</td><td>6.2x</td><td>5.4x</td><td>20.5x</td><td>17.5x</td><td>14.6x</td><td>23.3x</td></tr>
<tr class="fs-median"><td><em>median</em></td><td></td><td></td><td></td><td></td><td>26%</td><td>21%</td><td>20%</td><td>13.3x</td><td>11.0x</td><td>47.9x</td><td>38.6x</td><td>31.1x</td><td>50.7x</td></tr>
</tbody>
</table></div>

### B. EBITDA, net income, capex, FCF (reporting currency)

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Ticker</th><th>Ccy</th><th>LTM EBITDA</th><th>EBITDA FY1</th><th>EBITDA FY2</th><th>EBITDA mgn LTM</th><th>EBITDA mgn FY2</th><th>LTM NI</th><th>NI FY1</th><th>NI FY2</th><th>LTM capex</th><th>Capex FY1</th><th>Capex FY2</th><th>Capex/Sales FY1</th><th>LTM FCF</th></tr></thead>
<tbody>
<tr class="fs-group"><td colspan="15"><strong>Neocloud / GPU compute</strong></td></tr>
<tr><td>CRWV US</td><td class="txt">USD</td><td>4.9B</td><td>7.6B</td><td>16.7B</td><td>64%</td><td>63%</td><td>-1.9B</td><td>-2.2B</td><td>-1.1B</td><td>-21.5B</td><td>-35.5B</td><td>-48.0B</td><td>275%</td><td>-14.5B</td></tr>
<tr><td>NBIS US</td><td class="txt">USD</td><td>83M</td><td>1.3B</td><td>6.3B</td><td>6%</td><td>56%</td><td>62M</td><td>-686M</td><td>-848M</td><td>-11.1B</td><td>-22.7B</td><td>-29.0B</td><td>690%</td><td>-5.9B</td></tr>
<tr><td>IREN US</td><td class="txt">USD</td><td>-629M</td><td>1.8B</td><td>5.1B</td><td>-89%</td><td>73%</td><td>-703M</td><td>-417M</td><td>345M</td><td>-3.0B</td><td>-15.3B</td><td>-15.0B</td><td>543%</td><td>-908M</td></tr>
<tr><td>DOCN US</td><td class="txt">USD</td><td>322M</td><td>460M</td><td>730M</td><td>32%</td><td>40%</td><td>235M</td><td>177M</td><td>236M</td><td>-270M</td><td>-216M</td><td>-323M</td><td>18%</td><td>40M</td></tr>
<tr><td>ORCL US</td><td class="txt">USD</td><td>32.7B</td><td>51.7B</td><td>73.9B</td><td>49%</td><td>57%</td><td>17.1B</td><td>24.1B</td><td>33.0B</td><td>-55.7B</td><td>-91.7B</td><td>-104.0B</td><td>103%</td><td>-23.7B</td></tr>
<tr class="fs-group"><td colspan="15"><strong>Powered land / HPC developers (Fluidstack landlords and peers)</strong></td></tr>
<tr><td>APLD US</td><td class="txt">USD</td><td>-161M</td><td>366M</td><td>1.1B</td><td>-26%</td><td>55%</td><td>-244M</td><td>-319M</td><td>-164M</td><td>-2.9B</td><td>-5.9B</td><td>-5.2B</td><td>714%</td><td>-2.8B</td></tr>
<tr><td>WULF US</td><td class="txt">USD</td><td>-301M</td><td>59M</td><td>507M</td><td>-182%</td><td>58%</td><td>-1.9B</td><td>-1.4B</td><td>-121M</td><td>-2.2B</td><td>-3.5B</td><td>-3.5B</td><td>1422%</td><td>-2.5B</td></tr>
<tr><td>CIFR US</td><td class="txt">USD</td><td>-362M</td><td>-193M</td><td>532M</td><td>-190%</td><td>66%</td><td>-1.1B</td><td>-458M</td><td>-2M</td><td>-1.2B</td><td>-3.5B</td><td>-1.4B</td><td>1628%</td><td>-1.5B</td></tr>
<tr><td>CORZ US</td><td class="txt">USD</td><td>-473M</td><td>216M</td><td>651M</td><td>-108%</td><td>56%</td><td>-1.4B</td><td>-1.5B</td><td>93M</td><td>-1.5B</td><td>-2.1B</td><td>-4.2B</td><td>309%</td><td>-958M</td></tr>
<tr><td>HUT US</td><td class="txt">USD</td><td>-792M</td><td>-213M</td><td>313M</td><td>-249%</td><td>55%</td><td>-600M</td><td>-616M</td><td>-554M</td><td>-707M</td><td>-2.2B</td><td>-3.8B</td><td>736%</td><td>-796M</td></tr>
<tr><td>GLXY US</td><td class="txt">USD</td><td>-274M</td><td>-138M</td><td>432M</td><td>-0%</td><td>1%</td><td>-209M</td><td>-248M</td><td>-79M</td><td>-1.4B</td><td>-1.6B</td><td>-2.2B</td><td>4%</td><td>-2.2B</td></tr>
<tr class="fs-group"><td colspan="15"><strong>Data center REITs / operators</strong></td></tr>
<tr><td>EQIX US</td><td class="txt">USD</td><td>4.5B</td><td>5.2B</td><td>5.9B</td><td>46%</td><td>52%</td><td>1.5B</td><td>1.7B</td><td>1.9B</td><td>-6.5B</td><td>-5.7B</td><td>-6.0B</td><td>56%</td><td>-2.6B</td></tr>
<tr><td>DLR US</td><td class="txt">USD</td><td>3.0B</td><td>3.8B</td><td>4.3B</td><td>45%</td><td>54%</td><td>799M</td><td>1.0B</td><td>1.0B</td><td>-5.7B</td><td>-4.1B</td><td>-4.6B</td><td>58%</td><td>-2.8B</td></tr>
<tr><td>IRM US</td><td class="txt">USD</td><td>3.0B</td><td>3.0B</td><td>3.3B</td><td>40%</td><td>37%</td><td>419M</td><td>613M</td><td>717M</td><td>-2.1B</td><td>-2.2B</td><td>-2.1B</td><td>27%</td><td>-491M</td></tr>
<tr><td>GDS US</td><td class="txt">CNY</td><td>3.9B</td><td>6.0B</td><td>6.2B</td><td>32%</td><td>44%</td><td>3.7B</td><td>2.5B</td><td>1.2B</td><td>-4.4B</td><td>-9.5B</td><td>-11.7B</td><td>75%</td><td>-853M</td></tr>
<tr><td>NXT AU</td><td class="txt">AUD</td><td>373M</td><td>399M</td><td>756M</td><td>75%</td><td>57%</td><td>82M</td><td>-164M</td><td>-154M</td><td>-2.1B</td><td>-5.4B</td><td>-3.2B</td><td>684%</td><td>-2.0B</td></tr>
<tr><td>KDCREIT SP</td><td class="txt">SGD</td><td>362M</td><td>365M</td><td>387M</td><td>77%</td><td>75%</td><td>445M</td><td>285M</td><td>304M</td><td>-1.2B</td><td>-41M</td><td>-36M</td><td>9%</td><td>-919M</td></tr>
<tr><td>DBRG US</td><td class="txt">USD</td><td>180M</td><td>121M</td><td>154M</td><td>28%</td><td>36%</td><td>344M</td><td>na</td><td>na</td><td>-1M</td><td>na</td><td>na</td><td>na</td><td>255M</td></tr>
<tr class="fs-group"><td colspan="15"><strong>Infra software</strong></td></tr>
<tr><td>DDOG US</td><td class="txt">USD</td><td>142M</td><td>1.1B</td><td>1.4B</td><td>4%</td><td>25%</td><td>178M</td><td>956M</td><td>1.2B</td><td>-156M</td><td>-67M</td><td>-95M</td><td>1%</td><td>1.1B</td></tr>
<tr><td>NET US</td><td class="txt">USD</td><td>-51M</td><td>707M</td><td>980M</td><td>-2%</td><td>26%</td><td>-206M</td><td>472M</td><td>645M</td><td>-319M</td><td>-403M</td><td>-521M</td><td>14%</td><td>315M</td></tr>
<tr><td>NOW US</td><td class="txt">USD</td><td>2.9B</td><td>6.2B</td><td>7.5B</td><td>20%</td><td>39%</td><td>1.7B</td><td>4.2B</td><td>5.2B</td><td>-728M</td><td>-711M</td><td>-945M</td><td>4%</td><td>4.6B</td></tr>
<tr><td>DT US</td><td class="txt">USD</td><td>298M</td><td>700M</td><td>817M</td><td>14%</td><td>31%</td><td>151M</td><td>582M</td><td>686M</td><td>-28M</td><td>-27M</td><td>-35M</td><td>1%</td><td>571M</td></tr>
</tbody>
</table></div>

### C. Leverage and credit

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Ticker</th><th>Net debt</th><th>Net debt / EBITDA FY1</th><th>Tot debt / LTM EBITDA</th><th>LTM interest</th><th>Cost of debt</th><th>S&P</th><th>Moody's</th><th>5Y CDS (bp)</th><th>Model 1y default prob</th></tr></thead>
<tbody>
<tr class="fs-group"><td colspan="10"><strong>Neocloud / GPU compute</strong></td></tr>
<tr><td>CRWV US</td><td>26.7B</td><td>3.5x</td><td>10.6x</td><td>1.2B</td><td>10.48%</td><td class="txt">B+</td><td class="txt">Ba3</td><td>848</td><td>6.07%</td></tr>
<tr><td>NBIS US</td><td>1.3B</td><td>1.0x</td><td>121.5x</td><td>62M</td><td>4.09%</td><td>NR</td><td>nan</td><td>na</td><td>1.66%</td></tr>
<tr><td>IREN US</td><td>1.9B</td><td>1.1x</td><td>na</td><td>59M</td><td>7.20%</td><td>nan</td><td>nan</td><td>na</td><td>3.35%</td></tr>
<tr><td>DOCN US</td><td>1.3B</td><td>2.9x</td><td>4.4x</td><td>18M</td><td>5.32%</td><td>nan</td><td>nan</td><td>na</td><td>0.25%</td></tr>
<tr><td>ORCL US</td><td>135.5B</td><td>2.6x</td><td>5.1x</td><td>4.6B</td><td>6.54%</td><td>BBB-</td><td class="txt">Baa2</td><td>212</td><td>0.41%</td></tr>
<tr class="fs-group"><td colspan="10"><strong>Powered land / HPC developers (Fluidstack landlords and peers)</strong></td></tr>
<tr><td>APLD US</td><td>3.5B</td><td>9.6x</td><td>na</td><td>30M</td><td>7.94%</td><td class="txt">B+</td><td>nan</td><td>na</td><td>6.10%</td></tr>
<tr><td>WULF US</td><td>1.4B</td><td>24.3x</td><td>na</td><td>80M</td><td>6.71%</td><td>BB-</td><td>nan</td><td>na</td><td>5.89%</td></tr>
<tr><td>CIFR US</td><td>2.1B</td><td>na</td><td>na</td><td>37M</td><td>5.71%</td><td>nan</td><td>nan</td><td>na</td><td>8.21%</td></tr>
<tr><td>CORZ US</td><td>851M</td><td>3.9x</td><td>na</td><td>na</td><td>8.35%</td><td class="txt">B+</td><td>nan</td><td>na</td><td>3.02%</td></tr>
<tr><td>HUT US</td><td>384M</td><td>na</td><td>na</td><td>30M</td><td>6.58%</td><td>nan</td><td>nan</td><td>na</td><td>7.11%</td></tr>
<tr><td>GLXY US</td><td>5.2B</td><td>na</td><td>711.9x</td><td>59M</td><td>5.18%</td><td>nan</td><td>nan</td><td>na</td><td>3.17%</td></tr>
<tr class="fs-group"><td colspan="10"><strong>Data center REITs / operators</strong></td></tr>
<tr><td>EQIX US</td><td>19.5B</td><td>3.7x</td><td>5.1x</td><td>527M</td><td>5.21%</td><td class="txt">BBB+</td><td class="txt">Baa1</td><td>118</td><td>0.00%</td></tr>
<tr><td>DLR US</td><td>16.2B</td><td>4.2x</td><td>6.6x</td><td>438M</td><td>4.50%</td><td class="txt">BBB+</td><td class="txt">Baa2</td><td>na</td><td>0.01%</td></tr>
<tr><td>IRM US</td><td>18.9B</td><td>6.4x</td><td>6.6x</td><td>846M</td><td>5.66%</td><td>BB-</td><td class="txt">Ba3</td><td>124</td><td>0.05%</td></tr>
<tr><td>GDS US</td><td>33.2B</td><td>5.6x</td><td>12.0x</td><td>1.8B</td><td>2.03%</td><td>nan</td><td>nan</td><td>na</td><td>1.99%</td></tr>
<tr><td>NXT AU</td><td>2.5B</td><td>6.2x</td><td>9.0x</td><td>178M</td><td>6.28%</td><td>nan</td><td>nan</td><td>na</td><td>0.13%</td></tr>
<tr><td>KDCREIT SP</td><td>2.0B</td><td>5.5x</td><td>6.3x</td><td>51M</td><td>3.05%</td><td>nan</td><td>nan</td><td>na</td><td>0.00%</td></tr>
<tr><td>DBRG US</td><td>-84M</td><td>-0.7x</td><td>1.6x</td><td>na</td><td>5.60%</td><td>nan</td><td>nan</td><td>na</td><td>0.02%</td></tr>
<tr class="fs-group"><td colspan="10"><strong>Infra software</strong></td></tr>
<tr><td>DDOG US</td><td>-3.2B</td><td>-3.0x</td><td>9.0x</td><td>11M</td><td>5.06%</td><td>nan</td><td>nan</td><td>na</td><td>0.00%</td></tr>
<tr><td>NET US</td><td>-583M</td><td>-0.8x</td><td>57.4x</td><td>9M</td><td>5.22%</td><td>nan</td><td>nan</td><td>na</td><td>0.02%</td></tr>
<tr><td>NOW US</td><td>-3.9B</td><td>-0.6x</td><td>2.9x</td><td>23M</td><td>3.84%</td><td class="txt">A</td><td class="txt">A2</td><td>na</td><td>0.03%</td></tr>
<tr><td>DT US</td><td>-1.0B</td><td>-1.4x</td><td>0.5x</td><td>0M</td><td>2.81%</td><td>NR</td><td>nan</td><td>100</td><td>0.00%</td></tr>
</tbody>
</table></div>

### D. Reference bonds

UST 5Y 4.53%, 10Y 4.77% on the pull date.

<div class="fs-tablewrap"><table class="fs-table">
<thead><tr><th>Bond</th><th>Rank</th><th>Maturity</th><th>Size</th><th>Px</th><th>YTM</th><th>Spread vs 5Y UST</th><th>S&P</th><th>Moody's</th></tr></thead>
<tbody>
<tr><td>Meridian Arc HoldCo 6.25% 2031 sr sec (Fluidstack Indiana Ph1, 49.9% FS / 50.1% Coatue NF, Google-guaranteed lease)</td><td class="txt">Secured</td><td>2031-04-30</td><td>5.7B</td><td>95.0</td><td>7.55%</td><td>301bp</td><td>BB-</td><td class="txt">Ba2</td></tr>
<tr><td>WULF Compute 7.75% 2030 sr sec (Fluidstack lease, Google backstop)</td><td class="txt">Secured</td><td>2030-10-15</td><td>3.2B</td><td>103.8</td><td>6.68%</td><td>214bp</td><td>BB</td><td class="txt">Ba2</td></tr>
<tr><td>Cipher Compute 7.125% 2030 1st lien (Fluidstack lease, Google backstop)</td><td class="txt">1st lien</td><td>2030-11-15</td><td>1.7B</td><td>101.4</td><td>6.73%</td><td>220bp</td><td>nan</td><td class="txt">Ba3</td></tr>
<tr><td>Applied Digital 9.25% 2030 sr sec (CoreWeave leases)</td><td class="txt">Secured</td><td>2030-12-15</td><td>2.4B</td><td>105.2</td><td>7.80%</td><td>327bp</td><td>BB-</td><td>nan</td></tr>
<tr><td>CoreWeave 9.25% 2030 sr unsec</td><td class="txt">Sr Unsecured</td><td>2030-06-01</td><td>2.0B</td><td>92.2</td><td>11.89%</td><td>736bp</td><td>B</td><td>B1</td></tr>
<tr><td>CoreWeave 9.75% 2031 sr unsec</td><td class="txt">Sr Unsecured</td><td>2031-10-01</td><td>2.8B</td><td>88.7</td><td>12.83%</td><td>830bp</td><td>B</td><td>B1</td></tr>
<tr><td>Oracle 6.25% 2032 sr unsec</td><td class="txt">Sr Unsecured</td><td>2032-11-09</td><td>2.2B</td><td>99.3</td><td>6.39%</td><td>185bp</td><td>BBB-</td><td class="txt">Baa2</td></tr>
<tr><td>Iron Mountain 5.25% 2030 sr unsec</td><td class="txt">Sr Unsecured</td><td>2030-07-15</td><td>1.3B</td><td>97.9</td><td>5.85%</td><td>132bp</td><td>BB-</td><td class="txt">Ba3</td></tr>
<tr><td>Equinix 3.9% 2032 sr unsec</td><td class="txt">Sr Unsecured</td><td>2032-04-15</td><td>1.2B</td><td>92.4</td><td>5.49%</td><td>96bp</td><td class="txt">BBB+</td><td class="txt">Baa1</td></tr>
<tr><td>Digital Realty 5.55% 2028 sr unsec</td><td class="txt">Sr Unsecured</td><td>2028-01-15</td><td>900M</td><td>100.9</td><td>4.85%</td><td>32bp</td><td class="txt">BBB+</td><td class="txt">Baa2</td></tr>
<tr><td>Alphabet 4.5% 2035 sr unsec (credit-tenant reference)</td><td class="txt">Sr Unsecured</td><td>2035-05-15</td><td>1.2B</td><td>94.6</td><td>5.29%</td><td>75bp</td><td class="txt">AA+</td><td class="txt">Aa2</td></tr>
</tbody>
</table></div>

## Sources

Public filings, press and data:

1. Fluidstack, "Fluidstack raised $830M Series A," July 20, 2026.
2. Bloomberg News, "Fluidstack Seeks $1 Billion in New Funding at $18 Billion Valuation," April 14, 2026.
3. Latham & Watkins, Cahill and Cooley deal notices on the Meridian Arc HoldCo $5.7B 6.25% senior secured notes due 2031, April 16-17, 2026; LCD, "Meridian Arc inks $5.7B of high-yield bonds for data center build at 6.25%," April 17, 2026.
4. New Project Media, "Data center start-up Potentia behind 2.1 GW Indiana project now backed by Fluidstack and Google," May 12, 2026 (JV split, Fitch construction timetable).
5. Sullivan County, Indiana board records; DCD and WTHI coverage of the August 5, 2026 open house.
6. MyRGV, "Data center developer requesting Harlingen drainage agreement," September 1, 2026; ValleyCentral coverage of the Cameron County site, 2026.
7. TeraWulf Form 8-K, July 6, 2026 (Anthropic lease at Justified Data campus; sale of Abernathy JV interest to Fluidstack); TeraWulf Q1 2026 (May 8) and Q2 2026 (August 5) earnings calls.
8. Hut 8 press release December 17, 2025 and Q1 (May 6) and Q2 2026 (August 4) earnings calls on River Bend and the Anthropic diligence agreement.
9. Cipher Mining Q1 (May 5) and Q2 2026 (August 4) earnings calls and filings on the Barber Lake lease, Google backstop and warrants.
10. Applied Digital Q3 FY26 (April 8) and Q4 FY26 (July 27, 2026) earnings calls.
11. Core Scientific Q4 2025 (March 2) and Q2 2026 (August) earnings calls.
12. Alphabet Q1 (April 29) and Q2 2026 (July 22) earnings calls; Google-Blackstone TPU Co. announcement, May 18, 2026.
13. The Information, June 2026, on Anthropic developer LOIs, direct Google guarantees, and Google approaches to Crusoe and CoreWeave; Nscale-Anthropic announcement, August 26, 2026.
14. Destiny Tech100 Form 424B3 (July 16, 2026 purchase of Fluidstack Series B preferred shares); Sacra on the Macquarie GPU-collateralized facility; Moelis on the Solis Arx acquisition (June 2, 2026).
15. PitchBook, Morningstar and Penn Mutual Asset Management commentary on AI-infrastructure high-yield issuance and spreads, April to August 2026.
16. CoreWeave Q1 (May 7) and Q2 2026 (August 11) earnings calls; Nebius Q2 2026 (August 12); IREN Q4 FY26 (August 27); Digital Realty Q2 2026 (July 23); Equinix Q2 2026 (July 29); Oracle Q4 FY26 (June 10); Vertiv Q2 2026 (July 29).
17. Consensus estimates and trailing fundamentals for 22 public companies, and dealer mid quotes for the reference bonds, as of September 2, 2026. Bond identifiers: Meridian Arc US58990CAA18; WULF Compute US982911AA70; Cipher Compute US17253NAA54; Applied Digital US00202DAA54; CoreWeave US21873SAB43 / US21873SAG30.
18. Crusoe, Nscale and Lambda private valuation reports, 2026 (Bloomberg, The Information, Reuters).
19. Management plan figures (2026E-2030E revenue, EBITDA, FCF) and site economics are from investor materials circulated with the round in March 2026.

*This is research, not investment advice. AGTI may hold positions in securities discussed.*
