#!/usr/bin/env python3
"""
Pearl PoUW fleet economics — dual-use GPU vs bare GPU vs ASIC.

Illustrative monthly simulation for AGTI research. Published: https://agtico.github.io/assets/research/pearl-economics/
"""

from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Literal

import numpy as np

FleetKind = Literal["dual", "bare", "asic"]

FLEETS: tuple[FleetKind, ...] = ("dual", "bare", "asic")


@dataclass
class FleetParams:
    """Per-unit (one GPU or one ASIC box) monthly economics."""

    hash_units: float  # relative PoW hashrate vs baseline GPU
    power_kw: float
    capex_usd: float
    lifetime_months: float
    opex_usd: float  # staffing, hosting overhead per unit / month


@dataclass
class EconomyParams:
    months: int = 36
    blocks_per_month: float = 43_200.0  # ~1 min blocks
    block_reward_prl: float = 2_711.0  # ~mainnet coinbase order of magnitude
    prl_price_usd: float = 0.05
    prl_price_endogenous: bool = True
    prl_price_mean_revert: float = 0.12  # speed toward production-cost anchor
    prl_price_noise: float = 0.04

    electricity_usd_per_kwh: float = 0.08
    hours_per_month: float = 730.0

    # Dual-use only: net inference margin per GPU-month if fully utilized
    inference_margin_usd_per_gpu: float = 1_200.0
    inference_demand_gpus: float = 400.0
    mining_overhead_dual: float = 0.18  # fraction of hash lost when co-serving inference

    # Capacity adjustment (monthly)
    growth_rate: float = 0.35
    max_monthly_shrink: float = 0.25
    max_monthly_grow: float = 0.45
    min_units: float = 1.0
    max_units: float = 50_000.0

    dual: FleetParams = field(
        default_factory=lambda: FleetParams(
            hash_units=1.0,
            power_kw=0.70,  # H-class GPU average draw
            capex_usd=28_000.0,
            lifetime_months=30.0,
            opex_usd=120.0,
        )
    )
    bare: FleetParams = field(
        default_factory=lambda: FleetParams(
            hash_units=1.12,  # no inference serving overhead
            power_kw=0.70,
            capex_usd=28_000.0,
            lifetime_months=30.0,
            opex_usd=80.0,
        )
    )
    asic: FleetParams = field(
        default_factory=lambda: FleetParams(
            hash_units=3.5,  # illustrative matmul+zk ASIC advantage
            power_kw=0.35,
            capex_usd=12_000.0,
            lifetime_months=24.0,
            opex_usd=40.0,
        )
    )

    # Initial fleet (units)
    init_dual: float = 800.0
    init_bare: float = 150.0
    init_asic: float = 0.0


@dataclass
class ScenarioShock:
    name: str
    description: str
    prl_price_path: np.ndarray | None = None  # length months, overrides endogenous
    inference_demand_path: np.ndarray | None = None
    asic_hash_multiplier_path: np.ndarray | None = None  # scales asic.hash_units


@dataclass
class MonthRecord:
    month: int
    prl_price_usd: float
    cap_dual: float
    cap_bare: float
    cap_asic: float
    hash_dual: float
    hash_bare: float
    hash_asic: float
    hash_total: float
    share_dual: float
    share_bare: float
    share_asic: float
    hhi: float
    useful_hash_fraction: float
    inference_gpus_served: float
    revenue_dual_usd: float
    revenue_bare_usd: float
    revenue_asic_usd: float
    cost_dual_usd: float
    cost_bare_usd: float
    cost_asic_usd: float
    profit_dual_usd: float
    profit_bare_usd: float
    profit_asic_usd: float
    mining_usd_total: float
    inference_usd_total: float


def _fleet_params(params: EconomyParams, kind: FleetKind) -> FleetParams:
    return getattr(params, kind)


def unit_hash(
    params: EconomyParams,
    kind: FleetKind,
    asic_multiplier: float = 1.0,
) -> float:
    fp = _fleet_params(params, kind)
    h = fp.hash_units
    if kind == "dual":
        h *= 1.0 - params.mining_overhead_dual
    if kind == "asic":
        h *= asic_multiplier
    return h


def unit_cost_usd(params: EconomyParams, kind: FleetKind) -> float:
    fp = _fleet_params(params, kind)
    elec = fp.power_kw * params.hours_per_month * params.electricity_usd_per_kwh
    amort = fp.capex_usd / fp.lifetime_months
    return elec + amort + fp.opex_usd


def fleet_hash(cap: dict[FleetKind, float], params: EconomyParams, asic_mult: float) -> dict[FleetKind, float]:
    return {k: cap[k] * unit_hash(params, k, asic_mult) for k in FLEETS}


def hhi(shares: np.ndarray) -> float:
    return float(np.sum(np.square(shares)))


def _adjust_capacity(cap: float, profit_per_unit: float, cost_per_unit: float, params: EconomyParams) -> float:
    if cap <= 0:
        cap = params.min_units
    margin = profit_per_unit / max(cost_per_unit, 1e-9)
    delta = params.growth_rate * margin
    delta = np.clip(delta, -params.max_monthly_shrink, params.max_monthly_grow)
    cap *= 1.0 + delta
    return float(np.clip(cap, params.min_units, params.max_units))


def simulate(params: EconomyParams, scenario: ScenarioShock | None = None) -> list[MonthRecord]:
    scenario = scenario or ScenarioShock(name="custom", description="")
    months = params.months

    def path_or_default(p: np.ndarray | None, default: float) -> np.ndarray:
        if p is None:
            return np.full(months, default, dtype=float)
        if len(p) != months:
            raise ValueError(f"Scenario path length {len(p)} != months {months}")
        return p.astype(float)

    prl_path = path_or_default(scenario.prl_price_path, params.prl_price_usd)
    demand_path = path_or_default(scenario.inference_demand_path, params.inference_demand_gpus)
    asic_mult_path = path_or_default(scenario.asic_hash_multiplier_path, 1.0)

    cap: dict[FleetKind, float] = {
        "dual": params.init_dual,
        "bare": params.init_bare,
        "asic": params.init_asic,
    }

    records: list[MonthRecord] = []
    prl = float(prl_path[0])

    for t in range(months):
        if scenario.prl_price_path is not None:
            prl = float(prl_path[t])
        demand = float(demand_path[t])
        asic_mult = float(asic_mult_path[t])

        hashes = fleet_hash(cap, params, asic_mult)
        h_total = sum(hashes.values()) + 1e-12
        shares = {k: hashes[k] / h_total for k in FLEETS}

        block_value_usd = params.blocks_per_month * params.block_reward_prl * prl

        # Mining revenue by fleet
        rev_mine = {k: shares[k] * block_value_usd for k in FLEETS}

        # Inference: only dual-use; demand-limited
        served = min(cap["dual"], demand)
        rev_inf_dual = served * params.inference_margin_usd_per_gpu
        rev = {
            "dual": rev_mine["dual"] + rev_inf_dual,
            "bare": rev_mine["bare"],
            "asic": rev_mine["asic"],
        }

        costs = {k: cap[k] * unit_cost_usd(params, k) for k in FLEETS}
        profits = {k: rev[k] - costs[k] for k in FLEETS}
        profit_per_unit = {k: profits[k] / max(cap[k], 1e-9) for k in FLEETS}
        cpu = {k: unit_cost_usd(params, k) for k in FLEETS}

        records.append(
            MonthRecord(
                month=t,
                prl_price_usd=prl,
                cap_dual=cap["dual"],
                cap_bare=cap["bare"],
                cap_asic=cap["asic"],
                hash_dual=hashes["dual"],
                hash_bare=hashes["bare"],
                hash_asic=hashes["asic"],
                hash_total=h_total,
                share_dual=shares["dual"],
                share_bare=shares["bare"],
                share_asic=shares["asic"],
                hhi=hhi(np.array([shares[k] for k in FLEETS])),
                useful_hash_fraction=shares["dual"],
                inference_gpus_served=served,
                revenue_dual_usd=rev["dual"],
                revenue_bare_usd=rev["bare"],
                revenue_asic_usd=rev["asic"],
                cost_dual_usd=costs["dual"],
                cost_bare_usd=costs["bare"],
                cost_asic_usd=costs["asic"],
                profit_dual_usd=profits["dual"],
                profit_bare_usd=profits["bare"],
                profit_asic_usd=profits["asic"],
                mining_usd_total=sum(rev_mine.values()),
                inference_usd_total=rev_inf_dual,
            )
        )

        # Next-month capacity
        for k in FLEETS:
            cap[k] = _adjust_capacity(cap[k], profit_per_unit[k], cpu[k], params)

        # Endogenous PRL: anchor to marginal cost of an average hash-unit (bare GPU reference)
        if params.prl_price_endogenous and scenario.prl_price_path is None and t + 1 < months:
            weighted_cost_per_hash = sum(costs[k] / max(hashes[k], 1e-9) for k in FLEETS) / len(FLEETS)
            target_prl = weighted_cost_per_hash * h_total / (
                params.blocks_per_month * params.block_reward_prl + 1e-12
            )
            shock = np.random.default_rng(t + 42).normal(0.0, params.prl_price_noise)
            prl = (1 - params.prl_price_mean_revert) * prl + params.prl_price_mean_revert * target_prl
            prl = max(prl * (1.0 + shock), 1e-6)

    return records


def build_scenarios(months: int = 36) -> dict[str, ScenarioShock]:
    t = np.arange(months, dtype=float)

    prl_pump = np.concatenate([np.full(6, 0.05), np.linspace(0.05, 0.75, months - 6)])  # ~15×

    inference_boom = np.full(months, 400.0)
    inference_boom[t >= 4] = 400.0 + (t[t >= 4] - 3) * 180.0
    inference_boom = np.minimum(inference_boom, 4_000.0)

    asic_creep = np.ones(months)
    asic_creep[t >= 8] = 1.0 + 0.12 * (t[t >= 8] - 7)
    asic_creep = np.minimum(asic_creep, 4.0)

    return {
        "baseline": ScenarioShock(
            name="baseline",
            description="Moderate PRL ($0.05), 400 GPU inference demand, no ASIC creep.",
        ),
        "prl_pump": ScenarioShock(
            name="prl_pump",
            description="PRL ramps ~15× over year (exogenous); inference demand flat.",
            prl_price_path=prl_pump,
            inference_demand_path=np.full(months, 400.0),
        ),
        "inference_boom": ScenarioShock(
            name="inference_boom",
            description="API demand grows to ~4k GPUs; flat low PRL.",
            prl_price_path=np.full(months, 0.05),
            inference_demand_path=inference_boom,
        ),
        "asic_creep": ScenarioShock(
            name="asic_creep",
            description="ASIC hash/$ improves up to 4×; flat PRL and demand.",
            asic_hash_multiplier_path=asic_creep,
        ),
        "together_only": ScenarioShock(
            name="together_only",
            description="Tiny inference market (50 GPUs), low PRL — dual-use barely viable.",
            inference_demand_path=np.full(months, 50.0),
            prl_price_path=np.full(months, 0.04),
        ),
        "bitcoinification": ScenarioShock(
            name="bitcoinification",
            description="PRL pump + ASIC creep + flat inference — mission collapse stress test.",
            prl_price_path=prl_pump,
            inference_demand_path=np.full(months, 300.0),
            asic_hash_multiplier_path=asic_creep,
        ),
    }


def summarize(records: list[MonthRecord], scenario: ScenarioShock) -> dict:
    last = records[-1]
    mid = records[len(records) // 2]
    return {
        "scenario": scenario.name,
        "description": scenario.description,
        "month_0": {
            "useful_hash_pct": round(records[0].useful_hash_fraction * 100, 1),
            "asic_hash_pct": round(records[0].share_asic * 100, 1),
            "hhi": round(records[0].hhi, 3),
            "prl_usd": round(records[0].prl_price_usd, 4),
        },
        "mid": {
            "useful_hash_pct": round(mid.useful_hash_fraction * 100, 1),
            "asic_hash_pct": round(mid.share_asic * 100, 1),
            "hhi": round(mid.hhi, 3),
            "prl_usd": round(mid.prl_price_usd, 4),
        },
        "terminal": {
            "useful_hash_pct": round(last.useful_hash_fraction * 100, 1),
            "asic_hash_pct": round(last.share_asic * 100, 1),
            "bare_hash_pct": round(last.share_bare * 100, 1),
            "dual_cap": round(last.cap_dual, 0),
            "bare_cap": round(last.cap_bare, 0),
            "asic_cap": round(last.cap_asic, 0),
            "hhi": round(last.hhi, 3),
            "prl_usd": round(last.prl_price_usd, 4),
            "inference_share_of_revenue_pct": round(
                100 * last.inference_usd_total / max(last.inference_usd_total + last.mining_usd_total, 1e-9),
                1,
            ),
        },
    }


def records_to_arrays(records: list[MonthRecord]) -> dict[str, np.ndarray]:
    d = asdict(records[0])
    keys = d.keys()
    out: dict[str, np.ndarray] = {}
    for k in keys:
        out[k] = np.array([getattr(r, k) for r in records], dtype=float)
    return out


def save_csv(records: list[MonthRecord], path: Path) -> None:
    arrs = records_to_arrays(records)
    header = ",".join(arrs.keys())
    rows = np.column_stack([arrs[k] for k in arrs.keys()])
    np.savetxt(path, rows, delimiter=",", header=header, comments="")


def _inference_revenue_ylim(inf_pct: np.ndarray) -> tuple[float, float]:
    """Scale bottom panel to data — avoid pinning 0–100% when values are ~1%."""
    ymax = float(np.max(inf_pct))
    if ymax <= 0:
        return 0.0, 1.0
    if ymax <= 5:
        return 0.0, max(ymax * 1.3, 0.5)
    if ymax <= 30:
        return 0.0, ymax * 1.15
    return 0.0, min(100.0, ymax * 1.1)


def plot_scenario(records: list[MonthRecord], scenario: ScenarioShock, out_dir: Path) -> None:
    import matplotlib.pyplot as plt
    from matplotlib.ticker import MaxNLocator

    arrs = records_to_arrays(records)
    t = arrs["month"]

    fig, axes = plt.subplots(3, 1, figsize=(10, 10), sharex=True)
    fig.suptitle(f"Pearl PoUW economics — {scenario.name}", fontsize=13, fontweight="bold")

    ax = axes[0]
    ax.stackplot(
        t,
        arrs["share_dual"] * 100,
        arrs["share_bare"] * 100,
        arrs["share_asic"] * 100,
        labels=["Dual-use (useful proxy)", "Bare GPU", "ASIC"],
        colors=["#6ee58f", "#ff9f45", "#ff5a42"],
        alpha=0.85,
    )
    ax.set_ylabel("Hashrate share %")
    ax.set_ylim(0, 100)
    ax.legend(loc="upper right", fontsize=8)
    ax.grid(True, alpha=0.25)

    ax = axes[1]
    ax2 = ax.twinx()
    ax.plot(t, arrs["prl_price_usd"], color="#ff5a42", lw=2, label="PRL price ($)")
    ax2.plot(t, arrs["hhi"], color="#9aa2a0", ls="--", label="HHI")
    ax.set_ylabel("PRL ($)")
    ax2.set_ylabel("HHI")
    ax.grid(True, alpha=0.25)

    ax = axes[2]
    inf_pct = 100 * arrs["inference_usd_total"] / np.maximum(
        arrs["inference_usd_total"] + arrs["mining_usd_total"], 1e-9
    )
    ylo, yhi = _inference_revenue_ylim(inf_pct)
    ax.fill_between(t, inf_pct, color="#6ee58f", alpha=0.18)
    ax.plot(t, inf_pct, color="#6ee58f", lw=2.5, marker="o", markersize=2.5, markevery=max(1, len(t) // 12))
    ax.set_ylabel("Inference % of revenue")
    ax.set_xlabel("Month")
    ax.set_ylim(ylo, yhi)
    ax.yaxis.set_major_locator(MaxNLocator(nbins=5, prune=None))
    ax.grid(True, alpha=0.25)

    fig.tight_layout()
    fig.savefig(out_dir / f"{scenario.name}.png", dpi=150, bbox_inches="tight")
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description="Pearl PoUW fleet economic simulation")
    parser.add_argument("--scenario", default="baseline", help="Scenario name or 'all'")
    parser.add_argument("--months", type=int, default=36)
    parser.add_argument("--all", action="store_true", help="Run full scenario grid")
    parser.add_argument("--plot", action="store_true", help="Write PNG charts to output/")
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).resolve().parent / "output",
    )
    args = parser.parse_args()
    np.random.seed(args.seed)

    scenarios = build_scenarios(args.months)
    names = list(scenarios.keys()) if args.all or args.scenario == "all" else [args.scenario]
    if args.scenario not in scenarios and args.scenario != "all":
        raise SystemExit(f"Unknown scenario {args.scenario!r}. Choose from: {', '.join(scenarios)}")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    summaries = []

    for name in names:
        sc = scenarios[name]
        params = EconomyParams(months=args.months)
        if name == "together_only":
            params.init_dual = 60.0
            params.init_bare = 400.0
            params.init_asic = 0.0
        records = simulate(params, sc)
        save_csv(records, args.output_dir / f"{name}.csv")
        summaries.append(summarize(records, sc))
        if args.plot:
            plot_scenario(records, sc, args.output_dir)

    print(json.dumps(summaries, indent=2))
    (args.output_dir / "summary.json").write_text(json.dumps(summaries, indent=2))


if __name__ == "__main__":
    main()
