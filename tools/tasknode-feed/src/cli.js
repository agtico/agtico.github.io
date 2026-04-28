#!/usr/bin/env node
import { loadConfig } from './config.js';
import { buildFeed } from './pipeline.js';
import { writeFeed } from './publish/writeFeed.js';

async function main() {
  const command = process.argv[2] || 'publish';
  if (command !== 'publish') {
    throw new Error(`Unknown command: ${command}`);
  }

  const config = loadConfig();
  const feed = await buildFeed(config);
  await writeFeed(config, feed);
  process.stdout.write(`tasknode-feed: wrote ${feed.items.length} item(s) from ${config.source}\n`);
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
