#!/usr/bin/env python3
"""Verify the public XRPL P0 audit packet.

This is a static consistency check. It verifies that the article, manifest,
proof log, per-finding wrappers, markers, and local asset links agree.
It does not run rippled.
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import sys
import urllib.parse
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SITE_ROOT = ROOT.parents[2]
ARTICLE = SITE_ROOT / "_posts" / "2026-05-26-xrpl-rippled-open-p0-freeze-audit.md"
MANIFEST = ROOT / "repro_manifest.json"


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(condition: bool, message: str) -> None:
    if not condition:
        fail(message)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError:
        fail(f"missing file: {path}")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def local_asset_links(markdown: str) -> set[str]:
    links: set[str] = set()
    patterns = [
        r"\]\((/assets/[^)#]+)",
        r"href=\"(/assets/[^\"]+)\"",
        r"src=\"(/assets/[^\"]+)\"",
    ]
    for pattern in patterns:
        links.update(re.findall(pattern, markdown))
    return links


def main() -> int:
    manifest = json.loads(read_text(MANIFEST))
    article = read_text(ARTICLE)
    proof = manifest["proof"]
    records = manifest["records"]
    proof_log = ROOT / proof["log"]
    proof_text = read_text(proof_log)

    require(manifest["target"]["repo"] == "XRPLF/rippled", "unexpected target repo")
    require(manifest["target"]["tag"] == "3.1.3", "unexpected target tag")
    require(
        manifest["target"]["commit"] == "46b241ace8b30d9c9775d60ffba7d24b21903896",
        "unexpected target commit",
    )
    require(sha256(proof_log) == proof["sha256"], "proof log SHA-256 mismatch")

    require("RippleD 3.1.3 Audit: Reproduced P0-Class Evidence Packet" in article, "article title mismatch")
    require("## Evidence Packet" in article, "article missing evidence packet section")
    require("## Table Of Contents" in article, "article missing table of contents")
    require("47 cases, 9119 tests total, 0 failures" in proof_text, "proof log missing OpenP0Repro footer")
    require("ripple.tx.OpenP0ReproCrash had 0 failures." in proof_text, "proof log missing crash-control footer")

    anchors = set(re.findall(r'<a id="([^"]+)"></a>', article))
    seen_ids: set[str] = set()
    marker_count = 0

    for record in records:
        rid = record["id"]
        require(rid not in seen_ids, f"duplicate record id: {rid}")
        seen_ids.add(rid)

        script = ROOT / record["script"]
        require(script.exists(), f"missing repro script for {rid}: {script}")
        require(os.access(script, os.X_OK), f"repro script is not executable: {script}")
        script_text = read_text(script)
        require(f'"{rid}"' in script_text, f"repro script does not target {rid}: {script}")

        source = ROOT / record["source"]
        require(source.exists(), f"missing source for {rid}: {source}")
        require(record["anchor"] in anchors, f"missing article anchor for {rid}: {record['anchor']}")
        require(f"### {rid} - " in article, f"missing article section for {rid}")
        require(f"repros/{rid}.sh" in article, f"article missing repro link for {rid}")

        for marker in record["markers"]:
            marker_count += 1
            require(marker in proof_text, f"proof log missing marker for {rid}: {marker}")

    for link in local_asset_links(article):
        path = SITE_ROOT / urllib.parse.unquote(link.lstrip("/"))
        require(path.exists(), f"article links missing asset: {link}")

    print("packet-ok")
    print(f"records={len(records)} markers={marker_count} proof_sha256={proof['sha256']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
