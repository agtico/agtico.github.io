# DGS10 daily snapshot

## What this measures

DGS10 is the daily U.S. Treasury **10-year constant-maturity rate**, quoted as an annual percentage. FRED publishes the series and its observations. This snapshot procedure records exactly which CSV was retrieved so a reviewer can reproduce the input used by a research run.

- Series page: <https://fred.stlouisfed.org/series/DGS10>
- Public CSV source: <https://fred.stlouisfed.org/graph/fredgraph.csv?id=DGS10>
- Access: public; no login, API key, or paid account is required.

## Reproducible pull

Run these commands in an empty working directory:

```sh
curl --fail --location --silent --show-error "https://fred.stlouisfed.org/graph/fredgraph.csv?id=DGS10" --output dgs10.csv
date -u
sha256sum dgs10.csv
wc -l dgs10.csv
```

The command must exit successfully. The resulting file must be nonempty, and its first line must contain the `DGS10` column (currently `observation_date,DGS10`).

## Required deliverables for each snapshot

Record all of the following with the research artifact:

1. The retrieved `dgs10.csv` file.
2. The UTC retrieval time printed by `date -u`.
3. The SHA-256 checksum printed by `sha256sum dgs10.csv`.
4. The line count printed by `wc -l dgs10.csv`.
5. The exact commands above, including the source URL.

FRED may revise historical observations or append new ones. A checksum or line count obtained later can therefore differ from an earlier valid snapshot; the file and recorded retrieval time are the authoritative provenance for that run.

## Observed validation

The procedure above was executed on **Sat Nov 14 16:15:23 UTC 2026**.

| Check | Observed result |
| --- | --- |
| File nonempty | pass |
| CSV header | `observation_date,DGS10` |
| SHA-256 | `ef864cd0a56c983f248fb34b46fc6f4e2b35bcee54b10cb5d0332ebfaea99d5b` |
| Line count | `16924` |
