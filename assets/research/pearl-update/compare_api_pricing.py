"""Pearl Inference Platform list prices vs the cheapest live OpenRouter provider for the same models.

Pearl's prices are copied from https://pearlresearch.ai/pricing (observed 2026-10-12; the page is undated). OpenRouter's
are fetched live from its public API. "Qwen3.8-Flash-Next" has no exact OpenRouter id; qwen/qwen3.8-flash is the
nearest listing and is marked as approximate.

    python compare_api_pricing.py   -> data/api_pricing.csv, charts/api_pricing.png
"""
import csv
import json
import urllib.request
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

from economics import CHARTS, DATA, GREEN, MUTED, ORANGE, style  # noqa: E402

PEARL = [  # (label, OpenRouter id, Pearl input $/1M, Pearl output $/1M, exact match)
    ("Gemma 4 31B", "google/gemma-4-31b-it", 0.09, 0.34, True),
    ("Qwen3.8 Flash-Next*", "qwen/qwen3.8-flash", 0.14, 0.45, False),
    ("DeepSeek V4.1 Flash", "deepseek/deepseek-v4.1-flash", 0.14, 0.42, True),
    ("GLM-5.3 Flash", "z-ai/glm-5.3-flash", 0.07, 0.24, True),
    ("Qwen3.8 27B", "qwen/qwen3.8-27b", 0.20, 2.00, True),
    ("DeepSeek V4 Pro 0813", "deepseek/deepseek-v4-pro-0813", 1.10, 3.30, True),
    ("GLM-5.3", "z-ai/glm-5.3", 1.10, 3.50, True),
]
BLEND_OUT = 0.25          # blended price = 75% input + 25% output tokens, a typical chat mix


def cheapest(model_id):
    url = f"https://openrouter.ai/api/v1/models/{model_id}/endpoints"
    eps = json.load(urllib.request.urlopen(url, timeout=30))["data"]["endpoints"]
    eps = [(e["provider_name"], float(e["pricing"]["prompt"]) * 1e6, float(e["pricing"]["completion"]) * 1e6)
           for e in eps if float(e["pricing"]["prompt"]) > 0]
    return min(eps, key=lambda e: (1 - BLEND_OUT) * e[1] + BLEND_OUT * e[2]), len(eps)


def main():
    style()
    rows = []
    for label, oid, pin, pout, exact in PEARL:
        (prov, oin, oout), n = cheapest(oid)
        blend = lambda i, o: (1 - BLEND_OUT) * i + BLEND_OUT * o  # noqa: E731
        rows.append({"model": label, "openrouter_id": oid, "pearl_in": pin, "pearl_out": pout, "or_provider": prov,
                     "or_in": round(oin, 4), "or_out": round(oout, 4), "or_providers": n, "exact_match": exact,
                     "pearl_vs_cheapest_blended": round(blend(pin, pout) / blend(oin, oout), 2)})
    with (DATA / "api_pricing.csv").open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    fig, ax = plt.subplots(figsize=(10, 4.8))
    ys = range(len(rows))
    ratios = [r["pearl_vs_cheapest_blended"] for r in rows]
    ax.barh(list(ys), ratios, color=[GREEN if v <= 1.02 else ORANGE for v in ratios], height=0.6)
    ax.axvline(1, color=MUTED, ls="--", lw=1)
    for y, r in zip(ys, rows):
        ax.text(r["pearl_vs_cheapest_blended"] + 0.05, y, f"{r['pearl_vs_cheapest_blended']:.2f}x  (vs {r['or_provider']})",
                va="center", fontsize=9)
    ax.set_yticks(list(ys))
    ax.set_yticklabels([r["model"] for r in rows])
    ax.invert_yaxis()
    ax.set_xlabel("Pearl list price / cheapest OpenRouter provider (75/25 input/output blend)")
    ax.set_xlim(0, max(ratios) * 1.45)
    ax.set_title("Pearl Inference Platform vs OpenRouter's cheapest provider")
    ax.grid(True, axis="x", alpha=0.6)
    fig.text(0.01, 0.01, "* nearest OpenRouter listing (qwen3.8-flash); Pearl prices from pearlresearch.ai/pricing, 2026-10-12",
             fontsize=8, color=MUTED)
    fig.tight_layout(rect=(0, 0.03, 1, 1))
    fig.savefig(CHARTS / "api_pricing.png")
    plt.close(fig)
    for r in rows:
        print(r["model"], r["pearl_vs_cheapest_blended"], r["or_provider"], r["or_in"], r["or_out"])


if __name__ == "__main__":
    main()
