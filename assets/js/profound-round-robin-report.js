(function () {
  'use strict';

  var cfg = window.AGTI_PROFOUND_REPORT || {};
  var REPORT_ID = cfg.reportId || 'profound-round-robin-ai-sotp-2026-05-02';
  var TASKNODE_ORIGIN = cfg.tasknodeOrigin || 'https://tasknode.postfiat.org';
  var API_BASE = TASKNODE_ORIGIN + '/api';
  var CONTENT_URL = cfg.contentUrl || '/gated-content/reports/profound-round-robin-ai-sotp-2026-05-02.encrypted.v1.json';
  var TOKEN_KEY = 'agti:gated-report:viewer-token:' + REPORT_ID;
  var STATE_KEY = 'agti:gated-report:state:' + REPORT_ID;

  var panelEl = document.getElementById('prr-lock-panel');
  var statusEl = document.getElementById('prr-lock-status');
  var unlockEl = document.getElementById('prr-unlock');
  var bodyEl = document.getElementById('prr-report-body');
  var tooltipEl = document.getElementById('prr-tooltip');

  var tableState = {
    model: { key: 'rank', dir: 1 },
    stock: { key: 'market_cap_usd', dir: -1 },
  };

  function setStatus(message, isError) {
    if (!statusEl) return;
    statusEl.textContent = message;
    statusEl.classList.toggle('error', Boolean(isError));
  }

  function escapeHtml(value) {
    return String(value == null ? '' : value).replace(/[&<>"']/g, function (char) {
      return ({
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#39;',
      })[char];
    });
  }

  function formatUsd(value, opts) {
    if (value == null || !Number.isFinite(Number(value))) return 'n/a';
    var abs = Math.abs(Number(value));
    var decimals = opts && opts.decimals != null ? opts.decimals : 1;
    if (abs >= 1e12) return '$' + (Number(value) / 1e12).toFixed(decimals) + 'T';
    if (abs >= 1e9) return '$' + (Number(value) / 1e9).toFixed(decimals) + 'B';
    if (abs >= 1e6) return '$' + (Number(value) / 1e6).toFixed(decimals) + 'M';
    if (abs >= 1e3) return '$' + (Number(value) / 1e3).toFixed(decimals) + 'K';
    return '$' + Number(value).toFixed(2);
  }

  function formatCost(value) {
    if (value == null || !Number.isFinite(Number(value))) return 'n/a';
    var number = Number(value);
    var abs = Math.abs(number);
    if (abs === 0) return '$0';
    if (abs < 0.0001) return '<$0.0001';
    if (abs < 0.01) return '$' + number.toFixed(6).replace(/0+$/, '').replace(/\.$/, '');
    if (abs < 1) return '$' + number.toFixed(4);
    return formatUsd(number, { decimals: 2 });
  }

  function formatPct(value, decimals) {
    if (value == null || !Number.isFinite(Number(value))) return 'n/a';
    return Number(value).toFixed(decimals == null ? 1 : decimals) + '%';
  }

  function formatNum(value, decimals) {
    if (value == null || !Number.isFinite(Number(value))) return 'n/a';
    return Number(value).toLocaleString(undefined, {
      maximumFractionDigits: decimals == null ? 2 : decimals,
      minimumFractionDigits: decimals == null ? 0 : decimals,
    });
  }

  function formatKrwTrn(value) {
    if (value == null || !Number.isFinite(Number(value))) return 'n/a';
    return 'KRW ' + Number(value).toFixed(2) + 'T';
  }

  function shortModel(modelId) {
    var value = String(modelId || '');
    if (value.length <= 34) return value;
    var parts = value.split('/');
    var tail = parts[1] || value;
    return (parts[0] ? parts[0] + '/' : '') + tail.slice(0, 24) + '...';
  }

  function randomState() {
    var bytes = new Uint8Array(16);
    window.crypto.getRandomValues(bytes);
    return Array.from(bytes).map(function (byte) {
      return byte.toString(16).padStart(2, '0');
    }).join('');
  }

  function cleanReportUrl() {
    var url = new URL(window.location.href);
    ['index_viewer_code', 'index_id', 'state'].forEach(function (key) {
      url.searchParams.delete(key);
    });
    return url;
  }

  function startAuthorization() {
    var state = randomState();
    window.sessionStorage.setItem(STATE_KEY, state);
    var authUrl = new URL(TASKNODE_ORIGIN + '/index-auth/start');
    authUrl.searchParams.set('index_id', REPORT_ID);
    authUrl.searchParams.set('return_to', cleanReportUrl().toString());
    authUrl.searchParams.set('state', state);
    window.location.assign(authUrl.toString());
  }

  function waitForSodium() {
    if (!window.sodium || !window.sodium.ready) {
      return Promise.reject(new Error('Decrypt runtime unavailable.'));
    }
    return window.sodium.ready.then(function () {
      return window.sodium;
    });
  }

  function exchangeCodeIfPresent() {
    var url = new URL(window.location.href);
    var code = url.searchParams.get('index_viewer_code');
    if (!code) {
      return Promise.resolve(window.sessionStorage.getItem(TOKEN_KEY) || '');
    }
    var returnedState = url.searchParams.get('state') || '';
    var expectedState = window.sessionStorage.getItem(STATE_KEY) || '';
    if (expectedState && returnedState !== expectedState) {
      return Promise.reject(new Error('Authorization state mismatch.'));
    }
    setStatus('Finishing Task Node authorization...');
    return fetch(API_BASE + '/index-viewer/exchange', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        code: code,
        index_id: REPORT_ID,
        origin: window.location.origin,
      }),
    }).then(function (response) {
      if (!response.ok) {
        throw new Error('Task Node token exchange failed.');
      }
      return response.json();
    }).then(function (payload) {
      window.sessionStorage.setItem(TOKEN_KEY, payload.viewer_token);
      window.history.replaceState({}, document.title, cleanReportUrl().toString());
      return payload.viewer_token;
    });
  }

  function unlockReport(token) {
    if (!token) {
      setStatus('This report is available to Task Node users with an active wallet.');
      return Promise.resolve(false);
    }
    setStatus('Unlocking encrypted report...');
    return Promise.all([
      waitForSodium(),
      fetch(API_BASE + '/index-viewer/indices/' + encodeURIComponent(REPORT_ID) + '/unlock', {
        method: 'POST',
        headers: {
          Authorization: 'Bearer ' + token,
        },
      }).then(function (response) {
        if (!response.ok) {
          if (response.status === 401 || response.status === 403) {
            window.sessionStorage.removeItem(TOKEN_KEY);
          }
          return response.json().catch(function () {
            return {};
          }).then(function (payload) {
            throw new Error(payload.error || 'Task Node unlock failed.');
          });
        }
        return response.json();
      }),
      fetch(CONTENT_URL).then(function (response) {
        if (!response.ok) throw new Error('Encrypted report fetch failed.');
        return response.json();
      }),
    ]).then(function (results) {
      var sodium = results[0];
      var unlock = results[1];
      var blob = results[2];
      var fromBase64 = function (value) {
        return sodium.from_base64(value, sodium.base64_variants.ORIGINAL);
      };
      var plainBytes = sodium.crypto_aead_xchacha20poly1305_ietf_decrypt(
        null,
        fromBase64(blob.ciphertext),
        null,
        fromBase64(blob.nonce),
        fromBase64(unlock.content_key)
      );
      var payload = JSON.parse(new TextDecoder().decode(plainBytes));
      renderReport(payload);
      panelEl.hidden = true;
      bodyEl.hidden = false;
      return true;
    });
  }

  function metric(label, value, detail) {
    return (
      '<div class="prr-metric">' +
        '<span class="prr-label">' + escapeHtml(label) + '</span>' +
        '<strong>' + escapeHtml(value) + '</strong>' +
        '<span>' + escapeHtml(detail || '') + '</span>' +
      '</div>'
    );
  }

  function renderTickerTape(payload) {
    var market = payload.market || {};
    var stocksByTicker = {};
    (market.stocks || []).forEach(function (stock) {
      stocksByTicker[stock.ticker] = stock;
    });
    return (payload.ticker_focus || []).map(function (ticker) {
      var stock = stocksByTicker[ticker] || { ticker: ticker };
      var aiValue = stock.ai_labs_value_usd == null ? 'unpriced' : formatUsd(stock.ai_labs_value_usd);
      var aiPct = stock.ai_labs_value_pct_of_market_cap == null ? '' : ' / ' + formatPct(stock.ai_labs_value_pct_of_market_cap, 1);
      return (
        '<span class="prr-chip">' +
          '<strong>' + escapeHtml(ticker) + '</strong>' +
          '<span>' + escapeHtml(aiValue + aiPct) + ' AI labs</span>' +
        '</span>'
      );
    }).join('');
  }

  function renderReport(payload) {
    var benchmark = payload.benchmark || {};
    var top = (benchmark.leaderboard || [])[0] || {};
    var sotp = (payload.market || {}).softbank_sotp || {};
    var totalWall = (benchmark.run_wall_minutes || []).reduce(function (sum, value) {
      return sum + Number(value || 0);
    }, 0);
    bodyEl.innerHTML =
      '<section class="prr-hero">' +
        '<div>' +
          '<div class="prr-kicker">Task Node Gated Research</div>' +
          '<h2 class="prr-title">' + escapeHtml(payload.title || 'Profound Round Robin') + '</h2>' +
          '<p class="prr-dek">' + escapeHtml(payload.dek || '') + '</p>' +
          '<div class="prr-tape">' + renderTickerTape(payload) + '</div>' +
        '</div>' +
        '<div class="prr-scoreboard">' +
          metric('Top Model', shortModel(top.model_id), formatNum(top.avg_score_mean, 2) + ' average score') +
          metric('Benchmark Spend', formatUsd(benchmark.total_cost_usd, { decimals: 2 }), (benchmark.run_count || 0) + ' games, ' + (benchmark.model_count || 0) + ' finalists') +
          metric('Tokens', formatNum(benchmark.total_tokens_all_runs, 0), formatNum(benchmark.total_reasoning_tokens_all_runs, 0) + ' reasoning tokens') +
          metric('Wall Time', formatNum(totalWall, 1) + ' min', 'sum of completed game wall clocks') +
        '</div>' +
      '</section>' +
      renderOpeningTradeUpdate(payload) +
      renderSearchShare(payload) +
      renderConclusions(payload) +
      renderWrittenAnalysis(payload) +
      renderLabValuation(payload) +
      renderBenchmarkSection(payload) +
      renderReasoningSection(payload) +
      renderCostAdvantage(payload) +
      renderStatements(payload) +
      renderPriceAction(payload) +
      renderSotp(payload, sotp) +
      renderMarketTable(payload) +
      renderSkTelecomDeepDive(payload) +
      renderMethodology(payload);

    renderScatter('prr-score-cost-scatter', benchmark.leaderboard || [], benchmark.efficient_frontier || []);
    renderSearchTrendChart('prr-search-share-chart', payload.search_share || {});
    renderLabScatter('prr-lab-valuation-scatter', benchmark.lab_score_valuation || []);
    renderRankBars('prr-rank-bars', (benchmark.leaderboard || []).slice(0, 15));
    renderLineChart('prr-sbg-nikkei-chart', (((payload.market || {}).price_series || {}).softbank_vs_nikkei || []));
    renderLineChart('prr-arm-sp500-chart', (((payload.market || {}).price_series || {}).arm_vs_sp500 || []));
    mountModelTable(payload);
    mountStockTable(payload);
  }

  function renderOpeningTradeUpdate(payload) {
    var update = payload.opening_trade_update || {};
    if (!update.headline) return '';
    var trade = update.trade || {};
    var cache = update.index_cache || {};
    var positions = cache.positions || [];
    return (
      '<section class="prr-section prr-trade-update">' +
        '<div class="prr-section-head">' +
          '<div>' +
            '<span class="prr-label">Index Update</span>' +
            '<h2>' + escapeHtml(update.headline) + '</h2>' +
          '</div>' +
          '<div class="prr-trade-badge">' + escapeHtml(trade.data_provider ? 'Data: ' + trade.data_provider : 'Bloomberg') + '</div>' +
        '</div>' +
        '<div class="prr-trade-brief">' +
          '<strong>' + escapeHtml(trade.position || '') + '</strong>' +
          '<span>' + escapeHtml(trade.implementation || '') + '</span>' +
        '</div>' +
        '<div class="prr-analysis-stack">' +
          (update.paragraphs || []).map(function (text) {
            return '<article class="prr-analysis-block prr-thesis-text"><p>' + escapeHtml(text) + '</p></article>';
          }).join('') +
        '</div>' +
        renderIndexCache(cache, positions) +
        renderSupportImages(update.support_images || []) +
      '</section>'
    );
  }

  function renderIndexCache(cache, positions) {
    if (!cache.available) {
      return (
        '<div class="prr-cache-strip missing">' +
          '<span class="prr-label">agti_0 Cache</span>' +
          '<p>' + escapeHtml(cache.error || 'Index cache unavailable in this build.') + '</p>' +
        '</div>'
      );
    }
    return (
      '<div class="prr-cache-strip">' +
        '<div>' +
          '<span class="prr-label">agti_0 Cache</span>' +
          '<strong>' + escapeHtml(cache.snapshot_id || '') + '</strong>' +
          '<span>' + escapeHtml((cache.content_hash || '').slice(0, 16)) + ' content hash prefix</span>' +
        '</div>' +
        '<div class="prr-position-strip">' +
          positions.map(function (row) {
            return (
              '<article>' +
                '<span class="prr-label">' + escapeHtml(row.side || '') + '</span>' +
                '<strong>' + escapeHtml(row.bloomberg_symbol || '') + '</strong>' +
                '<span>' + escapeHtml((row.weight_percent || '') + '% / ' + (row.base_currency_value || '') + ' USD target') + '</span>' +
              '</article>'
            );
          }).join('') +
        '</div>' +
      '</div>'
    );
  }

  function renderSupportImages(images) {
    if (!images.length) return '';
    return (
      '<div class="prr-proof-grid">' +
        images.map(function (image) {
          return (
            '<figure>' +
              '<img src="' + escapeHtml(image.url || '') + '" alt="' + escapeHtml(image.label || 'supporting screenshot') + '" loading="lazy">' +
              '<figcaption><span class="prr-label">' + escapeHtml(image.label || '') + '</span>' + escapeHtml(image.caption || '') + '</figcaption>' +
            '</figure>'
          );
        }).join('') +
      '</div>'
    );
  }

  function renderConclusions(payload) {
    return (
      '<section class="prr-section">' +
        '<h2>Conclusions Up Front</h2>' +
        '<div class="prr-callouts">' +
          (payload.conclusions || []).map(function (text, index) {
            return (
              '<article class="prr-callout">' +
                '<span class="prr-label">Conclusion ' + (index + 1) + '</span>' +
                '<p>' + escapeHtml(text) + '</p>' +
              '</article>'
            );
          }).join('') +
        '</div>' +
      '</section>'
    );
  }

  function renderSearchShare(payload) {
    var trend = payload.search_share || {};
    if (!trend.available) return '';
    var summary = trend.summary || {};
    var latest = summary.latest_complete || {};
    var latestMa = summary.latest_7d_ma || latest;
    var trough = summary.min_7d_ratio || summary.min_complete_ratio || {};
    var latestRatio = summary.latest_7d_ratio || latestMa.chatgpt_to_claude_ratio_7d_ma || summary.latest_daily_ratio;
    return (
      '<section class="prr-section prr-search-share">' +
        '<div class="prr-section-head">' +
          '<div>' +
            '<span class="prr-label">San Francisco Topic Tape</span>' +
            '<h2>OpenAI Retakes Local Search Mindshare</h2>' +
          '</div>' +
          '<a class="prr-source-pill" href="' + escapeHtml(trend.source_url || '#') + '" rel="noopener noreferrer" target="_blank">Google Trends</a>' +
        '</div>' +
        '<div class="prr-grid-2">' +
          '<div class="prr-chart">' +
            '<div class="prr-chart-head"><p class="prr-chart-title">Topic Interest 7D MA + Ratio</p><span class="prr-chart-sub">raw export, geo 807</span></div>' +
            '<svg id="prr-search-share-chart" class="prr-svg" role="img" aria-label="ChatGPT to Claude Google Trends ratio"></svg>' +
          '</div>' +
          '<div class="prr-search-summary">' +
            '<div class="prr-search-big">' + formatNum(latestRatio, 2) + 'x</div>' +
            '<p>Latest complete 7-day moving-average ChatGPT/Claude topic ratio on the 90-day Google Trends chart in the ' + escapeHtml(trend.geo_label || 'selected region') + '.</p>' +
            '<div class="prr-search-metrics">' +
              metric('ChatGPT 7D MA', formatNum(latestMa.chatgpt_interest_7d_ma, 1), escapeHtml(latestMa.date || 'latest complete day')) +
              metric('Claude 7D MA', formatNum(latestMa.claude_interest_7d_ma, 1), escapeHtml(latestMa.date || 'latest complete day')) +
              metric('7D Ratio Trough', formatNum(trough.chatgpt_to_claude_ratio_7d_ma || trough.chatgpt_to_claude_ratio, 2) + 'x', escapeHtml(trough.date || 'lowest complete day')) +
              metric('Raw Latest', formatNum(latest.chatgpt_interest, 0) + ' / ' + formatNum(latest.claude_interest, 0), escapeHtml((latest.date || '') + ' ChatGPT / Claude')) +
            '</div>' +
            '<p class="prr-sotp-caption">' + escapeHtml((trend.interpretation || [])[1] || trend.note || '') + '</p>' +
          '</div>' +
        '</div>' +
      '</section>'
    );
  }

  function searchShareSeries(trend) {
    var rows = (trend.rows || []).filter(function (row) {
      return row.chatgpt_to_claude_ratio != null;
    });
    return [
      {
        symbol: 'CHATGPT_CLAUDE_RATIO',
        label: '90d daily ratio',
        points: rows.map(function (row) {
          return { date: row.date, value: Number(row.chatgpt_to_claude_ratio || 0) };
        }),
      }
    ];
  }

  function renderWrittenAnalysis(payload) {
    var sections = payload.written_analysis || [];
    if (!sections.length) return '';
    return (
      '<section class="prr-section">' +
        '<h2>Written Analysis</h2>' +
        '<div class="prr-analysis-stack">' +
          sections.map(function (section) {
            return (
              '<article class="prr-analysis-block">' +
                '<span class="prr-label">' + escapeHtml(section.title || '') + '</span>' +
                (section.paragraphs || []).map(function (text) {
                  return '<p>' + escapeHtml(text) + '</p>';
                }).join('') +
              '</article>'
            );
          }).join('') +
        '</div>' +
      '</section>'
    );
  }

  function renderLabValuation(payload) {
    var rows = ((payload.benchmark || {}).lab_score_valuation || []).slice(0, 14);
    return (
      '<section class="prr-section">' +
        '<h2>Lab Score vs Valuation</h2>' +
        '<div class="prr-grid-2">' +
          '<div class="prr-chart">' +
            '<div class="prr-chart-head"><p class="prr-chart-title">Top Model Score / Lab Value</p><span class="prr-chart-sub">x = valuation, log scale</span></div>' +
            '<svg id="prr-lab-valuation-scatter" class="prr-svg" role="img" aria-label="Lab score versus valuation"></svg>' +
          '</div>' +
          '<div class="prr-mini-table">' +
            '<table class="prr-table compact"><thead><tr><th>Lab</th><th>Top model</th><th class="num">Score</th><th class="num">Value</th></tr></thead><tbody>' +
              rows.map(function (row) {
                return (
                  '<tr>' +
                    '<td><div class="prr-model-cell">' + escapeHtml(row.provider) + '</div><span class="prr-muted">' + escapeHtml(row.valuation_type) + '</span></td>' +
                    '<td>' + escapeHtml(shortModel(row.top_model_id)) + '</td>' +
                    '<td class="num">' + formatNum(row.top_score, 2) + '</td>' +
                    '<td class="num">' + formatUsd(row.valuation_usd) + '</td>' +
                  '</tr>'
                );
              }).join('') +
            '</tbody></table>' +
          '</div>' +
        '</div>' +
      '</section>'
    );
  }

  function renderBenchmarkSection(payload) {
    var corr = ((payload.benchmark || {}).pairwise_correlations || []).slice(0, 6);
    return (
      '<section class="prr-section">' +
        '<h2>Benchmark Tape</h2>' +
        '<div class="prr-grid-2">' +
          '<div class="prr-chart">' +
            '<div class="prr-chart-head"><p class="prr-chart-title">Score vs Cost</p><span class="prr-chart-sub">line = efficient frontier</span></div>' +
            '<svg id="prr-score-cost-scatter" class="prr-svg" role="img" aria-label="Model score versus generation cost"></svg>' +
          '</div>' +
          '<div class="prr-chart">' +
            '<div class="prr-chart-head"><p class="prr-chart-title">Top 15 Score Stack</p><span class="prr-chart-sub">five-run average</span></div>' +
            '<svg id="prr-rank-bars" class="prr-svg" role="img" aria-label="Top model scores"></svg>' +
          '</div>' +
        '</div>' +
        '<div class="prr-table-controls">' +
          '<span class="prr-label">Leaderboard</span>' +
          '<button class="prr-small-action" id="prr-model-reset" type="button">Rank Sort</button>' +
        '</div>' +
        '<div id="prr-model-table" class="prr-table-wrap"></div>' +
        '<div class="prr-method" style="margin-top:18px">' +
          corr.map(function (row) {
            return '<span class="prr-method-tag">R' + row.runs.join('/R') + ' rho ' + formatNum(row.rank_spearman, 2) + '</span>';
          }).join('') +
        '</div>' +
      '</section>'
    );
  }

  function renderReasoningSection(payload) {
    var benchmark = payload.benchmark || {};
    var judges = (benchmark.judge_economics || []).slice().sort(function (a, b) {
      return Number(b.total_reasoning_tokens || 0) - Number(a.total_reasoning_tokens || 0);
    }).slice(0, 10);
    return (
      '<section class="prr-section">' +
        '<h2>Reasoning Token Tape</h2>' +
        '<div class="prr-callouts">' +
          '<article class="prr-callout"><span class="prr-label">All Runs</span><p>' + escapeHtml(formatNum(benchmark.total_reasoning_tokens_all_runs, 0) + ' reasoning tokens, ' + formatPct(benchmark.reasoning_share_pct_all_runs, 1) + ' of total token volume.') + '</p></article>' +
          '<article class="prr-callout"><span class="prr-label">Why It Matters</span><p>Published price per million tokens is only the sticker. Reasoning-heavy models can burn hidden completion tokens, so realized cost per benchmark point is the cleaner workflow metric.</p></article>' +
          '<article class="prr-callout"><span class="prr-label">Deployment Read</span><p>Use premium frontier endpoints when the extra intelligence matters; use frontier-line cheap models for repeated tasks where marginal quality is not worth a 10x-100x cost delta.</p></article>' +
        '</div>' +
        '<div class="prr-table-wrap">' +
          '<table class="prr-table compact"><thead><tr><th>Judge</th><th class="num">Votes</th><th class="num">Avg cost</th><th class="num">Avg tokens</th><th class="num">Reasoning</th><th class="num">Latency</th></tr></thead><tbody>' +
            judges.map(function (row) {
              return (
                '<tr>' +
                  '<td><div class="prr-model-cell">' + escapeHtml(row.judge_model_id) + '</div></td>' +
                  '<td class="num">' + formatNum(row.vote_count, 0) + '</td>' +
                  '<td class="num">' + formatCost(row.avg_cost_per_vote_usd) + '</td>' +
                  '<td class="num">' + formatNum(row.avg_total_tokens_per_vote, 0) + '</td>' +
                  '<td class="num">' + formatPct(row.reasoning_share_pct, 1) + '</td>' +
                  '<td class="num">' + formatNum(row.avg_latency_ms / 1000, 2) + 's</td>' +
                '</tr>'
              );
            }).join('') +
          '</tbody></table>' +
        '</div>' +
      '</section>'
    );
  }

  function renderCostAdvantage(payload) {
    var tape = (payload.benchmark || {}).cost_advantage || {};
    var cheap = tape.cheap_competent || [];
    var expensive = tape.expensive_models || [];
    var range = tape.top_cluster_cost_range || {};
    return (
      '<section class="prr-section">' +
        '<h2>Cost Advantage</h2>' +
        '<div class="prr-callouts">' +
          '<article class="prr-callout"><span class="prr-label">Top Cluster Range</span><p>' + escapeHtml(formatCost(range.min_cost_usd) + ' to ' + formatCost(range.max_cost_usd) + ' per generation among models within three points of the top score.') + '</p></article>' +
          '<article class="prr-callout"><span class="prr-label">China Tape</span><p>Several Chinese and China-adjacent providers clear the quality bar at tiny realized costs. That matters for margins if the use case is repeated agent work rather than one trophy response.</p></article>' +
          '<article class="prr-callout"><span class="prr-label">Premium Tape</span><p>Anthropic and OpenAI premium endpoints still define much of the ceiling, but the bill is real: reasoning tokens and higher output tariffs can dominate total workflow spend.</p></article>' +
        '</div>' +
        '<div class="prr-grid-2">' +
          renderCostList('Cheap Competent', cheap) +
          renderCostList('Most Expensive', expensive) +
        '</div>' +
      '</section>'
    );
  }

  function renderCostList(title, rows) {
    return (
      '<div class="prr-mini-table">' +
        '<span class="prr-label">' + escapeHtml(title) + '</span>' +
        '<table class="prr-table compact"><thead><tr><th>Model</th><th class="num">Score</th><th class="num">Cost</th><th class="num">$/1M tokens</th></tr></thead><tbody>' +
          rows.map(function (row) {
            return (
              '<tr>' +
                '<td><div class="prr-model-cell">' + escapeHtml(shortModel(row.model_id)) + '</div></td>' +
                '<td class="num">' + formatNum(row.avg_score_mean, 2) + '</td>' +
                '<td class="num">' + formatCost(row.avg_generation_cost_usd) + '</td>' +
                '<td class="num">' + formatUsd(row.generation_cost_per_1m_tokens_usd, { decimals: 1 }) + '</td>' +
              '</tr>'
            );
          }).join('') +
        '</tbody></table>' +
      '</div>'
    );
  }

  function renderStatements(payload) {
    var rows = ((payload.benchmark || {}).profound_statements || []).slice(0, 12);
    return (
      '<section class="prr-section">' +
        '<h2>Profound Statements</h2>' +
        '<div class="prr-statements">' +
          rows.map(function (row) {
            var score = row.statement_score_mean == null ? '' : ' / score ' + formatNum(row.statement_score_mean, 1) + ' n=' + formatNum(row.statement_score_count, 0);
            return (
              '<article class="prr-statement">' +
                '<div class="prr-statement-meta">#' + escapeHtml(row.rank) + ' ' + escapeHtml(shortModel(row.model_id)) + ' / round ' + escapeHtml(row.round) + escapeHtml(score) + '</div>' +
                '<p>' + escapeHtml(row.text) + '</p>' +
              '</article>'
            );
          }).join('') +
        '</div>' +
      '</section>'
    );
  }

  function renderPriceAction(payload) {
    return (
      '<section class="prr-section">' +
        '<h2>Public Price Action</h2>' +
        '<div class="prr-grid-2">' +
          '<div class="prr-chart">' +
            '<div class="prr-chart-head"><p class="prr-chart-title">SoftBank vs Nikkei</p><span class="prr-chart-sub">1Y normalized</span></div>' +
            '<svg id="prr-sbg-nikkei-chart" class="prr-svg" role="img" aria-label="SoftBank versus Nikkei normalized price action"></svg>' +
          '</div>' +
          '<div class="prr-chart">' +
            '<div class="prr-chart-head"><p class="prr-chart-title">ARM vs S&amp;P 500</p><span class="prr-chart-sub">1Y normalized</span></div>' +
            '<svg id="prr-arm-sp500-chart" class="prr-svg" role="img" aria-label="Arm versus S&P 500 normalized price action"></svg>' +
          '</div>' +
        '</div>' +
      '</section>'
    );
  }

  function renderSotp(payload, sotp) {
    var debt = ((payload.market || {}).softbank_debt_analysis || {});
    return (
      '<section class="prr-section">' +
        '<h2>SoftBank SOTP and Financing</h2>' +
        '<div class="prr-sotp-grid">' +
          '<div>' +
            '<span class="prr-sotp-number">' + formatPct((sotp.implied_openai_percent_of_marked_value || 0) * 100, 1) + '</span>' +
            '<p class="prr-sotp-caption">Implied OpenAI value after marking ARM and SoftBank Corp. public stakes against the OpenAI stake marked value.</p>' +
            '<ul class="prr-list">' +
              '<li>SoftBank Group market cap: ' + formatUsd(sotp.market_cap_usd) + '</li>' +
              '<li>ARM stake value: ' + formatUsd(sotp.arm_stake_value_usd) + '</li>' +
              '<li>SoftBank Corp. stake value: ' + formatUsd(sotp.softbank_corp_stake_value_usd) + '</li>' +
              '<li>OpenAI stake marked value: ' + formatUsd(sotp.openai_stake_marked_value_usd) + '</li>' +
            '</ul>' +
          '</div>' +
          '<article class="prr-callout">' +
            '<span class="prr-label">Setup</span>' +
            '<p>' + escapeHtml(sotp.interpretation || '') + '</p>' +
          '</article>' +
        '</div>' +
        '<div class="prr-grid-2 prr-debt-grid">' +
          '<div><span class="prr-label">OpenAI Stake Structure</span><ul class="prr-list">' +
            (debt.openai_stake_structure || []).map(function (item) { return '<li>' + escapeHtml(item) + '</li>'; }).join('') +
          '</ul></div>' +
          '<div><span class="prr-label">Debt Stack</span><ul class="prr-list">' +
            (debt.debt_stack || []).map(function (item) { return '<li>' + escapeHtml(item) + '</li>'; }).join('') +
          '</ul></div>' +
        '</div>' +
        '<p class="prr-sotp-caption">' + escapeHtml(debt.headline || '') + '</p>' +
      '</section>'
    );
  }

  function renderMarketTable(payload) {
    return (
      '<section class="prr-section">' +
        '<h2>Public Market Exposure</h2>' +
        '<div class="prr-table-controls">' +
          '<span class="prr-label">Valuation and price action</span>' +
          '<button class="prr-small-action" id="prr-stock-reset" type="button">Market Cap Sort</button>' +
        '</div>' +
        '<div id="prr-stock-table" class="prr-table-wrap"></div>' +
      '</section>'
    );
  }

  function renderSkTelecomDeepDive(payload) {
    var deep = ((payload.market || {}).sk_telecom_deep_dive || {});
    var rows = deep.comp_rows || [];
    var sensitivity = deep.anthropic_sensitivity || [];
    if (!rows.length) return '';
    return (
      '<section class="prr-section">' +
        '<h2>SK Telecom Deep Dive</h2>' +
        '<div class="prr-callouts">' +
          '<article class="prr-callout"><span class="prr-label">Unadjusted Multiple</span><p>SKT trades at ' + formatNum(rows[0].ev_ebitda, 2) + 'x EV/EBITDA versus ' + formatNum(deep.peer_average_ev_ebitda_ex_skt, 2) + 'x for KT and LG Uplus.</p></article>' +
          '<article class="prr-callout"><span class="prr-label">Premium</span><p>The unadjusted SKT premium to Korean telecom peers is ' + formatPct(deep.sk_premium_to_peer_avg_pct, 1) + ', so the Anthropic stake has to do real explanatory work.</p></article>' +
          '<article class="prr-callout"><span class="prr-label">Source Method</span><p>' + escapeHtml(deep.source_method || '') + '</p></article>' +
        '</div>' +
        '<div class="prr-grid-2">' +
          '<div class="prr-mini-table">' +
            '<span class="prr-label">Korean Telecom Comps</span>' +
            '<table class="prr-table compact"><thead><tr><th>Company</th><th class="num">EV</th><th class="num">EBITDA</th><th class="num">EV/EBITDA</th><th class="num">Debt/EBITDA</th><th class="num">FCF Yield</th></tr></thead><tbody>' +
              rows.map(function (row) {
                return (
                  '<tr>' +
                    '<td><div class="prr-model-cell">' + escapeHtml(row.company) + '</div><span class="prr-muted">' + escapeHtml(row.ticker) + '</span></td>' +
                    '<td class="num">' + formatKrwTrn(row.enterprise_value_krw_trn) + '</td>' +
                    '<td class="num">' + formatKrwTrn(row.ttm_ebitda_krw_trn) + '</td>' +
                    '<td class="num">' + formatNum(row.ev_ebitda, 2) + 'x</td>' +
                    '<td class="num">' + formatNum(row.debt_ebitda, 2) + 'x</td>' +
                    '<td class="num">' + formatPct(row.fcf_yield_pct, 1) + '</td>' +
                  '</tr>'
                );
              }).join('') +
            '</tbody></table>' +
          '</div>' +
          '<div class="prr-mini-table">' +
            '<span class="prr-label">Anthropic Stake Sensitivity</span>' +
            '<table class="prr-table compact"><thead><tr><th>Stake</th><th class="num">Value</th><th class="num">% Mkt Cap</th><th class="num">Adj. EV/EBITDA</th><th class="num">Peer Premium</th></tr></thead><tbody>' +
              sensitivity.map(function (row) {
                return (
                  '<tr>' +
                    '<td><div class="prr-model-cell">' + formatPct(row.ownership_percent, 2) + '</div><span class="prr-muted">' + escapeHtml(row.note || '') + '</span></td>' +
                    '<td class="num">' + formatUsd(row.stake_value_usd) + '</td>' +
                    '<td class="num">' + formatPct(row.stake_value_pct_of_market_cap, 1) + '</td>' +
                    '<td class="num">' + formatNum(row.adjusted_core_ev_ebitda, 2) + 'x</td>' +
                    '<td class="num">' + formatPct(row.premium_to_kt_lgu_peer_avg_pct, 1) + '</td>' +
                  '</tr>'
                );
              }).join('') +
            '</tbody></table>' +
          '</div>' +
        '</div>' +
        '<div class="prr-analysis-stack">' +
          (deep.thesis || []).map(function (text) {
            return '<article class="prr-analysis-block"><p>' + escapeHtml(text) + '</p></article>';
          }).join('') +
        '</div>' +
      '</section>'
    );
  }

  function renderMethodology(payload) {
    var method = payload.methodology || {};
    var market = payload.market || {};
    return (
      '<section class="prr-section">' +
        '<h2>Methodology</h2>' +
        '<div class="prr-method">' +
          '<span class="prr-method-tag">30 finalists</span>' +
          '<span class="prr-method-tag">3 rounds</span>' +
          '<span class="prr-method-tag">temperature 0</span>' +
          '<span class="prr-method-tag">peer scored</span>' +
          '<span class="prr-method-tag">five-run aggregate</span>' +
        '</div>' +
        '<p>' + escapeHtml(method.short || '') + '</p>' +
        '<div class="prr-grid-2">' +
          '<div><span class="prr-label">Details</span><ul class="prr-list">' +
            (method.details || []).map(function (item) { return '<li>' + escapeHtml(item) + '</li>'; }).join('') +
          '</ul></div>' +
          '<div><span class="prr-label">Limitations</span><ul class="prr-list">' +
            (method.limitations || []).map(function (item) { return '<li>' + escapeHtml(item) + '</li>'; }).join('') +
          '</ul></div>' +
        '</div>' +
        '<h2>Sources</h2>' +
        '<ul class="prr-source-list">' +
          (market.sources || []).map(function (url) {
            return '<li><a href="' + escapeHtml(url) + '" rel="noopener noreferrer" target="_blank">' + escapeHtml(url) + '</a></li>';
          }).join('') +
        '</ul>' +
      '</section>'
    );
  }

  function sortedRows(rows, state) {
    return rows.slice().sort(function (a, b) {
      var av = a[state.key];
      var bv = b[state.key];
      var an = Number(av);
      var bn = Number(bv);
      if (Number.isFinite(an) && Number.isFinite(bn)) {
        return (an - bn) * state.dir;
      }
      return String(av || '').localeCompare(String(bv || '')) * state.dir;
    });
  }

  function mountModelTable(payload) {
    var rows = (payload.benchmark || {}).leaderboard || [];
    var holder = document.getElementById('prr-model-table');
    var reset = document.getElementById('prr-model-reset');
    if (!holder) return;
    function draw() {
      var ordered = sortedRows(rows, tableState.model);
      holder.innerHTML =
        '<table class="prr-table">' +
          '<thead><tr>' +
            header('rank', '#') +
            header('model_id', 'Model') +
            header('provider', 'Provider') +
            header('avg_score_mean', 'Score') +
            header('score_mean_stdev', 'Score SD') +
            header('avg_rank', 'Avg Rank') +
            header('avg_generation_latency_ms', 'Latency') +
            header('avg_generation_cost_usd', 'Cost') +
            header('avg_generation_total_tokens', 'Tokens') +
            header('generation_reasoning_share_pct', 'Reasoning') +
            header('generation_cost_per_1m_tokens_usd', '$/1M Tok') +
            header('avg_vote_count', 'Votes') +
          '</tr></thead><tbody>' +
          ordered.map(function (row) {
            return (
              '<tr>' +
                '<td class="num">' + escapeHtml(row.rank) + '</td>' +
                '<td><div class="prr-model-cell">' + escapeHtml(row.model_id) + '</div></td>' +
                '<td class="prr-provider">' + escapeHtml(row.provider) + '</td>' +
                '<td class="num">' + formatNum(row.avg_score_mean, 2) + '</td>' +
                '<td class="num">' + formatNum(row.score_mean_stdev, 2) + '</td>' +
                '<td class="num">' + formatNum(row.avg_rank, 2) + '</td>' +
                '<td class="num">' + formatNum(row.avg_generation_latency_ms / 1000, 2) + 's</td>' +
                '<td class="num">' + formatCost(row.avg_generation_cost_usd) + '</td>' +
                '<td class="num">' + formatNum(row.avg_generation_total_tokens, 0) + '</td>' +
                '<td class="num">' + formatPct(row.generation_reasoning_share_pct, 1) + '</td>' +
                '<td class="num">' + formatUsd(row.generation_cost_per_1m_tokens_usd, { decimals: 1 }) + '</td>' +
                '<td class="num">' + formatNum(row.avg_vote_count, 0) + '</td>' +
              '</tr>'
            );
          }).join('') +
        '</tbody></table>';
      holder.querySelectorAll('th[data-key]').forEach(function (th) {
        th.addEventListener('click', function () {
          var key = th.getAttribute('data-key');
          if (tableState.model.key === key) {
            tableState.model.dir *= -1;
          } else {
            tableState.model.key = key;
            tableState.model.dir = key === 'rank' ? 1 : -1;
          }
          draw();
        });
      });
    }
    if (reset) {
      reset.addEventListener('click', function () {
        tableState.model = { key: 'rank', dir: 1 };
        draw();
      });
    }
    draw();
  }

  function mountStockTable(payload) {
    var rows = (payload.market || {}).stocks || [];
    var holder = document.getElementById('prr-stock-table');
    var reset = document.getElementById('prr-stock-reset');
    if (!holder) return;
    function draw() {
      var ordered = sortedRows(rows, tableState.stock);
      holder.innerHTML =
        '<table class="prr-table">' +
          '<thead><tr>' +
            header('ticker', 'Ticker') +
            header('company', 'Company') +
            header('market_cap_usd', 'Market Cap') +
            header('ai_labs_value_usd', 'AI Labs') +
            header('ai_labs_value_pct_of_market_cap', 'AI %') +
            header('market_cap_ex_ai_labs_usd', 'Ex AI Labs') +
            header('one_year_return_pct', '1Y') +
            header('one_year_excess_vs_smh_pct', 'Vs SMH') +
          '</tr></thead><tbody>' +
          ordered.map(function (row) {
            return (
              '<tr>' +
                '<td><div class="prr-model-cell">' + escapeHtml(row.ticker) + '</div></td>' +
                '<td>' + escapeHtml(row.company) + '</td>' +
                '<td class="num">' + formatUsd(row.market_cap_usd) + '</td>' +
                '<td class="num">' + (row.ai_labs_value_usd == null ? escapeHtml((row.unpriced_ai_labs || []).join(', ') || 'n/a') : formatUsd(row.ai_labs_value_usd)) + '</td>' +
                '<td class="num">' + formatPct(row.ai_labs_value_pct_of_market_cap, 1) + '</td>' +
                '<td class="num">' + formatUsd(row.market_cap_ex_ai_labs_usd) + '</td>' +
                '<td class="num">' + formatPct(row.one_year_return_pct, 1) + '</td>' +
                '<td class="num">' + formatPct(row.one_year_excess_vs_smh_pct, 1) + '</td>' +
              '</tr>'
            );
          }).join('') +
        '</tbody></table>';
      holder.querySelectorAll('th[data-key]').forEach(function (th) {
        th.addEventListener('click', function () {
          var key = th.getAttribute('data-key');
          if (tableState.stock.key === key) {
            tableState.stock.dir *= -1;
          } else {
            tableState.stock.key = key;
            tableState.stock.dir = key === 'ticker' || key === 'company' ? 1 : -1;
          }
          draw();
        });
      });
    }
    if (reset) {
      reset.addEventListener('click', function () {
        tableState.stock = { key: 'market_cap_usd', dir: -1 };
        draw();
      });
    }
    draw();
  }

  function header(key, label) {
    return '<th data-key="' + escapeHtml(key) + '">' + escapeHtml(label) + '</th>';
  }

  function renderScatter(id, rows, frontier) {
    var svg = document.getElementById(id);
    if (!svg || !rows.length) return;
    var width = svg.clientWidth || 640;
    var height = svg.clientHeight || 320;
    var pad = { left: 48, right: 18, top: 16, bottom: 38 };
    var xs = rows.map(function (r) { return Math.log10(Number(r.avg_generation_cost_usd || 0) + 0.00001); });
    var ys = rows.map(function (r) { return Number(r.avg_score_mean || 0); });
    var ls = rows.map(function (r) { return Number(r.avg_generation_latency_ms || 0); });
    var minX = Math.min.apply(null, xs);
    var maxX = Math.max.apply(null, xs);
    var minY = Math.floor(Math.min.apply(null, ys) / 5) * 5;
    var maxY = Math.ceil(Math.max.apply(null, ys) / 5) * 5;
    var maxL = Math.max.apply(null, ls) || 1;
    var xScale = function (x) {
      return pad.left + ((x - minX) / Math.max(maxX - minX, 0.00001)) * (width - pad.left - pad.right);
    };
    var yScale = function (y) {
      return height - pad.bottom - ((y - minY) / Math.max(maxY - minY, 0.00001)) * (height - pad.top - pad.bottom);
    };
    var html = [
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + (height - pad.bottom) + '" x2="' + (width - pad.right) + '" y2="' + (height - pad.bottom) + '"></line>',
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + pad.top + '" x2="' + pad.left + '" y2="' + (height - pad.bottom) + '"></line>',
    ];
    [minY, (minY + maxY) / 2, maxY].forEach(function (tick) {
      var y = yScale(tick);
      html.push('<line class="prr-gridline" x1="' + pad.left + '" y1="' + y + '" x2="' + (width - pad.right) + '" y2="' + y + '"></line>');
      html.push('<text x="8" y="' + (y + 4) + '">' + formatNum(tick, id === 'prr-search-share-chart' ? 1 : 0) + '</text>');
    });
    var frontierPoints = (frontier || []).map(function (row) {
      return xScale(Math.log10(Number(row.avg_generation_cost_usd || 0) + 0.00001)).toFixed(2) + ',' + yScale(Number(row.avg_score_mean || 0)).toFixed(2);
    }).join(' ');
    if (frontierPoints) {
      html.push('<polyline class="prr-frontier-line" points="' + frontierPoints + '"></polyline>');
    }
    rows.forEach(function (row, index) {
      var x = xScale(xs[index]);
      var y = yScale(ys[index]);
      var radius = 5 + Math.min(12, (ls[index] / maxL) * 12);
      html.push('<circle class="prr-point ' + (row.rank <= 5 ? 'top' : '') + '" data-index="' + index + '" cx="' + x + '" cy="' + y + '" r="' + radius.toFixed(2) + '"></circle>');
    });
    html.push('<text x="' + pad.left + '" y="' + (height - 8) + '">generation cost, log scale</text>');
    html.push('<text x="' + (width - 128) + '" y="14">score</text>');
    svg.setAttribute('viewBox', '0 0 ' + width + ' ' + height);
    svg.innerHTML = html.join('');
    svg.querySelectorAll('.prr-point').forEach(function (point) {
      point.addEventListener('mousemove', function (event) {
        var row = rows[Number(point.getAttribute('data-index'))];
        showTooltip(event, '<strong>' + escapeHtml(row.model_id) + '</strong><span>Score ' + formatNum(row.avg_score_mean, 2) + ' / cost ' + formatCost(row.avg_generation_cost_usd) + ' / reasoning ' + formatPct(row.generation_reasoning_share_pct, 1) + ' / latency ' + formatNum(row.avg_generation_latency_ms / 1000, 2) + 's</span>');
      });
      point.addEventListener('mouseleave', hideTooltip);
    });
  }

  function renderRankBars(id, rows) {
    var svg = document.getElementById(id);
    if (!svg || !rows.length) return;
    var width = svg.clientWidth || 640;
    var rowHeight = 19;
    var height = Math.max(svg.clientHeight || 320, rows.length * rowHeight + 44);
    var pad = { left: 178, right: 42, top: 20, bottom: 16 };
    var html = [
      '<defs><linearGradient id="prr-bar-gradient" x1="0" x2="1"><stop offset="0" stop-color="#7ff0d8"></stop><stop offset="1" stop-color="#ff5a42"></stop></linearGradient></defs>',
    ];
    rows.forEach(function (row, index) {
      var y = pad.top + index * rowHeight;
      var w = ((Number(row.avg_score_mean || 0) / 100) * (width - pad.left - pad.right));
      html.push('<text x="0" y="' + (y + 12) + '">' + escapeHtml(shortModel(row.model_id)) + '</text>');
      html.push('<rect class="prr-bar-track" x="' + pad.left + '" y="' + y + '" width="' + (width - pad.left - pad.right) + '" height="10"></rect>');
      html.push('<rect class="prr-bar" x="' + pad.left + '" y="' + y + '" width="' + w + '" height="10"></rect>');
      html.push('<text x="' + (pad.left + w + 7) + '" y="' + (y + 10) + '">' + formatNum(row.avg_score_mean, 1) + '</text>');
    });
    svg.setAttribute('viewBox', '0 0 ' + width + ' ' + height);
    svg.innerHTML = html.join('');
  }

  function renderLabScatter(id, rows) {
    var svg = document.getElementById(id);
    if (!svg || !rows.length) return;
    var width = svg.clientWidth || 640;
    var height = svg.clientHeight || 320;
    var pad = { left: 48, right: 20, top: 18, bottom: 38 };
    var xs = rows.map(function (row) { return Math.log10(Number(row.valuation_usd || 1)); });
    var ys = rows.map(function (row) { return Number(row.top_score || 0); });
    var minX = Math.min.apply(null, xs);
    var maxX = Math.max.apply(null, xs);
    var minY = Math.floor(Math.min.apply(null, ys) / 5) * 5;
    var maxY = Math.ceil(Math.max.apply(null, ys) / 5) * 5;
    var xScale = function (x) {
      return pad.left + ((x - minX) / Math.max(maxX - minX, 0.00001)) * (width - pad.left - pad.right);
    };
    var yScale = function (y) {
      return height - pad.bottom - ((y - minY) / Math.max(maxY - minY, 0.00001)) * (height - pad.top - pad.bottom);
    };
    var html = [
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + (height - pad.bottom) + '" x2="' + (width - pad.right) + '" y2="' + (height - pad.bottom) + '"></line>',
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + pad.top + '" x2="' + pad.left + '" y2="' + (height - pad.bottom) + '"></line>',
    ];
    [minY, (minY + maxY) / 2, maxY].forEach(function (tick) {
      var y = yScale(tick);
      html.push('<line class="prr-gridline" x1="' + pad.left + '" y1="' + y + '" x2="' + (width - pad.right) + '" y2="' + y + '"></line>');
      html.push('<text x="8" y="' + (y + 4) + '">' + formatNum(tick, 0) + '</text>');
    });
    rows.forEach(function (row, index) {
      var x = xScale(xs[index]);
      var y = yScale(ys[index]);
      var radius = row.top_rank <= 5 ? 9 : 6;
      html.push('<circle class="prr-point ' + (row.top_rank <= 5 ? 'top' : '') + '" data-index="' + index + '" cx="' + x + '" cy="' + y + '" r="' + radius + '"></circle>');
      html.push('<text x="' + (x + 10) + '" y="' + (y + 4) + '">' + escapeHtml(row.provider) + '</text>');
    });
    html.push('<text x="' + pad.left + '" y="' + (height - 8) + '">lab valuation / parent market cap, log scale</text>');
    html.push('<text x="' + (width - 128) + '" y="14">top score</text>');
    svg.setAttribute('viewBox', '0 0 ' + width + ' ' + height);
    svg.innerHTML = html.join('');
    svg.querySelectorAll('.prr-point').forEach(function (point) {
      point.addEventListener('mousemove', function (event) {
        var row = rows[Number(point.getAttribute('data-index'))];
        showTooltip(event, '<strong>' + escapeHtml(row.entity) + '</strong><span>' + escapeHtml(shortModel(row.top_model_id)) + ' / score ' + formatNum(row.top_score, 2) + ' / value ' + formatUsd(row.valuation_usd) + '</span>');
      });
      point.addEventListener('mouseleave', hideTooltip);
    });
  }

  function renderSearchTrendChart(id, trend) {
    var svg = document.getElementById(id);
    var rows = (trend.rows || []).filter(function (row) {
      return !row.is_partial && row.chatgpt_interest_7d_ma != null && row.claude_interest_7d_ma != null;
    });
    if (!svg || !rows.length) return;
    var width = svg.clientWidth || 640;
    var height = svg.clientHeight || 320;
    var pad = { left: 44, right: 58, top: 18, bottom: 36 };
    var times = rows.map(function (row) { return new Date(row.date).getTime(); });
    var ratios = rows.map(function (row) { return Number(row.chatgpt_to_claude_ratio_7d_ma || 0); }).filter(function (value) { return value > 0; });
    var minT = Math.min.apply(null, times);
    var maxT = Math.max.apply(null, times);
    var minRatio = Math.floor((Math.min.apply(null, ratios) - 0.05) * 10) / 10;
    var maxRatio = Math.ceil((Math.max.apply(null, ratios) + 0.05) * 10) / 10;
    var xScale = function (time) {
      return pad.left + ((time - minT) / Math.max(maxT - minT, 1)) * (width - pad.left - pad.right);
    };
    var leftScale = function (value) {
      return height - pad.bottom - ((value - 0) / 100) * (height - pad.top - pad.bottom);
    };
    var rightScale = function (value) {
      return height - pad.bottom - ((value - minRatio) / Math.max(maxRatio - minRatio, 0.00001)) * (height - pad.top - pad.bottom);
    };
    var html = [
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + (height - pad.bottom) + '" x2="' + (width - pad.right) + '" y2="' + (height - pad.bottom) + '"></line>',
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + pad.top + '" x2="' + pad.left + '" y2="' + (height - pad.bottom) + '"></line>',
      '<line class="prr-axis prr-axis-right" x1="' + (width - pad.right) + '" y1="' + pad.top + '" x2="' + (width - pad.right) + '" y2="' + (height - pad.bottom) + '"></line>',
    ];
    [0, 25, 50, 75, 100].forEach(function (tick) {
      var y = leftScale(tick);
      html.push('<line class="prr-gridline" x1="' + pad.left + '" y1="' + y + '" x2="' + (width - pad.right) + '" y2="' + y + '"></line>');
      html.push('<text x="8" y="' + (y + 4) + '">' + formatNum(tick, 0) + '</text>');
    });
    [minRatio, (minRatio + maxRatio) / 2, maxRatio].forEach(function (tick) {
      var y = rightScale(tick);
      html.push('<text x="' + (width - pad.right + 8) + '" y="' + (y + 4) + '" style="fill:#f8d56b">' + formatNum(tick, 1) + 'x</text>');
    });
    var series = [
      { label: 'ChatGPT 7D MA', color: '#7ff0d8', scale: leftScale, value: 'chatgpt_interest_7d_ma' },
      { label: 'Claude 7D MA', color: '#ff5a42', scale: leftScale, value: 'claude_interest_7d_ma' },
      { label: 'Ratio, right axis', color: '#f8d56b', scale: rightScale, value: 'chatgpt_to_claude_ratio_7d_ma', dash: true },
    ];
    series.forEach(function (entry, index) {
      var path = rows.map(function (row, pointIndex) {
        var x = xScale(new Date(row.date).getTime()).toFixed(2);
        var y = entry.scale(Number(row[entry.value] || 0)).toFixed(2);
        return (pointIndex === 0 ? 'M' : 'L') + x + ',' + y;
      }).join(' ');
      html.push('<path class="prr-line-series" d="' + path + '" stroke="' + entry.color + '"' + (entry.dash ? ' stroke-dasharray="6 5"' : '') + '></path>');
      html.push('<text x="' + (pad.left + 8) + '" y="' + (pad.top + 14 + index * 15) + '" style="fill:' + entry.color + '">' + escapeHtml(entry.label) + '</text>');
    });
    html.push('<text x="' + pad.left + '" y="' + (height - 8) + '">topic interest, 7d moving average</text>');
    html.push('<text x="' + (width - pad.right - 102) + '" y="14" style="fill:#f8d56b">ratio axis</text>');
    svg.setAttribute('viewBox', '0 0 ' + width + ' ' + height);
    svg.innerHTML = html.join('');
  }

  function renderLineChart(id, seriesRows) {
    var svg = document.getElementById(id);
    if (!svg || !seriesRows.length) return;
    var width = svg.clientWidth || 640;
    var height = svg.clientHeight || 320;
    var pad = { left: 44, right: 18, top: 18, bottom: 36 };
    var points = [];
    seriesRows.forEach(function (series) {
      (series.points || []).forEach(function (point) {
        points.push({ date: new Date(point.date).getTime(), value: Number(point.value || 0) });
      });
    });
    if (!points.length) return;
    var minT = Math.min.apply(null, points.map(function (point) { return point.date; }));
    var maxT = Math.max.apply(null, points.map(function (point) { return point.date; }));
    var minY = Math.floor(Math.min.apply(null, points.map(function (point) { return point.value; })) / 10) * 10;
    var maxY = Math.ceil(Math.max.apply(null, points.map(function (point) { return point.value; })) / 10) * 10;
    var xScale = function (time) {
      return pad.left + ((time - minT) / Math.max(maxT - minT, 1)) * (width - pad.left - pad.right);
    };
    var yScale = function (value) {
      return height - pad.bottom - ((value - minY) / Math.max(maxY - minY, 0.00001)) * (height - pad.top - pad.bottom);
    };
    var colors = ['#7ff0d8', '#ff5a42', '#f8d56b', '#9db7ff'];
    var html = [
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + (height - pad.bottom) + '" x2="' + (width - pad.right) + '" y2="' + (height - pad.bottom) + '"></line>',
      '<line class="prr-axis" x1="' + pad.left + '" y1="' + pad.top + '" x2="' + pad.left + '" y2="' + (height - pad.bottom) + '"></line>',
    ];
    var ticks = [minY];
    if (minY < 100 && maxY > 100) ticks.push(100);
    ticks.push(maxY);
    ticks.forEach(function (tick) {
      var y = yScale(tick);
      html.push('<line class="prr-gridline" x1="' + pad.left + '" y1="' + y + '" x2="' + (width - pad.right) + '" y2="' + y + '"></line>');
      html.push('<text x="8" y="' + (y + 4) + '">' + formatNum(tick, 0) + '</text>');
    });
    seriesRows.forEach(function (series, index) {
      var path = (series.points || []).map(function (point, pointIndex) {
        var x = xScale(new Date(point.date).getTime()).toFixed(2);
        var y = yScale(Number(point.value || 0)).toFixed(2);
        return (pointIndex === 0 ? 'M' : 'L') + x + ',' + y;
      }).join(' ');
      html.push('<path class="prr-line-series" d="' + path + '" stroke="' + colors[index % colors.length] + '"></path>');
      html.push('<text x="' + (pad.left + 8) + '" y="' + (pad.top + 14 + index * 15) + '" fill="' + colors[index % colors.length] + '">' + escapeHtml(series.label || series.symbol) + '</text>');
    });
    html.push('<text x="' + pad.left + '" y="' + (height - 8) + '">' + (id === 'prr-search-share-chart' ? 'ratio, ChatGPT / Claude' : 'indexed to 100 one year ago') + '</text>');
    svg.setAttribute('viewBox', '0 0 ' + width + ' ' + height);
    svg.innerHTML = html.join('');
  }

  function showTooltip(event, html) {
    if (!tooltipEl) return;
    tooltipEl.innerHTML = html;
    tooltipEl.hidden = false;
    var x = Math.min(event.clientX + 14, window.innerWidth - tooltipEl.offsetWidth - 12);
    var y = Math.min(event.clientY + 14, window.innerHeight - tooltipEl.offsetHeight - 12);
    tooltipEl.style.left = x + 'px';
    tooltipEl.style.top = y + 'px';
  }

  function hideTooltip() {
    if (tooltipEl) tooltipEl.hidden = true;
  }

  function init() {
    if (!panelEl || !bodyEl || !unlockEl) return;
    unlockEl.addEventListener('click', startAuthorization);
    exchangeCodeIfPresent()
      .then(unlockReport)
      .catch(function (err) {
        setStatus(err.message || 'Unable to unlock report.', true);
      });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
}());
