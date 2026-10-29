# Fusion to SolidCraft pipeline (N20 drive)

This rebuilds the N20 drive's printed parts natively in SolidCraft from the history of the Fusion design *Cam Rotation N20*. The bought parts come in as STEP. It was last run against Fusion version 22 with SolidCraft 0.26.1, on 2026-10-29.

Run the Node scripts from this folder. The Python scripts run inside Fusion, and still write to the temp folder of the session that made them (`OUT`, `BASE` and `D` near the top of each). Point those at this folder's `v22` before running them again. The `path` entries in `v22/*/placements.json` are from that run too; `assemble22.js` only uses their file names.

`v22/purchased/` holds STEP exports of the bought parts as they sit in the Fusion design. Some come from third parties (the XIAO, the N20 motor model, McMaster screws), under their own terms.

| Step | Script | Output |
|---|---|---|
| 1. Feature history of one Fusion component | `xrun.sh <Component>` (runs `extract20.py` in Fusion through `fusion.js`) | `v22/<Component>.json`, and `.smt` files for any solids with no history |
| 2. Solids with no history into extrudable slabs | `slabs.py` (in Fusion) | `v22/slabs.json` |
| 3. STEP exports | `export22.py`, `exportdrv.py` (in Fusion) | `v22/fusion_ref/` (print parts, for comparison), `v22/purchased/` (bought parts) |
| 4. SolidCraft script | `node convert26.js v22 out.json [--only=A,B] [--components] [--check] [--raw=ring,cap,dflat,flange,pdrelief,d1tool,socket]` | `--check` adds a measure after each feature and writes `checks.json` |
| 5. Volumes against Fusion | `node volcheck.js parts/<Component>` | Lists features more than 1 mm³ away from Fusion |
| 6. Assembly | `node assemble22.js out.json design.solidcraft --base=native.json --stand-in=Servo_Mount` | Native parts, STEP stand-ins, bought parts, and a hidden Fusion reference with `inspect.compare` colouring |

`--raw=` turns off named workarounds, to check whether a newer SolidCraft still needs them. `retry.sh` and `cutat.js` retry one failing feature against a saved prefix. `mkrepro.js` writes standalone bug repros; those are in `../repros`.
