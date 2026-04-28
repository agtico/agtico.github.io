import fs from 'node:fs/promises';

export async function loadFixtureEvents(config) {
  const raw = await fs.readFile(config.fixturePath, 'utf8');
  const parsed = JSON.parse(raw);
  return Array.isArray(parsed) ? parsed : parsed.items || [];
}
