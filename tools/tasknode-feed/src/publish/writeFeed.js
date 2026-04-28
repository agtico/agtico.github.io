import fs from 'node:fs/promises';
import path from 'node:path';

async function writeJson(filePath, payload) {
  await fs.mkdir(path.dirname(filePath), { recursive: true });
  await fs.writeFile(filePath, `${JSON.stringify(payload, null, 2)}\n`, 'utf8');
}

export async function writeFeed(config, payload) {
  if (config.dryRun) {
    process.stdout.write(`${JSON.stringify(payload, null, 2)}\n`);
    return;
  }
  await writeJson(config.outputJson, payload);
  await writeJson(config.outputData, payload);
}
