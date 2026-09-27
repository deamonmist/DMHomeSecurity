/* DMHomeSecurity website by Deamonmist - sent to your browser by the master camera.
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
