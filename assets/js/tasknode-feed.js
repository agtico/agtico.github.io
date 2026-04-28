(() => {
  const DEFAULT_FEED_URL = 'https://pftasks-api.fly.dev/activity/public-feed?limit=24';
  const POLL_INTERVAL_MS = 30000;
  const feedUrl = window.AGTI_TASK_FEED_URL || DEFAULT_FEED_URL;
  const previewRoot = document.querySelector('.feed-preview');
  const listRoot = document.querySelector('.feed-list');
  const statusRoot = document.querySelector('.feed-status');

  if (!previewRoot && !listRoot) {
    return;
  }

  function normalizeText(value, fallback = '') {
    const text = String(value || '').replace(/\s+/g, ' ').trim();
    return text || fallback;
  }

  function formatEventType(value) {
    return normalizeText(value, 'task activity').replace(/_/g, ' ');
  }

  function truncate(value, maxChars) {
    const text = normalizeText(value);
    if (!text || text.length <= maxChars) {
      return text;
    }
    return `${text.slice(0, Math.max(maxChars - 3, 1)).trimEnd()}...`;
  }

  function formatUtc(value, { includeYear = false } = {}) {
    const date = new Date(value);
    if (Number.isNaN(date.getTime())) {
      return 'UTC';
    }
    return new Intl.DateTimeFormat('en-US', {
      month: 'short',
      day: 'numeric',
      year: includeYear ? 'numeric' : undefined,
      hour: '2-digit',
      minute: '2-digit',
      hour12: false,
      timeZone: 'UTC',
      timeZoneName: 'short',
    }).format(date);
  }

  function createTextElement(tagName, className, text) {
    const element = document.createElement(tagName);
    if (className) {
      element.className = className;
    }
    element.textContent = text;
    return element;
  }

  function createTimeElement(item, includeYear) {
    const time = createTextElement('time', includeYear ? 'feed-time' : '', formatUtc(item.timestamp, { includeYear }));
    if (item.timestamp) {
      time.dateTime = item.timestamp;
    }
    return time;
  }

  function validLinks(item) {
    if (!Array.isArray(item.links)) {
      return [];
    }
    return item.links.filter((link) => {
      try {
        const url = new URL(link.url, window.location.origin);
        return url.protocol === 'https:' || url.protocol === 'http:';
      } catch {
        return false;
      }
    });
  }

  function createLink(link, className) {
    const anchor = document.createElement('a');
    anchor.className = className;
    anchor.href = link.url;
    anchor.target = '_blank';
    anchor.rel = 'noopener';
    anchor.textContent = `${normalizeText(link.label, 'PFTL proof')} `;
    const marker = document.createElement('span');
    marker.textContent = '›';
    anchor.appendChild(marker);
    return anchor;
  }

  function createTickerChip(ticker, className) {
    return createTextElement('span', className, `$${normalizeText(ticker)}`);
  }

  function renderPreviewItem(item) {
    const article = document.createElement('article');
    article.className = 'feed-preview-item';

    article.appendChild(createTextElement('div', 'feed-preview-meta', normalizeText(item.category, formatEventType(item.type))));
    article.appendChild(createTimeElement(item, false));
    article.appendChild(createTextElement('h3', '', normalizeText(item.title, 'Task Node update')));
    article.appendChild(createTextElement('p', '', truncate(item.summary, 150)));

    const tickers = Array.isArray(item.tickers) ? item.tickers.slice(0, 4) : [];
    if (tickers.length) {
      const tags = document.createElement('div');
      tags.className = 'feed-preview-tags';
      tickers.forEach((ticker) => tags.appendChild(createTickerChip(ticker, '')));
      article.appendChild(tags);
    }

    const links = validLinks(item).slice(0, 2);
    if (links.length) {
      const linkRow = document.createElement('div');
      linkRow.className = 'feed-preview-links';
      links.forEach((link) => linkRow.appendChild(createLink(link, 'feed-preview-link')));
      article.appendChild(linkRow);
    }

    return article;
  }

  function renderFeedItem(item) {
    const article = document.createElement('article');
    article.className = 'feed-item';

    const left = document.createElement('div');
    left.appendChild(createTimeElement(item, true));
    left.appendChild(createTextElement('span', 'feed-actor', normalizeText(item.actor, 'node')));

    const right = document.createElement('div');
    right.appendChild(createTextElement('div', 'feed-category', normalizeText(item.category, formatEventType(item.type))));
    right.appendChild(createTextElement('h2', '', normalizeText(item.title, 'Task Node update')));
    right.appendChild(createTextElement('p', '', normalizeText(item.summary)));

    const meta = document.createElement('div');
    meta.className = 'feed-meta';
    const tickers = Array.isArray(item.tickers) ? item.tickers : [];
    tickers.forEach((ticker) => meta.appendChild(createTickerChip(ticker, 'ticker-chip')));
    meta.appendChild(createTextElement('span', 'feed-chip', normalizeText(item.status, 'posted')));
    meta.appendChild(createTextElement('span', 'feed-chip', formatEventType(item.type)));
    right.appendChild(meta);

    const links = validLinks(item);
    if (links.length) {
      const linkRow = document.createElement('div');
      linkRow.className = 'feed-links';
      links.forEach((link) => linkRow.appendChild(createLink(link, 'feed-link')));
      right.appendChild(linkRow);
    }

    article.appendChild(left);
    article.appendChild(right);
    return article;
  }

  function replaceChildren(root, children) {
    if (!root) {
      return;
    }
    root.replaceChildren(...children);
  }

  function updateStatus(feed, itemCount) {
    if (!statusRoot) {
      return;
    }
    const count = statusRoot.querySelector('strong');
    const detail = statusRoot.querySelector('p');
    if (count) {
      count.textContent = String(itemCount);
    }
    if (detail) {
      const generatedAt = feed.generated_at ? formatUtc(feed.generated_at, { includeYear: true }) : 'UTC';
      detail.textContent = `Updated ${generatedAt} from ${normalizeText(feed.model, 'activity-channel')}.`;
    }
  }

  function renderFeed(feed) {
    const items = Array.isArray(feed?.items) ? feed.items : [];
    if (!items.length) {
      return;
    }
    replaceChildren(previewRoot, items.slice(0, 3).map(renderPreviewItem));
    replaceChildren(listRoot, items.map(renderFeedItem));
    updateStatus(feed, items.length);
  }

  async function refreshFeed() {
    try {
      const response = await fetch(feedUrl, {
        cache: 'no-store',
        headers: { Accept: 'application/json' },
      });
      if (!response.ok) {
        return;
      }
      renderFeed(await response.json());
    } catch {
      // Keep the statically rendered GitHub Pages feed as the fallback.
    }
  }

  refreshFeed();
  window.setInterval(refreshFeed, POLL_INTERVAL_MS);
})();
