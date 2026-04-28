# AGTI Task Node Feed Publisher

This utility turns Task Node records from the `pftasks` database into a public, semi-anonymous website feed.

It writes two artifacts:

- `assets/data/tasknode-feed.json`: public JSON endpoint for clients.
- `_data/tasknode_feed.json`: Jekyll data file used by `/task-node-feed/` and the homepage preview.

## Pipeline

1. Read recent Task Node activity rows from the `pftasks-api` Fly app, direct Postgres, or the fixture source for local work.
2. Prefer existing `activity_channel_events` enrichment when present, including anonymized Discord summaries, ticker arrays, and PFTL hashes.
3. Redact direct identifiers, configured sensitive terms, URLs, wallets, emails, phone numbers, UUIDs, and transaction hashes from text.
4. Convert task IDs and actor IDs into stable HMAC aliases such as `task:8a7f2c31` and `node:1b2c3d4e`.
5. Extract ticker tags from activity-channel output, task metadata, board metadata, and explicit `$TICKER` / `#TICKER` mentions.
6. Summarize the task with DeepSeek when an API key is configured, otherwise use the local redacted fallback.
7. Attach PFTL explorer links when activity-channel, task submission, reward, or verification transaction hashes exist.

## Local Commands

Install dependencies:

```bash
npm install
```

Publish the fixture feed:

```bash
npm run publish:fixture
```

Run tests:

```bash
npm test
```

Publish from Fly:

```bash
TASKNODE_FEED_SOURCE=fly \
TASKNODE_FEED_SOURCE_LABEL=fly-postgres \
TASKNODE_FEED_FLY_APP=pftasks-api \
FEED_ANON_PEPPER='stable-secret-pepper' \
npm run publish
```

Publish from a directly reachable Postgres database:

```bash
TASKNODE_FEED_SOURCE=postgres \
TASKNODE_DATABASE_URL='postgres://...' \
FEED_ANON_PEPPER='stable-secret-pepper' \
npm run publish
```

## Configuration

Required for live mode:

- `TASKNODE_FEED_SOURCE=fly` with `flyctl` auth, or `TASKNODE_DATABASE_URL` for a directly reachable `pftasks` Postgres database.
- `FEED_ANON_PEPPER`: stable secret used to generate public HMAC aliases.

Recommended:

- `DEEPSEEK_API_KEY`: enables LLM summaries. Without it, the local redacted fallback is used.
- `TASKNODE_FEED_REDACT_TERMS`: comma-separated names, clients, desks, or private terms that must never appear publicly.
- `PFTL_EXPLORER_BASE`: explorer URL base. Use either `https://example.com/tx/{hash}` or a base URL where the hash can be appended.

Optional:

- `TASKNODE_FEED_SOURCE`: `fly`, `postgres`, `fixture`, or `auto`.
- `TASKNODE_FEED_FLY_APP`: Fly app that can reach the `pftasks` database. Defaults to `pftasks-api`.
- `TASKNODE_FEED_SOURCE_LABEL`: public source label written to the feed artifact. Defaults to the selected source.
- `TASKNODE_FEED_LIMIT`: max items to publish. Defaults to `24`.
- `TASKNODE_FEED_LOOKBACK_DAYS`: recent task window. Defaults to `14`.
- `DEEPSEEK_MODEL`: defaults to `deepseek-v4-flash`.
- `DEEPSEEK_API_BASE`: defaults to `https://api.deepseek.com`.
- `DEEPSEEK_DO_NOT_TRAIN`: defaults to `1`.

The publisher also recognizes migration aliases from the Telegram service: `SPRS_DEEPSEEK_API_KEY`, `SPRS_TELEGRAM_DEEPSEEK_API_KEY`, `SPRS_DEEPSEEK_API_BASE`, `SPRS_TELEGRAM_DEEPSEEK_API_BASE`, and `SPRS_TELEGRAM_CONTEXT_SUMMARY_MODEL`.

## GitHub Pages Operation

The `Publish Task Node Feed` workflow runs every 30 minutes when repository variable `TASKNODE_FEED_ENABLED` is set to `true`. It installs this package, runs tests, publishes the feed files, and commits only the generated JSON artifacts when they changed.

Set these repository values before enabling the schedule:

- Secrets: `FLY_API_TOKEN`, `FEED_ANON_PEPPER`, and optionally `DEEPSEEK_API_KEY`, `SPRS_DEEPSEEK_API_KEY`, `SPRS_TELEGRAM_DEEPSEEK_API_KEY`, `TASKNODE_FEED_REDACT_TERMS`.
- Variables: `TASKNODE_FEED_ENABLED=true`, optionally `TASKNODE_FEED_FLY_APP`, `PFTL_EXPLORER_BASE`, `TASKNODE_FEED_LIMIT`, `TASKNODE_FEED_LOOKBACK_DAYS`, `DEEPSEEK_MODEL`.

The workflow does not run on every push, so feed bot commits do not create a publish loop. GitHub Pages will rebuild the site from the updated JSON files after the commit lands.
