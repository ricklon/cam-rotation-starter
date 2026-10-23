# Print files

Exported from Fusion and oriented for printing (z = 0 is the bed) by [`../fusion/orient_stl.py`](../fusion/orient_stl.py). All files are binary STL in millimetres.

<!-- parts -->
| Part | Orientation | Size on bed (mm) | Bed contact (mm²) |
|---|---|---|---|
| Servo_Mount | flange (wall face) down | 109.0 × 41.3 × 48.6 | 3858 |
| Control_Pod | front panel down, open back up | 39.2 × 38.0 × 24.0 | 955 |
| Adapter_075 | spline end down, socket up | 25.4 × 25.4 × 24.0 | 447 |
| Adapter_0625 | spline end down, socket up | 22.2 × 22.2 × 24.0 | 332 |
| Adapter_025 | spline end down, socket up | 12.6 × 12.6 × 24.0 | 84 |
| Bushing_075 | flange down | 28.5 × 28.5 × 6.0 | 267 |
| Bushing_0625 | flange down | 28.5 × 28.5 × 6.0 | 362 |
| Bushing_025 | flange down | 28.5 × 28.5 × 6.0 | 554 |
| Fit_Test_Coupon | flat side down | 80.0 × 45.0 × 6.0 | 2598 |
| Adapter_Clamp_0625 | 5/8 clamp, spline end down | 26.5 × 24.3 × 24.0 | 332 |
| Clamp_0625_cap | 5/8 clamp cap, split face down | 26.5 × 17.5 × 12.5 | 168 |
| Adapter_Clamp_025 | 1/4 clamp, spline end down | 19.9 × 16.3 × 24.0 | 130 |
| Clamp_025_cap | 1/4 clamp cap, split face down | 19.9 × 17.5 × 9.2 | 222 |
| Adapter_075_Sleeved | 3/4 sleeved, spline end down | 26.5 × 26.5 × 24.0 | 492 |
| Panel_Fit_Coupon | front panel down, like the pod | 27.0 × 38.0 × 20.7 | 955 |
| Dowel_Gauge_075 | 3/4 dowel gauge strip, flat | 151.5 × 28.8 × 4.0 | 2584 |
| Dowel_Gauge_0625 | 5/8 dowel gauge strip, flat | 132.4 × 25.6 × 4.0 | 2148 |
| Dowel_Gauge_025 | 1/4 dowel gauge strip, flat | 75.3 × 16.1 × 4.0 | 997 |
| Shim_075_for_19p0mm | shim for 19.0 mm dowel, on end | 21.5 × 21.5 × 17.5 | 73 |
| Shim_075_for_18p5mm | shim for 18.5 mm dowel, on end | 21.5 × 21.5 × 17.5 | 88 |
| Shim_075_for_18p0mm | shim for 18.0 mm dowel, on end | 21.5 × 21.5 × 17.5 | 102 |
| Shim_075_for_17p5mm | shim for 17.5 mm dowel, on end | 21.5 × 21.5 × 17.5 | 115 |
<!-- /parts -->

All files were checked: watertight, no non-manifold edges, and volumes match Fusion. `print_orientation_preview.png` shows how each sits on the bed. `raw/` holds the same parts in the model's coordinates, if you'd rather orient them yourself.

## Print order for a fit test

1. **Panel_Fit_Coupon**: a slice of the control pod with the button, switch and pot holes and the wall next to the button nut, printed the same way up as the pod. Fit all three controls and tighten the button's nut: it has only 0.2 mm to spare against the side wall. If anything is tight, fix it before printing the full pod (see "If the panel coupon doesn't fit" below).
2. **Fit_Test_Coupon** next; it is the quickest print. Push it onto the servo spline: the pocket that grips without forcing should be the 5.0 mm one the adapters use (`spline_dia`). If a different pocket fits best, change `spline_dia` in Fusion and re-export the adapters.
3. **Dowel_Gauge_075 / _0625 / _025**, one strip per dowel size you have. Each strip has six holes, labelled with how much bigger than the nominal dowel they are: +0.1, +0.2, **+0.3 A**, +0.4, **+0.5 B**, +0.7 mm. Try a real dowel in each:
   - **A** is the adapter socket clearance (`fit_clear`). The dowel should push in snugly by hand.
   - **B** is the bushing bore clearance (`hole_clear`). The dowel should slide and turn freely.
   - If a different hole gives the right feel, set `fit_clear` or `hole_clear` to that number in Fusion and re-export the adapters or bushings. A dowel that won't enter even +0.7 is oversize or swollen; one that rattles in +0.1 is sanded, so use the clamps or shims below.
4. **Bushings and adapters** for your dowel sizes, once the gauge says the clearances are right.
5. **Servo_Mount and Control_Pod** last. They are the long prints.

**Capacitor and diode seats:** the capacitor (C1) press-fits into a round seat in the outside of the electronics-side wall, beside the XIAO. The seat is 8.0 mm across and 4.9 mm deep, deeper than the can's radius, so its lips (7.8 mm apart) hold the can once it snaps in. The diode (D1) lies in a half-round channel on the inside of the same wall, at the floor, and its leads go out through two 1 mm holes to be soldered on the XIAO side (see [assembly step 4](../../docs/assembly.md#4-mount-and-wire-the-electronics)). If your capacitor isn't 8 mm, measure it and set `c1_seat_dia` to that.

## Suggested settings (PLA)

| Part | Layer | Walls | Infill | Supports |
|---|---|---|---|---|
| Fit_Test_Coupon | 0.2 mm | 3 | 20% | none |
| Dowel gauges | 0.16 mm | 3 | 20% | none |
| Adapters | 0.12–0.16 mm | 4 | 40% | none |
| Bushings | 0.16 mm | 3 | 30% | none |
| Control_Pod | 0.2 mm | 3 | 20% | build plate only, if the slicer flags the screw bosses |
| Servo_Mount | 0.2 mm | 3 | 25% | **yes**, build plate only (tree supports work well) under the base and the servo-tab shelf |

The finer layers on the adapters give a cleaner spline pocket and set-screw hole. Older STLs in the folder above this one are out of date; use this folder.

## Screws that go with these parts

| Where | Screw | Qty | Notes |
|---|---|---|---|
| Adapter to dowel | #4 × 3/8" flat-head wood screw (McMaster 90031A108) | 2 per adapter | 3/4" and 5/8" adapters. Two countersunk 3.1 mm holes: one on each side, 8 mm apart in height, so the screws never meet. Turn the adapter by hand to reach each one through the mount's open end. |
| 1/4" adapter to dowel | #4 × 1/4" flat-head wood screw (McMaster 92114A106, brass) | 2 | 3/8" is too long for a 1/4" dowel. |
| Control pod to mount | M3 × 8 wafer- or pan-head screw | 4 | Pilot holes are now 5.6 mm deep (pod_pilot_depth), leaving a 0.45 mm floor in the 6.1 mm wall. M3 × 6 also works. |
| Servo tabs, servo horn | the screws that come with the MG90S | 2 + 1 | |
| Wall | ~1/4" (#14) × 3/4" hex washer head screw, with 3/4" OD washers (McMaster 98026A029) | 2 + 2 | Keep the screw under 3/4" so it doesn't break through a 1/2" box wall. |

## Sanded dowels: clamps and shims

If a dowel has been sanded below its nominal size, use these instead of the plain adapters. Print only what you need.

| Dowel | Print | Hardware | How it holds |
|---|---|---|---|
| 5/8" sanded up to 1.5 mm | `Adapter_Clamp_0625` + `Clamp_0625_cap` | 2 × M2.5 × 10 button head (ISO 7380) | Two-piece clamp. The cap closes a 1.5 mm gap; the screws thread into 2.1 mm pilots in the fixed half. |
| 1/4" sanded up to 1.5 mm | `Adapter_Clamp_025` + `Clamp_025_cap` | 2 × M2.5 × 10 button head | Same design, on a 20 mm body. |
| 3/4" sanded | `Adapter_075_Sleeved` + one `Shim_075_for_…` | the two #4 × 3/8" wood screws | A larger bore takes a slit sleeve; the shim's holes line up with the adapter's screw holes. |

**Shims:** measure the dowel and print the shim closest to it: 19.0, 18.5, 18.0 or 17.5 mm. For any other size, set `shim_dowel` in Fusion and re-export. Put the slit away from the screw holes, line up the holes, slide the dowel in, then drive the two wood screws through.

**Clamps:** fit the cap loosely before sliding onto the shaft. Turn the adapter by hand so each screw faces an open end of the mount, then tighten evenly until the gap closes on the dowel. All clamp parts stay within 13.25 mm of the axis, 1.5 mm clear of the mount's side walls.

Print caps split face down. Print the clamp bodies, sleeved adapter and shims standing on end. Use 4 walls on the clamps for thread strength.

## If the panel coupon doesn't fit

![The panel fit coupon with the button, switch and pot fitted.](../../docs/images/panel_fit_coupon.png)

| Problem | Fix |
|---|---|
| Button won't go through its hole | The hole prints sideways and its top edge can sag. Ream it with a 12 mm (15/32") drill by hand, or raise `btn12_hole` from 12.2 to 12.4 mm. |
| Button nut hits the side wall | Turn the nut so a flat faces the wall. If it still binds, move the button over by lowering `btn12_x` from 7.0 to 6.5 mm (it still clears the switch). |
| Switch or pot won't go in | Ream the hole, or raise `pod_sw_hole` (6.2 mm) or `pod_pot_hole` (7.2 mm) by 0.2 mm. |
| Everything loose | Your printer prints holes large; lower the hole parameters, or reduce the slicer's hole compensation. |

Change the parameter in Fusion, then re-export the pod and the coupon.
