// node cutat.js script.json featureName prefix.json one.json : split a script just before the extrude named
// featureName: prefix.json builds everything before it and saves <prefix>.solidcraft; one.json is that extrude alone.
const fs = require('fs'), path = require('path');
const [src, feat, pre, one] = process.argv.slice(2);
const c = JSON.parse(fs.readFileSync(src)).commands;
const k = c.findIndex(x => x.command === 'solid.extrude' && x.params.name === feat);
if (k < 0) throw new Error('no extrude ' + feat);
const save = path.resolve(pre.replace(/\.json$/, '.solidcraft')).split(path.sep).join('/');
fs.writeFileSync(pre, JSON.stringify({ commands: [...c.slice(0, k), { command: 'file.save', params: { path: save, cache: true } }] }));
fs.writeFileSync(one, JSON.stringify({ commands: [c[k], { command: 'inspect.measure', params: { bodies: c[k].params.targets || [c[k].params.body_name] } }] }, null, 1));
console.log('split at', k, 'of', c.length, '->', save);
