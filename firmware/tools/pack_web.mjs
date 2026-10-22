// Copy web/dist into data/www as gzip files so the firmware can serve them compressed.
// Run by `npm run build` in web/. Uses only Node built-ins.
import { readdirSync, statSync, readFileSync, writeFileSync, mkdirSync, rmSync } from 'node:fs';
import { join, dirname, relative } from 'node:path';
import { gzipSync } from 'node:zlib';
import { fileURLToPath } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const src = join(root, 'web', 'dist');
const out = join(root, 'data', 'www');

rmSync(out, { recursive: true, force: true });
mkdirSync(out, { recursive: true });

let total = 0;
function walk(dir) {
  for (const name of readdirSync(dir)) {
    const p = join(dir, name);
    if (statSync(p).isDirectory()) { walk(p); continue; }
    const rel = relative(src, p);
    const dest = join(out, rel) + '.gz';
    mkdirSync(dirname(dest), { recursive: true });
    const gz = gzipSync(readFileSync(p), { level: 9 });
    writeFileSync(dest, gz);
    total += gz.length;
    console.log(`  ${rel}.gz  ${gz.length} B`);
  }
}
walk(src);
console.log(`data/www: ${(total / 1024).toFixed(1)} KB gzipped. Rebuild the firmware (idf.py build) to pack it into the storage image.`);
