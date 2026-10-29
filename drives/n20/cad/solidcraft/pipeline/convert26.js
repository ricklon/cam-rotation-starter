// node convert.js n20_features.json out.json : neutral feature JSON (from extract.py) -> SolidCraft command script
const fs = require('fs');
const path = require('path');
const IN = process.argv[2];
let j, SLABS = {};
if (fs.statSync(IN).isDirectory()) {
  // one extract per component (extract20.py): merge in timeline order
  j = { ops: [] };
  for (const f of fs.readdirSync(IN)) if (/^[A-Za-z0-9_]+.json$/.test(f) && f !== 'slabs.json') j.ops.push(...JSON.parse(fs.readFileSync(path.join(IN, f))).ops);
  j.ops.sort((a, b) => a.i - b.i);
  if (fs.existsSync(path.join(IN, 'slabs.json'))) SLABS = JSON.parse(fs.readFileSync(path.join(IN, 'slabs.json')));
} else j = JSON.parse(fs.readFileSync(IN));
const r = x => Math.round(x * 1e6) / 1e6;
const P = p => p.map(r);
const cmds = [];
let curOp = null;
const RAW = new Set((process.argv.find(a => a.startsWith('--raw=')) || '--raw=').slice(6).split(',').filter(Boolean));
const C = (command, params) => cmds.push({ command, params, op: curOp });

// polygonize a loop for the interior-point search
function poly(c) {
  if (c.t === 'line') return [c.p];
  if (c.t === 'circle') {
    const s = []; const n = 64;
    for (let i = 0; i < n; i++) {
      const a0 = 2 * Math.PI * i / n, a1 = 2 * Math.PI * (i + 1) / n;
      s.push([[c.c[0] + c.r * Math.cos(a0), c.c[1] + c.r * Math.sin(a0)], [c.c[0] + c.r * Math.cos(a1), c.c[1] + c.r * Math.sin(a1)]]);
    }
    return s;
  }
  if (c.t === 'arc') {
    // circle through s, m, e; sample from s through m to e
    const [ax, ay] = c.s, [bx, by] = c.m, [cx, cy] = c.e;
    const d = 2 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    const ux = ((ax * ax + ay * ay) * (by - cy) + (bx * bx + by * by) * (cy - ay) + (cx * cx + cy * cy) * (ay - by)) / d;
    const uy = ((ax * ax + ay * ay) * (cx - bx) + (bx * bx + by * by) * (ax - cx) + (cx * cx + cy * cy) * (bx - ax)) / d;
    const R = Math.hypot(ax - ux, ay - uy);
    const ang = (x, y) => Math.atan2(y - uy, x - ux);
    let a0 = ang(ax, ay), am = ang(bx, by), a1 = ang(cx, cy);
    const norm = a => { while (a < a0) a += 2 * Math.PI; return a; };
    am = norm(am); a1 = norm(a1);
    let sweep = a1 - a0; if (am > a1) sweep = sweep - 2 * Math.PI; // clockwise
    const s = []; const n = 32;
    for (let i = 0; i < n; i++) {
      const t0 = a0 + sweep * i / n, t1 = a0 + sweep * (i + 1) / n;
      s.push([[ux + R * Math.cos(t0), uy + R * Math.sin(t0)], [ux + R * Math.cos(t1), uy + R * Math.sin(t1)]]);
    }
    return s;
  }
  if (c.t === 'spline') { const s = []; for (let i = 0; i + 1 < c.pts.length; i++) s.push([c.pts[i], c.pts[i + 1]]); return s; }
  throw new Error('curve ' + c.t);
}
// a point inside the profile (even-odd over all its loops), away from its edges
function interior(loops) {
  const segs = loops.flatMap(l => l.curves.flatMap(poly));
  let ymin = Infinity, ymax = -Infinity;
  for (const [a, b] of segs) { ymin = Math.min(ymin, a[1], b[1]); ymax = Math.max(ymax, a[1], b[1]); }
  let best = null;
  for (let k = 1; k < 40; k++) {
    const y = ymin + (ymax - ymin) * (k / 40 + 0.0037);
    const xs = [];
    for (const [a, b] of segs) {
      if ((a[1] > y) !== (b[1] > y)) xs.push(a[0] + (y - a[1]) * (b[0] - a[0]) / (b[1] - a[1]));
    }
    xs.sort((p, q) => p - q);
    for (let i = 0; i + 1 < xs.length; i += 2) {
      const w = xs[i + 1] - xs[i];
      // clearance from y edges of the region, roughly
      const h = Math.min(y - ymin, ymax - y);
      const score = Math.min(w / 2, h);
      if (!best || score > best.score) best = { score, p: [(xs[i] + xs[i + 1]) / 2, y] };
    }
  }
  return best.p;
}

// SolidCraft 0.20's booleans fail on the clamp ring (its bore coincides with the socket bore) and then on the
// clamp gap. Equivalent build: join the ring as a solid disc right after the first cylinder, before the socket
// is cut. In Fusion the ring also refills the socket's lead-in chamfer, so the clamps have none: skip it.
for (const o of j.ops) {
  if (RAW.has('ring') || o.name !== 'Clamp ring (1)') continue;
  o.profiles[0] = o.profiles[0].filter(l => l.outer);
  const ops = j.ops.filter(p => p.comp === o.comp);
  j.ops.splice(j.ops.indexOf(o), 1);
  j.ops.splice(j.ops.indexOf(ops.find(p => p.name === 'Extrude1')) + 1, 0, o);
  o.reordered = true;
  ops.find(p => p.name === 'Chamfer2').skip = 'filled again by the clamp ring in Fusion';
}
// The clamp countersinks end exactly on the clearance-hole wall; SolidCraft 0.20's cut fails there. Run them
// 0.1 mm deeper: the extra tip lies inside the existing hole, so the result is the same.
for (const o of j.ops) if (!RAW.has('csk') && o.name === 'Clamp countersinks (1)') o.e1.d += Math.sign(o.e1.d) * 0.1;
// The D-flat fill's arc lies on the bore wall and SolidCraft 0.20's union fails. Push the arc 0.02 mm into the
// wall (solid there) keeping the flat where it is; it only reaches 0.02 mm into the bore's bed chamfer.
for (const o of j.ops) {
  if (RAW.has('dflat') || o.name !== 'N20 D flat (1)') continue;
  const cs = o.profiles[0][0].curves, arc = cs.find(c => c.t === 'arc'), line = cs.find(c => c.t === 'line');
  const R = Math.hypot(...arc.m) + 0.02, x = arc.s[0], y = Math.sqrt(R * R - x * x), k = Math.sign(arc.m[0]);
  arc.s = [x, -y]; arc.e = [x, y]; arc.m = [k * R, 0]; line.p = [[x, -y], [x, y]];
}
// On the 1/4 clamp even that union fails; stopping its fill 0.01 mm short of the socket floor goes through.
for (const o of j.ops) if (!RAW.has('dflat') && o.name === 'N20 D flat (1)' && o.comp === 'Adapter_Clamp_025') o.e1.d -= 0.01;
// The cap relief shares the gap's y = 0.75 face; start it inside the gap (already empty) instead.
for (const o of j.ops) {
  if (RAW.has('cap') || o.name !== 'Cap relief (1)') continue;
  for (const c of o.profiles[0][0].curves) for (const p of c.p) if (Math.abs(p[1] - 0.75) < 1e-6) p[1] = 0;
}
// SolidCraft 0.26's cut fails anywhere in the PD relief's region once the ledge and seat are in (the solid is
// closed and its volume matches Fusion's). The relief touches only the pad, the ledge and seat lie beside it, so
// cutting it straight after the pad gives the same mount.
if (!RAW.has('pdrelief')) {
  const rel = j.ops.find(o => o.comp === 'Servo_Mount' && o.name === 'PD relief');
  const pad = j.ops.find(o => o.comp === 'Servo_Mount' && o.name === 'PD pad');
  if (rel && pad) { j.ops.splice(j.ops.indexOf(rel), 1); j.ops.splice(j.ops.indexOf(pad) + 1, 0, rel); }
}
// The D1 lead slot runs on the D1 channel's axis; cutting either after the other gives SolidCraft 0.26 an
// "implausible result volume". The two as one tool body, cut once, goes through.
if (!RAW.has('d1tool')) {
  const ch = j.ops.find(o => o.comp === 'Servo_Mount' && o.name === 'D1 channel (under base) (1)');
  const sl = j.ops.find(o => o.comp === 'Servo_Mount' && o.name === 'D1 lead slot (under base) (1)');
  if (ch && sl) {
    ch.operation = 'new'; ch.bodies = ['D1 tool'];
    sl.operation = 'join'; sl.participants = ['D1 tool'];
    sl.then_cut = { target: 'Servo_Mount', tool: 'D1 tool' };
  }
}

// The cartridge socket can't be cut once the floor, sleeve and slot fill are joined in (any cut there that crosses
// z = 0, where the face plate's bore ends on the fill, has "no result", and so does a second cut next to a first).
// A cut there fails even right after the face plate and gearbox sleeve. Same mount another way: cut the socket from
// the mount before any N20 feature, and join the face plate, gearbox sleeve, floor, sleeve and fill as bodies with
// the socket already taken out of them.
if (!RAW.has('socket')) {
  const M = n => j.ops.find(o => o.comp === 'Servo_Mount' && o.name === n);
  const sock = M('Cartridge socket (1)'), gb = M('N20 face plate (1)');
  if (sock && gb) {
    const pts = sock.profiles[0][0].curves.flatMap(c => c.p);
    const xs = pts.map(p => p[0]), ys = pts.map(p => p[1]);
    const z0 = sock.plane.origin[2], box = { x0: Math.min(...xs), x1: Math.max(...xs), y0: Math.min(...ys), y1: Math.max(...ys), z0, z1: z0 + sock.e1.d };
    j.ops.splice(j.ops.indexOf(sock), 1); j.ops.splice(j.ops.indexOf(gb), 0, sock);
    for (const n of ['N20 face plate (1)', 'N20 gearbox sleeve (1)', 'Cartridge floor (1)', 'Cartridge sleeve (1)', 'Base-plate slot fill (no trapped supports)']) {
      const o = M(n); if (!o) continue;
      const tool = n.split(' (')[0] + ' tool';
      o.operation = 'new'; o.bodies = [tool];
      o.then_join = { target: 'Servo_Mount', tool, cut_box: box };
    }
  }
}

const ONLY =(process.argv.find(a => a.startsWith('--only=')) || '').slice(7);
if (ONLY) j.ops = j.ops.filter(o => ONLY.split(',').includes(o.comp));
let nsk = 0;
const PIECES = {};
const CHECK = process.argv.includes('--check');
const checks = [];
for (const o of j.ops) {
  if (CHECK && cmds.length) { /* measured after the previous op */ }
  curOp = o.i;
  if (o.skip) continue;
  if (o.type === 'ExtrudeFeature' && (RAW.has('cap') || o.name !== 'Cap relief (1)')) {
    const body = o.bodies[0];
    o.profiles.forEach((loops, k) => {
      const sk = `${o.comp}/${o.name}` + (o.profiles.length > 1 ? `#${k + 1}` : '');
      nsk++;
      C('sketch.create', { plane: { origin: P(o.plane.origin), x_dir: P(o.plane.x_dir), y_dir: P(o.plane.y_dir) }, name: sk });
      for (const l of loops) for (const c of l.curves) {
        if (c.t === 'line') C('sketch.line', { points: [P(c.p[0]), P(c.p[1])] });
        else if (c.t === 'circle') C('sketch.circle.center', { center: P(c.c), radius: r(c.r) });
        else if (c.t === 'arc') C('sketch.arc.three_point', { start: P(c.s), end: P(c.e), through: P(c.m) });
        else C('sketch.spline.fit_point', { points: c.pts.map(P) });
      }
      C('sketch.finish', {});
      const ex = { sketch: sk, profiles: [{ point: P(interior(loops)) }], name: sk };
      const e = o.e1;
      if (e.k === 'dist') { ex.distance = r(Math.abs(e.d)); if (e.d < 0) ex.direction = 'negative'; }
      else if (e.k === 'all') { ex.through_all = true; if (!e.pos) ex.direction = 'negative'; }
      else if (e.k === 'sym') { ex.distance = r(e.total); ex.direction = 'symmetric'; }
      if (o.e2) {
        if (o.e2.k === 'dist') ex.distance2 = r(Math.abs(o.e2.d));
        else if (o.e2.k === 'all') ex.distance2 = { through_all: true };
      }
      if (o.start_offset != null) ex.start_offset = r(o.start_offset);
      if (o.taper) ex.taper = `${r(o.taper)} deg`;
      if (o.operation === 'new') {
        if (k === 0) { ex.operation = 'new'; ex.body_name = body; }
        else { ex.operation = 'join'; ex.targets = [body]; }
      } else {
        ex.operation = o.operation;
        ex.targets = o.participants;
      }
      C('solid.extrude', ex);
    });
    if (o.then_cut) C('solid.combine', { target: o.then_cut.target, tools: [o.then_cut.tool], operation: 'cut' });
    if (o.then_join) {
      const b = o.then_join.cut_box;
      C('solid.box', { length: r(b.x1 - b.x0), width: r(b.y1 - b.y0), height: r(b.z1 - b.z0), corner: [r(b.x0), r(b.y0), r(b.z0)], operation: 'cut', targets: [o.then_join.tool] });
      C('solid.combine', { target: o.then_join.target, tools: [o.then_join.tool], operation: 'join' });
    }
  } else if (!RAW.has('cap') && o.name === 'Cap relief (1)') {
    // In Fusion this cut separates the cap into its own body; SolidCraft 0.20's cut has "no result" when the
    // body would fall into two pieces. Same bodies another way: a copy cut down to the cap's region (above the
    // relief, past the gap), and the cap's region plus the relief cut from the original.
    const cs = o.profiles[0][0].curves.flatMap(c => c.p);
    const x0 = Math.min(...cs.map(p => p[0])), x1 = Math.max(...cs.map(p => p[0])), y1 = Math.max(...cs.map(p => p[1]));
    const z0 = o.plane.origin[2], z1 = z0 + o.e1.d, top = 40;
    const [body, cap] = o.bodies;
    C('solid.move', { bodies: [body], translate: [0, 0, 0], copy: true });
    C('body.rename', { body: `${body} (2)`, name: cap });
    C('solid.box', { length: r(x1 - x0), width: r(y1), height: r(top - z1), corner: [r(x0), 0, r(z1)], body_name: 'cap region' });
    C('solid.combine', { target: cap, tools: ['cap region'], operation: 'intersect' });
    C('solid.box', { length: r(x1 - x0), width: r(y1), height: r(top - z0), corner: [r(x0), 0, r(z0)], body_name: 'cap and relief' });
    C('solid.combine', { target: body, tools: ['cap and relief'], operation: 'cut' });
  } else if (o.type === 'BaseFeature') {
    // A body Fusion holds with no history (planar faces only): rebuilt as extrudes of its slabs (slabs.py)
    for (const bb of o.base_bodies) {
      const sl = SLABS[path.basename(bb.file)];
      if (!sl) throw new Error('no slabs for ' + bb.file);
      const ax = 'XYZ'.indexOf(sl.axis);
      // sketch axes: the two world axes after the slab axis, so the normal is +axis
      const u = [0, 0, 0], v = [0, 0, 0]; u[(ax + 1) % 3] = 1; v[(ax + 2) % 3] = 1;
      sl.slabs.forEach((s, k) => {
        const sk = o.comp + '/' + bb.body + '#' + (k + 1), org = [0, 0, 0]; org[ax] = s.h0;
        const to2 = p => [r(p[(ax + 1) % 3]), r(p[(ax + 2) % 3])];
        C('sketch.create', { plane: { origin: org, x_dir: u, y_dir: v }, name: sk });
        for (const [a, b] of s.segs) C('sketch.line', { points: [to2(a), to2(b)] });
        C('sketch.finish', {});
        // a slab's section may be several separate outlines (none inside another); SolidCraft keeps separate
        // pieces as separate bodies, so each outline is its own body and the combine that uses this one takes them all
        const segs = s.segs.map(([a, b]) => [to2(a), to2(b)]), key = p => p.join(','), left = new Set(segs.keys()), loops = [];
        while (left.size) {
          const first = left.values().next().value, loop = [segs[first]]; left.delete(first);
          for (let grew = true; grew;) {
            grew = false;
            for (const i of left) {
              const ends = loop.flat().map(key);
              if (ends.includes(key(segs[i][0])) || ends.includes(key(segs[i][1]))) { loop.push(segs[i]); left.delete(i); grew = true; }
            }
          }
          loops.push(loop);
        }
        loops.forEach((lp, l) => {
          const name = `${bb.body} ${k + 1}.${l + 1}`;
          (PIECES[bb.body] = PIECES[bb.body] || []).push(name);
          const pt = interior([{ curves: lp.map(p => ({ t: 'line', p })) }]);
          C('solid.extrude', { sketch: sk, profiles: [{ point: P(pt) }], distance: r(s.h1 - s.h0), name: `${sk}.${l + 1}`, operation: 'new', body_name: name });
        });
      });
    }
  } else if (o.type === 'CombineFeature') {
    C('solid.combine', { target: o.target, tools: o.tools.flatMap(t => PIECES[t] || [t]), operation: o.operation, keep_tools: o.keep });
  } else if (!RAW.has('flange') && o.name === 'Flange bed chamfer (wall face)') {
    // SolidCraft 0.20 refuses any chamfer on these edges ("would grow the part"), though a fillet there works.
    // Same 45 deg bevel as a cut: the flange outline (corner radius 4 = ear_corner_r) shrunk by d over d,
    // taken out of a slab, and that ring cut from the mount.
    const s = o.sets[0], d = s.d, body = s.edges[0].body;
    const xs = s.edges.map(e => e.p[0]), ys = s.edges.map(e => e.p[1]), z = s.edges[0].p[2];
    const [x0, x1, y0, y1] = [Math.min(...xs), Math.max(...xs), Math.min(...ys), Math.max(...ys)];
    C('sketch.create', { plane: { origin: [0, 0, r(z - d)], x_dir: [1, 0, 0], y_dir: [0, 1, 0] }, name: 'Flange bed chamfer outline' });
    C('sketch.rectangle.rounded', { p0: [x0, y0], p1: [x1, y1], radius: 4 });
    C('sketch.finish', {});
    C('solid.extrude', { sketch: 'Flange bed chamfer outline', distance: r(d), taper: '-45 deg', operation: 'new', body_name: 'bevel keep' });
    C('solid.box', { length: r(x1 - x0 + 10), width: r(y1 - y0 + 10), height: r(d), corner: [r(x0 - 5), r(y0 - 5), r(z - d)], body_name: 'bevel ring' });
    C('solid.combine', { target: 'bevel ring', tools: ['bevel keep'], operation: 'cut' });
    C('solid.combine', { target: body, tools: ['bevel ring'], operation: 'cut' });
  } else {
    for (const s of o.sets) {
      const edges = s.edges.map(e => P(e.p));
      const body = s.edges[0].body;
      if (s.type.includes('Fillet')) C('solid.fillet', { edges, radius: r(s.r), body });
      else {
        const p = { edges, distance: r(s.d), body };
        if (s.d2 != null) p.distance2 = r(s.d2);
        if (s.angle != null) p.angle = `${r(s.angle)} deg`;
        C('solid.chamfer', p);
      }
    }
  }
}
if (CHECK) {
  // re-emit with a measurement after every feature
  const out = []; let k = 0;
  for (const o of j.ops) {
    if (o.skip) continue;
    while (k < cmds.length && cmds[k].op !== o.i) k++;
    while (k < cmds.length && cmds[k].op === o.i) out.push(cmds[k++]);
    const names = o.type === 'BaseFeature' ? [] : Object.keys(o.vols || {});
    if (names.length) { out.push({ command: 'inspect.measure', params: { bodies: names } }); checks.push({ step: out.length, i: o.i, name: o.comp + ' ' + o.name, vols: o.vols }); }
  }
  cmds.length = 0; cmds.push(...out);
  fs.writeFileSync('checks.json', JSON.stringify(checks));
}
if (process.argv.includes('--components')) {
  // one component per Fusion component, as in the Fusion browser; each feature built in its own
  const compOf = Object.fromEntries(j.ops.map(o => [o.i, o.comp]));
  const comps = [...new Set(j.ops.map(o => o.comp))];
  const out = [{ command: 'component.activate', params: { component: 'root' } }];
  for (const n of comps) out.push({ command: 'component.create', params: { name: n, activate: false } });
  let cur = null;
  for (const c of cmds) {
    const comp = compOf[c.op];
    if (comp && comp !== cur) { out.push({ command: 'component.activate', params: { component: comp } }); cur = comp; }
    out.push(c);
  }
  out.push({ command: 'component.activate', params: { component: 'root' } });
  cmds.length = 0; cmds.push(...out);
}
for (const c of cmds) delete c.op;
fs.writeFileSync(process.argv[3], JSON.stringify({ commands: cmds }, null, 1));
console.log('commands', cmds.length, 'sketches', nsk);
