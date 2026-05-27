#!/usr/bin/env python3
"""Build 12-month rolling sum of rippled stable semver releases (x.y.z tags)."""

from __future__ import annotations

import json
import re
import subprocess
import urllib.request
from collections import defaultdict
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DATA_PATH = ROOT / "data" / "rippled_stable_releases.json"
SVG_PATH = ROOT / "rippled_release_rolling_12m.svg"
REPO = Path("/home/pfrpc/repos/rippled")
SEMVER = re.compile(r"^\d+\.\d+\.\d+$")


def git_tag_dates() -> list[tuple[str, datetime]]:
    if not REPO.is_dir():
        return []
    tags = subprocess.check_output(["git", "tag", "-l"], cwd=REPO, text=True).splitlines()
    stable = sorted(
        [t.strip() for t in tags if SEMVER.match(t.strip())],
        key=lambda s: tuple(map(int, s.split("."))),
    )
    out: list[tuple[str, datetime]] = []
    for tag in stable:
        raw = subprocess.check_output(
            ["git", "log", "-1", "--format=%ci", tag], cwd=REPO, text=True
        ).strip()[:10]
        out.append((tag, datetime.strptime(raw, "%Y-%m-%d")))
    return out


def github_release_dates() -> list[tuple[str, datetime]]:
    url = "https://api.github.com/repos/XRPLF/rippled/releases?per_page=100"
    req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "agti-research"})
    with urllib.request.urlopen(req, timeout=60) as resp:
        releases = json.load(resp)
    out: list[tuple[str, datetime]] = []
    for rel in releases:
        tag = rel.get("tag_name", "")
        published = rel.get("published_at")
        if SEMVER.match(tag) and published:
            out.append((tag, datetime.strptime(published[:10], "%Y-%m-%d")))
    return out


def merge_release_lists(*lists: list[tuple[str, datetime]]) -> list[tuple[str, datetime]]:
    by_tag: dict[str, datetime] = {}
    for items in lists:
        for tag, dt in items:
            by_tag[tag] = dt
    return sorted(by_tag.items(), key=lambda x: (x[1], tuple(map(int, x[0].split(".")))))


def month_range(start: datetime, end: datetime) -> list[tuple[int, int]]:
    months: list[tuple[int, int]] = []
    y, m = start.year, start.month
    while (y, m) <= (end.year, end.month):
        months.append((y, m))
        m += 1
        if m > 12:
            m = 1
            y += 1
    return months


def build_series(releases: list[tuple[str, datetime]]):
    monthly: dict[tuple[int, int], int] = defaultdict(int)
    for _, dt in releases:
        monthly[(dt.year, dt.month)] += 1

    start = min(dt for _, dt in releases)
    end = max(dt for _, dt in releases)
    months = month_range(start, end)
    counts = [monthly.get(mo, 0) for mo in months]
    rolling = [sum(counts[max(0, i - 11) : i + 1]) for i in range(len(counts))]
    labels = [f"{y}-{m:02d}" for y, m in months]
    return months, labels, counts, rolling


def render_svg(labels: list[str], rolling: list[int], releases: list[tuple[str, datetime]]) -> None:
    start_idx = 11
    dates = [datetime.strptime(l, "%Y-%m") for l in labels]
    xs = dates[start_idx:]
    ys = rolling[start_idx:]
    if not xs:
        raise ValueError("Not enough months for rolling window")

    width, height, pad_l, pad_r, pad_t, pad_b = 980, 420, 68, 28, 72, 54
    plot_w = width - pad_l - pad_r
    plot_h = height - pad_t - pad_b

    xmin = xs[0].timestamp()
    xmax = xs[-1].timestamp()
    ymax = max(ys) * 1.12 or 1

    def xpos(dt: datetime) -> float:
        if xmax == xmin:
            return pad_l
        return pad_l + (dt.timestamp() - xmin) / (xmax - xmin) * plot_w

    def ypos(v: float) -> float:
        return pad_t + plot_h - (v / ymax) * plot_h

    points = " ".join(f"{xpos(d):.1f},{ypos(v):.1f}" for d, v in zip(xs, ys))
    area = (
        f"M {xpos(xs[0]):.1f},{ypos(0):.1f} L "
        + " L ".join(f"{xpos(d):.1f},{ypos(v):.1f}" for d, v in zip(xs, ys))
        + f" L {xpos(xs[-1]):.1f},{ypos(0):.1f} Z"
    )

    y_ticks = list(range(0, int(ymax) + 1, max(1, int(ymax // 6) or 1)))
    grid_lines = []
    for tick in y_ticks:
        y = ypos(tick)
        grid_lines.append(
            f'<line x1="{pad_l}" y1="{y:.1f}" x2="{width-pad_r}" y2="{y:.1f}" stroke="#243028" stroke-width="1"/>'
        )
        grid_lines.append(
            f'<text x="{pad_l-10}" y="{y+4:.1f}" text-anchor="end" fill="#9aa2a0" font-size="11" font-family="Inter, sans-serif">{tick}</text>'
        )

    year_marks = []
    year = xs[0].year
    while year <= xs[-1].year:
        dt = datetime(year, 1, 1)
        if dt >= xs[0]:
            x = xpos(dt)
            year_marks.append(
                f'<line x1="{x:.1f}" y1="{pad_t}" x2="{x:.1f}" y2="{height-pad_b}" stroke="#1a2420" stroke-width="1"/>'
            )
            year_marks.append(
                f'<text x="{x:.1f}" y="{height-18}" text-anchor="middle" fill="#9aa2a0" font-size="11" font-family="Inter, sans-serif">{year}</text>'
            )
        year += 1

    latest = ys[-1]
    lx, ly = xpos(xs[-1]), ypos(latest)
    svg = f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" role="img" aria-labelledby="title desc">
  <title id="title">XRPL rippled stable releases, 12-month rolling sum</title>
  <desc id="desc">Trailing twelve-month count of stable semver rippled releases tagged in XRPLF/rippled since {releases[0][1].date()}.</desc>
  <rect width="100%" height="100%" fill="#070b0a"/>
  <text x="{pad_l}" y="28" fill="#e8eeeb" font-size="18" font-weight="700" font-family="Inter, sans-serif">XRPL rippled stable releases · 12-month rolling count</text>
  <text x="{pad_l}" y="48" fill="#9aa2a0" font-size="12" font-family="Inter, sans-serif">Source: XRPLF/rippled semver tags (x.y.z) · {len(releases)} releases since {releases[0][1].date()} · latest rolling sum {latest}</text>
  {''.join(grid_lines)}
  {''.join(year_marks)}
  <path d="{area}" fill="#6ee58f" fill-opacity="0.16" stroke="none"/>
  <polyline points="{points}" fill="none" stroke="#6ee58f" stroke-width="2.5" stroke-linejoin="round" stroke-linecap="round"/>
  <circle cx="{lx:.1f}" cy="{ly:.1f}" r="4" fill="#ff9f45"/>
  <text x="{lx-8:.1f}" y="{ly-12:.1f}" text-anchor="end" fill="#ff9f45" font-size="12" font-family="Inter, sans-serif">{latest}</text>
  <text x="{pad_l}" y="{height-4}" fill="#c8d0cc" font-size="12" font-family="Inter, sans-serif">Month (year ticks)</text>
  <text transform="translate(16 {pad_t + plot_h/2:.0f}) rotate(-90)" fill="#c8d0cc" font-size="12" font-family="Inter, sans-serif">Releases in trailing 12 months</text>
</svg>
"""
    SVG_PATH.parent.mkdir(parents=True, exist_ok=True)
    SVG_PATH.write_text(svg, encoding="utf-8")


def main() -> None:
    releases = merge_release_lists(git_tag_dates(), github_release_dates())
    if not releases:
        raise SystemExit("No release data found")

    months, labels, counts, rolling = build_series(releases)
    payload = {
        "method": "Count stable semver rippled releases (x.y.z) tagged in XRPLF/rippled",
        "chain_inception_note": "XRP Ledger mainnet began 2013; earliest tagged stable release in repo is 2013-08-26 (0.12.0).",
        "release_count": len(releases),
        "releases": [{"tag": t, "date": d.strftime("%Y-%m-%d")} for t, d in releases],
        "monthly_counts": {labels[i]: counts[i] for i in range(len(labels))},
        "rolling_12m": {labels[i]: rolling[i] for i in range(len(labels))},
    }
    DATA_PATH.parent.mkdir(parents=True, exist_ok=True)
    DATA_PATH.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    render_svg(labels, rolling, releases)
    print(f"Wrote {DATA_PATH}")
    print(f"Wrote {SVG_PATH}")


if __name__ == "__main__":
    main()
