# N20 print files

Exported from the Fusion design *Cam Rotation N20* and oriented for printing (z = 0 is the bed) by [`../fusion/orient_stl.py`](../fusion/orient_stl.py). All files are binary STL in millimetres. `raw/` (not in git) holds the same parts in the model's coordinates.

<!-- parts -->
| Part | Orientation | Size on bed (mm) | Bed contact (mm²) |
|---|---|---|---|
| N20_Mount | flange (wall face) down | 109.0 × 41.3 × 48.6 | 3858 |
| N20_Fit_Coupon | flat side down, sleeve and bosses up | 71.2 × 24.8 × 8.0 | 1484 |
| N20_Adapter_075 | D-bore end down, socket up | 25.4 × 25.4 × 24.0 | 462 |
| N20_Adapter_0625 | D-bore end down, socket up | 22.2 × 22.2 × 24.0 | 347 |
| N20_Adapter_025 | D-bore end down, socket up | 12.6 × 12.6 × 24.0 | 99 |
| N20_Adapter_Clamp_0625 | 5/8 clamp, D-bore end down | 26.5 × 24.3 × 24.0 | 347 |
| N20_Adapter_Clamp_025 | 1/4 clamp, socket end down (more bed contact) | 19.9 × 16.3 × 24.0 | 130 |
| N20_Adapter_075_Sleeved | 3/4 sleeved, D-bore end down | 26.5 × 26.5 × 24.0 | 507 |
| N20_Clamp_0625_cap | 5/8 clamp cap, split face down | 26.5 × 17.5 × 12.5 | 163 |
| N20_Clamp_025_cap | 1/4 clamp cap, split face down | 19.9 × 17.5 × 9.2 | 218 |
<!-- /parts -->

Parts the N20 drive shares with the MG90S drive aren't repeated here. Print them from [`drives/mg90s/cad/stl`](../../../mg90s/cad/stl/README.md): `Control_Pod`, `Panel_Fit_Coupon`, the bushings, the dowel gauges, the shims and the clamp caps (`Clamp_0625_cap`, `Clamp_025_cap`).

## Print the fit coupon first

`N20_Fit_Coupon` is a 20-minute print that checks everything new before the long mount print:

| Section | Check | If it's wrong |
|---|---|---|
| Motor plate and sleeve (centre) | The gearbox drops into the sleeve and sits flat on the plate. Two pairs of 1.9 mm holes are in the plate: a **diagonal** pair (as in the N20 model, 9.4 mm apart) and an **inline** pair across the 12 mm width (9 mm apart). See which pair lines up with the threaded holes in your gearbox face. | Tight sleeve: raise `n20_clear` (0.3 mm). If the inline pair matches, set `n20_hole_x` = 4.5 mm and `n20_hole_y` = 0. |
| D-bore bosses (left) | Push the motor shaft into each boss. One notch = 0.10 mm clearance, two = 0.15 mm, three = 0.25 mm. Pick the one that goes on by hand and doesn't wobble. | Set `n20_bore_clear` to that value (default 0.15 mm, two notches). |
| DRV8833 pocket (right) | The module drops into the 1 mm pocket, component side up. | Raise `board_clear` (0.3 mm). |

Change the parameter in Fusion, then re-export the mount and the adapters.

## Suggested settings (PLA)

| Part | Layer | Walls | Infill | Supports |
|---|---|---|---|---|
| N20_Fit_Coupon | 0.16 mm | 3 | 20% | none |
| Adapters | 0.12–0.16 mm | 4 | 40% | none |
| N20_Mount | 0.2 mm | 3 | 25% | **yes**, build plate only, under the base, as for the MG90S mount |

Printed flange down, the mount's new face plate (the 2 mm shelf the motor screws to) is a 29 mm bridge between the side walls. Most printers bridge that cleanly; if yours sags, add supports there, or print a test bridge first.

## Fitting the motor and the driver

- **Motor:** push the gearbox up into the sleeve from below, shaft first, until its face sits on the face plate. Fix it with two **M1.6 × 4** screws from above, through the plate into the gearbox face. The screw heads sit in the 2.2 mm gap under the adapter.
- **Adapter:** slides onto the D-shaft; the flat in the bore lines up with the flat on the shaft. Grip screws into the dowel as on the MG90S drive.
- **DRV8833 module:** solder its wires first, then stick it into the pocket on the underside of the base with a square of double-sided foam tape, component side out. Leave its **J1** jumper open.
