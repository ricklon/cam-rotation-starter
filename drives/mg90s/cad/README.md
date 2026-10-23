# CAD

The design lives in Autodesk Fusion, as the document *Cam Rotation Starter* in MovementStudy › Cam Rotation Tool. Tag `v1.0-mg90s` matches Fusion version 47 of that document. This folder holds copies you can open without Fusion, the print files, and the scripts used to build and document the model.

| Folder | What's in it |
|---|---|
| [`step/`](step) | `cam-rotation-starter.step`, the full assembly (opens in FreeCAD, Onshape, SolidWorks and most CAD tools), and `cam-rotation-starter.f3d`, a Fusion archive with the full parametric history |
| [`stl/`](stl/README.md) | Print-ready parts, already oriented, with print settings |
| [`fusion/`](fusion) | Python scripts run inside Fusion (wiring, renders) and on the command line (STL orientation) |

## Opening the parametric model

Import `step/cam-rotation-starter.f3d` into Fusion (File → Open → Open from my computer). All dimensions are driven by **user parameters** (Modify → Change Parameters); change one and the model rebuilds.

| Parameter | Default | What it controls |
|---|---|---|
| `box_wall_thk` | 0.5 in | Thickness of the box wall the unit mounts on |
| `dowel_large`, `dowel_mid`, `dowel_small` | 3/4, 5/8, 1/4 in | Shaft sizes; adapters, bushings and wall holes follow |
| `fit_clear` | 0.3 mm | Clearance between dowel and adapter socket |
| `hole_clear` | 0.5 mm | Clearance between dowel and bushing bore |
| `spline_dia` | 5.0 mm | Servo spline pocket; set it from the fit-test coupon |
| `socket_depth` | 18 mm | How far the dowel goes into the adapter |
| `grip_screw_dia`, `grip_z1`, `grip_z2` | 3.1, 11, 19 mm | The adapters' #4 wood-screw holes |
| `pod_pilot_depth` | 5.6 mm | M3 pilot holes for the pod screws |
| `clamp_gap` | 1.5 mm | Split-clamp take-up for sanded dowels |
| `shim_dowel` | 18.5 mm | Measured diameter a 3/4" shim is made for |
| `hs_len` | 3.8 mm | Servo horn screw length under the head (measured) |
| `c1_seat_dia`, `c1_seat_depth` | 8.0, 4.9 mm | Capacitor press-fit seat in the electronics-side wall; set the diameter to your capacitor's |
| `d1_seat_dia`, `d1_lead_slot`, `d1_lead_hole` | 2.9, 1.1, 1.0 mm | Diode channel on the inside of that wall, and the holes its leads go out through |

The XIAO in the model is the S3 Sense, used as a stand-in; it has the same footprint as the ESP32-C6.

## Scripts

**`fusion/wires.py`** rebuilds every wire in the model's `Wiring` component from a list of waypoints, with colours and sizes from the [wire list](../docs/wiring.md). Run it in Fusion's script runner, or through the Fusion MCP server with `exec(open('cad/fusion/wires.py').read())`. Edit the waypoint lists to change a route.

**`fusion/photo.py`** and **`fusion/steps.py`** render the documentation images into `docs/images`. Set the `CAM_REPO` environment variable, or edit `REPO`, to point at your checkout.

**`fusion/orient_stl.py`** turns STLs exported from Fusion into print-ready files:

```sh
# export bodies from Fusion into cad/stl/raw/ (model coordinates), then:
python3 cad/fusion/orient_stl.py
```

For each part it tries the sensible orientations, keeps the one with the most flat contact on the bed (which also needs the least support), drops it to z = 0, centres it, and updates the parts table in `stl/README.md`.
