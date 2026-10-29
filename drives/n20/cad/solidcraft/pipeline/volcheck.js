// node volcheck.js parts/<comp> : per-feature volumes, SolidCraft (inspect.measure results) against Fusion (extract vols)
const fs = require('fs');
const base = process.argv[2];
const checks = JSON.parse(fs.readFileSync(base + '.checks.json'));
let out;
try { out = JSON.parse(fs.readFileSync(base + '.out.json', 'utf8')); } catch (e) { console.log(base, 'no results'); process.exit(); }
const res = out.results || out;
const vol = (r, name) => {
  const s = JSON.stringify(r);
  const bs = (r && (r.bodies || (r.result && r.result.bodies))) || [];
  const b = bs.find(x => x.name === name);
  return b ? b.volume_mm3 : null;
};
let worst = 0, rows = [];
for (const c of checks) {
  const r = res[c.step - 1];
  if (!r) { rows.push(`  ${c.name}: not reached`); continue; }
  for (const [n, fv] of Object.entries(c.vols)) {
    const sv = vol(r.result || r, n);
    const d = sv == null ? null : sv - fv;
    if (d != null) worst = Math.max(worst, Math.abs(d));
    if (d == null || Math.abs(d) > 1) rows.push(`  ${c.name} [${n}]: fusion ${fv.toFixed(2)} solidcraft ${sv == null ? '-' : sv.toFixed(2)}`);
  }
}
const last = checks[checks.length - 1];
console.log(base.split('/').pop(), 'checks', checks.length, 'worst |dV|', worst.toFixed(3), 'mm3');
rows.forEach(r => console.log(r));
