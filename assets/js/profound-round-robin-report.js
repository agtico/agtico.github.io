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
      return (
        '<span class="prr-chip">' +
          '<strong>' + escapeHtml(ticker) + '</strong>' +
          '<span>' + escapeHtml(aiValue) + ' AI labs</span>' +
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
      renderConclusions(payload) +
      renderBenchmarkSection(payload) +
      renderStatements(payload) +
      renderSotp(payload, sotp) +
      renderMarketTable(payload) +
      renderMethodology(payload);

    renderScatter('prr-score-cost-scatter', benchmark.leaderboard || []);
    renderRankBars('prr-rank-bars', (benchmark.leaderboard || []).slice(0, 15));
    mountModelTable(payload);
    mountStockTable(payload);
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

  function renderBenchmarkSection(payload) {
    var corr = ((payload.benchmark || {}).pairwise_correlations || []).slice(0, 6);
    return (
      '<section class="prr-section">' +
        '<h2>Benchmark Tape</h2>' +
        '<div class="prr-grid-2">' +
          '<div class="prr-chart">' +
            '<div class="prr-chart-head"><p class="prr-chart-title">Score vs Cost</p><span class="prr-chart-sub">bubble = latency</span></div>' +
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

  function renderStatements(payload) {
    var rows = ((payload.benchmark || {}).profound_statements || []).slice(0, 12);
    return (
      '<section class="prr-section">' +
        '<h2>Profound Statements</h2>' +
        '<div class="prr-statements">' +
          rows.map(function (row) {
            return (
              '<article class="prr-statement">' +
                '<div class="prr-statement-meta">#' + escapeHtml(row.rank) + ' ' + escapeHtml(shortModel(row.model_id)) + ' / round ' + escapeHtml(row.round) + '</div>' +
                '<p>' + escapeHtml(row.text) + '</p>' +
              '</article>'
            );
          }).join('') +
        '</div>' +
      '</section>'
    );
  }

  function renderSotp(payload, sotp) {
    return (
      '<section class="prr-section">' +
        '<h2>SoftBank SOTP</h2>' +
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
                '<td class="num">' + formatUsd(row.avg_generation_cost_usd, { decimals: 4 }) + '</td>' +
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

  function renderScatter(id, rows) {
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
      html.push('<text x="8" y="' + (y + 4) + '">' + formatNum(tick, 0) + '</text>');
    });
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
        showTooltip(event, '<strong>' + escapeHtml(row.model_id) + '</strong><span>Score ' + formatNum(row.avg_score_mean, 2) + ' / cost ' + formatUsd(row.avg_generation_cost_usd, { decimals: 4 }) + ' / latency ' + formatNum(row.avg_generation_latency_ms / 1000, 2) + 's</span>');
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
