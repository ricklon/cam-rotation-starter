// node mkrepro.js script.json "Extrude name" out.json [replace.json] : a standalone repro, everything the script does
// before that extrude, then the extrude (or the commands in replace.json in its place), then a measure.
const fs = require('fs');
const [src, feat, out, rep] = process.argv.slice(2);
const c = JSON.parse(fs.readFileSync(src)).commands.filter(x => x.command !== 'inspect.measure');
const k = c.findIndex(x => x.command === 'solid.extrude' && x.params.name === feat);
if (k < 0) throw new Error('no extrude ' + feat);
const body = (c[k].params.targets || [])[0] || 'Servo_Mount';
const tail = rep ? JSON.parse(fs.readFileSync(rep)).commands : [c[k]];
const m = { command: 'inspect.measure', params: { bodies: [body] } };
fs.writeFileSync(out, JSON.stringify({ commands: [...c.slice(0, k), m, ...tail, m] }, null, 1));
console.log(out, 'commands', k + tail.length + 2);
