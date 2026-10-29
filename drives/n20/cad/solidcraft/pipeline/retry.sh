#!/bin/bash
# retry.sh script.json "Feature name" : builds everything before that extrude once, then tries variants of it in
# parallel against the saved state and reports each one's result volume.
cd "$(dirname "$0")"
SC="/c/Program Files/SolidCraft/solidcraft-cli.exe"
tag=$(echo "$2" | tr -c 'A-Za-z0-9' '_' | cut -c1-40)
mkdir -p retry
node cutat.js "$1" "$2" "retry/${tag}_pre.json" "retry/${tag}_one.json" || exit 1
timeout 1500 "$SC" run "retry/${tag}_pre.json" --quiet > "retry/${tag}_pre.log" 2>&1 || { echo "prefix failed"; tail -2 "retry/${tag}_pre.log"; exit 1; }
node - "$tag" <<'EOF'
const fs = require('fs'), tag = process.argv[2];
const [e, m] = JSON.parse(fs.readFileSync(`retry/${tag}_one.json`)).commands;
const p = e.params, tgt = (p.targets || [])[0];
const V = {
  tool: [{ command: 'solid.extrude', params: { ...p, operation: 'new', targets: undefined, body_name: 'retry tool' } },
         { command: 'solid.combine', params: { target: tgt, tools: ['retry tool'], operation: p.operation } }],
  deeper: [{ command: 'solid.extrude', params: { ...p, distance: p.distance + 0.01 } }],
  outside: [{ command: 'solid.extrude', params: { ...p, distance: p.distance + 0.02, start_offset: (p.start_offset || 0) + (p.direction === 'negative' ? 0.01 : -0.01) } }],
  shallower: [{ command: 'solid.extrude', params: { ...p, distance: p.distance - 0.01 } }],
};
for (const [k, c] of Object.entries(V)) fs.writeFileSync(`retry/${tag}_${k}.json`, JSON.stringify({ commands: [...c, m] }));
EOF
for k in tool deeper outside shallower; do
  ( "$SC" run "retry/${tag}_$k.json" --in "retry/${tag}_pre.solidcraft" --results --json-out "retry/${tag}_$k.out.json" --quiet > "retry/${tag}_$k.log" 2>&1
    echo "$k exit $?: $(grep -v -E 'panicked|tolerance|BACKTRACE|^$' "retry/${tag}_$k.log" | tail -1) $(grep -o '"volume_mm3": [0-9.]*' "retry/${tag}_$k.out.json" 2>/dev/null | tail -1)" ) &
done
wait
