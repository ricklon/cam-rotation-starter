// Generates the step-by-step wiring pictures in this folder and ../../wiring-steps.md,
// from the same hole map as ../perfboard.svg. Run from this folder: node make_steps.mjs
// Each step's wire ID, colour and cut length are checked against ../../cut-list.csv.
import { writeFileSync, readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', '..');

// ---- geometry (mm on the board, as in perfboard.md: u across the columns, v along the rows) ----
const S = 8;                          // px per mm
const VW = 600;                       // view width: the pictures are read on a phone
const W = 38.1, H = 50.8;
const OX = (VW - W * S) / 2, OY = 236;
const colIdx = { A: 0, B: 1, C: 2, D: 3, E: 4, F: 7, G: 8, H: 9, I: 10, J: 11 };
const COLS = 'ABCDEFGHIJ';
let mirror = false;                   // true: seen from the underside, flipped left to right
const sg = () => (mirror ? -1 : 1);
const cu = c => (colIdx[c] - 5.5) * 2.54;
const rv = r => (r - 9) * 2.54;
const ux = u => OX + (W / 2 + sg() * u) * S;
const vy = v => OY + (H / 2 + v) * S;
const hx = c => ux(cu(c));
const hy = r => vy(rv(r));
const mm = v => v * S;
const lane = (c, d) => hx(c) + sg() * mm(d);
const hole = h => [hx(h[0]), hy(+h.slice(1))];
// The ElectroCookie board's underside print is mirrored: a hole named A2 on the component side is printed J2
// underneath (A↔J, B↔I, C↔H, D↔G, E↔F, same row). The guide names holes by the component side.
const printedCol = c => COLS[COLS.length - 1 - COLS.indexOf(c)];
const under = h => (/^[A-J]\d+$/.test(h) ? printedCol(h[0]) + h.slice(1) : h);
const rectU = (u0, u1, v0, v1, attrs) => {
  const xa = ux(u0), xb = ux(u1);
  return `<rect x="${Math.min(xa, xb).toFixed(1)}" y="${vy(v0).toFixed(1)}" width="${Math.abs(xb - xa).toFixed(1)}" height="${(mm(v1 - v0)).toFixed(1)}" ${attrs}/>`;
};
const pts = p => p.map(([x, y]) => `${x.toFixed(1)},${y.toFixed(1)}`).join(' ');

// ---- colours ----
const WIRE = { red: '#d12a1f', black: '#1b1b1b', green: '#1f9a3c', blue: '#2f66d8', white: '#ffffff', yellow: '#f2c300', tin: '#b9b9c2' };
const INK = '#1c2729', MUTE = '#4f5b5e', DONE = '#9ea5a9', RING = '#d4147a', HL = '#ffd84d';
const FONT = 'font-family="IBM Plex Sans,ui-sans-serif,system-ui,sans-serif"';
const MONO = 'font-family="IBM Plex Mono,ui-monospace,SFMono-Regular,Menlo,monospace"';
const esc = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');

// ---- link routes on the underside (same lanes as perfboard.svg) ----
const L = (from, x, to) => {
  const dn = to[1] < from[1] ? -1 : 1;
  return [[hx(from[0]), hy(from[1])], [x(), hy(from[1]) + dn * mm(0.9)], [x(), hy(to[1]) - dn * mm(0.9)], [hx(to[0]), hy(to[1])]];
};
const ROUTE = {
  W16: () => L(['B', 7], () => lane('B', 0.6), ['C', 3]),
  W18: () => L(['B', 8], () => lane('B', 1.27), ['C', 4]),
  W17: () => L(['B', 2], () => lane('B', 1.9), ['C', 17]),
  W4: () => L(['C', 8], () => lane('B', 0.6), ['C', 16]),
  W5: () => L(['C', 12], () => lane('C', 0.6), ['C', 5]),
  W12: () => L(['C', 14], () => lane('C', 1.27), ['C', 6]),
  W21: () => [[hx('A'), hy(15)], [lane('A', 0.7), hy(14.5)], [lane('A', 0.7), hy(10.6)], [lane('I', -0.6), hy(10.6)], [hx('I'), hy(10)]],
  W22: () => [[hx('C'), hy(2)], [hx('C'), hy(1.6)], [lane('I', -0.7), hy(1.6)], [lane('I', -0.7), hy(6.6)], [hx('H'), hy(7)]],
  W23: () => [[hx('A'), hy(16)], [lane('A', -0.9), hy(15.65)], [lane('A', -0.9), hy(7.4)], [lane('H', -0.9), hy(7.4)], [hx('H'), hy(8)]],
  W13: () => [[hx('C'), hy(13)], [lane('C', 1.9), hy(12.6)], [lane('C', 1.9), hy(6.5)], [hx('J'), hy(6.5)], [hx('J'), hy(6)]],
  // U1's pads to the buses, on the component side. Its - pad is the outer one (u -16.5) and its + pad the inner one (u -11):
  // W2 goes straight to A8, and W1 passes between A8 and B8 to A7, so they don't cross
  W1: () => [[ux(-11.0), vy(0.3)], [ux(-11.0), vy(-0.9)], [ux(-12.7), hy(8)], [ux(-12.7), hy(7) + mm(0.9)], [hx('A'), hy(7)]],
  W2: () => [[ux(-16.5), vy(0.3)], [ux(-16.5), vy(-0.9)], [hx('A'), hy(8) + mm(0.9)], [hx('A'), hy(8)]],
};

// a wire drawn either done (grey, thin) or now (bold, real colour, dark outline)
function wire(p, colour, mode, behind) {
  if (mode === 'done') return `<polyline points="${pts(p)}" fill="none" stroke="${DONE}" stroke-width="3" ${behind ? 'stroke-dasharray="7 5"' : ''} stroke-linecap="round" stroke-linejoin="round"/>`;
  return `<polyline points="${pts(p)}" fill="none" stroke="#1b1b1b" stroke-width="9" stroke-linecap="round" stroke-linejoin="round"/>` +
    `<polyline points="${pts(p)}" fill="none" stroke="${WIRE[colour]}" stroke-width="6" stroke-linecap="round" stroke-linejoin="round"/>`;
}

// ---- parts ----
const PART = {
  D1(mode) {
    const g = mode === 'done';
    return `<line x1="${hx('A')}" y1="${hy(3)}" x2="${hx('A')}" y2="${hy(2)}" stroke="${g ? DONE : '#8d8d96'}" stroke-width="${g ? 3 : 5}"/>` +
      `<circle cx="${hx('A')}" cy="${hy(3)}" r="${mm(1.35)}" fill="${g ? '#d5d9dc' : '#151515'}" stroke="${g ? DONE : '#fff'}" stroke-width="2"/>` +
      (g ? '' : `<circle cx="${hx('A')}" cy="${hy(3)}" r="${mm(0.75)}" fill="none" stroke="#e8e8e8" stroke-width="2.5"/>`) +
      `<circle cx="${hx('A')}" cy="${hy(2)}" r="${mm(0.6)}" fill="${g ? DONE : '#8d8d96'}"/>` +
      (g ? '' : `<rect x="${Math.min(hx('A'), hx('A') + sg() * 250) + (mirror ? -mm(1.2) : mm(1.2))}" y="${hy(4.5) - 14}" width="240" height="24" rx="5" fill="#ffffff" stroke="${INK}" stroke-width="1.5"/>` +
        `<text x="${hx('A') + sg() * (mm(1.2) + 120)}" y="${hy(4.5) + 4}" text-anchor="middle" ${FONT} font-size="15" font-weight="700" fill="${INK}">band on top, lead over to A2</text>`);
  },
  J4(mode) {
    const g = mode === 'done', u = cu('J'), vm = (rv(2) + rv(3)) / 2;
    return rectU(u - 2.4, u + 2.1, vm - 3, vm + 3, `rx="2" fill="${g ? '#e6e9eb' : '#f4f0e2'}" stroke="${g ? DONE : '#1b1b1b'}" stroke-width="2"`) +
      [2, 3].map(r => `<rect x="${hx('J') - mm(0.5)}" y="${hy(r) - mm(0.5)}" width="${mm(1)}" height="${mm(1)}" fill="${g ? DONE : '#111'}"/>`).join('');
  },
  RV1(mode) {
    const g = mode === 'done', u = cu('J');
    return rectU(u + 2, u + 11.5, rv(11) - 5.5, rv(11) + 5.5, `rx="3" fill="${g ? '#e6e9eb' : '#2a6e3a'}" fill-opacity="${g ? 1 : 0.9}" stroke="${g ? DONE : '#1b1b1b'}" stroke-width="2"`) +
      `<circle cx="${ux(u + 6.75)}" cy="${hy(11)}" r="${mm(3)}" fill="${g ? '#f1f3f4' : '#c9c9cf'}" stroke="${g ? DONE : '#555'}" stroke-width="1.5"/>` +
      [10, 11, 12].map(r => `<line x1="${hx('J')}" y1="${hy(r)}" x2="${ux(u + 2)}" y2="${hy(r)}" stroke="${g ? DONE : '#8d8d96'}" stroke-width="3"/><rect x="${hx('J') - mm(0.6)}" y="${hy(r) - mm(0.6)}" width="${mm(1.2)}" height="${mm(1.2)}" fill="${g ? DONE : '#111'}"/>`).join('');
  },
  C1(mode) {
    // standing can seen from above: axis at u -13.5, v -9.6, stripe on the row 4 (-v) side; sleeved leads out to C7 and B4
    const g = mode === 'done', cx = ux(-13.5), cy = vy(-9.6), r = mm(4), cyc = cy - mm(2.6), hw = Math.sqrt(r * r - mm(2.6) ** 2);
    return [[[-11.75, -9.6], 'C7'], [[-15.25, -9.6], 'B4']].map(([[u, v], h]) =>
      `<polyline points="${pts([[ux(u), vy(v)], hole(h)])}" fill="none" stroke="${g ? DONE : '#2b2b2b'}" stroke-width="${g ? 3 : 5}" stroke-linecap="round"/>`).join('') +
      `<circle cx="${cx}" cy="${cy}" r="${r}" fill="${g ? '#e6e9eb' : '#1e3c8c'}" fill-opacity="${g ? 0.9 : 0.9}" stroke="${g ? DONE : '#1b1b1b'}" stroke-width="2"/>` +
      `<path d="M ${cx - hw} ${cyc} A ${r} ${r} 0 0 1 ${cx + hw} ${cyc} Z" fill="${g ? '#f1f3f4' : '#c8c8cd'}"/>` +
      `<text x="${cx}" y="${cy + 8}" text-anchor="middle" ${FONT} font-size="${g ? 15 : 17}" font-weight="700" fill="${g ? MUTE : '#fff'}">C1</text>` +
      (g ? '' : `<text x="${cx}" y="${cyc - 3}" text-anchor="middle" ${FONT} font-size="15" font-weight="700" fill="${INK}">−</text>`);
  },
  C3(mode) {
    const g = mode === 'done';
    return rectU(12.6, 24.1, -9, -1, `rx="6" fill="${g ? '#e6e9eb' : '#1e3c8c'}" stroke="${g ? DONE : '#1b1b1b'}" stroke-width="2"`) +
      rectU(12.6, 24.1, -4.1, -1.1, `rx="3" fill="${g ? '#f1f3f4' : '#c8c8cd'}"`) +
      [[-6.75, 7], [-3.25, 8]].map(([v, r]) => `<polyline points="${pts([[ux(12.6), vy(v)], [ux(11.6), vy(v)], [hx('I'), hy(r)]])}" fill="none" stroke="${g ? DONE : '#8d8d96'}" stroke-width="3"/>`).join('') +
      (g ? '' : `<text x="${ux(18.3)}" y="${vy(-5.6)}" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="#fff">+</text><text x="${ux(18.3)}" y="${vy(-1.9)}" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="${INK}">−</text>`);
  },
  U3(mode) { return module(mode, (rv(1) + rv(6)) / 2, 18.5, 16, 'U3 DRV8833', [['D', 3, 'VCC'], ['D', 4, 'GND'], ['D', 5, 'IN3'], ['D', 6, 'IN4'], ['H', 2, 'OUT4'], ['H', 3, 'OUT3'], ['H', 6, 'EEP']], rv(4.6)); },
  U2(mode) {
    const s = module(mode, rv(8) - 1.3 + 13, 18, 26, 'U2 Super Mini', [['D', 12, '18'], ['D', 13, '19'], ['D', 14, '20'], ['D', 15, '3V3'], ['D', 16, 'GND'], ['D', 17, '5V'],
      ['H', 11, '4'], ['H', 12, '3'], ['H', 13, '2'], ['H', 14, '1'], ['H', 15, '0']], rv(9.6));
    return rectU(-4.5, 4.5, rv(17) + 0.2, rv(17) + 2.8, `rx="4" fill="${mode === 'done' ? '#e6e9eb' : '#bbb'}" stroke="${mode === 'done' ? DONE : '#555'}"`) + s;
  },
  U1(mode) {
    const g = mode === 'done';
    return rectU(-19.3, -8.3, -1.2, 26.8, `rx="3" fill="${g ? '#dfe3e6' : '#202020'}" fill-opacity="${g ? 0.85 : 0.88}" stroke="${g ? DONE : '#fff'}" stroke-width="2"`) +
      rectU(-18.3, -9.3, 19.5, 27.1, `rx="4" fill="${g ? '#eef0f1' : '#bbb'}" stroke="${g ? DONE : '#fff'}"`) +
      [-16.5, -11.0].map(u => rectU(u - 1, u + 1, -0.7, 1.3, `fill="${g ? DONE : '#d7b95a'}"`)).join('') +
      `<text x="${ux(-13.8)}" y="${vy(8.5)}" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="${g ? MUTE : '#fff'}">U1</text>` +
      (g ? '' : `<text x="${ux(-13.8)}" y="${vy(11)}" text-anchor="middle" ${FONT} font-size="14" fill="#fff">PD board</text><text x="${ux(-13.8)}" y="${vy(13.2)}" text-anchor="middle" ${FONT} font-size="14" fill="#fff">on foam</text>` +
        `<text x="${ux(-16.5)}" y="${vy(3.6)}" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="#fff">−</text><text x="${ux(-11)}" y="${vy(3.6)}" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="#fff">+</text>`);
  },
};
function module(mode, vc, w, h, name, pins, vText) {
  const g = mode === 'done';
  let s = rectU(-w / 2, w / 2, vc - h / 2, vc + h / 2, `rx="4" fill="${g ? '#dfe3e6' : '#2b2b2b'}" fill-opacity="${g ? 0.6 : 0.82}" stroke="${g ? DONE : '#fff'}" stroke-width="2"`);
  for (const [c, r, n] of pins) {
    s += `<rect x="${hx(c) - mm(0.85)}" y="${hy(r) - mm(0.85)}" width="${mm(1.7)}" height="${mm(1.7)}" fill="${g ? DONE : '#111'}" stroke="#fff" stroke-width="1"/>`;
    if (!g) {
      const inward = (c === 'D') !== mirror;   // label toward the channel
      s += `<text x="${hx(c) + (inward ? mm(1.4) : -mm(1.4))}" y="${hy(r) + 5}" text-anchor="${inward ? 'start' : 'end'}" ${MONO} font-size="14" font-weight="700" fill="#fff">${n}</text>`;
    }
  }
  s += `<text x="${ux(0)}" y="${vy(vText)}" text-anchor="middle" ${FONT} font-size="15" font-weight="700" fill="${g ? MUTE : '#fff'}">${name}</text>`;
  return s;
}

// wires that come in from behind the board: a hole and a stub in the done state
const OFF = {
  W11: { hole: 'A4', side: 'L', ly: 3.2 },
  W6: { hole: 'I14', side: 'R', ly: 14.65 },
};
function offPath(id) {
  const o = OFF[id], [x, y] = hole(o.hole);
  const edge = o.side === 'L' ? OX - 34 : OX + mm(W) + 34;
  if (o.side === 'L') return [[x, y], [OX + 6, hy(o.ly)], [edge, hy(o.ly)]];
  return [[x, y], [x + mm(1.27), y], [x + mm(1.27), hy(o.ly)], [edge, hy(o.ly)]];
}
// the motor pair leaves J4's mouth toward the right-hand edge
const motorPath = pin => { const r = pin === 1 ? 3 : 2; return [[hx('J') + mm(2.1), hy(r)], [OX + mm(W) + 34, hy(r)]]; };

// ---- the steps, in perfboard.md's build order ----
const STEPS = [
  { id: 'D1', kind: 'part', side: 'top', title: 'D1, the 1N5817 diode, standing', colour: 'black', fromTo: 'anode A3, cathode A2',
    what: 'D1 joins the 5 V that feeds the motor driver (strip 3 left) to the Super Mini\'s 5 V (strip 2 left), one way only.',
    how: 'Stand it in A3 with the banded (cathode) end at the top, bend the top lead over and down into A2, and solder both from the underside.',
    check: 'Diode test, red probe on A3 and black on A2: about 0.2–0.3 V. Reversed: open.',
    ends: [{ hole: 'A3', role: 'anode +' }, { hole: 'A2', role: 'band −' }], strips: ['A3', 'A2'] },

  { id: 'W16', kind: 'link', colour: 'red', from: 'B7', to: 'C3', fromRole: '5V bus', toRole: 'VCC',
    what: 'Takes +5 V from the bus (row 7) to the motor driver\'s VCC strip and D1\'s anode.',
    check: 'B7 connects to A3 (D1\'s anode). B7 must not connect to B8 (the ground bus).' },
  { id: 'W18', kind: 'link', colour: 'black', from: 'B8', to: 'C4', fromRole: 'GND bus', toRole: 'GND',
    what: 'Takes ground from the bus (row 8) to the motor driver\'s GND strip.',
    check: 'B8 connects to A4. B8 must not connect to B7, and C4 must not connect to C3.' },
  { id: 'W5', kind: 'link', colour: 'green', from: 'C12', to: 'C5', fromRole: 'GPIO18', toRole: 'IN3',
    what: 'MOT_IN1: the Super Mini\'s GPIO18 to the driver\'s IN3, the forward PWM.',
    check: 'C12 connects to A5. It should not touch C13 or C11 next door.' },
  { id: 'W12', kind: 'link', colour: 'blue', from: 'C14', to: 'C6', fromRole: 'GPIO20', toRole: 'IN4',
    what: 'MOT_IN2: GPIO20 to the driver\'s IN4, the reverse PWM.',
    check: 'C14 connects to A6. It should not touch C13, C15 or W5.' },
  { id: 'W4', kind: 'link', colour: 'black', from: 'C8', to: 'C16', fromRole: 'GND bus', toRole: 'U2 GND',
    what: 'Ground from the bus to the Super Mini\'s GND strip.',
    check: 'C16 connects to B8. C16 must not connect to C15 (3V3) or C17 (5V).' },
  { id: 'W22', kind: 'link', colour: 'red', from: 'C2', to: 'H7', fromRole: 'D1 −', toRole: 'C3 +',
    what: 'The Super Mini\'s 5 V, after D1, across the channel to the strip where C3\'s + lead will go.',
    check: 'H7 connects to A2. H7 must not connect to H8 or H6.' },
  { id: 'W21', kind: 'link', colour: 'yellow', from: 'A15', to: 'I10', fromRole: '3V3', toRole: 'RV1 top',
    what: 'The Super Mini\'s 3.3 V to the strip where the knob\'s pin 1 will go. Yellow, not red: this is 3.3 V only.',
    check: 'A15 connects to J10. I10 must not connect to I11 (the wiper strip), and A15 must not connect to A16.' },
  { id: 'W13', kind: 'link', colour: 'white', from: 'C13', to: 'J6', fromRole: 'GPIO19', toRole: 'EEP',
    what: 'DRV_SLEEP: GPIO19 to the driver\'s EEP pin, which wakes it.',
    check: 'C13 connects to J6. It should not touch C12, C14, or W22 where they cross.' },
  { id: 'W17', kind: 'link', colour: 'red', from: 'B2', to: 'C17', fromRole: 'D1 −', toRole: 'U2 5V',
    what: 'The Super Mini\'s 5 V: from D1\'s cathode strip down the board to the 5V pin\'s strip.',
    check: 'C17 connects to A2 and to H7. C17 must not connect to C16 (GND).' },
  { id: 'W23', kind: 'link', colour: 'black', from: 'A16', to: 'H8', fromRole: 'GND', toRole: 'C3 −',
    what: 'Ground to the strip where C3\'s − lead will go.',
    check: 'H8 connects to B8. H8 must not connect to H7 (5 V) or H9.' },

  { id: 'J4', kind: 'part', side: 'top', title: 'J4, the motor plug (JST-PH 2-pin, side entry)', colour: 'white', fromTo: 'pin 1 in J3, pin 2 in J2',
    what: 'The motor\'s plug: pin 1 in the OUT3 strip (row 3 right), pin 2 in the OUT4 strip (row 2 right).',
    how: 'Splay the 2 mm-pitch pins a little so they reach the holes, mouth toward the right-hand edge. Push in and solder from the underside.',
    check: 'J3 must not connect to J2. Each pin sits flat, mouth facing off the board.',
    ends: [{ hole: 'J3', role: 'pin 1' }, { hole: 'J2', role: 'pin 2' }], strips: ['J3', 'J2'] },
  { id: 'RV1', kind: 'part', side: 'top', title: 'RV1, the 10 kΩ speed knob', colour: 'green', fromTo: 'pins J10, J11, J12',
    what: 'The knob: pin 1 in the 3V3 strip (fed by W21), the wiper in the GPIO4 strip, pin 3 in the GPIO3 strip.',
    how: 'Snip off its two mounting tabs if it has them. Shaft up, body off the right-hand edge. Solder from the underside.',
    check: 'J10 to J12 reads about 10 kΩ. J10 connects to A15. J11 to J10 changes as you turn the shaft.',
    ends: [{ hole: 'J10', role: '3V3' }, { hole: 'J11', role: 'wiper' }, { hole: 'J12', role: 'GPIO3' }], strips: ['J10', 'J11', 'J12'] },
  { id: 'C3', kind: 'part', side: 'top', title: 'C3, the 470 µF hold-up capacitor, lying down', colour: 'blue', fromTo: '+ I7, − I8',
    what: 'Keeps the Super Mini running through a motor start or stall.',
    how: 'Bend both leads down 90° about 1 mm from the can. **+ lead in I7, the striped (−) side toward I8.** Lay it flat, its end past the right-hand edge.',
    check: 'The stripe faces row 8. I7 connects to A2, I8 connects to B8, and I7 must not connect to I8 (a meter on resistance shows it charging, then open).',
    ends: [{ hole: 'I7', role: '+' }, { hole: 'I8', role: '− stripe' }], strips: ['I7', 'I8'] },
  { id: 'U3', kind: 'part', side: 'top', title: 'U3, the DRV8833 module', colour: 'black', fromTo: 'pins D3–D6, H2, H3, H6',
    what: 'The motor driver: VCC, GND, IN3 and IN4 down column D, OUT4, OUT3 and EEP in column H.',
    how: '**Check the labels on your module first** (see perfboard.md). Only these seven pins are fitted. Component side up, solder from the underside.',
    check: 'D3 (VCC) must not connect to D4 (GND). VCC connects to B7, GND to B8, H6 (EEP) to C13.', ends: [] },
  { id: 'U2', kind: 'part', side: 'top', title: 'U2, the ESP32-C6 Super Mini', colour: 'black', fromTo: 'pins D12–D17, H11–H15',
    what: 'The controller: GPIO18–20, 3V3, GND and 5V down column D; GPIO4 to GPIO0 in column H. H10 (GPIO5) stays out.',
    how: 'Component side up, USB-C past row 17. Solder from the underside.',
    check: 'The 5V, 3V3 and GND pins must not connect to each other. 5V connects to A2, GND to B8, 3V3 to J10.', ends: [] },
  { id: 'U1', kind: 'part', side: 'top', title: 'U1, the PD trigger board, on foam pads', colour: 'black', fromTo: 'over A–C, rows 9–17',
    what: 'The USB-C power input. It isn\'t soldered into holes: it sits on two 3 mm foam pads over the Super Mini\'s left-hand strips.',
    how: 'Screw terminal desoldered and switches set to 5 V (Before you start). Stick it down with its USB-C past row 17, beside the Super Mini\'s, and its two output pads toward row 8. **Find + and − by the marking printed on your own PD board** (+ / − or VBUS / GND), not by this picture: boards differ. On ours, − (GND) is the outer pad, toward the column A edge, and + (VBUS) the inner one, toward the channel, as drawn.',
    check: 'Before soldering W1 and W2: plug a USB-C charger into U1 and measure across its two pads on DC volts. The pad that reads +5 V against the other is + (W1, red); the other is − (W2, black). Unplug it again. Its USB-C sits about 1.4 mm past the board edge, and it lies flat.', ends: [] },
  { id: 'W1', kind: 'jumper', colour: 'red', from: 'U1 +', to: 'A7', fromRole: 'U1 + pad', toRole: '5V bus',
    what: 'U1\'s 5 V output, from its + (VBUS) pad, down to the +5V bus (row 7 left). On our board + is the inner pad; go by the marking on yours. Run it between A8 and B8, so it doesn\'t cross W2.',
    check: 'Before soldering, make sure this is the pad marked + (or VBUS), the one that read +5 V in step 17. After: U1\'s + pad connects to B7 and A3. It must not connect to U1\'s − pad.' },
  { id: 'W2', kind: 'jumper', colour: 'black', from: 'U1 −', to: 'A8', fromRole: 'U1 − pad', toRole: 'GND bus',
    what: 'U1\'s ground, from its − (GND) pad, down to the GND bus (row 8 left). On our board − is the outer pad; go by the marking on yours.',
    check: 'Before soldering, make sure this is the pad marked − (or GND). After: U1\'s − pad connects to B8 and the Super Mini\'s GND. U1 + must not connect to U1 −.' },

  // W11 comes first on the drive: C1 stands over A4, so W11's joint there has to be made before C1 goes on
  { id: 'W11', kind: 'off', colour: 'black', from: 'button lead B', to: 'A4', toRole: 'GND', offText: 'the button (lead B)',
    what: 'The button\'s ground side, straight from the button to GND (strip 4 left). Solder it before C1: C1 stands over A4.',
    check: 'The button\'s lead B connects to B8. It must not connect to I14. Keep the joint at A4 low and neat: C1 stands just above it.' },
  { id: 'C1', kind: 'part', side: 'top', title: 'C1, the 470 µF bulk capacitor, standing', colour: 'blue', fromTo: '+ C7, − B4',
    what: 'Steadies the 5 V bus when the motor starts. It stands over columns A–C, rows 4–7, between D1 and W1/W2, and its top comes through the cover\'s 9 mm hole.',
    how: '**+ is the long leg, − is the striped side** (see the sketch). Slip heat-shrink over both leads: they cross the pads of rows 5 and 6 (IN3, IN4) on the way out. Stand the can 1.5 mm off the board over A4–C6, stripe toward row 4, bend the + lead out to C7 and the − lead to B4, and solder them from the underside.',
    check: 'The stripe faces row 4. C7 connects to A7, B4 connects to B8, and C7 must not connect to B4 (on resistance it shows the capacitor charging, then open). Neither lead touches the pads of rows 5 or 6: A5 and A6 must not connect to A7 or B8.',
    ends: [{ hole: 'C7', role: '+ long' }, { hole: 'B4', role: '− stripe' }], strips: ['C7', 'B4'] },
  { id: 'W6', kind: 'off', colour: 'white', from: 'button lead A', to: 'I14', toRole: 'GPIO1', offText: 'the button (lead A)',
    what: 'BTN: the button\'s other lead, straight to GPIO1 (strip 14 right). Pressing the button joins GPIO1 to ground.',
    check: 'The button\'s lead A connects to H14 (GPIO1), and to B8 only while you hold the button down. I14 must not connect to I13 or I15.' },

  { id: 'W14', kind: 'motor', colour: 'red', from: 'motor tab', to: 'motor plug pin 1', toRole: 'motor plug (J4) pin 1 (hole J3, OUT3)', pin: 1,
    what: 'The motor\'s own red lead, to the driver\'s OUT3 through pin 1 of the motor plug (J4).',
    check: 'With the plug in, the motor plug\'s housing pin 1 connects to the motor tab its lead comes from, and to H3.' },
  { id: 'W15', kind: 'motor', colour: 'black', from: 'motor tab', to: 'motor plug pin 2', toRole: 'motor plug (J4) pin 2 (hole J2, OUT4)', pin: 2,
    what: 'The motor\'s own black lead, to the driver\'s OUT4 through pin 2 of the motor plug (J4).',
    check: 'With the plug in, pin 2 connects to the other motor tab and to H2. On resistance, J3 to J2 reads the winding of the motor, a few ohms or more: 0 Ω means the leads or C2 are shorted.' },
];

// ---- cut list: the one source for colour, gauge and length ----
const csv = readFileSync(join(DOCS, 'cut-list.csv'), 'utf8').replace(/\r/g, '').trim().split('\n');
// columns: wire,colour,gauge,from,to,routed_mm,cut_mm,notes. The file isn't quoted, so the gauge and
// notes columns may hold bare commas: find routed and cut as the first two all-digit fields, and the
// 'from' column as the first field after the gauge that names a terminal.
const CUT = {};
for (const line of csv.slice(1)) {
  const f = line.split(',');
  let i = 2; while (!(/^\d+$/.test(f[i]) && /^\d+$/.test(f[i + 1]))) i++;
  let g = 3; while (g < i && !/^(perfboard|U1|C1|J3|J4|S1)/.test(f[g])) g++;
  CUT[f[0]] = { colour: f[1], gauge: f.slice(2, g).join(','), from: f[g], to: f.slice(g + 1, i).join(','), routed: +f[i], cut: +f[i + 1] };
}

const errors = [];
for (const s of STEPS) {
  if (s.kind === 'part') continue;
  const c = CUT[s.id];
  if (!c) { errors.push(`${s.id}: not in cut-list.csv`); continue; }
  if (c.colour !== s.colour) errors.push(`${s.id}: colour ${s.colour} here, ${c.colour} in cut-list.csv`);
  s.cut = c.cut; s.gauge = c.gauge;
  const holes = [s.from, s.to].filter(h => /^[A-J]\d+$/.test(h));
  for (const h of holes) if (!(`${c.from} ${c.to}`).includes(h)) errors.push(`${s.id}: hole ${h} not in cut-list.csv (${c.from} → ${c.to})`);
}
if (errors.length) { console.error(errors.join('\n')); process.exit(1); }

// ---- drawing a step ----
function board(highlightStrips) {
  let o = '';
  o += `<rect x="${OX}" y="${OY}" width="${mm(W)}" height="${mm(H)}" rx="${mm(2)}" fill="#e3eaf1" stroke="#9fb0c2" stroke-width="1.5"/>`;
  for (let r = 1; r <= 17; r++) for (const [a, b] of [['A', 'E'], ['F', 'J']]) {
    const hl = highlightStrips.some(h => +h.slice(1) === r && (colIdx[h[0]] < 5) === (a === 'A'));
    const xa = Math.min(hx(a), hx(b)), xb = Math.max(hx(a), hx(b));
    o += `<rect x="${xa - mm(0.95)}" y="${hy(r) - mm(0.95)}" width="${xb - xa + mm(1.9)}" height="${mm(1.9)}" rx="${mm(0.95)}" fill="${hl ? HL : '#c6d3e2'}"/>`;
  }
  for (const [x, y, d] of [[0, -20.3, 3.2], [0, 20.3, 3.2], [-15.9, -22.25, 2.2], [15.9, -22.25, 2.2], [-15.9, 22.25, 2.2], [15.9, 22.25, 2.2]])
    o += `<circle cx="${ux(x)}" cy="${vy(y)}" r="${mm(d / 2)}" fill="#ffffff" stroke="#9fb0c2"/>`;
  for (let r = 1; r <= 17; r++) for (const c of COLS)
    o += `<circle cx="${hx(c)}" cy="${hy(r)}" r="${mm(0.7)}" fill="#c9a640"/><circle cx="${hx(c)}" cy="${hy(r)}" r="${mm(0.35)}" fill="#5a4c22"/>`;
  return o;
}
function axisLabels(skipRows) {
  let o = '';
  for (const c of COLS) o += `<text x="${hx(c)}" y="${OY - 10}" text-anchor="middle" ${MONO} font-size="18" font-weight="700" fill="${INK}">${mirror ? printedCol(c) : c}</text>`;
  for (let r = 1; r <= 17; r++) for (const side of ['L', 'R']) {
    if (skipRows.has(`${side}${r}`)) continue;
    const x = side === 'L' ? OX - 15 : OX + mm(W) + 15;
    o += `<text x="${x}" y="${hy(r) + 5}" text-anchor="middle" ${MONO} font-size="15" font-weight="700" fill="${MUTE}">${r}</text>`;
  }
  return o;
}
function wrap(text, n) {
  const words = text.replace(/\*\*/g, '').split(' '); const lines = []; let cur = '';
  for (const w of words) { if ((cur + ' ' + w).trim().length > n) { lines.push(cur.trim()); cur = w; } else cur += ' ' + w; }
  if (cur.trim()) lines.push(cur.trim());
  return lines;
}
function sideOf(id) { const s = STEPS.find(t => t.id === id); return s.kind === 'link' ? 'under' : 'top'; }

function drawItem(s, mode, viewSide) {
  const behind = viewSide === 'top';          // links seen through the board from the top
  if (s.kind === 'part') return PART[s.id](mode);
  if (s.kind === 'link') return wire(ROUTE[s.id](), s.colour, mode, behind);
  if (s.kind === 'jumper') return wire(ROUTE[s.id](), s.colour, mode, false);
  if (s.kind === 'off') {
    const p = offPath(s.id), [x, y] = p[0];
    if (mode === 'done') return `<circle cx="${x}" cy="${y}" r="${mm(0.9)}" fill="none" stroke="${DONE}" stroke-width="3"/>` + wire(p.slice(0, 2).map((q, i) => i ? [(q[0] + x) / 2, (q[1] + y) / 2] : q), s.colour, 'done', true);
    // the wire runs behind the board (dashed, lighter) and comes up through its hole (a dot in a ring): it never crosses the top
    const c = s.colour === 'white' ? '#8f8f88' : WIRE[s.colour];
    return `<polyline points="${pts(p)}" fill="none" stroke="${c}" stroke-opacity="0.7" stroke-width="4" stroke-dasharray="9 6" stroke-linecap="round" stroke-linejoin="round"/>` +
      `<circle cx="${x}" cy="${y}" r="8" fill="#ffffff" stroke="#1b1b1b" stroke-width="2"/><circle cx="${x}" cy="${y}" r="6" fill="none" stroke="${c}" stroke-width="2.5"/><circle cx="${x}" cy="${y}" r="3" fill="${s.colour === 'white' ? '#444' : c}"/>`;
  }
  if (s.kind === 'motor') {
    const p = motorPath(s.pin);
    if (mode === 'done') return wire(p, s.colour, 'done', false);
    return PART.J4('now') + wire(p, s.colour, 'now', false);
  }
}

function endsOf(s) {
  if (s.kind === 'part') return s.ends.map(e => ({ ...e, xy: hole(e.hole) }));
  if (s.kind === 'link') return [{ hole: s.from, role: s.fromRole, xy: hole(s.from) }, { hole: s.to, role: s.toRole, xy: hole(s.to) }];
  if (s.kind === 'jumper') { const p = ROUTE[s.id](); return [{ hole: s.from.replace(' ', ' '), role: 'pad', xy: p[0], pad: true }, { hole: s.to, role: s.toRole, xy: hole(s.to) }]; }
  if (s.kind === 'off') { const h = OFF[s.id].hole; return [{ hole: h, role: s.toRole, xy: hole(h), off: s.offText }]; }
  if (s.kind === 'motor') { const h = s.pin === 1 ? 'J3' : 'J2'; return [{ hole: h, role: `J4 pin ${s.pin}`, xy: hole(h), motor: true }]; }
}

// the board seen edge-on, for a wire that comes from the mount: along the gap behind the board, up through its hole,
// soldered on the component side. Not to scale: the 1.6 mm board and the 5.8 mm gap are drawn bigger.
function sideView(s, y0) {
  const h = OFF[s.id].hole, c = WIRE[s.colour], o = [];
  const bt = y0 + 70, bb = bt + 12, mt = bb + 64, hx0 = 340;
  o.push(`<text x="24" y="${y0 + 18}" ${FONT} font-size="17" font-weight="700" fill="${INK}">Seen from the side (edge-on):</text>`);
  o.push(`<text x="${hx0}" y="${y0 + 44}" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="#0e4a52">component side: you solder here</text>`);
  o.push(`<rect x="70" y="${bt}" width="460" height="${bb - bt}" fill="#c6d3e2" stroke="#7d91a8" stroke-width="1.5"/>`);
  o.push(`<rect x="${hx0 - 5}" y="${bt}" width="10" height="${bb - bt}" fill="#ffffff"/>`);
  o.push(`<text x="76" y="${bt - 8}" ${FONT} font-size="16" font-weight="600" fill="${MUTE}">perfboard</text>`);
  o.push(`<rect x="24" y="${mt}" width="${VW - 48}" height="34" fill="#5f9e5f"/>`);
  o.push(`<text x="${VW / 2}" y="${mt + 23}" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="#ffffff">mount (its −Y face)</text>`);
  o.push(`<text x="536" y="${bb + 24}" text-anchor="end" ${FONT} font-size="16" fill="${MUTE}">underside, facing the mount</text>`);
  // the wire: along the gap from the left, up through the hole, a solder fillet and a short trimmed stub on top
  const p = [[24, mt - 14], [hx0 - 40, mt - 14], [hx0, bb + 14], [hx0, bt - 14]];
  o.push(`<polyline points="${pts(p)}" fill="none" stroke="#1b1b1b" stroke-width="9" stroke-linejoin="round"/>`);
  o.push(`<polyline points="${pts(p)}" fill="none" stroke="${c}" stroke-width="6" stroke-linejoin="round"/>`);
  o.push(`<path d="M ${hx0 - 13} ${bt} Q ${hx0} ${bt - 16} ${hx0 + 13} ${bt} Z" fill="#c0c0c8" stroke="#6d6d75" stroke-width="1.5"/>`);
  o.push(`<text x="${hx0 + 22}" y="${bt - 10}" ${FONT} font-size="16" font-weight="600" fill="${INK}">solder, trim the stub</text>`);
  o.push(`<text x="${hx0 - 22}" y="${bt - 10}" text-anchor="end" ${MONO} font-size="18" font-weight="700" fill="${RING}">${h}</text>`);
  o.push(`<text x="28" y="${bb + 24}" ${FONT} font-size="16" font-weight="600" fill="${INK}">from ${esc(s.offText)} →</text>`);
  return { svg: o.join('\n'), h: mt + 34 - y0 };
}

// one orientation picture for the start of the "on the drive" steps
function onDriveSvg() {
  const alt = 'The perfboard on the drive, seen edge-on. The mount is at the bottom; its -Y face holds the board on supports, with a gap between them. The board\'s underside faces the mount and its component side faces out, where you solder. The button\'s two wires come from the pod through the base tunnel, run up the mount\'s face in the gap behind the board, and each comes up through its own hole to be soldered on top. C1 stands on the board and its top comes through the cover. Nothing wraps round the board\'s edge.';
  const o = [], bt = 236, bb = 248, mt = 360;
  o.push(`<text x="24" y="34" ${FONT} font-size="20" font-weight="700" fill="${INK}">The board on the drive, seen edge-on</text>`);
  o.push(`<text x="24" y="60" ${FONT} font-size="16" fill="${MUTE}">Not to scale: the board and the gap behind it are drawn bigger.</text>`);
  // cover over everything, with C1 through its hole
  o.push(`<polyline points="64,${bt} 64,150 536,150 536,${bt}" fill="none" stroke="#8a8a8a" stroke-width="2.5" stroke-dasharray="8 6"/>`);
  o.push(`<text x="530" y="142" text-anchor="end" ${FONT} font-size="16" font-weight="600" fill="${MUTE}">cover (goes on last)</text>`);
  o.push(`<rect x="100" y="112" width="44" height="${bt - 112 - 6}" rx="5" fill="#1e3c8c"/><text x="122" y="104" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="${INK}">C1</text>`);
  o.push(`<rect x="180" y="200" width="150" height="30" fill="#2b2b2b" fill-opacity="0.75"/><rect x="350" y="196" width="150" height="34" fill="#2b2b2b" fill-opacity="0.75"/>`);
  o.push(`<text x="340" y="186" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="#0e4a52">component side: faces out, you solder here</text>`);
  o.push(`<rect x="64" y="${bt}" width="472" height="${bb - bt}" fill="#c6d3e2" stroke="#7d91a8" stroke-width="1.5"/>`);
  for (const x of [64, 516]) o.push(`<rect x="${x}" y="${bb}" width="20" height="${mt - bb}" fill="#5f9e5f"/>`);
  o.push(`<rect x="24" y="${mt}" width="${VW - 48}" height="40" fill="#5f9e5f"/>`);
  o.push(`<text x="${VW / 2}" y="${mt + 26}" text-anchor="middle" ${FONT} font-size="17" font-weight="700" fill="#ffffff">mount (its −Y face), with the supports</text>`);
  o.push(`<text x="94" y="${bb + 22}" ${FONT} font-size="16" fill="${MUTE}">underside faces the mount</text>`);
  // the button's two wires: along the gap from the base tunnel, each up through its own hole
  [['black', 300], ['white', 450]].forEach(([col, x], i) => {
    const yy = mt - 12 - i * 8, p = [[24, yy], [x - 18, yy], [x, bb + 8], [x, bt - 10]];
    o.push(`<polyline points="${pts(p)}" fill="none" stroke="#1b1b1b" stroke-width="7" stroke-linejoin="round"/><polyline points="${pts(p)}" fill="none" stroke="${WIRE[col]}" stroke-width="4.5" stroke-linejoin="round"/>`);
    o.push(`<path d="M ${x - 9} ${bt} Q ${x} ${bt - 12} ${x + 9} ${bt} Z" fill="#c0c0c8" stroke="#6d6d75"/>`);
  });
  o.push(`<text x="94" y="${mt - 64}" ${FONT} font-size="16" font-weight="600" fill="${INK}">the button's two wires,</text>`);
  o.push(`<text x="94" y="${mt - 46}" ${FONT} font-size="16" font-weight="600" fill="${INK}">through the base tunnel →</text>`);
  o.push(`<text x="24" y="${mt + 72}" ${FONT} font-size="17" font-weight="700" fill="${INK}">Nothing wraps round the edge: each button wire runs</text>`);
  o.push(`<text x="24" y="${mt + 96}" ${FONT} font-size="17" font-weight="700" fill="${INK}">behind the board and comes up through its own hole.</text>`);
  return { alt, svg: sketch(mt + 114, alt, o.join('\n')) };
}

function svgFor(n, s) {
  const viewSide = s.kind === 'link' ? 'under' : 'top';
  mirror = viewSide === 'under';
  const done = STEPS.slice(0, n - 1);
  const ends = endsOf(s);
  const strips = s.kind === 'part' ? (s.strips || []) : ends.filter(e => /^[A-J]\d+$/.test(e.hole) && !e.motor).map(e => e.hole);
  // margin labels, one per end, on the side the hole is on (off-board wires and the motor leads have their own)
  const labels = [];
  for (const e of ends) {
    if (e.off || e.motor) continue;
    const left = e.xy[0] < OX + mm(W) / 2;
    labels.push({ ...e, left, y: e.xy[1] });
  }
  for (const side of [true, false]) {
    const g = labels.filter(l => l.left === side).sort((a, b) => a.y - b.y);
    for (let i = 1; i < g.length; i++) if (g[i].y - g[i - 1].y < (mirror ? 44 : 24)) g[i].y = g[i - 1].y + (mirror ? 44 : 24);
  }
  const skip = new Set();
  for (const l of labels) for (let r = 1; r <= 17; r++) if (hy(r) - l.y > -12 && hy(r) - l.y < (mirror ? 30 : 12)) skip.add(`${l.left ? 'L' : 'R'}${r}`);
  const partExt = ['C3', 'RV1'].filter(id => done.some(d => d.id === id) || s.id === id);
  const ROWS = { C3: [5, 6, 7, 8], RV1: [9, 10, 11, 12, 13] };
  if (viewSide === 'top') for (const id of partExt) for (const r of ROWS[id]) skip.add(`R${r}`);
  // off-board labels: two lines round the leader's row
  for (const e of ends) if (e.off || e.motor) {
    const side = e.off ? OFF[s.id].side : 'R', yl = e.off ? hy(OFF[s.id].ly) : e.xy[1];
    for (let r = 1; r <= 17; r++) if (hy(r) - yl > -30 && hy(r) - yl < (e.off ? 44 : 30)) skip.add(`${side}${r}`);
  }

  let o = [];
  const title = s.kind === 'part' ? s.title : `${s.id} · ${s.colour}`;
  o.push(`<rect width="${VW}" height="__VH__" fill="#ffffff"/>`);
  // header
  o.push(`<text x="24" y="34" ${FONT} font-size="17" font-weight="700" fill="${MUTE}">STEP ${n} OF ${STEPS.length}</text>`);
  const pill = viewSide === 'under' ? ['UNDERSIDE', '#7a3e00', '#ffe2c2'] : ['COMPONENT SIDE', '#0e4a52', '#d4eef1'];
  o.push(`<rect x="${VW - 24 - (pill[0].length * 11 + 24)}" y="14" width="${pill[0].length * 11 + 24}" height="30" rx="15" fill="${pill[2]}"/>`);
  o.push(`<text x="${VW - 24 - (pill[0].length * 11 + 24) / 2}" y="35" text-anchor="middle" ${FONT} font-size="16" font-weight="700" fill="${pill[1]}">${pill[0]}</text>`);
  if (s.kind === 'part') {
    o.push(`<text x="24" y="78" ${FONT} font-size="${s.title.length > 34 ? 22 : 26}" font-weight="700" fill="${INK}">${esc(s.title.split(',')[0])}</text>`);
    o.push(`<text x="24" y="108" ${FONT} font-size="18" fill="${MUTE}">${esc(s.title.split(',').slice(1).join(',').trim())}</text>`);
    o.push(`<text x="24" y="146" ${MONO} font-size="24" font-weight="700" fill="${INK}">${esc(s.fromTo)}</text>`);
  } else {
    o.push(`<circle cx="38" cy="70" r="13" fill="${WIRE[s.colour]}" stroke="#1b1b1b" stroke-width="2"/>`);
    o.push(`<text x="62" y="80" ${FONT} font-size="32" font-weight="700" fill="${INK}">${s.id}</text>`);
    o.push(`<text x="${62 + s.id.length * 22 + 14}" y="80" ${FONT} font-size="24" fill="${INK}">${s.colour} · cut <tspan font-weight="700">${s.cut} mm</tspan>${s.kind === 'motor' ? ' from the end cap' : ''}</text>`);
    o.push(`<text x="24" y="122" ${MONO} font-size="${(s.from + s.to).length > 22 ? 21 : 26}" font-weight="700" fill="${INK}">${esc(s.from)} → ${esc(s.to)}` +
      (viewSide === 'under' ? `<tspan ${FONT} font-size="18" font-weight="600" fill="${RING}">  printed ${under(s.from)} → ${under(s.to)} underneath</tspan>` : '') + `</text>`);
    o.push(`<text x="24" y="152" ${FONT} font-size="16" fill="${MUTE}">${esc(s.kind === 'motor' ? 'the motor\'s own lead, about 26 AWG (assumed)' : s.gauge)}</text>`);
  }
  const orient = viewSide === 'under' ? 'Underside, as you see it. Letters are as PRINTED here (mirrored).' : (s.kind === 'off' ? 'Push it up from the underside, solder on this side.' : 'Column A on the left, row 1 at the top.');
  o.push(`<text x="24" y="${OY - 40}" ${FONT} font-size="16" font-weight="600" fill="${viewSide === 'under' ? '#7a3e00' : MUTE}">${orient}</text>`);

  o.push(board(strips));
  // done items: links on the far side first, then the near side
  for (const d of done) if (drawItem(d, 'done', viewSide) && ((d.kind === 'link') === (viewSide === 'top'))) o.push(drawItem(d, 'done', viewSide));
  for (const d of done) if ((d.kind === 'link') !== (viewSide === 'top')) o.push(drawItem(d, 'done', viewSide));
  o.push(axisLabels(skip));
  // now
  o.push(drawItem(s, 'now', viewSide));
  // end rings and labels
  for (const e of ends) o.push(`<circle cx="${e.xy[0].toFixed(1)}" cy="${e.xy[1].toFixed(1)}" r="11" fill="none" stroke="${RING}" stroke-width="4"/>`);
  const rightLimit = OX + mm(W) + (partExt.length && viewSide === 'top' ? mm(6.6) : 0);
  for (const l of labels) {
    const xl = l.left ? OX - 32 : Math.max(OX + mm(W) + 32, rightLimit + 8);
    const xe = l.left ? l.xy[0] - 11 : l.xy[0] + 11;
    o.push(`<polyline points="${pts([[xe, l.xy[1]], [l.left ? OX - 4 : rightLimit + 2, l.y], [xl + (l.left ? 4 : -4), l.y]])}" fill="none" stroke="${RING}" stroke-width="2.5"/>`);
    const anchor = `text-anchor="${l.left ? 'end' : 'start'}"`;
    if (mirror) {
      o.push(`<text x="${xl}" y="${l.y + 7}" ${anchor} ${MONO} font-size="22" font-weight="700" fill="${RING}">${esc(under(l.hole))}</text>`);
      o.push(`<text x="${xl}" y="${l.y + 27}" ${anchor} ${FONT} font-size="14" font-weight="600" fill="${INK}">top ${esc(l.hole)} · ${esc(l.role)}</text>`);
      continue;
    }
    const name = l.hole;
    o.push(`<text x="${xl}" y="${l.y + 7}" ${anchor} ${MONO} font-size="20" font-weight="700" fill="${RING}">${esc(name)}</text>`);
    o.push(`<text x="${xl + (l.left ? -name.length * 12.3 - 6 : name.length * 12.3 + 6)}" y="${l.y + 6}" text-anchor="${l.left ? 'end' : 'start'}" ${FONT} font-size="14" font-weight="600" fill="${INK}">${esc(l.pad ? '' : l.role)}</text>`);
  }
  // off-board ends
  for (const e of ends) {
    if (e.off) {
      const o2 = OFF[s.id], left = o2.side === 'L', x = left ? OX - 38 : OX + mm(W) + 30, y = hy(o2.ly);
      o.push(`<text x="${x}" y="${y - 6}" text-anchor="${left ? 'end' : 'start'}" ${MONO} font-size="20" font-weight="700" fill="${RING}">${e.hole}</text>`);
      o.push(`<text x="${x}" y="${y + 16}" text-anchor="${left ? 'end' : 'start'}" ${FONT} font-size="14" font-weight="600" fill="${INK}">behind board</text>`);
      o.push(`<text x="${x}" y="${y + 34}" text-anchor="${left ? 'end' : 'start'}" ${FONT} font-size="14" font-weight="600" fill="${INK}">→ ${esc(e.off.replace(/^the /, '').replace(/ \(lead .\)$/, ''))}</text>`);
    }
    if (e.motor) {
      const x = OX + mm(W) + 38, y = e.xy[1];
      o.push(`<text x="${x}" y="${y - 8}" ${FONT} font-size="17" font-weight="700" fill="${RING}">motor plug pin ${s.pin}</text>`);
      o.push(`<text x="${x}" y="${y + 12}" ${FONT} font-size="14" font-weight="600" fill="${INK}">to the motor</text>`);
    }
  }
  // through-board wires: a key for the top view, then the board seen edge-on
  let y = OY + mm(H) + 52;
  if (s.kind === 'off') {
    const h = OFF[s.id].hole, c = s.colour === 'white' ? '#8f8f88' : WIRE[s.colour];
    let ky = OY + mm(H) + 36;
    const key = [
      [yc => `<circle cx="36" cy="${yc}" r="8" fill="#fff" stroke="#1b1b1b" stroke-width="2"/><circle cx="36" cy="${yc}" r="3" fill="${s.colour === 'white' ? '#444' : c}"/>`,
        `${h}: the wire comes up through here from below (printed ${under(h)} underneath).`],
      [yc => `<line x1="22" y1="${yc}" x2="50" y2="${yc}" stroke="${c}" stroke-width="4" stroke-dasharray="9 6"/>`,
        `Dashed: behind the board, between it and the mount, back to ${s.offText} in the pod.`],
    ];
    for (const [icon, text] of key) {
      const lines = wrap(text, 56);
      o.push(icon(ky - 5));
      lines.forEach((t, i) => o.push(`<text x="60" y="${ky + i * 21}" ${FONT} font-size="16" fill="${INK}">${esc(t)}</text>`));
      ky += lines.length * 21 + 8;
    }
    const sv = sideView(s, ky + 4);
    o.push(sv.svg);
    y = ky + 4 + sv.h + 40;
  }
  // check
  const chk = wrap('Check: ' + s.check, 58);
  o.push(`<rect x="16" y="${y - 26}" width="${VW - 32}" height="${chk.length * 22 + 18}" rx="8" fill="#f4f6f7" stroke="#d5dddc"/>`);
  chk.forEach((t, i) => o.push(`<text x="28" y="${y + i * 22}" ${FONT} font-size="16" fill="${INK}">${i === 0 ? `<tspan font-weight="700">Check:</tspan>${esc(t.slice(6))}` : esc(t)}</text>`));
  const VH = Math.ceil(y + chk.length * 22 + 10);
  const alt = altFor(n, s, viewSide);
  return `<?xml version="1.0" encoding="UTF-8"?>\n<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${VW} ${VH}" role="img" aria-label="${esc(alt)}">\n` + o.join('\n').replace('__VH__', VH) + '\n</svg>\n';
}

function altFor(n, s, viewSide) {
  const view = viewSide === 'under' ? 'The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there' : 'The perfboard seen from the component side, column A on the left';
  const prev = n === 2 ? ', with the earlier step drawn in grey' : n > 2 ? `, with the ${n - 1} earlier steps drawn in grey` : '';
  let now;
  if (s.kind === 'part') now = `${s.title.split(',')[0]} drawn in colour, ${s.fromTo}${s.ends.length ? ', its holes circled' : ''}`;
  else if (s.kind === 'off') now = `Hole ${OFF[s.id].hole} (printed ${under(OFF[s.id].hole)} on the underside) is marked with a dot in a ring where ${s.id}, ${s.colour}, comes up from below; a dashed line shows it running behind the board to the ${OFF[s.id].side === 'L' ? 'left' : 'right'} edge, on to ${s.offText}. Below, a side view shows the board edge-on over the mount: the wire runs along the gap behind the board, up through the hole, and is soldered on the component side with its stub trimmed`;
  else if (s.kind === 'motor') now = `${s.id}, ${s.colour}, drawn bold from the motor plug's mouth off the right-hand edge; the motor plug (J4) pin ${s.pin} is hole ${s.pin === 1 ? 'J3' : 'J2'}`;
  else if (viewSide === 'under') now = `${s.id}, ${s.colour}, drawn bold from ${s.from} to ${s.to} (printed ${under(s.from)} and ${under(s.to)} on the underside), both ends circled and labelled with both names`;
  else now = `${s.id}, ${s.colour}, drawn bold from ${s.from} to ${s.to}, both ends circled`;
  return `Step ${n}: ${view}${prev}. ${now}.`;
}

// ---- polarity sketches for the two parts that only go in one way ----
const sketch = (h, alt, body) => `<?xml version="1.0" encoding="UTF-8"?>\n<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${VW} ${h}" role="img" aria-label="${esc(alt)}">\n<rect width="${VW}" height="${h}" fill="#ffffff"/>\n${body}\n</svg>\n`;
const T = (x, y, t, size = 16, opts = '') => `<text x="${x}" y="${y}" ${FONT} font-size="${size}" ${/fill=/.test(opts) ? '' : `fill="${INK}"`} ${opts}>${t}</text>`;
const POLARITY = {
  D1: {
    file: 'd1-polarity.svg',
    alt: 'Which end of the 1N5817 is which. Its body is black with a grey band at one end. The band end is the cathode, minus, and goes to A2; the plain end is the anode, plus, and goes to A3. In the schematic symbol, a triangle pointing at a bar, the bar is the band. Standing on the board, seen from the side: the body stands in A3 with the band at the top, and the top lead bends over and down into A2.',
    svg() {
      const o = [];
      o.push(T(24, 34, 'D1 · 1N5817: which end is which', 20, 'font-weight="700"'));
      // the part, lying
      o.push(`<line x1="40" y1="110" x2="560" y2="110" stroke="#8d8d96" stroke-width="5"/>`);
      o.push(`<rect x="200" y="84" width="200" height="52" rx="8" fill="#151515"/>`);
      o.push(`<rect x="356" y="84" width="26" height="52" fill="#c9c9cf"/>`);
      o.push(T(40, 74, 'anode (+)', 18, 'font-weight="700"'), T(40, 158, 'plain end → A3', 16));
      o.push(T(560, 74, 'cathode (−)', 18, 'font-weight="700" text-anchor="end"'), T(560, 158, 'band end → A2', 16, 'text-anchor="end"'));
      o.push(`<line x1="369" y1="140" x2="369" y2="168" stroke="${RING}" stroke-width="2.5"/>`, T(369, 188, 'band', 17, `font-weight="700" text-anchor="middle" fill="${RING}"`));
      // symbol
      const sy = 250;
      o.push(`<line x1="150" y1="${sy}" x2="270" y2="${sy}" stroke="${INK}" stroke-width="3"/><polygon points="270,${sy - 24} 270,${sy + 24} 312,${sy}" fill="${INK}"/><line x1="314" y1="${sy - 26}" x2="314" y2="${sy + 26}" stroke="${INK}" stroke-width="5"/><line x1="314" y1="${sy}" x2="450" y2="${sy}" stroke="${INK}" stroke-width="3"/>`);
      o.push(T(140, sy + 6, 'A3  +', 18, 'font-weight="700" text-anchor="end"'), T(460, sy + 6, '−  A2', 18, 'font-weight="700"'));
      o.push(`<line x1="314" y1="${sy + 30}" x2="314" y2="${sy + 44}" stroke="${RING}" stroke-width="2.5"/>`, T(314, sy + 64, 'the bar is the band', 17, `font-weight="700" text-anchor="middle" fill="${RING}"`));
      // standing on the board, from the side
      const by = 500, a3 = 260, a2 = 330;
      o.push(T(24, 348, 'On the board, seen from the side:', 17, 'font-weight="700"'));
      o.push(`<rect x="150" y="${by}" width="300" height="12" fill="#c6d3e2"/>`);
      o.push(`<line x1="${a3}" y1="${by + 12}" x2="${a3}" y2="${by - 20}" stroke="#8d8d96" stroke-width="5"/>`);
      o.push(`<rect x="${a3 - 17}" y="${by - 100}" width="34" height="80" rx="6" fill="#151515"/><rect x="${a3 - 17}" y="${by - 100}" width="34" height="14" fill="#c9c9cf"/>`);
      o.push(`<path d="M ${a3} ${by - 100} L ${a3} ${by - 112} Q ${a3} ${by - 124} ${a3 + 12} ${by - 124} L ${a2 - 12} ${by - 124} Q ${a2} ${by - 124} ${a2} ${by - 112} L ${a2} ${by + 12}" fill="none" stroke="#8d8d96" stroke-width="5"/>`);
      o.push(T(a3, by + 40, 'A3', 18, 'font-weight="700" text-anchor="middle"'), T(a2, by + 40, 'A2', 18, 'font-weight="700" text-anchor="middle"'));
      o.push(`<line x1="${a3 - 22}" y1="${by - 93}" x2="200" y2="${by - 93}" stroke="${RING}" stroke-width="2.5"/>`, T(194, by - 87, 'band at the top', 16, `font-weight="700" text-anchor="end" fill="${RING}"`));
      return sketch(by + 60, this.alt, o.join('\n'));
    },
  },
  C1: {
    file: 'c1-polarity.svg',
    alt: 'Which leg of C1 is which, and how it stands. The 470 µF can is blue with a light stripe down one side, marked with minus signs. The leg on the stripe side is minus and goes to B4; the other leg is plus, the longer one on a new part, and goes to C7. Seen from the side, the can stands 1.5 mm off the board with both legs sleeved and bent out to their holes.',
    svg() {
      const o = [];
      o.push(T(24, 34, 'C1 · 470 µF: which leg is which', 20, 'font-weight="700"'));
      // standing can, front view: stripe on the left
      const x0 = 230, y0 = 70, w = 120, h = 190, by = y0 + h;
      o.push(`<rect x="${x0}" y="${y0}" width="${w}" height="${h}" rx="12" fill="#1e3c8c"/>`);
      o.push(`<rect x="${x0}" y="${y0}" width="38" height="${h}" fill="#c8c8cd"/>`);
      for (let y = y0 + 40; y < by - 10; y += 46) o.push(T(x0 + 19, y, '−', 26, 'font-weight="700" text-anchor="middle"'));
      o.push(`<line x1="${x0 + 30}" y1="${by}" x2="${x0 + 30}" y2="${by + 50}" stroke="#8d8d96" stroke-width="5"/>`);
      o.push(`<line x1="${x0 + 90}" y1="${by}" x2="${x0 + 90}" y2="${by + 80}" stroke="#8d8d96" stroke-width="5"/>`);
      o.push(T(x0 - 14, by + 44, '− → B4', 20, 'font-weight="700" text-anchor="end"'), T(x0 - 14, by + 66, 'stripe side', 15, 'text-anchor="end"'));
      o.push(T(x0 + 104, by + 62, '+ → C7', 20, 'font-weight="700"'), T(x0 + 104, by + 84, 'the long leg', 15));
      o.push(`<line x1="${x0 - 4}" y1="${y0 + 90}" x2="${x0 - 30}" y2="${y0 + 90}" stroke="${RING}" stroke-width="2.5"/>`, T(x0 - 36, y0 + 96, 'stripe = −', 18, `font-weight="700" text-anchor="end" fill="${RING}"`));
      o.push(T(24, by + 128, 'Go by the stripe, not the leg length: the legs may be cut.', 16));
      o.push(T(24, by + 152, 'Sleeve both legs; stand it 1.5 mm up, stripe toward row 4.', 16, 'font-weight="700"'));
      return sketch(by + 174, this.alt, o.join('\n'));
    },
  },
  C3: {
    file: 'c3-polarity.svg',
    alt: 'Which lead of C3 is which. The 470 µF capacitor is a blue can with a light stripe printed down one side, marked with minus signs. The lead on the stripe side is minus and goes to I8; the other lead, usually the longer one, is plus and goes to I7. Lying on the board, the stripe faces row 8.',
    svg() {
      const o = [];
      o.push(T(24, 34, 'C3 · 470 µF: which lead is which', 20, 'font-weight="700"'));
      const y0 = 80;
      o.push(`<rect x="250" y="${y0}" width="280" height="110" rx="16" fill="#1e3c8c"/>`);
      o.push(`<rect x="250" y="${y0 + 72}" width="280" height="38" fill="#c8c8cd"/><rect x="250" y="${y0 + 72}" width="280" height="38" rx="0" fill="none"/>`);
      for (let x = 290; x < 520; x += 60) o.push(T(x, y0 + 100, '−', 26, 'font-weight="700" text-anchor="middle"'));
      o.push(`<line x1="250" y1="${y0 + 30}" x2="70" y2="${y0 + 30}" stroke="#8d8d96" stroke-width="5"/>`);
      o.push(`<line x1="250" y1="${y0 + 90}" x2="120" y2="${y0 + 90}" stroke="#8d8d96" stroke-width="5"/>`);
      o.push(T(70, y0 + 18, '+ → I7', 20, 'font-weight="700"'), T(70, y0 + 60, 'usually the longer lead', 15));
      o.push(T(120, y0 + 124, '− → I8', 20, `font-weight="700" fill="${INK}"`));
      o.push(`<line x1="505" y1="${y0 + 112}" x2="505" y2="${y0 + 124}" stroke="${RING}" stroke-width="2.5"/>`);
      o.push(T(560, y0 + 140, 'stripe = −', 18, `font-weight="700" text-anchor="end" fill="${RING}"`));
      o.push(T(24, y0 + 180, 'Go by the stripe, not the lead length: the leads may be cut.', 16));
      o.push(T(24, y0 + 204, 'Lying on the board, the stripe faces row 8.', 16, 'font-weight="700"'));
      return sketch(y0 + 226, this.alt, o.join('\n'));
    },
  },
};

// ---- "Do this": the step as plain actions, each hole by the name printed on the side being worked ----
const atUnder = h => `printed **${under(h)}** (top ${h})`;          // working on the underside
const atTop = h => `**${h}** (printed ${under(h)} underneath)`;      // working on the component side
const solderUnder = hs => `Turn the board over (underside up) and solder ${hs.length > 1 ? 'each lead' : 'it'} at ${hs.map(atUnder).join(', ')}. Trim the leads.`;
const DO = {
  D1: () => ['The banded end is the cathode (−); the plain end is the anode (+). See the sketch.',
    `On top: stand the anode lead in ${atTop('A3')}, band at the top, and bend the top lead over and down into ${atTop('A2')}.`, solderUnder(['A3', 'A2'])],
  J4: () => ['Its mouth faces the right-hand edge, off the board. Splay the 2 mm-pitch pins a little so they reach the holes.',
    `On top: pin 1 in ${atTop('J3')}, pin 2 in ${atTop('J2')}.`, solderUnder(['J3', 'J2'])],
  RV1: () => ['Snip off its two mounting tabs if it has them. Shaft up, body off the right-hand edge.',
    `On top: its three pins in ${atTop('J10')}, ${atTop('J11')} and ${atTop('J12')}.`, solderUnder(['J10', 'J11', 'J12'])],
  C3: () => ['The striped side is −; the other lead is +. See the sketch.',
    `Bend both leads down 90° about 1 mm from the can. On top: + in ${atTop('I7')}, − (stripe) in ${atTop('I8')}; lay it flat, its end past the right-hand edge.`, solderUnder(['I7', 'I8'])],
  U3: () => ['Component side up. Check the labels on your module: VCC, GND, IN3, IN4 down one row; OUT4, OUT3, EEP on the other (see perfboard.md).',
    'On top: its pins in **D3–D6** (printed G3–G6 underneath) and **H2, H3, H6** (printed C2, C3, C6 underneath).',
    'Turn the board over (underside up) and solder each pin at printed **G3–G6** (top D3–D6) and **C2, C3, C6** (top H2, H3, H6).'],
  U2: () => ['Component side up, USB-C past row 17.',
    'On top: its pins in **D12–D17** (printed G12–G17 underneath) and **H11–H15** (printed C11–C15 underneath). H10 stays empty.',
    'Turn the board over (underside up) and solder each pin at printed **G12–G17** (top D12–D17) and **C11–C15** (top H11–H15).'],
  U1: () => ['Its screw terminal is off and its switches are set to 5 V (Before you start).',
    'On top: stick it down on its two foam pads over columns A–C, rows 9–17, USB-C past row 17 beside the Super Mini\'s, output pads toward row 8. Nothing goes into holes.',
    'Find + and − by the marking printed on your PD board, then measure them with a charger plugged in (the check).'],
  C1: () => ['The striped side is −; the long leg is +. See the sketch. Slip heat-shrink over both legs.',
    `On top: stand it 1.5 mm up over A4–C6, stripe toward row 4; bend the + leg out to ${atTop('C7')} and the − leg to ${atTop('B4')}.`, solderUnder(['C7', 'B4'])],
};
function doThis(s) {
  let a;
  if (s.kind === 'part') a = DO[s.id]();
  else if (s.kind === 'link') a = ['Turn the board over (underside up).',
    `Start at hole ${atUnder(s.from)}: push the tinned end in and solder it.`,
    'Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.',
    `End at hole ${atUnder(s.to)}: push it in and solder it.`];
  else if (s.kind === 'jumper') a = ['Component side up; this one stays on top.',
    `Start at U1's ${s.id === 'W1' ? '+ (VBUS)' : '− (GND)'} pad, found by the marking on your board: solder the end to it.`,
    s.id === 'W1' ? 'Run the wire between A8 and B8, clear of W2.' : 'Run the wire straight to A8.',
    `End at hole **${s.to}**: push it in and solder it on top.`];
  else if (s.kind === 'off') {
    const h = OFF[s.id].hole;
    a = [`Start at ${s.offText}: the button's own lead, or 30 AWG spliced to it in the pod (see [the button](wiring.md#the-button-s1)).`,
      'Run the wire behind the board, between the board and the mount.',
      `Push the other end up through the hole printed **${under(h)}** on the underside (top ${h}).`,
      `Turn to the top and solder it at **${h}**. Trim the stub.`];
  } else if (s.kind === 'motor') a = [`Measure the motor's ${s.colour} lead ${s.cut} mm from the end cap, cut it there and strip 2 mm.`,
    'Crimp a JST-PH contact on.',
    `Push the contact into pin ${s.pin} of the motor plug's PHR-2 housing until it clicks.`,
    s.pin === 1 ? 'Leave the housing unplugged until W15 is in it too.' : 'Plug the housing into the motor plug (J4) on the board.'];
  a.push('Check, as below.');
  return '**Do this:**\n\n' + a.map((t, i) => `${i + 1}. ${t}`).join('\n') + '\n';
}

// ---- markdown ----
const pad2 = n => String(n).padStart(2, '0');
const file = (n, s) => `step-${pad2(n)}-${s.id}.svg`;
const md = [];
md.push(`# Step-by-step wiring: N20 perfboard (rev C)

One wire, link or part per step, in the [build order](perfboard.md#build-order). Each picture shows everything already done in grey and the current step in its real colour, with its end holes circled and named. The [hole map](images/perfboard.svg) and the [wire list](wiring.md#point-to-point-wire-list) show the whole board at once; this page is the same board taken one step at a time. The wire IDs, colours and cut lengths come from the [cut list](cut-list.csv).

**Reading the pictures**

- Columns **A–E** and **F–J**, rows **1–17**. Every hole in a row's half is joined to the other four in that half: a **strip**. The strips that the current step joins are drawn yellow.
- **Component side** pictures show the board as in the hole map, column A on the left.
- **Hole names are the component side's.** This board's underside letters are **mirrored**: the hole called A2 here is printed **J2** underneath (A↔J, B↔I, C↔H, D↔G, E↔F; the row numbers are the same). Wherever you work from the underside, go by the printed underside name given in brackets.
- **Underside** pictures show the board as you see it turned over, labelled with the letters printed on the underside, so they read A–J left to right just like the board in your hand. Each circled end shows its printed name large and its component-side name small.
- Checks use the component-side names: probe from the component side, or use the printed names in brackets.
- A **check** at the end of each step says what should connect and what must not. Test each one on the meter's **continuity setting**: connected means it beeps (or reads near 0 Ω). The few checks that need the diode-test or resistance setting say so.

## Before you start

- **Wire:** 30 AWG stranded, in red (+5 V), black (GND), green, blue, white and yellow. Only W14 and W15 differ: they're the motor's own leads.
- **Tin every stripped end** before it goes through a hole, so the strands don't splay.
- **Laying a link:** solder one end, press the wire flat along its lane and pinch the corners square with tweezers, hold the corners down with Kapton tape, then solder the far end. Keep it off the holes it crosses, and don't anchor wires through spare holes: every hole is part of a strip. Once the step's check passes, a dab of hot glue can hold it for good.
- **Prepare the modules** (build step 1): desolder U1's screw terminal and set its switches to 5 V. Pull the unused pins out of the header strips: the DRV8833 keeps D3–D6 and H2, H3, H6; the Super Mini keeps D12–D17 and H11–H15 (H10, GPIO5, stays out). Fit the strips to the modules with the perfboard as a jig, and solder them **to the modules only**.
`);
const sections = { 1: '## On the bench: D1 and the links', 12: '## On the bench: J4, RV1 and C3', 15: '## On the bench: the modules and U1', 20: '## On the drive: W11, C1 and W6 (the button)', 23: '## The motor leads' };
const intro = {
  2: '**The ten links** go on the underside, shortest first. Turn the board over: from here to step 11 the pictures show the underside as you see it, with the mirrored letters printed there; each hole is given by both names.',
  20: 'With the board on the drive and the button in its pod mount. The button\'s two wires (W11, W6) run straight from it, through the base tunnel and behind the board: push each **up from the underside** and solder it on the component side. There\'s no direction switch and no panel connector, so the pod stays wired to the board. W11 goes first, because C1 stands over its hole.',
  23: 'W14 and W15 are the N20\'s own factory leads, red and black, left soldered to its tabs (assumed about 26 AWG, unconfirmed). Solder C2 across the tabs first if it isn\'t on yet. Measure each lead from the motor\'s end cap, cut it, strip 2 mm and crimp a JST-PH contact on, then push the contact into the PHR-2 housing of the motor plug (J4). Routing and the wire clamp are in [wiring.md](wiring.md#j4-and-the-motor-wire-clamp). Red and black here don\'t mean +5 V and GND: this pair only runs from the motor to the motor plug (J4).',
};
STEPS.forEach((s, i) => {
  const n = i + 1;
  if (sections[n]) md.push(sections[n] + '\n');
  if (intro[n]) md.push(intro[n] + '\n');
  if (n === 20) md.push(`![${onDriveSvg().alt}](images/steps-wiring/on-the-drive.svg)\n`);
  const side = s.kind === 'link' ? 'underside (link)' : s.kind === 'off' ? 'in from the underside, soldered on the component side' : s.kind === 'motor' ? 'off the board: crimped into the motor plug\'s (J4\'s) housing' : 'component side';
  if (s.kind === 'part') {
    md.push(`### Step ${n}: ${s.title}\n`);
    md.push(`![${altFor(n, s, 'top')}](images/steps-wiring/${file(n, s)})\n`);
    if (POLARITY[s.id]) md.push(`![${POLARITY[s.id].alt}](images/steps-wiring/${POLARITY[s.id].file})\n`);
    md.push(doThis(s));
    md.push(`- **Part:** ${s.id}`);
    md.push(`- **Holes:** ${s.fromTo}`);
    md.push(`- **Side:** ${side}`);
    md.push(`- **What it does:** ${s.what}`);
    if (s.how) md.push(`- **How:** ${s.how}`);
  } else {
    md.push(`### Step ${n}: ${s.id}, ${s.colour}, ${s.from} → ${s.to}\n`);
    md.push(`![${altFor(n, s, s.kind === 'link' ? 'under' : 'top')}](images/steps-wiring/${file(n, s)})\n`);
    md.push(doThis(s));
    md.push(`- **Wire:** ${s.id}, ${s.colour}, ${s.kind === 'motor' ? 'the motor\'s own lead' : s.gauge}`);
    md.push(`- **Cut:** ${s.cut} mm${s.kind === 'motor' ? ', measured from the motor\'s end cap' : ''}`);
    md.push(`- **From → to:** ${s.from} → ${s.to}` + (s.kind === 'link' ? ` (printed ${under(s.from)} → ${under(s.to)} on the underside)` : s.kind === 'off' ? ` (push it in at printed ${under(OFF[s.id].hole)} on the underside)` : ''));
    md.push(`- **Side:** ${side}`);
    md.push(`- **What it does:** ${s.what}`);
  }
  md.push(`- **Check:** ${s.check}\n`);
});
md.push(`## Last checks

1. Go through the [terminal check](wiring.md#terminal-check): every terminal should hold exactly the wires it lists.
2. **+5V, +3V3 and GND must not connect to each other**, or to the strip next door. Check B7 to B8, A15 to A16, and C17 to C16.
3. Then follow [bring-up](wiring.md#bring-up) before the first power-on.

The button's wires are covered in [wiring.md](wiring.md#the-button-s1): use its own leads if they reach, or splice in the pod. The pod no longer unplugs from the drive.

_Generated by [make_steps.mjs](images/steps-wiring/make_steps.mjs) from the same hole map as [perfboard.svg](images/perfboard.svg); the wire IDs, colours and cut lengths are checked against [cut-list.csv](cut-list.csv) when it runs._
`);

STEPS.forEach((s, i) => writeFileSync(join(HERE, file(i + 1, s)), svgFor(i + 1, s)));
for (const p of Object.values(POLARITY)) writeFileSync(join(HERE, p.file), p.svg());
writeFileSync(join(HERE, 'on-the-drive.svg'), onDriveSvg().svg);
writeFileSync(join(DOCS, 'wiring-steps.md'), md.join('\n'));
console.log(`${STEPS.length} steps written`);
