#!/usr/bin/env python3
"""Compare Pearl Together pricing vs OpenRouter Gemma 4 31B routes.

Public APIs only — no keys required for catalog/endpoint metadata.
"""

from __future__ import annotations

import argparse
import json
import urllib.request
from dataclasses import dataclass


@dataclass(frozen=True)
class PriceRow:
    source: str
    model_id: str
    input_per_1m: float
    output_per_1m: float
    context: int | None
    notes: str = ""


def _get_json(url: str) -> dict:
    req = urllib.request.Request(url, headers={"User-Agent": "agti-pearl-pricing/1.0"})
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.load(resp)


def per_1m(token_price: str | float | None) -> float:
    return float(token_price or 0) * 1_000_000


def fetch_openrouter() -> tuple[PriceRow, list[PriceRow]]:
    models = _get_json("https://openrouter.ai/api/v1/models")["data"]
    agg = next(m for m in models if m["id"] == "google/gemma-4-31b-it")
    agg_row = PriceRow(
        source="OpenRouter aggregate",
        model_id=agg["id"],
        input_per_1m=per_1m(agg["pricing"]["prompt"]),
        output_per_1m=per_1m(agg["pricing"]["completion"]),
        context=agg.get("context_length"),
        notes="Default router price; usually picks cheapest healthy provider",
    )

    endpoints_payload = _get_json(
        "https://openrouter.ai/api/v1/models/google/gemma-4-31b-it/endpoints"
    )["data"]["endpoints"]

    provider_rows: list[PriceRow] = []
    for ep in endpoints_payload:
        pr = ep["pricing"]
        provider_rows.append(
            PriceRow(
                source=f"OpenRouter → {ep['provider_name']}",
                model_id=ep["model_id"],
                input_per_1m=per_1m(pr["prompt"]),
                output_per_1m=per_1m(pr["completion"]),
                context=ep.get("context_length"),
                notes=f"quant={ep.get('quantization')}",
            )
        )
    provider_rows.sort(key=lambda r: r.input_per_1m)
    return agg_row, provider_rows


def together_catalog() -> list[PriceRow]:
    """Together list prices from public model pages (May 2026)."""
    return [
        PriceRow(
            source="Together direct",
            model_id="google/gemma-4-31B-it",
            input_per_1m=0.39,
            output_per_1m=0.97,
            context=262_144,
            notes="FP8, 256K ctx — together.ai/models/gemma-4-31b",
        ),
        PriceRow(
            source="Together direct",
            model_id="pearl-ai/gemma-4-31b-it",
            input_per_1m=0.28,
            output_per_1m=0.86,
            context=32_000,
            notes="Pearl INT8, 32K ctx — marketed as ~25% off",
        ),
    ]


def blended_cost(row: PriceRow, input_m: float, output_m: float) -> float:
    return row.input_per_1m * input_m + row.output_per_1m * output_m


def pct_delta(new: float, base: float) -> float:
    if base == 0:
        return 0.0
    return 100.0 * (new - base) / base


def main() -> None:
    parser = argparse.ArgumentParser(description="Pearl vs OpenRouter Gemma pricing")
    parser.add_argument("--input-m", type=float, default=1.0, help="Input tokens (millions)")
    parser.add_argument("--output-m", type=float, default=0.2, help="Output tokens (millions)")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    agg, providers = fetch_openrouter()
    together = together_catalog()
    cheapest = providers[0]
    pearl = together[1]
    together_std = together[0]

    summary = {
        "openrouter_aggregate": agg.__dict__,
        "cheapest_openrouter_endpoint": cheapest.__dict__,
        "together_standard": together_std.__dict__,
        "together_pearl": pearl.__dict__,
        "workload": {"input_m": args.input_m, "output_m": args.output_m},
        "blended_usd": {
            "openrouter_cheapest": blended_cost(cheapest, args.input_m, args.output_m),
            "openrouter_aggregate": blended_cost(agg, args.input_m, args.output_m),
            "together_standard": blended_cost(together_std, args.input_m, args.output_m),
            "together_pearl": blended_cost(pearl, args.input_m, args.output_m),
        },
        "pearl_vs_cheapest_pct": {
            "input": pct_delta(pearl.input_per_1m, cheapest.input_per_1m),
            "output": pct_delta(pearl.output_per_1m, cheapest.output_per_1m),
            "blended": pct_delta(
                blended_cost(pearl, args.input_m, args.output_m),
                blended_cost(cheapest, args.input_m, args.output_m),
            ),
        },
        "pearl_vs_together_standard_pct": {
            "input": pct_delta(pearl.input_per_1m, together_std.input_per_1m),
            "output": pct_delta(pearl.output_per_1m, together_std.output_per_1m),
            "blended": pct_delta(
                blended_cost(pearl, args.input_m, args.output_m),
                blended_cost(together_std, args.input_m, args.output_m),
            ),
        },
        "interpretation": (
            "Pearl is ~25% below Together's own 262K list price, but ~2.3× above "
            "OpenRouter's cheapest google/gemma-4-31b-it routes. OpenRouter exposes "
            "Together's 32K $0.28/$0.86 endpoint (Pearl stack) as one provider option — "
            "not a separate pearl-ai model slug."
        ),
    }

    if args.json:
        print(json.dumps(summary, indent=2))
        return

    print("Pearl / Gemma 4 31B pricing comparison\n")
    print(f"{'Source':28} {'Model':32} {'In $/M':>8} {'Out $/M':>8} {'Ctx':>8}  Notes")
    print("-" * 105)
    for row in [agg, cheapest, together_std, pearl]:
        ctx = f"{row.context:,}" if row.context else "?"
        print(
            f"{row.source:28} {row.model_id:32} {row.input_per_1m:8.3f} {row.output_per_1m:8.3f} {ctx:>8}  {row.notes}"
        )

    print(f"\nWorkload: {args.input_m}M input + {args.output_m}M output tokens")
    for k, v in summary["blended_usd"].items():
        print(f"  {k:24} ${v:.3f}")

    p = summary["pearl_vs_cheapest_pct"]
    print(
        f"\nPearl vs OpenRouter cheapest: input {p['input']:+.0f}%, output {p['output']:+.0f}%, blended {p['blended']:+.0f}%"
    )
    p2 = summary["pearl_vs_together_standard_pct"]
    print(
        f"Pearl vs Together 262K list:  input {p2['input']:+.0f}%, output {p2['output']:+.0f}%, blended {p2['blended']:+.0f}%"
    )
    print(f"\n{summary['interpretation']}")


if __name__ == "__main__":
    main()
