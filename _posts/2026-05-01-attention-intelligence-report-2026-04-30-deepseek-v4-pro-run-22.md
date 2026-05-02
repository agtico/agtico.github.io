---
layout: report
title: "Attention Intelligence Report | 2026-04-30"
date: "2026-05-01 23:32:57 +0000"
summary: "Automated AGTI attention intelligence report from Wikipedia trend-cloud data as of 2026-04-30; searched clouds: 119."
category: Attention Intelligence
tags:
  - AGTI
  - Attention Intelligence
  - Wikipedia Trends
  - Alt Data
attention_as_of_date: 2026-04-30
mimetic_report_run_id: 22
source_report_run_id: 17
source_attention_run_id: 384
model_name: deepseek-v4-pro
searched_cloud_count: 119
gated_content_id: attention-intelligence-report-2026-04-30-deepseek-v4-pro-run-22
gated_content_url: /gated-content/reports/attention-intelligence-report-2026-04-30-deepseek-v4-pro-run-22.encrypted.v1.json
gated_access_manifest_url: /gated-content/reports/attention-intelligence-report-2026-04-30-deepseek-v4-pro-run-22.access-manifest.v0.json
---

<style>
  .gated-report-panel {
    border: 1px solid var(--line-strong);
    background: linear-gradient(135deg, rgba(255,255,255,.055), transparent 36%), var(--panel);
    padding: 24px;
    margin-bottom: 34px;
  }
  .gated-report-panel[hidden],
  .gated-report-body[hidden] {
    display: none;
  }
  .gated-report-status {
    margin: 0 0 18px;
    color: rgba(232,238,235,.74);
    font-size: 16px;
    line-height: 1.55;
  }
  .gated-report-status.error {
    color: var(--red-2);
  }
  .gated-report-button {
    min-height: 42px;
    border: 1px solid rgba(255,45,32,.72);
    color: var(--red-2);
    background: linear-gradient(90deg, rgba(255,45,32,.10), rgba(255,45,32,.025));
    padding: 0 16px;
    font: 900 11px var(--mono);
    letter-spacing: .14em;
    text-transform: uppercase;
    cursor: pointer;
  }
  .gated-report-body h3 {
    margin: 30px 0 10px;
    color: var(--text);
    font: 400 24px/1.18 var(--mono);
    letter-spacing: -.025em;
  }
  .gated-report-body table {
    width: 100%;
    border-collapse: collapse;
    margin: 24px 0 30px;
    font-size: 14px;
    line-height: 1.45;
  }
  .gated-report-body th,
  .gated-report-body td {
    border: 1px solid var(--line);
    padding: 10px;
    vertical-align: top;
  }
  .gated-report-body th {
    color: var(--text);
    background: rgba(255,255,255,.045);
    font: 900 11px var(--mono);
    letter-spacing: .12em;
    text-transform: uppercase;
  }
  .gated-report-body code {
    font-family: var(--mono);
    font-size: .88em;
    color: var(--amber);
  }
</style>

<div class="gated-report-panel" id="gated-report-panel">
  <p class="gated-report-status" id="gated-report-status">This report is available to Task Node users with an active wallet.</p>
  <button class="gated-report-button" id="gated-report-unlock" type="button">Unlock with Task Node</button>
</div>

<div class="gated-report-body" id="gated-report-body" hidden></div>

<script src="https://cdn.jsdelivr.net/npm/libsodium@0.8.1/dist/modules/libsodium.js"></script>
<script src="https://cdn.jsdelivr.net/npm/libsodium-wrappers@0.8.1/dist/modules/libsodium-wrappers.js"></script>
<script>
(function () {
  'use strict';

  var REPORT_ID = 'attention-intelligence-report-2026-04-30-deepseek-v4-pro-run-22';
  var TASKNODE_ORIGIN = 'https://tasknode.postfiat.org';
  var TASKNODE_API_BASE = TASKNODE_ORIGIN + '/api';
  var CONTENT_URL = '{{ "/gated-content/reports/attention-intelligence-report-2026-04-30-deepseek-v4-pro-run-22.encrypted.v1.json" | relative_url }}';
  var TOKEN_KEY = 'agti:gated-report:viewer-token:' + REPORT_ID;
  var STATE_KEY = 'agti:gated-report:state:' + REPORT_ID;

  var panelEl = document.getElementById('gated-report-panel');
  var statusEl = document.getElementById('gated-report-status');
  var unlockEl = document.getElementById('gated-report-unlock');
  var bodyEl = document.getElementById('gated-report-body');

  function setStatus(message, isError) {
    statusEl.textContent = message;
    statusEl.classList.toggle('error', Boolean(isError));
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

  function escapeHtml(value) {
    return String(value || '').replace(/[&<>"']/g, function (char) {
      return ({
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#39;'
      })[char];
    });
  }

  function inlineMarkdown(value) {
    return escapeHtml(value)
      .replace(/`([^`]+)`/g, '<code>$1</code>')
      .replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>')
      .replace(/\*([^*]+)\*/g, '<em>$1</em>');
  }

  function isTableSeparator(line) {
    return line.split('|').filter(Boolean).every(function (cell) {
      return /^:?-{3,}:?$/.test(cell.trim());
    });
  }

  function renderTable(rows) {
    var parsed = rows.map(function (line) {
      return line.replace(/^\||\|$/g, '').split('|').map(function (cell) {
        return cell.trim();
      });
    });
    var header = parsed[0] || [];
    var bodyRows = parsed.slice(1).filter(function (row) {
      return !isTableSeparator('|' + row.join('|') + '|');
    });
    var html = '<table><thead><tr>' + header.map(function (cell) {
      return '<th>' + inlineMarkdown(cell) + '</th>';
    }).join('') + '</tr></thead><tbody>';
    html += bodyRows.map(function (row) {
      return '<tr>' + row.map(function (cell) {
        return '<td>' + inlineMarkdown(cell) + '</td>';
      }).join('') + '</tr>';
    }).join('');
    return html + '</tbody></table>';
  }

  function startsBlock(line) {
    return /^#{2,3}\s/.test(line) || /^-\s+/.test(line) || /^>\s?/.test(line) || /^\|/.test(line);
  }

  function renderMarkdown(markdown) {
    var lines = String(markdown || '').split(/\r?\n/);
    var html = [];
    var i = 0;
    while (i < lines.length) {
      var line = lines[i];
      if (!line.trim()) {
        i += 1;
        continue;
      }
      if (line.indexOf('### ') === 0) {
        html.push('<h3>' + inlineMarkdown(line.slice(4)) + '</h3>');
        i += 1;
        continue;
      }
      if (line.indexOf('## ') === 0) {
        html.push('<h2>' + inlineMarkdown(line.slice(3)) + '</h2>');
        i += 1;
        continue;
      }
      if (line.indexOf('>') === 0) {
        var quote = [];
        while (i < lines.length && lines[i].indexOf('>') === 0) {
          quote.push(lines[i].replace(/^>\s?/, ''));
          i += 1;
        }
        html.push('<blockquote><p>' + inlineMarkdown(quote.join(' ')) + '</p></blockquote>');
        continue;
      }
      if (line.indexOf('- ') === 0) {
        var items = [];
        while (i < lines.length && lines[i].indexOf('- ') === 0) {
          items.push(lines[i].slice(2));
          i += 1;
        }
        html.push('<ul>' + items.map(function (item) {
          return '<li>' + inlineMarkdown(item) + '</li>';
        }).join('') + '</ul>');
        continue;
      }
      if (line.indexOf('|') === 0) {
        var tableRows = [];
        while (i < lines.length && lines[i].indexOf('|') === 0) {
          tableRows.push(lines[i]);
          i += 1;
        }
        html.push(renderTable(tableRows));
        continue;
      }
      var para = [];
      while (i < lines.length && lines[i].trim() && !startsBlock(lines[i])) {
        para.push(lines[i]);
        i += 1;
      }
      html.push('<p>' + inlineMarkdown(para.join(' ')) + '</p>');
    }
    return html.join('');
  }

  function sha256Hex(text) {
    return window.crypto.subtle.digest('SHA-256', new TextEncoder().encode(text)).then(function (digest) {
      return Array.from(new Uint8Array(digest)).map(function (byte) {
        return byte.toString(16).padStart(2, '0');
      }).join('');
    });
  }

  function fetchEncryptedBlob() {
    return fetch(CONTENT_URL, { cache: 'no-store' }).then(function (response) {
      if (!response.ok) {
        throw new Error('Encrypted report artifact unavailable.');
      }
      return response.json();
    });
  }

  function requestUnlock(token) {
    return fetch(TASKNODE_API_BASE + '/index-viewer/indices/' + encodeURIComponent(REPORT_ID) + '/unlock', {
      method: 'POST',
      headers: {
        Authorization: 'Bearer ' + token
      },
      cache: 'no-store'
    }).then(function (response) {
      return response.json().then(function (payload) {
        if (!response.ok) {
          throw new Error(payload && payload.error ? payload.error : 'Unable to unlock report.');
        }
        return payload;
      });
    });
  }

  function decryptReport(blob, unlockPayload) {
    return waitForSodium().then(function (libsodium) {
      var fromBase64 = function (value) {
        return libsodium.from_base64(value, libsodium.base64_variants.ORIGINAL);
      };
      var plaintextBytes = libsodium.crypto_aead_xchacha20poly1305_ietf_decrypt(
        null,
        fromBase64(blob.ciphertext),
        null,
        fromBase64(blob.nonce),
        fromBase64(unlockPayload.content_key)
      );
      return libsodium.to_string(plaintextBytes);
    });
  }

  function loadReport(token) {
    setStatus('Unlocking report...', false);
    return Promise.all([fetchEncryptedBlob(), requestUnlock(token)])
      .then(function (results) {
        var blob = results[0];
        var unlockPayload = results[1];
        return decryptReport(blob, unlockPayload).then(function (markdown) {
          return sha256Hex(markdown).then(function (hash) {
            if (blob.content_hash && hash !== blob.content_hash) {
              throw new Error('Report integrity check failed.');
            }
            bodyEl.innerHTML = renderMarkdown(markdown);
            bodyEl.hidden = false;
            panelEl.hidden = true;
          });
        });
      })
      .catch(function (error) {
        window.sessionStorage.removeItem(TOKEN_KEY);
        unlockEl.hidden = false;
        setStatus(error.message || 'Unable to unlock report.', true);
      });
  }

  function exchangeCode(code, state) {
    var expectedState = window.sessionStorage.getItem(STATE_KEY);
    if (expectedState && state && expectedState !== state) {
      return Promise.reject(new Error('Task Node authorization state mismatch.'));
    }
    return fetch(TASKNODE_API_BASE + '/index-viewer/exchange', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        code: code,
        index_id: REPORT_ID
      }),
      cache: 'no-store'
    }).then(function (response) {
      return response.json().then(function (payload) {
        if (!response.ok) {
          throw new Error(payload && payload.error ? payload.error : 'Unable to complete Task Node authorization.');
        }
        window.sessionStorage.setItem(TOKEN_KEY, payload.viewer_token);
        window.sessionStorage.removeItem(STATE_KEY);
        window.history.replaceState({}, document.title, cleanReportUrl().toString());
        return payload.viewer_token;
      });
    });
  }

  function boot() {
    unlockEl.addEventListener('click', startAuthorization);
    var params = new URLSearchParams(window.location.search);
    var code = params.get('index_viewer_code');
    var state = params.get('state');
    var indexId = params.get('index_id');
    if (code && (!indexId || indexId === REPORT_ID)) {
      unlockEl.hidden = true;
      setStatus('Completing Task Node authorization...', false);
      exchangeCode(code, state)
        .then(loadReport)
        .catch(function (error) {
          unlockEl.hidden = false;
          setStatus(error.message || 'Unable to complete Task Node authorization.', true);
        });
      return;
    }
    var existingToken = window.sessionStorage.getItem(TOKEN_KEY);
    if (existingToken) {
      unlockEl.hidden = true;
      loadReport(existingToken);
    }
  }

  boot();
}());
</script>
