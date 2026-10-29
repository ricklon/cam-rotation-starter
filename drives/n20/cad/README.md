# CAD

The design lives in Autodesk Fusion, as the document *Cam Rotation N20* in MovementStudy › Cam Rotation Tool, copied from the MG90S drive's *Cam Rotation Starter*. No release is tagged yet; the snapshots in `step/` match Fusion version 20 of that document. When this drive is tagged `vMAJOR.MINOR-n20`, record here which Fusion version the tag matches. This folder holds copies you can open without Fusion, the print files, and the scripts used to build and document the model.

| Folder | What's in it |
|---|---|
| [`step/`](step) | `cam-rotation-n20.step`, the full assembly (opens in FreeCAD, Onshape, SolidWorks and most CAD tools), and `cam-rotation-n20.f3d`, a Fusion archive with the full parametric history |
| [`stl/`](stl/README.md) | Print-ready parts for this drive, already oriented, with print settings and the test-print order. Bushings, dowel gauges and shims come from [the MG90S drive](../../mg90s/cad/stl/README.md). |
| [`fusion/`](fusion) | Python scripts run inside Fusion (wiring, renders) and on the command line (STL orientation) |
| [`solidcraft/`](solidcraft) | A trial rebuild in SolidCraft 0.26.1 from Fusion version 22: `N20 SolidCraft v22.solidcraft`, the [pipeline](solidcraft/pipeline/README.md) that builds it, and [bug repros](solidcraft/repros/README.md) for SolidCraft's developers. The printed parts are built natively except the mount, which is Fusion's STEP until SolidCraft can build it; the bought parts are STEP imports. Fusion is still the design of record. |

## Opening the parametric model

Import `step/cam-rotation-n20.f3d` into Fusion (File → Open → Open from my computer). It keeps the MG90S drive's user parameters (see [the MG90S CAD notes](../../mg90s/cad/README.md#opening-the-parametric-model)); the parameters added for the N20 are listed under [Fusion design](../README.md#fusion-design) in this drive's README.

## Scripts

**`fusion/wires.py`** rebuilds every wire in the model's `Wiring` component from a list of waypoints, with colours and sizes from the [wire list](../docs/wiring.md). Run it in Fusion's script runner, or through the Fusion MCP server with `exec(open('drives/n20/cad/fusion/wires.py').read())`.

**`fusion/shots.py`** renders this drive's `docs/images`. It reuses the MG90S drive's `photo.py`, so set `CAM_REPO` to your checkout (it sets `CAM_DRIVE` itself), exec it with *Cam Rotation N20* active, and call `run_shots()`.

**`fusion/orient_stl.py`** turns STLs exported from Fusion into `cad/stl/raw/` into print-ready files and rewrites the parts table in `stl/README.md`, as in the MG90S drive.
