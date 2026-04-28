import fs from 'node:fs/promises';
import path from 'node:path';

function fallbackSummary(event) {
  const body = String(event.body || event.raw_context || '')
    .replace(/\b(client|customer|company|organization|org|account|contact):\s*\[redacted-[^\]]+\]\.?\s*/gi, '')
    .replace(/\[redacted(?:-[^\]]+)?\]/g, 'private reference')
    .replace(/\s+/g, ' ')
    .trim();
  const cleanedBody = body
    .split(/(?<=[.!?])\s+/)
    .filter((sentence) => !/private reference/i.test(sentence))
    .join(' ')
    .trim() || body.replace(/private reference/gi, 'redacted context');
  if (!cleanedBody) {
    return 'Task Node published an anonymized network update.';
  }
  return cleanedBody.length <= 220 ? cleanedBody : `${cleanedBody.slice(0, 217).trim()}...`;
}

function parseSummaryPayload(text) {
  const raw = String(text || '').trim();
  if (!raw) {
    return null;
  }
  try {
    const parsed = JSON.parse(raw);
    return {
      title: typeof parsed.title === 'string' ? parsed.title.trim() : '',
      summary: typeof parsed.summary === 'string' ? parsed.summary.trim() : '',
      category: typeof parsed.category === 'string' ? parsed.category.trim() : '',
      tags: Array.isArray(parsed.tags)
        ? parsed.tags.map((tag) => String(tag || '').trim()).filter(Boolean).slice(0, 6)
        : [],
    };
  } catch {
    return {
      title: '',
      summary: raw,
      category: '',
      tags: [],
    };
  }
}

export async function summarizeEvent(event, config) {
  if (!config.deepseekEnabled || !config.deepseekApiKey) {
    return {
      title: event.title,
      summary: fallbackSummary(event),
      category: event.department,
      tags: [],
      model: 'local-redacted-fallback',
    };
  }

  const promptPath = path.join(config.toolRoot, 'prompts', 'summarize-public-update.md');
  const systemPrompt = await fs.readFile(promptPath, 'utf8');
  const payload = {
    model: config.deepseekModel,
    messages: [
      { role: 'system', content: systemPrompt },
      { role: 'user', content: JSON.stringify({
        title: event.title,
        body: event.body,
        context: event.raw_context,
        department: event.department,
        status: event.status,
      }) },
    ],
    temperature: 0.2,
    max_tokens: 280,
    response_format: { type: 'json_object' },
  };

  const response = await fetch(`${config.deepseekApiBase.replace(/\/+$/, '')}/chat/completions`, {
    method: 'POST',
    headers: {
      Authorization: `Bearer ${config.deepseekApiKey}`,
      'Content-Type': 'application/json',
      ...(config.deepseekDoNotTrain ? { do_not_train: '1', opt_out: 'training' } : {}),
    },
    body: JSON.stringify(payload),
  });

  if (!response.ok) {
    const detail = await response.text().catch(() => '');
    throw new Error(`DeepSeek request failed: ${response.status} ${detail.slice(0, 200)}`);
  }

  const parsed = await response.json();
  const text = parsed?.choices?.[0]?.message?.content || '';
  const summary = parseSummaryPayload(text) || {};

  return {
    title: summary.title || event.title,
    summary: summary.summary || fallbackSummary(event),
    category: summary.category || event.department,
    tags: summary.tags || [],
    model: parsed?.model || config.deepseekModel,
  };
}
