"""Pearl (PRL) network and mining economics, October 2026: the charts and tables of AGTI's "Engaging with PRL".

Inputs (fetch_chain.py, fetch_market.py): data/chain.csv (Blockbook samples), data/market.csv (CoinGecko daily).
Pearl difficulty is not in hash units, so hashrate is implied as H = k * difficulty / block_time and k is calibrated to
the network hashrate pools published on 2026-10-12 (Hashrate.no 51.30 EH/s, Kryptex 43.84 EH/s: midpoint, with the band
shown). Basu (arXiv 2606.04819) measured ~24 EH/s on 30-31 May: printed as an out-of-sample check.

    python economics.py            -> data/derived.csv, charts/*.png, printed summary
"""
import csv
import datetime as dt
import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.dates as mdates  # noqa: E402
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

HERE = Path(__file__).resolve().parent
DATA, CHARTS = HERE / "data", HERE / "charts"

# ---- protocol constants (pearl-research-labs/pearl master, node/blockchain/validate.go CalcBlockSubsidy and
# node/chaincfg/params.go): supply(h) = MAX * h / (h + C), C = four years of 194 s blocks
MAX_SUPPLY = 2_100_000_000
TARGET_BLOCK_S = 194
EMISSION_C = (4 * 365 * 24 * 3600) // TARGET_BLOCK_S          # 650,226

# ---- calibration and hardware (sources in the article)
PUBLISHED_EHS = {"Hashrate.no": 51.30, "Kryptex": 43.84}      # 2026-10-12
H100_THS, H100_W = 610.0, 610.0                               # Oyster pool benchmark (same basis as gpu_economics.py)
POWER_USD_KWH = 0.08                                          # industrial power assumption
POOL_FEE = 0.02                                               # typical pool fee (Kryptex PPS+ 2%)

EVENTS = [  # (date, label)
    ("2026-04-27", "mainnet genesis"),
    ("2026-06-02", "akoya: consumer + AMD miner"),
    ("2026-06-25", "BigONE listing"),
    ("2026-07-23", "dense-only fork"),
    ("2026-08-11", "salted-seed hardfork"),
    ("2026-09-09", "FP8 PR #311 opened"),
]

BG, PANEL, GRID, TEXT, MUTED = "#070b0a", "#0d1411", "#243028", "#f2f5f3", "#8aa898"
GREEN, ORANGE, RED, MINT = "#6ee58f", "#ff9f45", "#ff5a42", "#9dffc8"


def style():
    plt.rcParams.update({
        "figure.facecolor": BG, "axes.facecolor": PANEL, "axes.edgecolor": GRID, "axes.labelcolor": TEXT,
        "axes.titlecolor": TEXT, "xtick.color": MUTED, "ytick.color": MUTED, "grid.color": GRID, "text.color": TEXT,
        "font.size": 11, "axes.titlesize": 13, "axes.titleweight": "bold", "legend.facecolor": PANEL,
        "legend.edgecolor": GRID, "savefig.facecolor": BG, "savefig.dpi": 160,
    })


def day(ts):
    return dt.datetime.fromtimestamp(ts, dt.timezone.utc)


def load():
    chain = [{k: float(v) for k, v in r.items()} for r in csv.DictReader((DATA / "chain.csv").open())]
    market = {r["date"]: {k: float(v) for k, v in r.items() if k != "date"} for r in csv.DictReader((DATA / "market.csv").open())}
    return chain, market


def derive(chain, market):
    rows = []
    for a, b in zip(chain, chain[1:]):
        dh, dts = b["height"] - a["height"], b["time"] - a["time"]
        if dh <= 0 or dts <= 0:
            continue
        rows.append({"time": b["time"], "height": int(b["height"]), "difficulty": b["difficulty"],
                     "block_s": dts / dh, "reward": b["coinbase_prl"]})
    # smooth the block time over ~2,500 blocks so the implied hashrate is not driven by sample noise
    bt = np.array([r["block_s"] for r in rows])
    w = 5
    smooth = np.convolve(bt, np.ones(w) / w, mode="same")
    smooth[:w] = bt[:w]
    smooth[-w:] = [bt[-w:].mean()] * w
    for r, s in zip(rows, smooth):
        r["block_s_smooth"] = s
    tip = rows[-1]
    k_mid = (sum(PUBLISHED_EHS.values()) / len(PUBLISHED_EHS)) * tip["block_s_smooth"] / tip["difficulty"]
    k_lo = min(PUBLISHED_EHS.values()) * tip["block_s_smooth"] / tip["difficulty"]
    k_hi = max(PUBLISHED_EHS.values()) * tip["block_s_smooth"] / tip["difficulty"]
    for r in rows:
        r["ehs"] = k_mid * r["difficulty"] / r["block_s_smooth"]
        r["ehs_lo"], r["ehs_hi"] = (k * r["difficulty"] / r["block_s_smooth"] for k in (k_lo, k_hi))
        r["prl_per_day"] = 86400 / r["block_s_smooth"] * r["reward"]
        d = day(r["time"]).strftime("%Y-%m-%d")
        r["price"] = market.get(d, {}).get("price_usd")
        r["volume"] = market.get(d, {}).get("volume_usd")
        r["h100_prl_day"] = H100_THS / (r["ehs"] * 1e6) * r["prl_per_day"] * (1 - POOL_FEE)
        r["h100_usd_day"] = r["h100_prl_day"] * r["price"] if r["price"] else None
        r["issuance_usd_day"] = r["prl_per_day"] * r["price"] if r["price"] else None
    return rows


def supply(h):
    return MAX_SUPPLY * h / (h + EMISSION_C)


def mark_events(ax, ymax):
    for d, label in EVENTS:
        x = dt.datetime.fromisoformat(d).replace(tzinfo=dt.timezone.utc)
        ax.axvline(x, color=MUTED, lw=0.8, ls=":", alpha=0.8)
        ax.text(x, ymax * 0.99, label + " ", rotation=90, va="top", ha="right", fontsize=8, color=MUTED)


def fmt_dates(ax):
    ax.xaxis.set_major_locator(mdates.MonthLocator())
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%b"))
    ax.grid(True, alpha=0.6)


def chart_hashrate(rows):
    fig, ax = plt.subplots(figsize=(11, 5.6))
    x = [day(r["time"]) for r in rows]
    ax.fill_between(x, [r["ehs_lo"] for r in rows], [r["ehs_hi"] for r in rows], color=GREEN, alpha=0.18, lw=0)
    ax.plot(x, [r["ehs"] for r in rows], color=GREEN, lw=2.2, label="implied network hashrate (EH/s)")
    may = dt.datetime(2026, 5, 31, tzinfo=dt.timezone.utc)
    ax.scatter([may], [24], color=ORANGE, zorder=5, s=46, label="Basu measurement, 30-31 May (24 EH/s)")
    top = max(r["ehs_hi"] for r in rows) * 1.35
    ax.set_ylim(0, top)
    mark_events(ax, top)
    ax2 = ax.twinx()
    ax2.plot(x, [r["block_s_smooth"] / 60 for r in rows], color=ORANGE, lw=1.2, alpha=0.85, label="block time (min)")
    ax2.axhline(TARGET_BLOCK_S / 60, color=ORANGE, lw=0.8, ls="--", alpha=0.6)
    ax2.set_ylim(0, 6)
    ax2.set_ylabel("block time (min); dashed = 3:14 target", color=ORANGE)
    ax2.tick_params(axis="y", colors=ORANGE)
    ax.set_ylabel("EH/s (band: Kryptex to Hashrate.no calibration)")
    ax.set_title("Pearl network hashrate, genesis to today")
    fmt_dates(ax)
    h1, l1 = ax.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax.legend(h1 + h2, l1 + l2, loc="upper center", bbox_to_anchor=(0.5, -0.1), ncol=3, fontsize=9, frameon=False)
    fig.tight_layout()
    fig.savefig(CHARTS / "hashrate.png")
    plt.close(fig)


def chart_emission(rows, chain):
    tip = rows[-1]
    now = day(tip["time"])
    fig, ax = plt.subplots(figsize=(11, 4.8))
    hx = [day(r["time"]) for r in chain]
    ax.plot(hx, [supply(r["height"]) / 1e9 for r in chain], color=GREEN, lw=2.6, label="mined supply (actual)")
    fut = np.linspace(tip["height"], tip["height"] + 10 * 365.25 * 86400 / TARGET_BLOCK_S, 300)
    fx = [now + dt.timedelta(seconds=float(h - tip["height"]) * TARGET_BLOCK_S) for h in fut]
    ax.plot(fx, [supply(h) / 1e9 for h in fut], color=GREEN, lw=1.6, ls="--", label="projected at the 3:14 target")
    genesis = day(chain[0]["time"])
    sched = np.linspace(0, (now - genesis).total_seconds() / TARGET_BLOCK_S, 100)
    ax.plot([genesis + dt.timedelta(seconds=float(h) * TARGET_BLOCK_S) for h in sched], [supply(h) / 1e9 for h in sched],
            color=MUTED, lw=1.2, ls=":", label="if blocks had run on target from genesis")
    sup_now = supply(tip["height"])
    ideal_now = supply((now - genesis).total_seconds() / TARGET_BLOCK_S)
    ax.annotate(f"today: {sup_now / 1e6:,.0f}M mined; on-target schedule would be {ideal_now / 1e6:,.0f}M",
                (now, sup_now / 1e9), xytext=(18, 18), textcoords="offset points", color=ORANGE, fontsize=9.5,
                arrowprops=dict(arrowstyle="-", color=ORANGE, lw=0.8))
    for yrs, lab in ((1, "+1 yr"), (None, "50% mined")):
        h = tip["height"] + yrs * 365.25 * 86400 / TARGET_BLOCK_S if yrs else EMISSION_C
        xx = now + dt.timedelta(seconds=float(h - tip["height"]) * TARGET_BLOCK_S)
        ax.scatter([xx], [supply(h) / 1e9], color=MINT, s=26, zorder=5)
        ax.annotate(f"{lab} ({xx:%b %Y}): {supply(h) / 1e6:,.0f}M", (xx, supply(h) / 1e9), xytext=(8, -16),
                    textcoords="offset points", color=MINT, fontsize=9)
    ax.axhline(MAX_SUPPLY / 1e9, color=MUTED, ls="--", lw=0.8)
    ax.text(fx[-1], MAX_SUPPLY / 1e9 * 0.97, "2.1B cap ", ha="right", va="top", color=MUTED, fontsize=9)
    ax.set_ylabel("billion PRL")
    ax.set_ylim(0, 2.2)
    ax.set_title("Supply: no halvings; half of 2.1B is mined about four years in")
    ax.xaxis.set_major_locator(mdates.YearLocator())
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%Y"))
    ax.grid(True, alpha=0.6)
    ax.legend(loc="lower right", fontsize=9)
    fig.tight_layout()
    fig.savefig(CHARTS / "emission.png")
    plt.close(fig)


def chart_market(market):
    days = sorted(market)
    x = [dt.datetime.fromisoformat(d).replace(tzinfo=dt.timezone.utc) for d in days]
    fig, ax = plt.subplots(figsize=(11, 4.8))
    ax.plot(x, [market[d]["price_usd"] for d in days], color=GREEN, lw=2.2, label="PRL price (USD)")
    ax.set_ylabel("USD")
    ax2 = ax.twinx()
    ax2.bar(x, [market[d]["volume_usd"] / 1e6 for d in days], color=ORANGE, alpha=0.35, width=0.8, label="24h volume ($M)")
    ax2.set_ylabel("24h volume, $M", color=ORANGE)
    ax2.tick_params(axis="y", colors=ORANGE)
    lo = min(days, key=lambda d: market[d]["price_usd"])
    hi = max(days, key=lambda d: market[d]["price_usd"])
    for d, txt in ((lo, "low"), (hi, "high")):
        xx = dt.datetime.fromisoformat(d).replace(tzinfo=dt.timezone.utc)
        ax.annotate(f"daily {txt} ${market[d]['price_usd']:.2f} ({d[5:]})", (xx, market[d]["price_usd"]),
                    xytext=(10, 12 if txt == "low" else -18), textcoords="offset points", color=MINT, fontsize=9)
    ax.set_title("PRL price and volume since CoinGecko coverage (22 Jun)")
    fmt_dates(ax)
    ax.set_zorder(ax2.get_zorder() + 1)
    ax.patch.set_visible(False)
    h1, l1 = ax.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax.legend(h1 + h2, l1 + l2, loc="upper left", fontsize=9)
    fig.tight_layout()
    fig.savefig(CHARTS / "market.png")
    plt.close(fig)


def chart_h100(rows, rent_usd_hr):
    rs = [r for r in rows if r["h100_usd_day"] is not None]
    x = [day(r["time"]) for r in rs]
    fig, ax = plt.subplots(figsize=(11, 4.8))
    ax.plot(x, [r["h100_usd_day"] for r in rs], color=GREEN, lw=2.2, label="gross PRL revenue per H100 per day (USD)")
    power = H100_W / 1000 * 24 * POWER_USD_KWH
    ax.axhline(power, color=MINT, ls="--", lw=1.2, label=f"owned H100: power at ${POWER_USD_KWH:.2f}/kWh (${power:.2f}/day)")
    lo, hi = rent_usd_hr
    ax.axhspan(lo * 24, hi * 24, color=RED, alpha=0.18, lw=0, label=f"rented H100: ${lo:.2f}-{hi:.2f}/hr")
    ax.set_ylabel("USD per day")
    ax.set_title("Mining an H100: gross revenue vs the two cost bases")
    fmt_dates(ax)
    ax.legend(loc="upper left", fontsize=9)
    fig.tight_layout()
    fig.savefig(CHARTS / "h100_revenue.png")
    plt.close(fig)


def chart_issuance(rows):
    rs = [r for r in rows if r["issuance_usd_day"] and r["volume"]]
    x = [day(r["time"]) for r in rs]
    fig, ax = plt.subplots(figsize=(11, 4.6))
    ax.plot(x, [r["issuance_usd_day"] / 1e6 for r in rs], color=ORANGE, lw=2.2, label="new PRL issued per day ($M)")
    ax.plot(x, [r["volume"] / 1e6 for r in rs], color=GREEN, lw=1.6, label="exchange volume per day ($M)")
    ax.set_ylabel("$M per day")
    ax2 = ax.twinx()
    ax2.plot(x, [r["issuance_usd_day"] / r["volume"] for r in rs], color=MUTED, lw=1, ls=":", label="issuance / volume")
    ax2.set_ylabel("issuance as a share of volume", color=MUTED)
    ax.set_title("Daily miner issuance vs daily traded volume")
    fmt_dates(ax)
    h1, l1 = ax.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax.legend(h1 + h2, l1 + l2, loc="upper left", fontsize=9)
    fig.tight_layout()
    fig.savefig(CHARTS / "issuance_vs_volume.png")
    plt.close(fig)


def chart_sensitivity(rows, rent_usd_hr):
    tip = rows[-1]
    prices = np.linspace(0.25, 4.0, 31)
    hashes = np.linspace(20, 200, 31)                       # EH/s
    prl_day = tip["prl_per_day"]
    rent = sum(rent_usd_hr) / 2 * 24
    margin = np.array([[H100_THS / (h * 1e6) * prl_day * p - rent for p in prices] for h in hashes])
    fig, ax = plt.subplots(figsize=(9.5, 6))
    vmax = np.abs(margin).max()
    im = ax.imshow(margin, origin="lower", aspect="auto", cmap="RdYlGn", vmin=-vmax, vmax=vmax,
                   extent=[prices[0], prices[-1], hashes[0], hashes[-1]])
    cs = ax.contour(prices, hashes, margin, levels=[0], colors=[TEXT], linewidths=1.6)
    ax.clabel(cs, fmt=lambda v: "rented-H100 breakeven", fontsize=9, colors=[TEXT])
    ax.scatter([tip["price"]], [tip["ehs"]], color=TEXT, edgecolor=BG, s=70, zorder=5)
    ax.annotate("today", (tip["price"], tip["ehs"]), xytext=(8, 6), textcoords="offset points", color=TEXT)
    ax.set_xlabel("PRL price (USD)")
    ax.set_ylabel("network hashrate (EH/s)")
    ax.set_title(f"Rented-H100 daily margin (USD) at today's issuance; rent ${sum(rent_usd_hr) / 2:.2f}/hr")
    fig.colorbar(im, ax=ax, label="USD/day")
    fig.tight_layout()
    fig.savefig(CHARTS / "h100_margin_heatmap.png")
    plt.close(fig)


def main(rent_usd_hr=(2.25, 3.99)):   # H100 SXM: TensorDock to RunPod, 2026-10-12
    style()
    CHARTS.mkdir(exist_ok=True)
    chain, market = load()
    rows = derive(chain, market)
    with (DATA / "derived.csv").open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    chart_hashrate(rows)
    chart_emission(rows, chain)
    chart_market(market)
    chart_h100(rows, rent_usd_hr)
    chart_issuance(rows)
    chart_sensitivity(rows, rent_usd_hr)
    tip = rows[-1]
    may = min(rows, key=lambda r: abs(r["time"] - dt.datetime(2026, 5, 31, tzinfo=dt.timezone.utc).timestamp()))
    print(f"tip h={tip['height']} {day(tip['time']):%Y-%m-%d %H:%M} block {tip['block_s_smooth']:.0f}s "
          f"implied {tip['ehs']:.1f} EH/s [{tip['ehs_lo']:.1f}-{tip['ehs_hi']:.1f}] issuance {tip['prl_per_day']:,.0f} PRL/day "
          f"= ${tip['issuance_usd_day'] / 1e6:.2f}M/day; volume ${tip['volume'] / 1e6:.2f}M")
    print(f"H100: {tip['h100_prl_day']:.1f} PRL/day = ${tip['h100_usd_day']:.2f}/day gross; power "
          f"${H100_W / 1000 * 24 * POWER_USD_KWH:.2f}/day; rent ${rent_usd_hr[0] * 24:.0f}-{rent_usd_hr[1] * 24:.0f}/day")
    print(f"check: implied hashrate {day(may['time']):%Y-%m-%d} = {may['ehs']:.1f} EH/s (Basu measured ~24 EH/s, 30-31 May)")
    be = (sum(rent_usd_hr) / 2 * 24) / tip["h100_prl_day"]
    print(f"rented-H100 breakeven PRL price at today's hashrate: ${be:.2f}; owned (power only): "
          f"${H100_W / 1000 * 24 * POWER_USD_KWH / tip['h100_prl_day']:.3f}")
    genesis = day(chain[0]["time"])
    ideal = supply((day(tip["time"]) - genesis).total_seconds() / TARGET_BLOCK_S)
    print(f"front-loading: {tip['height'] - (day(tip['time']) - genesis).total_seconds() / TARGET_BLOCK_S:,.0f} blocks ahead "
          f"of the 3:14 schedule; {supply(tip['height']) / 1e6 - ideal / 1e6:,.0f}M PRL mined early ({ideal / 1e6:,.0f}M on schedule)")
    one_year = tip["height"] + int(365.25 * 86400 / TARGET_BLOCK_S)
    print(f"supply now {supply(tip['height']) / 1e6:,.1f}M; +1y {supply(one_year) / 1e6:,.1f}M "
          f"(+{(supply(one_year) / supply(tip['height']) - 1):.0%} inflation over the next year)")


if __name__ == "__main__":
    main()
