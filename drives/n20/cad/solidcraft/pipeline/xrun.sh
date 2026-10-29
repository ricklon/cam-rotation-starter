#!/bin/bash
# xrun.sh COMP : extract one component from Fusion into v22/COMP.json
cd "$(dirname "$0")"
sed "s/__COMP__/$1/g" extract20.py > "v22/x_$1.py"
timeout 600 node fusion.js "v22/x_$1.py" | node -e "let t='';process.stdin.on('data',d=>t+=d).on('end',()=>{try{const j=JSON.parse(t);console.log('$1:',(j.message||j.error).slice(-600))}catch(e){console.log('$1 raw:',t.slice(-600))}})"
