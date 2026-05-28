#!/usr/bin/env python3
"""Fetch the official OFAC SDN advanced XML and extract XRP addresses."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
import urllib.request
import zipfile
from io import BytesIO
from datetime import datetime, timezone


DEFAULT_URL = "https://sanctionslistservice.ofac.treas.gov/api/download/sdn_advanced.zip"
XRP_LABEL = "Digital Currency Address - XRP"
XRP_ADDRESS_RE = re.compile(r"\br[1-9A-HJ-NP-Za-km-z]{25,34}\b")


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def sha256_json(value: object) -> str:
    payload = json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(payload).hexdigest()


def extract_xrp_addresses(xml_text: str) -> list[str]:
    addresses: set[str] = set()
    type_ids = re.findall(
        r'<FeatureType\s+ID="([^"]+)"[^>]*>\s*'
        + re.escape(XRP_LABEL)
        + r"\s*</FeatureType>",
        xml_text,
        flags=re.IGNORECASE,
    )
    for type_id in type_ids:
        feature_re = re.compile(
            r'<Feature\s+ID="[^"]+"\s+FeatureTypeID="' + re.escape(type_id) + r'">.*?</Feature>',
            flags=re.IGNORECASE | re.DOTALL,
        )
        for feature in feature_re.findall(xml_text):
            addresses.update(XRP_ADDRESS_RE.findall(feature))

    for match in re.finditer(re.escape(XRP_LABEL), xml_text, re.IGNORECASE):
        window = xml_text[match.start() : match.end() + 800]
        addresses.update(XRP_ADDRESS_RE.findall(window))
    return sorted(addresses)


def extract_xml(download: bytes, url: str) -> tuple[bytes, str]:
    if url.lower().endswith(".zip") or download.startswith(b"PK\x03\x04"):
        with zipfile.ZipFile(BytesIO(download)) as archive:
            names = [name for name in archive.namelist() if name.lower().endswith(".xml")]
            if not names:
                raise ValueError("downloaded ZIP did not contain an XML file")
            name = sorted(names)[0]
            return archive.read(name), name
    return download, url.rsplit("/", 1)[-1]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", default=DEFAULT_URL)
    parser.add_argument("--known-address", default="rnXyVQzgxZe7TR1EPzTkGj2jxH4LMJYh66")
    args = parser.parse_args()

    request = urllib.request.Request(args.url, headers={"User-Agent": "postfiat-xrpl-audit/1"})
    with urllib.request.urlopen(request, timeout=90) as handle:
        download = handle.read()

    xml, xml_member = extract_xml(download, args.url)
    xml_text = xml.decode("utf-8", errors="replace")
    xrp_addresses = extract_xrp_addresses(xml_text)
    subset = {
        "label": XRP_LABEL,
        "xrp_addresses": xrp_addresses,
    }
    snapshot = {
        "source": "official OFAC SDN advanced XML",
        "url": args.url,
        "retrieved_utc": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "download_sha256": sha256_bytes(download),
        "download_size_bytes": len(download),
        "xml_member": xml_member,
        "xml_sha256": sha256_bytes(xml),
        "xml_size_bytes": len(xml),
        "parser": "FeatureTypeID + label-window regex parser v2",
        "xrp_label": XRP_LABEL,
        "xrp_addresses": xrp_addresses,
        "xrp_address_count": len(xrp_addresses),
        "known_user_signal": args.known_address,
        "known_user_signal_present": args.known_address in xrp_addresses,
        "xrp_subset_sha256": sha256_json(subset),
    }
    snapshot["snapshot_sha256"] = sha256_json(snapshot)
    json.dump(snapshot, sys.stdout, indent=2, sort_keys=True)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
