// CamBox web app: live control, story editing, and settings for one cam drive.
// Plain ES modules: Vite bundles it, but it also runs unbundled straight off the device.

/** @returns {any} */
const $ = (sel, /** @type {ParentNode} */ root = document) => root.querySelector(sel);

/** Tiny element builder: h('div', {class: 'x', onclick: fn}, child, ...) */
function h(tag, attrs = {}, ...kids) {
  const el = document.createElement(tag);
  for (const [k, v] of Object.entries(attrs || {})) {
    if (v === undefined || v === null || v === false) continue;
    if (k.startsWith('on')) el.addEventListener(k.slice(2), v);
    else if (k === 'class') el.className = v;
    else if (k === 'value') el.value = v;
    else if (k === 'checked') el.checked = !!v;
    else el.setAttribute(k, v === true ? '' : v);
  }
  for (const k of kids.flat()) if (k !== null && k !== undefined && k !== false) el.append(k instanceof Node ? k : String(k));
  return el;
}

// ---------------------------------------------------------------- API

async function api(method, path, body) {
  const res = await fetch(path, {
    method,
    headers: body !== undefined ? { 'Content-Type': 'application/json' } : undefined,
    body: body !== undefined ? JSON.stringify(body) : undefined,
  });
  const text = await res.text();
  let data = {};
  try { data = text ? JSON.parse(text) : {}; } catch { data = { error: text }; }
  if (!res.ok) throw new Error(data.error || `${res.status} ${res.statusText}`);
  return data;
}

let toastTimer = 0;
function toast(msg) {
  const t = $('#toast');
  t.textContent = msg;
  t.hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => (t.hidden = true), 2600);
}

// ---------------------------------------------------------------- live connection

const state = {
  tab: 'play',
  status: null,
  stories: [],
  settings: null,
  editing: null,      // story being edited: {name, loop, steps:[{label,speed,ms,ramp}]}
  isNew: false,
  recording: null,    // {t0, points:[{t,v}], lastV, settleTimer}
};

let ws = null;
let wsRetry = 500;

function connect() {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  ws = new WebSocket(`${proto}://${location.host}/ws`);
  ws.onopen = () => { wsRetry = 500; setConn('Connected', 'ok'); };
  ws.onmessage = (e) => {
    try { state.status = JSON.parse(e.data); updateLive(); } catch { /* ignore partial frames */ }
  };
  ws.onclose = () => {
    setConn('Reconnecting…', 'bad');
    setTimeout(connect, wsRetry);
    wsRetry = Math.min(wsRetry * 2, 5000);
  };
}

function send(msg) {
  if (ws && ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(msg));
}

/** Send at most every `ms`, always delivering the last value. */
function throttle(fn, ms) {
  let last = 0, timer = null, pending = null;
  const fn2 = (...args) => {
    pending = args;
    const now = Date.now();
    if (now - last >= ms) { last = now; fn(...pending); pending = null; }
    else if (!timer) timer = setTimeout(() => { timer = null; last = Date.now(); if (pending) fn(...pending); pending = null; }, ms - (now - last));
  };
  fn2.cancel = () => { clearTimeout(timer); timer = null; pending = null; };
  return fn2;
}
const sendSpeed = throttle((v) => send({ t: 'speed', v }), 80);
const sendMaster = throttle((v) => send({ t: 'master', v }), 120);
const sendTrim = throttle((v) => send({ t: 'trim', v }), 80);

function setConn(text, cls) {
  const c = $('#conn');
  c.textContent = text;
  c.className = `conn ${cls}`;
}

// ---------------------------------------------------------------- shared bits

const DIR_LABEL = { '1': 'Forward', '0': 'Stop (switch centered)', '-1': 'Reverse' };

function gauge(id) {
  return h('div', { class: 'gauge', id, role: 'img', 'aria-label': 'Speed' }, h('div', { class: 'fill' }), h('div', { class: 'mid' }));
}

function setGauge(el, speed) {
  if (!el) return;
  const f = $('.fill', el);
  const pct = Math.max(-100, Math.min(100, speed)) / 2; // -50..50 % of width
  f.style.left = `${pct >= 0 ? 50 : 50 + pct}%`;
  f.style.width = `${Math.abs(pct)}%`;
  el.setAttribute('aria-label', `Speed ${Math.round(speed)} percent`);
}

function storySelect(id, selected, includeNone) {
  return h('select', { id },
    includeNone ? h('option', { value: '' }, '(none)') : null,
    state.stories.map((n) => h('option', { value: n, selected: n === selected }, n)));
}

async function loadStories() {
  try { state.stories = (await api('GET', '/api/stories')).sort(); } catch (e) { toast(`Couldn't load stories: ${e.message}`); }
}

// ---------------------------------------------------------------- Play tab

function renderPlay() {
  const s = state.status || {};
  return [
    h('section', { class: 'card', 'aria-live': 'polite' },
      h('div', { class: 'now' },
        h('div', { class: 'eyebrow', id: 'nowMode' }, '—'),
        h('div', { class: 'title', id: 'nowTitle' }, '—'),
        h('div', { class: 'beat', id: 'nowBeat' }, '')),
      gauge('liveGauge'),
      h('div', { class: 'pills', id: 'pills' })),

    h('section', { class: 'card' },
      h('div', { class: 'row' },
        h('button', { class: 'btn primary big', id: 'btnPlay', onclick: () => api('POST', '/api/pause').catch((e) => toast(e.message)) }, 'Play / Pause'),
        h('button', { class: 'btn big', onclick: () => api('POST', '/api/next').catch((e) => toast(e.message)) }, 'Next story')),
      h('button', { class: 'btn stop big', onclick: () => { sendSpeed.cancel(); send({ t: 'stop' }); const m = $('#manual'); if (m) m.value = '0'; } }, 'Stop'),
      h('div', { class: 'row' },
        h('label', { class: 'field grow' }, 'Play a story', storySelect('playPick', s.story, false)),
        h('button', { class: 'btn', style: 'align-self:end', onclick: () => {
          const name = $('#playPick').value;
          if (name) api('POST', '/api/play', { name }).catch((e) => toast(e.message));
        } }, 'Play'))),

    h('section', { class: 'card' },
      h('h2', {}, 'Drive it by hand'),
      h('input', { type: 'range', id: 'manual', min: -100, max: 100, step: 1, value: 0, 'aria-label': 'Manual speed',
        oninput: (e) => sendSpeed(Number(e.target.value)) }),
      h('div', { class: 'row muted' }, h('span', {}, '◀ Reverse'), h('span', { class: 'grow', style: 'text-align:center' }, 'Stop'), h('span', {}, 'Forward ▶')),
      h('p', { class: 'muted', style: 'margin:0;font-size:14px' }, 'The direction switch on the box still applies: center stops, reverse flips whatever you send.')),

    h('section', { class: 'card', id: 'masterCard' },
      h('h2', {}, 'Story speed'),
      h('input', { type: 'range', id: 'master', min: 0, max: 100, step: 1, value: s.master ?? 100, 'aria-label': 'Story master speed',
        oninput: (e) => { sendMaster(Number(e.target.value)); $('#masterVal').textContent = `${e.target.value}%`; } }),
      h('div', { class: 'muted' }, 'Scales every beat of the story: ', h('span', { class: 'mono', id: 'masterVal' }, `${s.master ?? 100}%`), '. The knob on the box does the same.')),
  ];
}

function updateLive() {
  const s = state.status;
  if (!s) return;
  const story = s.mode === 'story';
  const set = (id, txt) => { const el = document.getElementById(id); if (el && el.textContent !== txt) el.textContent = txt; };
  set('nowMode', story ? (s.resting ? 'Resting between loops' : s.paused ? 'Story · paused' : 'Story · playing') : 'Manual');
  set('nowTitle', story ? s.story : (Math.abs(s.target) > 0.5 ? `Turning at ${Math.round(s.target)}%` : 'Stopped'));
  set('nowBeat', story && s.steps ? `Beat ${s.step + 1} of ${s.steps}${s.step_label ? ` · ${s.step_label}` : ''} · loop ${s.loops + 1}` : '');
  setGauge(document.getElementById('liveGauge'), s.speed);
  const pills = document.getElementById('pills');
  if (pills) {
    pills.replaceChildren(
      h('span', { class: `pill ${s.dir === 0 ? 'warn' : ''}` }, `Switch: ${DIR_LABEL[String(s.dir)]}`),
      h('span', { class: 'pill' }, `Knob ${s.knob}%`),
      h('span', { class: 'pill mono' }, outputLabel(s)),
      story && !s.paused ? h('span', { class: 'pill on' }, 'Playing') : null);
  }
  const mc = document.getElementById('masterCard');
  if (mc) mc.hidden = !story;
  const mv = /** @type {HTMLInputElement} */ (document.getElementById('master'));
  if (mv && document.activeElement !== mv) { mv.value = s.master; set('masterVal', `${s.master}%`); }
  const tl = document.getElementById('trimLive');
  if (tl) tl.textContent = outputLabel(s);
}

// What the motor output is doing: servo pulse width, or N20 PWM duty.
function outputLabel(s) {
  if (s.drive === 'n20') return s.duty ? `${s.duty}% duty` : 'stopped';
  return s.pulse_us ? `${s.pulse_us} µs` : 'released';
}

// ---------------------------------------------------------------- Stories tab

const blankStory = () => ({ name: '', loop: true, steps: [{ label: 'Begin', speed: 30, ms: 3000, ramp: 800 }] });

function renderStories() {
  if (state.editing) return renderEditor();
  return [
    h('section', { class: 'card' },
      h('div', { class: 'row' },
        h('h2', { class: 'grow' }, 'Stories on this box'),
        h('button', { class: 'btn primary', onclick: () => { state.editing = blankStory(); state.isNew = true; render(); } }, 'New story')),
      state.stories.length === 0
        ? h('p', { class: 'muted', style: 'margin:0' }, 'No stories yet. Make one with New story, or import a .json file.')
        : h('div', { class: 'list' }, state.stories.map((n) => h('div', { class: 'item' },
            h('span', { class: 'name' }, n),
            h('button', { class: 'btn small primary', onclick: () => api('POST', '/api/play', { name: n }).then(() => toast(`Playing ${n}`)).catch((e) => toast(e.message)) }, 'Play'),
            h('button', { class: 'btn small', onclick: () => editStory(n) }, 'Edit'),
            h('button', { class: 'btn small', onclick: () => exportStory(n) }, 'Export'),
            h('button', { class: 'btn small', onclick: () => deleteStory(n) }, 'Delete')))),
      h('label', { class: 'btn', style: 'justify-self:start' }, 'Import story (.json)',
        h('input', { type: 'file', accept: '.json,application/json', hidden: true, onchange: importStory }))),
  ];
}

async function editStory(name) {
  try {
    const s = await api('GET', `/api/stories/${encodeURIComponent(name)}`);
    state.editing = { name, loop: s.loop !== false, steps: (s.steps || []).map((x) => ({ label: x.label || '', speed: x.speed, ms: x.ms, ramp: x.ramp ?? 300 })) };
    state.isNew = false;
    render();
  } catch (e) { toast(e.message); }
}

async function deleteStory(name) {
  const item = [...document.querySelectorAll('.item')].find((el) => $('.name', el).textContent === name);
  // Two-tap delete instead of a confirm() dialog.
  const btn = item && [...item.querySelectorAll('button')].pop();
  if (btn && btn.dataset.armed !== '1') { btn.dataset.armed = '1'; btn.textContent = 'Tap again'; setTimeout(() => { btn.dataset.armed = ''; btn.textContent = 'Delete'; }, 3000); return; }
  try { await api('DELETE', `/api/stories/${encodeURIComponent(name)}`); await loadStories(); render(); toast(`Deleted ${name}`); }
  catch (e) { toast(e.message); }
}

async function exportStory(name) {
  try {
    const s = await api('GET', `/api/stories/${encodeURIComponent(name)}`);
    const blob = new Blob([JSON.stringify(s, null, 2)], { type: 'application/json' });
    const a = h('a', { href: URL.createObjectURL(blob), download: `${name}.json` });
    document.body.append(a); a.click(); a.remove();
  } catch (e) { toast(e.message); }
}

async function importStory(e) {
  const file = e.target.files[0];
  if (!file) return;
  try {
    const s = JSON.parse(await file.text());
    const name = (s.name || file.name.replace(/\.json$/i, '')).replace(/[^A-Za-z0-9_-]/g, '-').slice(0, 31);
    await api('PUT', `/api/stories/${encodeURIComponent(name)}`, s);
    await loadStories(); render(); toast(`Imported ${name}`);
  } catch (err) { toast(`Import failed: ${err.message}`); }
}

function timelineSvg(steps) {
  const W = 600, H = 120, pad = 6;
  const total = steps.reduce((t, s) => t + (+s.ramp || 0) + (+s.ms || 0), 0) || 1;
  const y = (v) => H / 2 - (v / 100) * (H / 2 - pad);
  let t = 0, prev = 0;
  let d = `M0 ${y(0)}`;
  for (const s of steps) {
    t += +s.ramp || 0; d += ` L${(t / total) * W} ${y(+s.speed || 0)}`;
    t += +s.ms || 0;   d += ` L${(t / total) * W} ${y(+s.speed || 0)}`;
    prev = s.speed;
  }
  const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
  svg.setAttribute('viewBox', `0 0 ${W} ${H}`);
  svg.setAttribute('class', 'timeline');
  svg.setAttribute('role', 'img');
  svg.setAttribute('aria-label', `Speed over ${(total / 1000).toFixed(1)} seconds`);
  svg.innerHTML = `<line x1="0" x2="${W}" y1="${H / 2}" y2="${H / 2}" stroke="currentColor" opacity=".25"/>
    <path d="${d}" fill="none" stroke="var(--green)" stroke-width="3" stroke-linejoin="round"/>
    <text x="6" y="14" font-size="11" fill="currentColor" opacity=".6">forward</text>
    <text x="6" y="${H - 6}" font-size="11" fill="currentColor" opacity=".6">reverse</text>
    <text x="${W - 6}" y="14" font-size="11" text-anchor="end" fill="currentColor" opacity=".6">${(total / 1000).toFixed(1)} s</text>`;
  return svg;
}

function renderEditor() {
  const st = state.editing;
  const refreshTimeline = () => { const old = $('#tl'); if (old) { const n = timelineSvg(st.steps); n.id = 'tl'; old.replaceWith(n); } };
  const tl = timelineSvg(st.steps); tl.id = 'tl';
  const num = (step, key, scale, label) => h('label', { class: 'field' }, label,
    h('input', { type: 'number', min: key === 'speed' ? -100 : 0, max: key === 'speed' ? 100 : 600, step: key === 'speed' ? 1 : 0.1,
      value: key === 'speed' ? step.speed : +(step[key] / 1000).toFixed(2),
      oninput: (e) => { const v = Number(e.target.value); step[key] = key === 'speed' ? Math.max(-100, Math.min(100, v)) : Math.round(v * scale); refreshTimeline(); } }));

  const stepsEl = h('div', { class: 'steps' }, st.steps.map((step, i) => h('div', { class: 'step' },
    h('div', { class: 'head' },
      h('span', { class: 'num' }, String(i + 1).padStart(2, '0')),
      h('input', { type: 'text', value: step.label, placeholder: 'What happens in this beat', 'aria-label': `Beat ${i + 1} label`, class: 'grow',
        oninput: (e) => (step.label = e.target.value.slice(0, 31)) }),
      h('button', { class: 'btn small', 'aria-label': 'Move up', disabled: i === 0, onclick: () => { [st.steps[i - 1], st.steps[i]] = [st.steps[i], st.steps[i - 1]]; render(); } }, '↑'),
      h('button', { class: 'btn small', 'aria-label': 'Move down', disabled: i === st.steps.length - 1, onclick: () => { [st.steps[i + 1], st.steps[i]] = [st.steps[i], st.steps[i + 1]]; render(); } }, '↓'),
      h('button', { class: 'btn small', 'aria-label': 'Remove beat', disabled: st.steps.length === 1, onclick: () => { st.steps.splice(i, 1); render(); } }, '✕')),
    h('div', { class: 'nums' }, num(step, 'speed', 1, 'Speed ±100'), num(step, 'ms', 1000, 'Hold (s)'), num(step, 'ramp', 1000, 'Ramp in (s)')))));

  const rec = state.recording;
  return [
    h('section', { class: 'card' },
      h('div', { class: 'row' },
        h('button', { class: 'btn', onclick: () => { stopRecording(false); state.editing = null; render(); } }, '← Stories'),
        h('h2', { class: 'grow' }, state.isNew ? 'New story' : `Editing ${st.name}`)),
      h('div', { class: 'grid2' },
        h('label', { class: 'field' }, 'Name (letters, digits, - and _)',
          h('input', { type: 'text', value: st.name, maxlength: 31, disabled: !state.isNew, oninput: (e) => (st.name = e.target.value) })),
        h('label', { class: 'check', style: 'align-self:end' }, h('input', { type: 'checkbox', checked: st.loop, onchange: (e) => (st.loop = e.target.checked) }), 'Loop forever')),
      tl),

    h('section', { class: 'card' },
      h('div', { class: 'row' },
        h('h2', { class: 'grow' }, 'Record by performing'),
        rec ? h('span', { class: 'rec' }, '● Recording') : null),
      h('p', { class: 'muted', style: 'margin:0;font-size:14px' }, 'Press Record, then move the slider the way you would turn the crank. Each time you settle on a speed it becomes a beat. Recording replaces the beats below.'),
      h('input', { type: 'range', id: 'recSlider', min: -100, max: 100, value: 0, disabled: !rec, 'aria-label': 'Performance speed',
        oninput: (e) => recordInput(Number(e.target.value)) }),
      h('div', { class: 'row' },
        rec ? h('button', { class: 'btn stop', onclick: () => stopRecording(true) }, 'Finish recording')
            : h('button', { class: 'btn primary', onclick: startRecording }, 'Record'))),

    h('section', { class: 'card' },
      h('h2', {}, `Beats (${st.steps.length})`),
      stepsEl,
      h('button', { class: 'btn', style: 'justify-self:start', disabled: st.steps.length >= 64,
        onclick: () => { st.steps.push({ label: '', speed: 0, ms: 2000, ramp: 500 }); render(); } }, 'Add beat')),

    h('section', { class: 'card' },
      h('div', { class: 'row' },
        h('button', { class: 'btn primary big', onclick: () => saveStory(false) }, 'Save'),
        h('button', { class: 'btn big', onclick: () => saveStory(true) }, 'Save and play'))),
  ];
}

async function saveStory(andPlay) {
  const st = state.editing;
  const name = (st.name || '').trim();
  if (!/^[A-Za-z0-9_-]{1,31}$/.test(name)) { toast('Name: 1–31 letters, digits, - or _'); return; }
  const body = { name, loop: st.loop, steps: st.steps.map((s) => ({ label: s.label, speed: Math.round(s.speed), ms: Math.max(0, Math.round(s.ms)), ramp: Math.max(0, Math.round(s.ramp)) })) };
  try {
    await api('PUT', `/api/stories/${encodeURIComponent(name)}`, body);
    state.isNew = false;
    await loadStories();
    if (andPlay) await api('POST', '/api/play', { name });
    toast(andPlay ? `Saved and playing ${name}` : `Saved ${name}`);
    render();
  } catch (e) { toast(`Save failed: ${e.message}`); }
}

function startRecording() {
  state.recording = { t0: performance.now(), points: [{ t: 0, v: 0 }], lastV: 0, settle: null };
  render();
  $('#recSlider')?.focus();
}

function recordInput(v) {
  const r = state.recording;
  if (!r) return;
  sendSpeed(v);
  clearTimeout(r.settle);
  // A beat is recorded once the slider rests for a moment at a new speed.
  r.settle = setTimeout(() => {
    if (Math.abs(v - r.lastV) >= 4) { r.points.push({ t: performance.now() - r.t0, v }); r.lastV = v; }
  }, 250);
}

function stopRecording(keep) {
  const r = state.recording;
  if (!r) return;
  clearTimeout(r.settle);
  state.recording = null;
  sendSpeed.cancel();
  send({ t: 'speed', v: 0 });
  if (keep && r.points.length > 1) {
    const end = performance.now() - r.t0;
    const pts = r.points.slice(1);
    state.editing.steps = pts.map((p, i) => {
      const next = i + 1 < pts.length ? pts[i + 1].t : end;
      const ramp = Math.min(800, Math.max(200, Math.round((p.t - (i ? pts[i - 1].t : 0)) * 0.25)));
      return { label: `Beat ${i + 1}`, speed: p.v, ramp, ms: Math.max(0, Math.round(next - p.t)) };
    }).slice(0, 64);
    toast(`Recorded ${state.editing.steps.length} beats`);
  } else if (keep) {
    toast('Nothing recorded: move the slider and pause on each speed.');
  }
  render();
}

// ---------------------------------------------------------------- Settings tab

function renderSettings() {
  const s = state.settings;
  if (!s) return [h('section', { class: 'card' }, h('p', { class: 'muted' }, 'Loading settings…'))];
  const numField = (key, label, min, max, step = 1, scale = 1) => h('label', { class: 'field' }, label,
    h('input', { type: 'number', id: `set_${key}`, min, max, step, value: +(s[key] / scale).toFixed(2) }));
  const check = (key, label) => h('label', { class: 'check' }, h('input', { type: 'checkbox', id: `set_${key}`, checked: s[key] }), label);
  const text = (key, label, type = 'text', placeholder = '') => h('label', { class: 'field' }, label,
    h('input', { type, id: `set_${key}`, value: type === 'password' ? '' : s[key] || '', placeholder, autocomplete: 'off' }));
  const net = state.status?.net || {};
  const servo = state.status?.drive !== 'n20';   // the N20 stops dead at zero: no trim, no pulse range

  const trimInput = h('input', { type: 'range', id: 'trimRange', min: 1300, max: 1700, step: 1, value: s.trim_us,
    oninput: (e) => { $('#trimVal').textContent = `${e.target.value} µs`; sendTrim(Number(e.target.value)); } });
  const nudge = (d) => h('button', { class: 'btn small', onclick: () => { trimInput.value = String(Number(trimInput.value) + d); trimInput.dispatchEvent(new Event('input')); } }, d > 0 ? `+${d}` : `${d}`);

  return [
    servo && h('section', { class: 'card' },
      h('h2', {}, 'Stop trim'),
      h('p', { class: 'muted', style: 'margin:0;font-size:14px' }, 'Every continuous servo stops at a slightly different pulse. Move this until the shaft stands still, then save. The servo follows as you drag.'),
      trimInput,
      h('div', { class: 'row' }, nudge(-5), nudge(-1), h('span', { class: 'mono grow', style: 'text-align:center', id: 'trimVal' }, `${s.trim_us} µs`), nudge(1), nudge(5)),
      h('div', { class: 'row' },
        h('button', { class: 'btn primary', onclick: () => saveSettings({ trim_us: Number(trimInput.value) }) }, 'Save trim'),
        h('button', { class: 'btn', onclick: () => { send({ t: 'trim_end' }); trimInput.value = String(s.trim_us); $('#trimVal').textContent = `${s.trim_us} µs`; } }, 'Cancel'),
        h('span', { class: 'muted mono', id: 'trimLive' }, ''))),

    h('section', { class: 'card' },
      h('h2', {}, 'Motion'),
      h('div', { class: 'grid2' },
        servo && numField('range_us', 'Full-speed offset (µs)', 50, 900),
        numField('slew', 'Smoothing (% per second)', 10, 2000),
        numField('idle_release_ms', 'Release after stop (s, 0 = never)', 0, 60, 0.1, 1000)),
      h('div', { class: 'grid2' }, check('knob_enabled', 'Use the speed knob'), check('switch_installed', 'Direction switch is wired')),
      h('button', { class: 'btn primary', style: 'justify-self:start', onclick: () => saveSettings(collect([...(servo ? ['range_us'] : []), 'slew', ['idle_release_ms', 1000], 'knob_enabled', 'switch_installed'])) }, 'Save motion')),

    h('section', { class: 'card' },
      h('h2', {}, 'Gallery mode'),
      check('autoplay', 'Play a story automatically when powered on'),
      h('div', { class: 'grid2' },
        h('label', { class: 'field' }, 'Story', storySelect('set_autoplay_story', s.autoplay_story, true)),
        numField('rest_every', 'Rest after every N loops (0 = never)', 0, 1000),
        numField('rest_ms', 'Rest length (s)', 0, 600, 0.5, 1000)),
      h('button', { class: 'btn primary', style: 'justify-self:start', onclick: () => saveSettings(collect(['autoplay', 'autoplay_story', 'rest_every', ['rest_ms', 1000]])) }, 'Save gallery')),

    h('section', { class: 'card' },
      h('h2', {}, 'Wi-Fi'),
      h('p', { class: 'muted', style: 'margin:0;font-size:14px' },
        `The box runs its own network${net.ap_ssid ? ` "${net.ap_ssid}"` : ''}. Optionally it can also join yours${net.sta_ip ? ` (joined, ${net.sta_ip})` : ''}. Changes apply after a restart. Hold the panel button while powering up to reset these.`),
      h('div', { class: 'grid2' },
        text('ap_ssid', 'Box network name (blank = CamBox-XXXX)'),
        text('ap_pass', 'Box password (8+ characters, blank = open)', 'password', s.ap_pass_set ? 'unchanged' : ''),
        text('sta_ssid', 'Join this network (optional)'),
        text('sta_pass', 'Its password', 'password', s.sta_pass_set ? 'unchanged' : ''),
        text('hostname', 'Name on the network (.local)')),
      h('div', { class: 'row' },
        h('button', { class: 'btn primary', onclick: saveNetwork }, 'Save Wi-Fi'),
        h('button', { class: 'btn', id: 'btnRestart', onclick: () => api('POST', '/api/restart').then(() => toast('Restarting… reconnect in a few seconds')).catch(() => toast('Restarting…')) }, 'Restart box'))),

    h('section', { class: 'card' },
      h('h2', {}, 'About this box'),
      h('div', { class: 'mono muted', style: 'font-size:13px' },
        `Network ${net.ap_ssid || '—'} · ${net.hostname || 'cambox'}.local · free memory ${state.status?.free_heap ?? '—'} bytes · up ${state.status?.uptime_s ?? '—'} s`)),
  ];
}

function collect(keys) {
  const out = {};
  for (const k of keys) {
    const [key, scale] = Array.isArray(k) ? k : [k, 1];
    const el = /** @type {HTMLInputElement} */ (document.getElementById(`set_${key}`));
    if (!el) continue;
    if (el.type === 'checkbox') out[key] = el.checked;
    else if (el.type === 'number') out[key] = Math.round(Number(el.value) * scale);
    else out[key] = el.value;
  }
  return out;
}

async function saveNetwork() {
  const body = collect(['ap_ssid', 'sta_ssid', 'hostname']);
  for (const k of ['ap_pass', 'sta_pass']) {
    const v = /** @type {HTMLInputElement} */ (document.getElementById(`set_${k}`)).value;
    if (v) body[k] = v;
  }
  if (body.ap_pass && body.ap_pass.length < 8) { toast('Box password needs 8 or more characters'); return; }
  const res = await saveSettings(body);
  if (res && res.restart_needed) toast('Saved. Restart the box to use the new Wi-Fi settings.');
}

async function saveSettings(body) {
  try {
    const res = await api('POST', '/api/settings', body);
    state.settings = res;
    send({ t: 'trim_end' });
    if (!res.restart_needed) toast('Saved');
    return res;
  } catch (e) { toast(`Save failed: ${e.message}`); return null; }
}

// ---------------------------------------------------------------- tabs

function render() {
  document.querySelectorAll('.tabs button').forEach((/** @type {HTMLButtonElement} */ b) => b.setAttribute('aria-selected', String(b.dataset.tab === state.tab)));
  const views = { play: renderPlay, stories: renderStories, settings: renderSettings };
  $('#app').replaceChildren(...views[state.tab]().filter(Boolean));
  updateLive();
}

document.querySelectorAll('.tabs button').forEach((/** @type {HTMLButtonElement} */ b) => b.addEventListener('click', async () => {
  if (state.recording) stopRecording(false);
  if (state.tab === 'settings') send({ t: 'trim_end' });
  state.tab = b.dataset.tab;
  if (state.tab === 'settings') state.settings = await api('GET', '/api/settings').catch(() => state.settings);
  if (state.tab !== 'stories') state.editing = null;
  render();
  try { localStorage.setItem('cambox.tab', state.tab); } catch { /* private mode */ }
}));

(async function start() {
  try { const t = localStorage.getItem('cambox.tab'); if (t && ['play', 'stories', 'settings'].includes(t)) state.tab = t; } catch { /* ignore */ }
  render();
  connect();
  try { state.status = await api('GET', '/api/status'); } catch { /* the socket will fill it in */ }
  await loadStories();
  if (state.tab === 'settings') state.settings = await api('GET', '/api/settings').catch(() => null);
  render();
})();
