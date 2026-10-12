"""Per-GPU PRL mining economics at today's network state, and the FP8 hard fork's effect on which GPUs keep mining.

Benchmarks are operator- or user-published (miner versions and power methods differ); rental prices are published
floors on 2026-10-12 (availability and permission to mine not verified). FP8 eligibility follows the fp8-scheme branch:
miner/miner-base/src/miner_base/devices.py maps SM90 -> HOPPER and SM100/SM120 -> BLACKWELL and rejects everything else,
and node/blockchain/validate.go accepts only V4 (FP8) certificates from Fp8ForkHeight on.

    python gpu_economics.py   -> data/gpu_economics.csv, charts/gpu_margins.png, charts/fp8_fork_scenarios.png
"""
import csv

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from economics import CHARTS, DATA, GREEN, MINT, MUTED, ORANGE, POWER_USD_KWH, RED, TEXT, style  # noqa: E402

POOL_FEE = 0.02
AMORT_MONTHS = 36
# Purchase prices (USD; used/refurbished where quoted, observed 2026-10-10..12 or Q3 2026 broker benchmarks):
# RTX 3090/4090 used median asks (CresvaMent), RTX 5090 used tracked low (BestValueGPU), L40S/A100/H100/H200 used
# and B200 new midpoints (Compute Exchange, Q3 2026), MI300X 20-99 unit quote midpoint (Gatewell).
# Running cost: consumer rigs hosted all-in at $0.09/kWh on wall power = 1.2 x GPU watts (Endemine);
# datacenter GPUs in a hosted slot (GPU Rent Hub: $95/mo PCIe, $240/mo HGX/OAM) plus power at $0.07/kWh x PUE 1.3.
CONSUMER_ALLIN_KWH, RIG_OVERHEAD = 0.09, 1.2
DC_POWER_KWH, DC_PUE = 0.07, 1.3
COST = {  # capex, hosted slot $/month (None = consumer all-in)
    "RTX 3090": (1450, None), "RTX 4090": (3199, None), "RTX 5090": (3822, None),
    "L40S": (6500, 95), "A100 80GB": (8250, 95), "MI300X": (18000, 240),
    "H100 SXM": (24500, 240), "H200 SXM": (32500, 240), "B200": (40000, 240)}


def running_cost(name, watts):
    capex, slot = COST[name]
    if slot is None:
        return watts * RIG_OVERHEAD / 1000 * 24 * CONSUMER_ALLIN_KWH
    return slot / 30.44 + watts / 1000 * 24 * DC_POWER_KWH * DC_PUE


# (GPU, TH/s, W, benchmark source, cheapest published rent $/hr, rent source, FP8-eligible)
GPUS = [
    ("RTX 3090", 132.2, 267, "Hashrate.no (SRBMiner, Sep 29)", 0.12, "Vast floor", False),
    ("RTX 4090", 236, 420, "Oyster pool", 0.14, "Vast floor", False),
    ("RTX 5090", 412, 475, "Oyster pool", 0.37, "Vast floor", True),
    ("L40S", 259, 350, "Kryptex", 1.09, "RunPod", False),
    ("A100 80GB", 187, 300, "Oyster pool", 1.50, "TensorDock (PCIe)", False),
    ("MI300X", 504, 750, "Oyster pool", 2.39, "RunPod", False),
    ("H100 SXM", 610, 610, "Oyster pool", 2.25, "TensorDock", True),
    ("H200 SXM", 742, 680, "Oyster pool", 5.29, "RunPod", True),
    ("B200", 1310, 1000, "Oyster pool", 6.25, "Vast floor", True),
]


def network():
    rows = list(csv.DictReader((DATA / "derived.csv").open()))
    tip = rows[-1]
    return float(tip["prl_per_day"]) / (float(tip["ehs"]) * 1e6), float(tip["price"])   # PRL per TH/s per day, USD


def main():
    style()
    prl_per_th, price = network()
    out = []
    for name, ths, w, src, rent, rsrc, fp8 in GPUS:
        prl = ths * prl_per_th * (1 - POOL_FEE)
        gross = prl * price
        capex = COST[name][0]
        power = running_cost(name, w)                      # power (+ hosting) per day
        full = capex / (AMORT_MONTHS * 30.44) + power
        out.append({"gpu": name, "ths": ths, "watts": w, "ths_per_kw": round(ths / w * 1000), "prl_day": round(prl, 2),
                    "gross_usd_day": round(gross, 2), "power_usd_day": round(power, 2), "owned_net_usd_day": round(gross - power, 2),
                    "capex_usd": capex, "full_cost_usd_day": round(full, 2), "full_net_usd_day": round(gross - full, 2),
                    "rent_usd_hr": rent, "rented_net_usd_day": round(gross - rent * 24, 2),
                    "breakeven_price_power": round(power / prl, 3), "breakeven_price_full": round(full / prl, 2),
                    "breakeven_price_rented": round(rent * 24 / prl, 2),
                    "fp8_eligible": fp8, "benchmark": src, "rent_source": rsrc})
    with (DATA / "gpu_economics.csv").open("w", newline="") as f:
        wr = csv.DictWriter(f, fieldnames=list(out[0]))
        wr.writeheader()
        wr.writerows(out)

    fig, ax = plt.subplots(figsize=(11, 5.8))
    y = np.arange(len(out))
    bars = (("breakeven_price_power", MINT, "running cost only (card already paid for, otherwise idle)", -0.27),
            ("breakeven_price_full", GREEN, f"full owned cost (purchase price over {AMORT_MONTHS} mo + running cost)", 0.0),
            ("breakeven_price_rented", RED, "rented at the cheapest published rate", 0.27))
    for key, color, label, dy in bars:
        ax.barh(y + dy, [r[key] for r in out], height=0.26, color=color, label=label)
        for yy, r in zip(y, out):
            ax.text(r[key] + 0.08, yy + dy, f"${r[key]:.2f}", va="center", fontsize=7.5, color=TEXT)
    ax.axvline(price, color=ORANGE, lw=1.6, ls="--")
    ax.text(price + 0.05, len(out) - 0.4, f"PRL today ${price:.2f}", color=ORANGE, fontsize=9)
    ax.set_yticks(y)
    ax.set_yticklabels([f"{r['gpu']}{'  [FP8]' if r['fp8_eligible'] else ''}" for r in out])
    ax.invert_yaxis()
    ax.set_xlim(0, max(r["breakeven_price_rented"] for r in out) * 1.12)
    ax.set_xlabel(f"breakeven PRL price (USD), {POOL_FEE:.0%} pool fee, today's network")
    ax.set_title("Breakeven PRL price by GPU under three cost bases ([FP8] = keeps mining after the fork)")
    ax.grid(True, axis="x", alpha=0.6)
    ax.legend(loc="upper right", fontsize=8.5)
    fig.tight_layout()
    fig.savefig(CHARTS / "gpu_margins.png")
    plt.close(fig)

    # FP8 fork: if only Hopper/Blackwell keep mining, the same issuance is split over the eligible share of hashrate
    fig, ax = plt.subplots(figsize=(10, 4.6))
    share = np.linspace(0.15, 1.0, 100)
    for name, color in (("RTX 5090", GREEN), ("H100 SXM", MINT), ("B200", ORANGE)):
        r = next(x for x in out if x["gpu"] == name)
        ax.plot(share * 100, r["gross_usd_day"] / share, color=color, lw=2.2, label=f"{name}: gross USD/day (today {r['gross_usd_day']:.2f})")
    for name, color in (("RTX 5090", GREEN), ("H100 SXM", MINT), ("B200", ORANGE)):
        r = next(x for x in out if x["gpu"] == name)
        ax.axhline(r["rent_usd_hr"] * 24, color=color, ls=":", lw=1)
    ax.set_xlim(100, 15)
    ax.set_xlabel("share of today's hashrate that is Hopper / Blackwell (eligible after the fork), %")
    ax.set_ylabel("gross USD per GPU per day")
    ax.set_title("FP8 fork, day one: eligible miners split the same issuance (dotted = rent)")
    ax.grid(True, alpha=0.6)
    ax.legend(loc="upper left", fontsize=9)
    ax.text(0.99, 0.02, "holds price and issuance fixed; assumes FP8 per-GPU throughput equals today's int7 benchmark",
            transform=ax.transAxes, ha="right", fontsize=8, color=MUTED)
    fig.tight_layout()
    fig.savefig(CHARTS / "fp8_fork_scenarios.png")
    plt.close(fig)
    forward_curve(out)
    payback(out, price)
    print(f"network: {prl_per_th:.4f} PRL per TH/s per day at ${price:.2f}")
    for r in out:
        print(f"{r['gpu']:10s} {r['ths']:6.0f} TH/s {r['ths_per_kw']:5d} TH/kW  {r['prl_day']:6.2f} PRL/d  gross ${r['gross_usd_day']:6.2f}  "
              f"owned ${r['owned_net_usd_day']:6.2f}  rented ${r['rented_net_usd_day']:7.2f} (rent ${r['rent_usd_hr']}/h)  "
              f"BE power ${r['breakeven_price_power']:.2f} full ${r['breakeven_price_full']:.2f} rented ${r['breakeven_price_rented']:.2f}  FP8 {r['fp8_eligible']}")


def payback(out, price, months=24):
    """24-month value of an RTX 5090 bought today at the quoted used price: PRL mined (emission decay x hashrate
    scenarios, int7 rules) x a flat PRL price, minus running cost, against the purchase price."""
    from economics import EMISSION_C, MAX_SUPPLY, TARGET_BLOCK_S
    tip = list(csv.DictReader((DATA / "derived.csv").open()))[-1]
    h0, ehs0 = int(tip["height"]), float(tip["ehs"])
    r = next(x for x in out if x["gpu"] == "RTX 5090")
    days = np.arange(months * 30.44)
    h = h0 + days * 86400 / TARGET_BLOCK_S
    reward = MAX_SUPPLY * EMISSION_C / (h + EMISSION_C) ** 2
    rows = []
    for g in (0.0, 0.5, 1.5):
        ehs = ehs0 * (1 + g) ** (days / 365.25)
        prl = (r["ths"] * (1 - POOL_FEE) * 86400 / TARGET_BLOCK_S * reward / (ehs * 1e6)).sum()
        net = prl * price - r["power_usd_day"] * len(days)
        rows.append({"hashrate_growth": g, "prl_24m": round(prl), "net_usd_24m": round(net), "capex": COST["RTX 5090"][0],
                     "net_minus_capex": round(net - COST["RTX 5090"][0]),
                     "price_to_pay_back": round((COST["RTX 5090"][0] + r["power_usd_day"] * len(days)) / prl, 2)})
    with (DATA / "payback_5090.csv").open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    for x in rows:
        print("5090 payback", x)


def forward_curve(out, months=24):
    """PRL per RTX 5090 per day over the next two years: emission decay at the 3:14 target, times hashrate growth
    scenarios. This is the PRL-denominated cash flow a stripped mining-yield token would pay (before price)."""
    from economics import EMISSION_C, MAX_SUPPLY, TARGET_BLOCK_S
    rows = list(csv.DictReader((DATA / "derived.csv").open()))
    tip = rows[-1]
    h0, ehs0 = int(tip["height"]), float(tip["ehs"])
    r5090 = next(x for x in out if x["gpu"] == "RTX 5090")
    t = np.linspace(0, months, 200)
    blocks_day = 86400 / TARGET_BLOCK_S
    h = h0 + t * 30.44 * blocks_day
    reward = MAX_SUPPLY * EMISSION_C / (h + EMISSION_C) ** 2
    fig, ax = plt.subplots(figsize=(10.5, 4.8))
    for g, color in ((0.0, GREEN), (0.5, MINT), (1.5, ORANGE), (4.0, RED)):
        ehs = ehs0 * (1 + g) ** (t / 12)
        prl = r5090["ths"] * (1 - POOL_FEE) * blocks_day * reward / (ehs * 1e6)
        cum = np.cumsum(prl) * (t[1] - t[0]) * 30.44
        ax.plot(t, prl, color=color, lw=2.2, label=f"hashrate +{g:.0%}/yr: {cum[-1]:,.0f} PRL over {months} months")
    ax.set_xlabel("months from today")
    ax.set_ylabel("PRL per RTX 5090 per day")
    ax.set_title("Forward PRL yield per RTX 5090 (emission decay x hashrate growth), int7 rules")
    ax.grid(True, alpha=0.6)
    ax.legend(fontsize=9)
    fig.tight_layout()
    fig.savefig(CHARTS / "forward_yield_5090.png")
    plt.close(fig)
    h12 = h0 + 12 * 30.44 * blocks_day
    print(f"block reward now {MAX_SUPPLY * EMISSION_C / (h0 + EMISSION_C) ** 2:,.0f} PRL; in 12 months "
          f"{MAX_SUPPLY * EMISSION_C / (h12 + EMISSION_C) ** 2:,.0f} PRL")


if __name__ == "__main__":
    main()
