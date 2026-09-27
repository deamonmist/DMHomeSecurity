// Made automatically by tools/pack_web.py - edit the files in web/ instead, then re-run it.
#pragma once

static const char WEB_INDEX_HTML[] = R"DMWEB(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>DMHomeSecurity</title>
<link rel="stylesheet" href="/app.css">
</head>
<body>
<header id="top"><h1 id="title">DMHomeSecurity</h1></header>

<!-- Main page: one large camera on top, the others below -->
<main id="grid">
  <div id="hero" class="slot hero"></div>
  <div id="row"></div>
  <p id="empty" hidden>No cameras enabled. Open settings (gear) → Manage cameras.</p>
</main>

<!-- One camera full screen, with its settings on the left -->
<section id="detail" hidden>
  <aside id="controls">
    <div class="aside-head">
      <h2 id="d-name">Camera</h2>
      <div id="d-meta" class="meta"></div>
    </div>
    <div id="d-body"></div>
  </aside>
  <div id="stage" class="slot stage">
    <button id="fs-btn" class="icon-btn" title="Full screen" aria-label="Full screen">⛶</button>
  </div>
  <div id="strip"></div>
  <button id="ctl-toggle" class="pill" aria-controls="controls">Adjust</button>
</section>

<button id="back" class="pill back" hidden>← Back</button>

<button id="gear" class="gear" title="Settings" aria-label="Settings">
  <svg viewBox="0 0 24 24" width="26" height="26" aria-hidden="true"><path fill="currentColor" d="M19.4 13a7.6 7.6 0 0 0 0-2l2.1-1.6-2-3.5-2.5 1a7.4 7.4 0 0 0-1.7-1L15 3.3h-4l-.4 2.6a7.4 7.4 0 0 0-1.7 1l-2.5-1-2 3.5L6.6 11a7.6 7.6 0 0 0 0 2l-2.1 1.6 2 3.5 2.5-1c.5.4 1.1.7 1.7 1l.4 2.6h4l.4-2.6c.6-.3 1.2-.6 1.7-1l2.5 1 2-3.5zM13 15.5a3.5 3.5 0 1 1 0-7 3.5 3.5 0 0 1 0 7z" transform="translate(-1 0)"/></svg>
</button>

<!-- Settings panel opened by the gear button -->
<aside id="config" hidden>
  <div class="cfg-head">
    <h2>Configuration</h2>
    <button id="cfg-cancel" class="pill ghost">Cancel</button>
  </div>
  <div id="cfg-body"></div>
</aside>
<button id="save" class="pill save" hidden>Save theme</button>

<div id="toast" role="status" aria-live="polite"></div>
<script src="/app.js"></script>
</body>
</html>
)DMWEB";

static const char WEB_APP_CSS[] = R"DMWEB(/* DMHomeSecurity website styles.
   The colours below are defaults; the website replaces them with the ones
   you choose in the gear menu. */
:root {
  --bg: #101114;
  --panel: #1a1c21;
  --text: #e8e6e3;
  --muted: color-mix(in srgb, var(--text) 55%, transparent);
  --accent: #e0a040;
  --tile-bg: #000000;
  --border-w: 2px;
  --border-c: #3a3d44;
  --radius: 10px;
  --gap: 14px;
  color-scheme: dark;
}
* { box-sizing: border-box; }
html, body { margin: 0; height: 100%; }
body {
  background: var(--bg);
  color: var(--text);
  font: 15px/1.4 system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
  -webkit-font-smoothing: antialiased;
}
button { font: inherit; color: inherit; cursor: pointer; }
input, select { font: inherit; }
[hidden] { display: none !important; }

/* ---------- header ---------- */
#top { padding: 18px 20px 6px; text-align: center; }
#title { margin: 0; font-size: clamp(22px, 4vw, 34px); font-weight: 650; letter-spacing: .01em; }

/* ---------- camera element (moved between slots, never recreated) ---------- */
.cam {
  position: relative; width: 100%; height: 100%;
  background: var(--tile-bg);
  border: var(--cam-border-w, var(--border-w)) solid var(--cam-border-c, var(--border-c));
  border-radius: var(--radius);
  overflow: hidden; cursor: pointer;
}
.cam img { display: block; width: 100%; height: 100%; object-fit: contain; }
.cam .label {
  position: absolute; left: 0; right: 0; bottom: 0;
  padding: 18px 12px 8px; font-weight: 600; font-size: 14px;
  background: linear-gradient(transparent, rgba(0,0,0,.65));
  color: var(--text); display: flex; align-items: center; gap: 8px;
  pointer-events: none;
}
.cam .dot { width: 8px; height: 8px; border-radius: 50%; background: #3c3; flex: none; }
.cam.offline .dot { background: #c33; }
.cam .state {
  position: absolute; inset: 0; display: none; place-items: center;
  color: var(--muted); font-size: 14px; letter-spacing: .05em; text-transform: uppercase;
}
.cam.offline .state, .cam.waiting .state { display: grid; }
.cam.offline img, .cam.waiting img { visibility: hidden; }

/* ---------- main page ---------- */
#grid { max-width: 1200px; margin: 0 auto; padding: 10px 20px 90px; display: grid; gap: var(--gap); }
.slot.hero { aspect-ratio: 4 / 3; max-height: 58vh; width: 100%; justify-self: center; }
.slot.hero .cam { width: 100%; }
@supports (aspect-ratio: 1) { .slot.hero { width: min(100%, calc(58vh * 4 / 3)); } }
#row { display: grid; gap: var(--gap); justify-content: center;
  grid-template-columns: repeat(auto-fit, minmax(min(100%, 200px), 340px)); }
#row .slot { aspect-ratio: 4 / 3; }
#empty { text-align: center; color: var(--muted); }

/* ---------- detail view ---------- */
#detail {
  position: fixed; inset: 0; background: var(--bg); z-index: 10;
  display: grid; grid-template-columns: 300px 1fr; grid-template-rows: 1fr auto;
  grid-template-areas: "ctl stage" "ctl strip";
}
#controls {
  grid-area: ctl; overflow-y: auto; background: var(--panel);
  padding: 64px 16px 24px; border-right: 1px solid color-mix(in srgb, var(--text) 12%, transparent);
}
.aside-head h2 { margin: 0 0 4px; font-size: 20px; }
.meta { color: var(--muted); font-size: 12px; margin-bottom: 12px; overflow-wrap: anywhere; }
#stage { grid-area: stage; position: relative; padding: 16px 16px 8px; min-height: 0; }
#stage .cam { cursor: default; }
#strip { grid-area: strip; display: flex; gap: 12px; padding: 8px 16px 16px; overflow-x: auto; }
#strip .slot { flex: 0 0 auto; width: clamp(96px, 14vw, 150px); aspect-ratio: 1; }
#strip .cam img { object-fit: cover; }
#strip .cam .label { font-size: 12px; padding: 12px 8px 6px; }
#fs-btn { position: absolute; top: 26px; right: 26px; z-index: 2; }
#ctl-toggle { display: none; }

/* ---------- controls ---------- */
.group { margin: 18px 0 6px; font-size: 11px; font-weight: 700; letter-spacing: .08em; text-transform: uppercase; color: var(--accent); }
.ctl { display: grid; grid-template-columns: 1fr auto; align-items: center; gap: 4px 10px; margin: 8px 0; font-size: 14px; }
.ctl input[type=range] { grid-column: 1 / -1; width: 100%; accent-color: var(--accent); }
.ctl .val { color: var(--muted); font-variant-numeric: tabular-nums; font-size: 12px; }
.ctl select, .field input, .field select, .camrow input[type=text], .camrow select {
  background: var(--bg); color: var(--text); border: 1px solid color-mix(in srgb, var(--text) 20%, transparent);
  border-radius: 6px; padding: 5px 8px; max-width: 170px;
}
.switch { position: relative; width: 38px; height: 22px; flex: none; }
.switch input { opacity: 0; width: 0; height: 0; position: absolute; }
.switch span { position: absolute; inset: 0; border-radius: 11px; background: color-mix(in srgb, var(--text) 25%, transparent); transition: .15s; }
.switch span::after { content: ""; position: absolute; width: 16px; height: 16px; left: 3px; top: 3px; border-radius: 50%; background: #fff; transition: .15s; }
.switch input:checked + span { background: var(--accent); }
.switch input:checked + span::after { transform: translateX(16px); }
.btn-row { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 18px; }
.note { color: var(--muted); font-size: 12px; margin: 8px 0; }

/* ---------- buttons ---------- */
.pill {
  border: 1px solid color-mix(in srgb, var(--text) 25%, transparent); background: var(--panel);
  border-radius: 999px; padding: 7px 14px; font-weight: 600; font-size: 14px;
}
.pill:hover { border-color: var(--accent); }
.pill.ghost { background: transparent; }
.pill.danger:hover { border-color: #d44; color: #f77; }
.icon-btn { width: 36px; height: 36px; border-radius: 8px; border: 0; background: rgba(0,0,0,.5); color: #fff; font-size: 18px; }
.back { position: fixed; top: 14px; left: 14px; z-index: 30; }
.gear {
  position: fixed; right: 18px; bottom: 18px; z-index: 20; width: 54px; height: 54px; border-radius: 50%;
  border: 1px solid color-mix(in srgb, var(--text) 20%, transparent); background: var(--panel); color: var(--text);
  display: grid; place-items: center; box-shadow: 0 4px 18px rgba(0,0,0,.35); transition: transform .3s;
}
.gear:hover { transform: rotate(45deg); color: var(--accent); }
body.configuring .gear, body.detail-open .gear { display: none; }
.save {
  position: fixed; right: 18px; bottom: 18px; z-index: 40; background: var(--accent); color: #111;
  border-color: transparent; padding: 12px 22px; font-size: 15px; box-shadow: 0 4px 18px rgba(0,0,0,.4);
  animation: pop .2s ease-out;
}
@keyframes pop { from { transform: scale(.7); opacity: 0; } }

/* ---------- configuration drawer ---------- */
#config {
  position: fixed; top: 0; right: 0; bottom: 0; width: min(380px, 100%); z-index: 35;
  background: var(--panel); overflow-y: auto; padding: 16px 16px 90px;
  border-left: 1px solid color-mix(in srgb, var(--text) 12%, transparent); box-shadow: -8px 0 30px rgba(0,0,0,.35);
}
@media (min-width: 900px) { body.configuring #grid, body.configuring #top { margin-right: 380px; } }
.cfg-head { display: flex; align-items: center; justify-content: space-between; }
.cfg-head h2 { margin: 0; font-size: 20px; }
.field { display: flex; align-items: center; justify-content: space-between; gap: 10px; margin: 10px 0; font-size: 14px; }
.field input[type=text] { max-width: 200px; width: 100%; }
.field input[type=range] { width: 140px; accent-color: var(--accent); }
.camrow {
  border: 1px solid color-mix(in srgb, var(--text) 14%, transparent); border-radius: 8px;
  padding: 8px 10px; margin: 8px 0; display: grid; gap: 6px;
}
.camrow .top { display: flex; gap: 6px; align-items: center; }
.camrow .top input[type=text] { flex: 1; min-width: 0; }
.camrow .sub { display: flex; gap: 10px; align-items: center; justify-content: space-between; font-size: 12px; color: var(--muted); }
.camrow .sub .left { display: flex; gap: 10px; align-items: center; }
.mini { padding: 2px 8px; font-size: 12px; border-radius: 6px; background: transparent; border: 1px solid color-mix(in srgb, var(--text) 20%, transparent); }
.mini:disabled { opacity: .3; cursor: default; }
.badge { font-size: 10px; padding: 1px 6px; border-radius: 4px; background: color-mix(in srgb, var(--text) 12%, transparent); text-transform: uppercase; letter-spacing: .05em; }

/* ---------- color picker ---------- */
.swatch { width: 34px; height: 24px; border-radius: 6px; border: 1px solid color-mix(in srgb, var(--text) 35%, transparent); padding: 0; }
.swatch.none { background: repeating-linear-gradient(45deg, transparent 0 4px, color-mix(in srgb, var(--text) 30%, transparent) 4px 6px) !important; }
.picker {
  position: fixed; z-index: 100; background: var(--panel); border-radius: 12px; padding: 12px; width: 220px;
  border: 1px solid color-mix(in srgb, var(--text) 18%, transparent); box-shadow: 0 10px 40px rgba(0,0,0,.5);
}
.picker canvas { display: block; width: 196px; height: 196px; cursor: crosshair; touch-action: none; }
.picker input[type=range] { width: 100%; margin: 10px 0 6px; }
.picker .prow { display: flex; gap: 8px; align-items: center; }
.picker .prow input { flex: 1; min-width: 0; background: var(--bg); color: var(--text); border: 1px solid color-mix(in srgb, var(--text) 20%, transparent); border-radius: 6px; padding: 4px 6px; font-family: ui-monospace, monospace; }
.picker .preview { width: 28px; height: 28px; border-radius: 6px; border: 1px solid rgba(255,255,255,.3); }

#toast {
  position: fixed; left: 50%; bottom: 24px; transform: translateX(-50%) translateY(20px); opacity: 0;
  background: var(--panel); border: 1px solid color-mix(in srgb, var(--text) 20%, transparent);
  padding: 8px 16px; border-radius: 999px; transition: .2s; pointer-events: none; z-index: 200;
}
#toast.show { opacity: 1; transform: translateX(-50%); }

/* ---------- phone ---------- */
@media (max-width: 760px) {
  #grid { padding: 8px 12px 90px; }
  #detail { grid-template-columns: 1fr; grid-template-rows: 1fr auto; grid-template-areas: "stage" "strip"; }
  #controls {
    position: fixed; left: 0; right: 0; bottom: 0; max-height: 62vh; z-index: 15; padding-top: 16px;
    border-right: 0; border-top: 1px solid color-mix(in srgb, var(--text) 15%, transparent);
    border-radius: 14px 14px 0 0; transform: translateY(100%); transition: transform .2s;
  }
  #detail.ctl-open #controls { transform: none; }
  #stage { padding: 64px 12px 8px; }
  #fs-btn { top: 74px; right: 22px; }
  #ctl-toggle { display: block; position: fixed; top: 14px; right: 14px; z-index: 30; }
}
)DMWEB";

static const char WEB_APP_JS[] = R"DMWEB(/* DMHomeSecurity website by Deamonmist - sent to your browser by the master camera.
 *
 * Where the data comes from:
 *   /api/cams    list of cameras (from the master)
 *   /api/theme   title and colours (from the master)
 *   each camera's own /stream, /api/status and /api/control
 *
 * Each camera's video is opened once and simply moved around the page when
 * you switch views, so switching is instant and doesn't open extra streams.
 */
'use strict';

const $ = (s, el = document) => el.querySelector(s);
const h = (tag, props = {}, ...kids) => {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(props)) {
    if (k === 'class') e.className = v;
    else if (k === 'style') e.style.cssText = v;
    else if (k.startsWith('on')) e.addEventListener(k.slice(2), v);
    else if (v === true) e.setAttribute(k, '');
    else if (v !== false && v != null) e.setAttribute(k, v);
  }
  for (const k of kids.flat()) if (k != null) e.append(k.nodeType ? k : document.createTextNode(k));
  return e;
};
const clone = o => JSON.parse(JSON.stringify(o));
const BLANK = 'data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==';

/* ---------------------------------------------------------------- theme */
const DEFAULT_THEME = {
  title: 'DMHomeSecurity',
  bg: '#101114', panel: '#1a1c21', text: '#e8e6e3', accent: '#e0a040', tileBg: '#000000',
  border: { on: true, width: 2, radius: 10, color: '#3a3d44' },
  camBorders: {},        // optional border colour for individual cameras
  featured: null,        // which camera is large on the main page (null = master)
};
function mergeTheme(t) {
  const m = Object.assign(clone(DEFAULT_THEME), t || {});
  m.border = Object.assign(clone(DEFAULT_THEME.border), (t && t.border) || {});
  m.camBorders = Object.assign({}, (t && t.camBorders) || {});
  return m;
}
function applyTheme(t) {
  const r = document.documentElement.style;
  r.setProperty('--bg', t.bg);
  r.setProperty('--panel', t.panel);
  r.setProperty('--text', t.text);
  r.setProperty('--accent', t.accent);
  r.setProperty('--tile-bg', t.tileBg);
  r.setProperty('--border-w', t.border.on ? t.border.width + 'px' : '0px');
  r.setProperty('--border-c', t.border.color);
  r.setProperty('--radius', t.border.radius + 'px');
  $('#title').textContent = t.title;
  document.title = t.title;
  for (const c of S.cams) {
    const el = camEl(c);
    const override = t.camBorders[c.id];
    if (override) el.style.setProperty('--cam-border-c', override);
    else el.style.removeProperty('--cam-border-c');
  }
}

/* ---------------------------------------------------------------- state */
const S = {
  self: null,
  cams: [],               // from /api/cams
  theme: mergeTheme({}),
  view: 'grid',           // 'grid' | 'detail'
  detailId: null,
  config: null,           // null, or { theme, cams } drafts while configuring
};
const els = new Map();    // camId -> { root, img, name, retry, src }

const streamUrl = c => c.kind === 'local' ? '/stream' : c.kind === 'node' ? c.url + '/stream' : c.url;
const apiBase   = c => c.kind === 'local' ? '' : c.kind === 'node' ? c.url : null;
const visibleCams = () => S.cams.filter(c => c.enabled);
const byId = id => S.cams.find(c => c.id === id);

/* ---------------------------------------------------------- camera tiles */
function camEl(c) {
  let e = els.get(c.id);
  if (!e) {
    const img = h('img', { alt: '', decoding: 'async' });
    const name = h('span');
    const root = h('div', { class: 'cam waiting', 'data-id': c.id },
      img, h('div', { class: 'state' }, 'Connecting…'),
      h('div', { class: 'label' }, h('span', { class: 'dot' }), name));
    e = { root, img, name, retry: null, src: null };
    img.addEventListener('load', () => { if (img.src !== BLANK) root.classList.remove('waiting', 'offline'); });
    img.addEventListener('error', () => {
      if (!e.src) return;
      markOffline(e, true);
      clearTimeout(e.retry);
      e.retry = setTimeout(() => { if (e.src) startStream(c.id, true); }, 4000);
    });
    root.addEventListener('click', () => onTileClick(c.id));
    els.set(c.id, e);
  }
  e.name.textContent = c.name;
  return e.root;
}
function markOffline(e, off) {
  e.root.classList.toggle('offline', off);
  e.root.classList.remove('waiting');
  $('.state', e.root).textContent = off ? 'Offline' : 'Connecting…';
}
function startStream(id, force) {
  const c = byId(id), e = els.get(id);
  if (!c || !e) return;
  if (c.kind === 'node' && !c.online) { stopStream(id); markOffline(e, true); return; }
  const url = streamUrl(c);
  if (!force && e.src === url) return;
  e.src = url;
  e.root.classList.add('waiting');
  e.root.classList.remove('offline');
  $('.state', e.root).textContent = 'Connecting…';
  e.img.src = url + (url.includes('?') ? '&' : '?') + '_=' + Date.now();
}
function stopStream(id) {
  const e = els.get(id);
  if (!e) return;
  clearTimeout(e.retry);
  e.src = null;
  e.img.src = BLANK;               // closes the MJPEG connection
}
function syncStreams() {
  const live = document.visibilityState === 'visible';
  const want = new Set(live ? visibleCams().map(c => c.id) : []);
  for (const id of els.keys()) if (!want.has(id)) stopStream(id);
  for (const id of want) startStream(id, false);
}

/* ----------------------------------------------------------------- views */
function featuredId() {
  const vis = visibleCams();
  const f = S.theme.featured;
  if (f && vis.some(c => c.id === f)) return f;
  if (vis.some(c => c.id === S.self)) return S.self;
  return vis.length ? vis[0].id : null;
}
function slot(child) { return h('div', { class: 'slot' }, child); }

function render() {
  const vis = visibleCams();
  const inDetail = S.view === 'detail' && vis.some(c => c.id === S.detailId);
  if (S.view === 'detail' && !inDetail) S.view = 'grid';

  $('#grid').hidden = inDetail;
  $('#top').hidden = inDetail;
  $('#detail').hidden = !inDetail;
  $('#back').hidden = !inDetail;
  document.body.classList.toggle('detail-open', inDetail);

  if (inDetail) {
    const cur = byId(S.detailId);
    const stage = $('#stage');
    [...stage.querySelectorAll('.cam')].forEach(n => n.remove());
    stage.append(camEl(cur));
    const strip = $('#strip');
    strip.replaceChildren(...vis.filter(c => c.id !== cur.id).map(c => slot(camEl(c))));
  } else {
    const fid = featuredId();
    $('#hero').replaceChildren(...(fid ? [camEl(byId(fid))] : []));
    $('#row').replaceChildren(...vis.filter(c => c.id !== fid).map(c => slot(camEl(c))));
    $('#empty').hidden = vis.length > 0;
  }
  // hide cameras that are switched off
  for (const [id, e] of els) if (!vis.some(c => c.id === id)) e.root.remove();
  for (const c of S.cams) if (c.kind === 'node') camEl(c).classList.toggle('offline', !c.online);
  applyTheme(S.config ? S.config.theme : S.theme);
  syncStreams();
}

function onTileClick(id) {
  if (S.view === 'detail' && S.detailId === id) return;
  openDetail(id);
}
function openDetail(id) {
  S.view = 'detail';
  S.detailId = id;
  $('#detail').classList.remove('ctl-open');
  history.replaceState(null, '', '#cam=' + encodeURIComponent(id));
  render();
  loadControls(id);
}
function closeDetail() {
  S.view = 'grid';
  history.replaceState(null, '', location.pathname);
  if (document.fullscreenElement) document.exitFullscreen();
  render();
}

/* -------------------------------------------------------- camera controls */
// Labels for the camera settings panel. Any setting the camera reports that
// isn't listed here still appears, as a simple slider.
const CONTROLS = [
  ['Image'],
  { k: 'framesize', l: 'Resolution', t: 'framesize' },
  { k: 'fps', l: 'Max frame rate', t: 'range', unit: ' fps' },
  { k: 'quality', l: 'JPEG compression', t: 'range', hint: 'lower = sharper, bigger' },
  { k: 'brightness', l: 'Brightness', t: 'range' },
  { k: 'contrast', l: 'Contrast', t: 'range' },
  { k: 'saturation', l: 'Saturation', t: 'range' },
  { k: 'sharpness', l: 'Sharpness', t: 'range' },
  { k: 'special_effect', l: 'Effect', t: 'select', o: ['None', 'Negative', 'Grayscale', 'Red tint', 'Green tint', 'Blue tint', 'Sepia'] },
  { k: 'hmirror', l: 'Mirror', t: 'toggle' },
  { k: 'vflip', l: 'Flip vertical', t: 'toggle' },
  ['Exposure'],
  { k: 'aec', l: 'Auto exposure', t: 'toggle' },
  { k: 'aec2', l: 'AEC DSP', t: 'toggle' },
  { k: 'ae_level', l: 'Exposure level', t: 'range', show: s => s.aec },
  { k: 'aec_value', l: 'Exposure', t: 'range', show: s => !s.aec },
  { k: 'agc', l: 'Auto gain', t: 'toggle' },
  { k: 'gainceiling', l: 'Gain ceiling', t: 'select', o: ['2×', '4×', '8×', '16×', '32×', '64×', '128×'], show: s => s.agc },
  { k: 'agc_gain', l: 'Gain', t: 'range', show: s => !s.agc },
  ['White balance'],
  { k: 'awb', l: 'Auto white balance', t: 'toggle' },
  { k: 'awb_gain', l: 'AWB gain', t: 'toggle' },
  { k: 'wb_mode', l: 'WB mode', t: 'select', o: ['Auto', 'Sunny', 'Cloudy', 'Office', 'Home'], show: s => s.awb_gain },
  ['Advanced'],
  { k: 'bpc', l: 'Black pixel correction', t: 'toggle' },
  { k: 'wpc', l: 'White pixel correction', t: 'toggle' },
  { k: 'raw_gma', l: 'Raw gamma', t: 'toggle' },
  { k: 'lenc', l: 'Lens correction', t: 'toggle' },
  { k: 'dcw', l: 'Downsize (DCW)', t: 'toggle' },
  { k: 'colorbar', l: 'Test pattern', t: 'toggle' },
];

let ctlToken = 0;
async function loadControls(id) {
  const c = byId(id);
  const token = ++ctlToken;
  $('#d-name').textContent = c.name;
  $('#d-meta').textContent = '';
  const body = $('#d-body');
  const base = apiBase(c);
  if (base === null) {
    body.replaceChildren(h('p', { class: 'note' }, 'External stream — no camera controls available.'),
      h('p', { class: 'meta' }, c.url));
    return;
  }
  body.replaceChildren(h('p', { class: 'note' }, 'Loading settings…'));
  let st;
  try {
    const r = await fetch(base + '/api/status', { cache: 'no-store' });
    st = await r.json();
  } catch (err) {
    if (token === ctlToken) body.replaceChildren(h('p', { class: 'note' }, 'Camera not reachable.'),
      h('div', { class: 'btn-row' }, h('button', { class: 'pill', onclick: () => loadControls(id) }, 'Retry')));
    return;
  }
  if (token !== ctlToken) return;
  $('#d-meta').textContent = `${st.sensor} · ${st.ip} · ${st.fps} fps · ${st.rssi} dBm · ${st.id}`;
  buildControls(c, base, st);
}

function buildControls(c, base, st) {
  const s = st.settings, ranges = st.ranges || {};
  const body = $('#d-body');
  const nodes = [];
  const rows = [];            // [{def, el}] for show() re-evaluation
  const known = new Set(CONTROLS.filter(d => d.k).map(d => d.k));
  const defs = [...CONTROLS, ...Object.keys(s).filter(k => !known.has(k)).map(k => ({ k, l: k, t: 'range' }))];

  const send = async (k, v) => {
    s[k] = v;
    rows.forEach(r => { if (r.def.show) r.el.hidden = !r.def.show(s); });
    try {
      const r = await fetch(`${base}/api/control?var=${encodeURIComponent(k)}&val=${v}`);
      if (!r.ok) throw 0;
    } catch { toast('Setting not accepted'); }
  };

  let pendingGroup = null;
  for (const d of defs) {
    if (Array.isArray(d)) { pendingGroup = d[0]; continue; }
    if (!(d.k in s)) continue;
    if (pendingGroup) { nodes.push(h('div', { class: 'group' }, pendingGroup)); pendingGroup = null; }
    const [lo, hi] = ranges[d.k] || [0, 1];
    let el;
    if (d.t === 'toggle') {
      const cb = h('input', { type: 'checkbox' }); cb.checked = !!s[d.k];
      cb.addEventListener('change', () => send(d.k, cb.checked ? 1 : 0));
      el = h('label', { class: 'ctl' }, h('span', {}, d.l), h('span', { class: 'switch' }, cb, h('span')));
    } else if (d.t === 'select' || d.t === 'framesize') {
      const opts = d.t === 'framesize' ? st.framesizes : d.o;
      const sel = h('select', {}, ...opts.map((o, i) => h('option', { value: i }, o)).slice(lo, hi + 1));
      sel.value = s[d.k];
      sel.addEventListener('change', () => send(d.k, +sel.value));
      el = h('label', { class: 'ctl' }, h('span', {}, d.l), sel);
    } else {
      const val = h('span', { class: 'val' }, s[d.k] + (d.unit || ''));
      const rg = h('input', { type: 'range', min: lo, max: hi, step: 1, value: s[d.k] });
      rg.addEventListener('input', () => { val.textContent = rg.value + (d.unit || ''); });
      rg.addEventListener('change', () => send(d.k, +rg.value));
      el = h('label', { class: 'ctl', title: d.hint || '' }, h('span', {}, d.l), val, rg);
    }
    if (d.show) el.hidden = !d.show(s);
    rows.push({ def: d, el });
    nodes.push(el);
  }

  nodes.push(h('div', { class: 'btn-row' },
    h('button', { class: 'pill', onclick: () => window.open(base + '/capture', '_blank') }, 'Snapshot'),
    h('button', { class: 'pill', onclick: async () => {
      await fetch(base + '/api/control?var=reset').catch(() => {});
      toast('Defaults restored'); loadControls(c.id);
    } }, 'Reset'),
    h('button', { class: 'pill danger', onclick: async () => {
      await fetch(base + '/api/reboot', { method: 'POST' }).catch(() => {});
      toast('Rebooting ' + c.name + '…');
    } }, 'Reboot'),
  ));
  nodes.push(h('p', { class: 'note' }, 'Changes apply live and are saved on the camera.'));
  body.replaceChildren(...nodes);
}

/* ------------------------------------------------------------ color wheel */
const hsv2rgb = (hh, s, v) => {
  const f = n => { const k = (n + hh / 60) % 6; return v - v * s * Math.max(0, Math.min(k, 4 - k, 1)); };
  return [f(5), f(3), f(1)].map(x => Math.round(x * 255));
};
const rgb2hsv = (r, g, b) => {
  r /= 255; g /= 255; b /= 255;
  const mx = Math.max(r, g, b), mn = Math.min(r, g, b), d = mx - mn;
  let hh = 0;
  if (d) hh = mx === r ? ((g - b) / d) % 6 : mx === g ? (b - r) / d + 2 : (r - g) / d + 4;
  return [(hh * 60 + 360) % 360, mx ? d / mx : 0, mx];
};
const hex2rgb = x => { const m = /^#?([0-9a-f]{6})$/i.exec(x || ''); if (!m) return null; const n = parseInt(m[1], 16); return [n >> 16, (n >> 8) & 255, n & 255]; };
const rgb2hex = ([r, g, b]) => '#' + [r, g, b].map(v => v.toString(16).padStart(2, '0')).join('');

let openPicker = null;
function closePicker() { if (openPicker) { openPicker.remove(); openPicker = null; } }
document.addEventListener('pointerdown', e => {
  if (openPicker && !openPicker.contains(e.target) && !e.target.classList.contains('swatch')) closePicker();
});

// A colour button that opens a colour wheel. allowNone adds a "None" button.
function colorInput(value, onChange, allowNone = false) {
  const sw = h('button', { class: 'swatch', type: 'button', title: 'Pick colour' });
  const paint = v => { sw.style.background = v || ''; sw.classList.toggle('none', !v); };
  paint(value);
  sw.addEventListener('click', () => {
    if (openPicker) { closePicker(); return; }
    let [hh, s, v] = rgb2hsv(...(hex2rgb(value) || [128, 128, 128]));
    const SIZE = 196, R = SIZE / 2;
    const cv = h('canvas', { width: SIZE * 2, height: SIZE * 2 });
    const ctx = cv.getContext('2d');
    const bright = h('input', { type: 'range', min: 0, max: 100, value: Math.round(v * 100) });
    const hex = h('input', { type: 'text', maxlength: 7, spellcheck: 'false' });
    const prev = h('div', { class: 'preview' });
    const none = allowNone ? h('button', { class: 'mini', type: 'button', onclick: () => { value = null; paint(null); onChange(null); closePicker(); } }, 'None') : null;

    const drawWheel = () => {
      const S2 = SIZE * 2, img = ctx.createImageData(S2, S2), d = img.data;
      for (let y = 0; y < S2; y++) for (let x = 0; x < S2; x++) {
        const dx = x / 2 - R, dy = y / 2 - R, dist = Math.hypot(dx, dy), i = (y * S2 + x) * 4;
        if (dist > R) { d[i + 3] = 0; continue; }
        const [r, g, b] = hsv2rgb((Math.atan2(dy, dx) * 180 / Math.PI + 360) % 360, dist / R, v);
        d[i] = r; d[i + 1] = g; d[i + 2] = b; d[i + 3] = dist > R - 1 ? 128 : 255;
      }
      ctx.putImageData(img, 0, 0);
      const a = hh * Math.PI / 180, px = (R + Math.cos(a) * s * R) * 2, py = (R + Math.sin(a) * s * R) * 2;
      ctx.lineWidth = 4; ctx.strokeStyle = v > .5 ? '#000' : '#fff';
      ctx.beginPath(); ctx.arc(px, py, 12, 0, 7); ctx.stroke();
    };
    const commit = (fromHex) => {
      const hx = rgb2hex(hsv2rgb(hh, s, v));
      if (!fromHex) hex.value = hx;
      prev.style.background = hx; value = hx; paint(hx); onChange(hx);
    };
    const pick = e => {
      const r = cv.getBoundingClientRect();
      const dx = e.clientX - r.left - R, dy = e.clientY - r.top - R;
      hh = (Math.atan2(dy, dx) * 180 / Math.PI + 360) % 360;
      s = Math.min(1, Math.hypot(dx, dy) / R);
      drawWheel(); commit();
    };
    cv.addEventListener('pointerdown', e => { cv.setPointerCapture(e.pointerId); pick(e); });
    cv.addEventListener('pointermove', e => { if (e.buttons) pick(e); });
    bright.addEventListener('input', () => { v = bright.value / 100; drawWheel(); commit(); });
    hex.addEventListener('input', () => {
      const rgb = hex2rgb(hex.value);
      if (!rgb) return;
      [hh, s, v] = rgb2hsv(...rgb); bright.value = Math.round(v * 100); drawWheel(); commit(true);
    });

    const pop = h('div', { class: 'picker' }, cv, bright, h('div', { class: 'prow' }, prev, hex, none));
    document.body.append(pop);
    const r = sw.getBoundingClientRect();
    pop.style.left = Math.max(8, Math.min(innerWidth - 236, r.left - 100)) + 'px';
    pop.style.top = (r.bottom + 300 < innerHeight ? r.bottom + 6 : Math.max(8, r.top - 300)) + 'px';
    openPicker = pop;
    hex.value = rgb2hex(hsv2rgb(hh, s, v)); prev.style.background = hex.value;
    drawWheel();
  });
  return sw;
}

/* ----------------------------------------------------- configuration mode */
function enterConfig() {
  if (S.view === 'detail') closeDetail();
  S.config = { theme: clone(S.theme), cams: clone(S.cams) };
  document.body.classList.add('configuring');
  $('#config').hidden = false;
  $('#save').hidden = false;
  buildConfig();
}
function exitConfig() {
  closePicker();
  S.config = null;
  document.body.classList.remove('configuring');
  $('#config').hidden = true;
  $('#save').hidden = true;
  render();
}
// Show your changes on the page right away, before you press Save.
function previewDraft() {
  const d = S.config;
  const saved = S.cams;
  S.cams = d.cams.map(dc => Object.assign({}, saved.find(c => c.id === dc.id) || { online: true }, dc));
  const t = S.theme; S.theme = d.theme;
  render();
  S.theme = t; S.cams = saved;
}

function field(label, control) { return h('label', { class: 'field' }, h('span', {}, label), control); }
function textInput(value, onInput, attrs = {}) {
  const i = h('input', Object.assign({ type: 'text', value }, attrs));
  i.addEventListener('input', () => onInput(i.value));
  return i;
}
function toggle(checked, onChange) {
  const cb = h('input', { type: 'checkbox' }); cb.checked = checked;
  cb.addEventListener('change', () => onChange(cb.checked));
  return h('span', { class: 'switch' }, cb, h('span'));
}
function slider(value, min, max, onInput) {
  const r = h('input', { type: 'range', min, max, value });
  r.addEventListener('input', () => onInput(+r.value));
  return r;
}

function buildConfig() {
  const d = S.config, t = d.theme;
  const upd = () => previewDraft();
  const body = $('#cfg-body');

  const site = [
    h('div', { class: 'group' }, 'Page'),
    field('Title', textInput(t.title, v => { t.title = v; upd(); }, { maxlength: 60 })),
    field('Background', colorInput(t.bg, v => { t.bg = v; upd(); })),
    field('Panels', colorInput(t.panel, v => { t.panel = v; upd(); })),
    field('Text', colorInput(t.text, v => { t.text = v; upd(); })),
    field('Accent', colorInput(t.accent, v => { t.accent = v; upd(); })),
    field('Tile background', colorInput(t.tileBg, v => { t.tileBg = v; upd(); })),
    h('div', { class: 'group' }, 'Viewport borders'),
    field('Show borders', toggle(t.border.on, v => { t.border.on = v; upd(); })),
    field('Width', slider(t.border.width, 1, 12, v => { t.border.width = v; upd(); })),
    field('Corner radius', slider(t.border.radius, 0, 32, v => { t.border.radius = v; upd(); })),
    field('Border colour', colorInput(t.border.color, v => { t.border.color = v; upd(); })),
  ];

  const list = h('div');
  const drawList = () => {
    list.replaceChildren(...d.cams.map((c, i) => {
      const live = S.cams.find(x => x.id === c.id);
      const status = c.kind === 'local' ? 'this camera' : c.kind === 'mjpeg' ? 'external'
        : live && live.online ? 'online' : 'offline';
      const feat = h('input', { type: 'radio', name: 'featured', title: 'Show large on main page' });
      feat.checked = (t.featured || S.self) === c.id;
      feat.addEventListener('change', () => { t.featured = c.id; upd(); });
      const move = (dir) => { const j = i + dir; [d.cams[i], d.cams[j]] = [d.cams[j], d.cams[i]]; drawList(); upd(); };
      return h('div', { class: 'camrow' },
        h('div', { class: 'top' },
          textInput(c.name, v => { c.name = v; upd(); }, { maxlength: 40, 'aria-label': 'Camera name' }),
          colorInput(t.camBorders[c.id] || null, v => { if (v) t.camBorders[c.id] = v; else delete t.camBorders[c.id]; upd(); }, true),
          toggle(c.enabled, v => { c.enabled = v; upd(); })),
        h('div', { class: 'sub' },
          h('span', { class: 'left' },
            h('label', { title: 'Featured (large tile)' }, feat, ' Large'),
            h('span', { class: 'badge' }, status),
            c.kind === 'local' ? null : h('span', {}, (c.url || '').replace(/^http:\/\//, ''))),
          h('span', { class: 'left' },
            h('button', { class: 'mini', type: 'button', disabled: i === 0, onclick: () => move(-1) }, '↑'),
            h('button', { class: 'mini', type: 'button', disabled: i === d.cams.length - 1, onclick: () => move(1) }, '↓'),
            c.kind === 'local' ? null : h('button', { class: 'mini', type: 'button', title: 'Remove',
              onclick: () => { d.cams.splice(i, 1); delete t.camBorders[c.id]; drawList(); upd(); } }, '✕'))));
    }));
  };
  drawList();

  // "Add a camera" form
  const addName = h('input', { type: 'text', placeholder: 'Name' });
  const addType = h('select', {}, h('option', { value: 'node' }, 'DMHomeSecurity camera (IP)'), h('option', { value: 'mjpeg' }, 'Other MJPEG URL'));
  const addUrl = h('input', { type: 'text', placeholder: '192.168.1.60' });
  addType.addEventListener('change', () => { addUrl.placeholder = addType.value === 'node' ? '192.168.1.60' : 'http://host/stream'; });
  const add = () => {
    let u = addUrl.value.trim();
    if (!u) return toast('Enter an address');
    if (!/^https?:\/\//.test(u)) u = 'http://' + u;
    if (addType.value === 'node') u = u.replace(/\/+$/, '');
    d.cams.push({ id: 'new-' + Date.now(), name: addName.value.trim() || 'Camera ' + (d.cams.length + 1), url: u, kind: addType.value, enabled: true });
    addName.value = addUrl.value = '';
    drawList(); upd();
  };

  body.replaceChildren(
    ...site,
    h('div', { class: 'group' }, 'Manage cameras'),
    h('p', { class: 'note' }, 'Rename, reorder, hide (switch) or remove cameras. The swatch sets a per-camera border colour. DMHomeSecurity slave cameras add themselves automatically; a removed slave that is still powered will reappear — hide it instead.'),
    list,
    h('div', { class: 'camrow' },
      h('div', { class: 'top' }, addName, addType),
      h('div', { class: 'top' }, addUrl, h('button', { class: 'pill', type: 'button', onclick: add }, 'Add'))),
    h('div', { class: 'btn-row' },
      h('button', { class: 'pill ghost', type: 'button', onclick: () => {
        S.config.theme = mergeTheme({ title: t.title }); buildConfig(); upd();
      } }, 'Reset colours')),
  );
  upd();
}

async function saveConfig() {
  const d = S.config;
  const btn = $('#save');
  btn.disabled = true; btn.textContent = 'Saving…';
  try {
    const [rt, rc] = await Promise.all([
      fetch('/api/theme', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(d.theme) }),
      fetch('/api/cams', { method: 'POST', headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(d.cams.map(({ id, name, url, kind, enabled }) =>
          ({ id: id.startsWith('new-') ? '' : id, name, url, kind, enabled }))) }),
    ]);
    if (!rt.ok || !rc.ok) throw new Error('save failed');
    const cams = await rc.json();
    S.theme = mergeTheme(d.theme);
    S.self = cams.self; S.cams = cams.cams;
    exitConfig();
    toast('Theme saved');
  } catch (e) {
    toast('Save failed — is the master reachable?');
  } finally {
    btn.disabled = false; btn.textContent = 'Save theme';
  }
}

/* ------------------------------------------------------------------ misc */
let toastT;
function toast(msg) {
  const t = $('#toast');
  t.textContent = msg; t.classList.add('show');
  clearTimeout(toastT); toastT = setTimeout(() => t.classList.remove('show'), 2200);
}

async function refreshCams() {
  if (S.config) return;                 // don't clobber drafts
  try {
    const r = await fetch('/api/cams', { cache: 'no-store' });
    const j = await r.json();
    const before = JSON.stringify(S.cams.map(c => [c.id, c.name, c.url, c.enabled, c.online]));
    S.self = j.self; S.cams = j.cams;
    const after = JSON.stringify(S.cams.map(c => [c.id, c.name, c.url, c.enabled, c.online]));
    if (before !== after) {
      render();
      if (S.view === 'detail') $('#d-name').textContent = byId(S.detailId)?.name || '';
    }
  } catch { /* master not reachable right now; try again next time */ }
}

async function init() {
  $('#back').addEventListener('click', closeDetail);
  $('#gear').addEventListener('click', enterConfig);
  $('#cfg-cancel').addEventListener('click', exitConfig);
  $('#save').addEventListener('click', saveConfig);
  $('#ctl-toggle').addEventListener('click', () => $('#detail').classList.toggle('ctl-open'));
  $('#fs-btn').addEventListener('click', () => {
    const st = $('#stage');
    if (document.fullscreenElement) document.exitFullscreen();
    else if (st.requestFullscreen) st.requestFullscreen().catch(() => {});
  });
  document.addEventListener('keydown', e => {
    if (e.key === 'Escape') { if (openPicker) closePicker(); else if (S.view === 'detail') closeDetail(); }
  });
  document.addEventListener('visibilitychange', syncStreams);

  try {
    const [t, c] = await Promise.all([
      fetch('/api/theme').then(r => r.json()).catch(() => ({})),
      fetch('/api/cams').then(r => r.json()),
    ]);
    S.theme = mergeTheme(t);
    S.self = c.self; S.cams = c.cams;
  } catch {
    toast('Could not reach the master camera');
  }
  const m = /cam=([^&]+)/.exec(location.hash);
  if (m && byId(decodeURIComponent(m[1]))) openDetail(decodeURIComponent(m[1]));
  else render();
  setInterval(refreshCams, 10000);
}
init();
)DMWEB";
