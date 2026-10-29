// node assemble22.js out.json design.solidcraft : after the native build (v22_full.solidcraft), add the bought
// parts as STEP imports in a "Purchased" component, Fusion's print parts in a hidden "Fusion reference" component,
// and compare each native body with Fusion's (red: only Fusion has material, green: only SolidCraft).
const fs = require('fs');
const V = 'v22';
const ref = JSON.parse(fs.readFileSync(`${V}/fusion_ref/placements.json`));
// parts SolidCraft can't build yet: Fusion's STEP stands in, visible, in its own component
const STAND_IN = (process.argv.find(a => a.startsWith('--stand-in=')) || '--stand-in=').slice(11).split(',').filter(Boolean);
const base = process.argv.find(a => a.startsWith('--base='));
const buy = JSON.parse(fs.readFileSync(`${V}/purchased/placements.json`));
const cmds = base ? JSON.parse(fs.readFileSync(base.slice(7))).commands : [];
const C = (command, params) => cmds.push({ command, params });
const mat = m => { m = m.slice(0, 12); for (const i of [3, 7, 11]) m[i] *= 10; return m; };  // cm -> mm
const fwd = p => p.replace(/\\/g, '/');
// placements.json holds the paths of the run that wrote it; find the files beside it instead
const here = (dir, p) => fwd(require('path').resolve(V, dir, fwd(p).split('/').pop()));

C('component.activate', { component: 'root' });
C('component.create', { name: 'Purchased' });
for (const [occ, p] of Object.entries(buy)) C('file.insert_step', { path: here('purchased', p.path), matrix: mat(p.matrix), name: occ.replace(/:\d+$/, '') + (occ.match(/:(\d+)$/)[1] === '1' ? '' : ' ' + occ.match(/:(\d+)$/)[1]) });
C('component.activate', { component: 'root' });

for (const comp of STAND_IN) {
  const p = ref[comp];
  C('component.create', { name: comp + ' (Fusion STEP)' });
  C('file.insert_step', { path: here('fusion_ref', p.path), matrix: mat(p.matrix), name: comp + ' (Fusion STEP)', names: 'solid' });
  C('component.activate', { component: 'root' });
}
C('component.create', { name: 'Fusion reference' });
const pairs = [];
for (const [comp, p] of Object.entries(ref)) {
  if (STAND_IN.includes(comp)) continue;
  C('file.insert_step', { path: here('fusion_ref', p.path), matrix: mat(p.matrix), name: `Fusion ${comp}`, names: 'solid' });
  for (const b of p.bodies) { C('body.rename', { body: `${b} (2)`, name: `Fusion ${b}` }); pairs.push(b); }
}
C('component.activate', { component: 'root' });
for (const b of pairs) C('inspect.compare', { a: `Fusion ${b}`, b, tolerance: 0.01, color: { only_in_a: '#e03030', only_in_b: '#20a040' } });
C('browser.visibility', { visible: false, items: pairs.map(b => `Fusion ${b}`) });
C('file.save', { path: fwd(process.argv[3]), cache: true });
fs.writeFileSync(process.argv[2], JSON.stringify({ commands: cmds }, null, 1));
console.log('commands', cmds.length, 'pairs', pairs.length);
