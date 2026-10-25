# N20 drive

A version of the drive built around a 6 V N20 metal gearmotor and a DRV8833 motor driver, in place of the MG90S servo. The N20 is smaller, quieter and turns more smoothly at low speed than a modified servo, and you choose its speed and torque by picking a gear ratio.

**Status: fit testing.** The parts and wiring plan are settled, the firmware drives the N20, and the mount and adapters are designed. The first printed mount showed the motor's screw holes are the inline pair, and that the boards need room for solder and parts underneath; `N20_Mount` was updated for both on 2026-10-24 (see [cad/stl](cad/stl/README.md)). Wiring, schematic and cut list are in [docs/wiring.md](docs/wiring.md); the assembly guide comes next.

- **Fusion design:** *Cam Rotation N20* in MovementStudy › Cam Rotation Tool, copied from *Cam Rotation Starter*. The N20 reference model is *N20 DC MOTOR*, copied into the same folder.

## What carries over from the MG90S drive

| Part | Reused as is? | Notes |
|---|---|---|
| XIAO ESP32-C6, USB-C PD trigger board (5 V), 470 µF capacitor, 1N5817 diode | Yes | Same power path: PD 5 V feeds the motor side; the XIAO gets 5 V through the diode. |
| Control pod, 12 mm button, speed knob, direction switch | Yes | Same pins as the MG90S drive (D1, D2, D3, D4). |
| Bushings, dowel gauges, shims, clamp caps, control pod | Yes | Print them from [`drives/mg90s/cad/stl`](../mg90s/cad/stl/README.md). |
| Dowel adapters and clamp bodies | No | Same dowel sockets and grip screws, but a 3 mm D-shaft bore through the base in place of the servo spline pocket and horn screw. `N20_Adapter_*` in [cad/stl](cad/stl/README.md). |
| Mount | Changed | `N20_Mount`: same walls, flange, pod face and XIAO pocket; the PD board sits on a 3.5 mm pad on the outside of the wall instead of in a flush pocket. The motor hangs on the dowel axis from a 2 mm face plate at the old spline height, in a sleeve that holds the gearbox, and reaches about 4 mm further below the base than the servo did. The DRV8833 sits in a pocket on the underside of the base. Both the XIAO's and the DRV8833's pockets have a 2.5 mm relief under their pin rows for wire ends and solder. |
| 330 Ω signal resistor (R1) | Dropped | Not needed for a motor driver input. |
| Firmware | Shared | Same story player and web app; a DRV8833 motor output in place of the servo pulse (see below). |

## New parts

| Ref | Qty | Part | Notes |
|---|---|---|---|
| M1 | 1 | N20 gearmotor, **6 V, 100:1**, no encoder | Chosen for speed (see Decisions). Other ratios fit the same mount. Typical 6 V no-load speeds are about 300 rpm at 100:1, 100 rpm at 298:1 and 30 rpm at 1000:1; at 5 V expect about 80% of that. 298:1 is closest to the MG90S's top speed (about 100 rpm) with more torque. |
| U3 | 1 | DRV8833 dual motor driver module, 12-pin (two rows of six 0.1" pins: IN4 IN3 GND VCC IN2 IN1 / EEP OUT1–OUT4 ULT), sold in 5-packs (e.g. "EC Buying") | Only channel 1 (IN1/IN2 → OUT1/OUT2) is used. Motor supply 2.7–10.8 V, about 1.5 A per channel. Leave the J1 jumper open. |
| — | 2 | M1.6 × 4 mm screws | Hold the N20 by its gearbox face, through the 2 mm face plate. Often included with the motor. |
| — | 1 | Double-sided foam tape, about 15 × 15 mm | Holds the DRV8833 module in its pocket. |
| — | 1 | 0.1 µF ceramic capacitor | Solder across the N20's terminals to cut electrical noise. |

## Wiring plan

| From | To | Why |
|---|---|---|
| PD board + (5 V) | C1 +, DRV8833 VCC, D1 anode | Motor supply (VCC on this module is the DRV8833's VM); the capacitor buffers start-up current |
| PD board GND | C1 −, DRV8833 GND, XIAO GND | Common ground |
| D1 cathode | XIAO 5V | Same as the MG90S drive |
| XIAO D0 (GPIO0) | DRV8833 IN1 | PWM forward |
| XIAO D5 (GPIO23) | DRV8833 IN2 | PWM reverse |
| XIAO D8 (GPIO19) | DRV8833 EEP (nSLEEP) | Keeps the driver asleep until the firmware is running, so the motor can't twitch at power-up. Leave the module's J1 jumper open: bridging it ties EEP high. |
| DRV8833 OUT1, OUT2 | N20 terminals | Swap them if forward runs backwards |

IN3, IN4, OUT3, OUT4 and ULT (the fault output) stay unconnected.

D1–D4 keep their MG90S jobs (button, knob, switch). D0 changes from the servo signal to AIN1.

The full wire list (W1–W15), schematic and cut list are in [docs/wiring.md](docs/wiring.md).

## Firmware

Both drives build from the one `firmware/` folder; the motor output is chosen at build time. Build the N20 version with the `sdkconfig.n20` fragment (see [Drive type](../../firmware/README.md#drive-type)):

- `motor_drv8833.c` has the same speed interface as the servo's `servo.c`: two LEDC PWM channels at 20 kHz (above hearing) in slow-decay mode, so the motor brakes between pulses and stops dead at zero
- nSLEEP stays low until the firmware runs, and goes low again after the motor has been stopped for `idle_release_ms`
- the web app hides the stop trim and the µs range on an N20 box

To check on hardware: the lowest speed at which your N20 still turns under the cam's load. If it stalls and hums at low knob settings, the firmware needs a minimum duty setting.

## Decisions

- **Gear ratio: 100:1.** About 300 rpm at 6 V, so about 240 rpm at the 5 V the PD board supplies: faster than the MG90S (about 100 rpm), with less torque. Story speeds will feel about twice as fast at the same percentage; lower the speeds in a story, or the knob, to match.
- **Layout: direct drive.** The N20 sits on the dowel axis, like the servo spline, and turns the dowel through an adapter with a 3 mm D-shaft bore.
- **Driver: the 12-pin DRV8833 module** described above: 18.5 × 16 mm, its two pin rows 0.6" (15.24 mm) apart along the 18.5 mm side.
- **Gearbox screw holes: the inline pair,** 9 mm apart across the 12 mm width, at (±4.5, 0). The first mount followed the N20 reference model's diagonal pair (9.4 mm apart), which didn't match our motors. The reference model in Fusion still shows the diagonal holes.
- **Room to solder under the boards.** Wires soldered through the XIAO's and the DRV8833's pin rows leave ends and solder on the back of the board, so each pocket has a 2.5 mm relief under both pin rows (`solder_relief_d`). The boards rest on the strip between the rows.
- **Room under the PD board: a 3.5 mm pad.** The USB-C PD board has parts all over its underside, and the wall behind its pocket was already used up by the zip-tie tunnel and the clamp-head relief on the inside. So the board moved 3.5 mm out (`pd_relief_d`) onto a pad on the outside of the wall: it rests on a 1 mm ledge round its edges, with 3.5 mm clear under the rest. The zip-tie tunnel and slots didn't move; the tie runs out of its slots just above and below the pad and over the board. The mount is 3.5 mm wider on that side (44.8 mm), and the PD board's wires (W1, W2, W4, L1–L3) end 3.5 mm further out; W2 and L2 are cut 5 mm longer.

## Still to decide

- An assembly guide for the N20 drive (the MG90S one covers most steps; the motor, sleeve and module steps are new).

## Fusion design

*Cam Rotation N20* holds the N20 changes on top of the MG90S design. The servo and its screws are hidden, not deleted. New user parameters, all prefixed `n20_` or `drv_`:

| Parameter | Default | What it controls |
|---|---|---|
| `n20_face_z` | 0 mm | Height of the gearbox face (the adapter bottom stays at 4.2 mm) |
| `n20_clear` | 0.3 mm | Clearance around the gearbox in its sleeve |
| `n20_plate_thk`, `n20_tube_wall` | 2 mm | Face plate and sleeve wall |
| `n20_hole_x`, `n20_hole_y` | 4.5, 0 mm | Face screw holes, at (+x, +y) and (−x, −y): 4.5, 0 is the inline pair; 3.8, 2.8 the diagonal pair |
| `n20_screw_clear`, `n20_shaft_hole` | 1.9, 4.5 mm | Holes in the face plate |
| `n20_shaft_dia`, `n20_flat` | 3.0, 2.5 mm | The motor shaft and its D flat |
| `n20_bore_clear` | 0.15 mm | D-bore clearance; set it from the fit coupon |
| `drv_len`, `drv_wid`, `drv_thk` | 18.5, 16, 1.6 mm | The DRV8833 module |
| `drv_x`, `drv_pocket_d` | 18.15, 1 mm | Its pocket on the underside of the base |
| `pd_relief_d` | 3.5 mm | Room under the PD board: the thickness of the pad it sits on, outside the electronics-side wall |
| `solder_relief_d` | 2.5 mm | Relief under the XIAO's and the DRV8833's pin rows, below the pocket floor. Under the DRV8833 it leaves 0.5 mm of base; 2.0 mm leaves 1 mm. |
| `wall_relief_d`, `wall_relief_z0`, `wall_relief_h` | 0.6, 15, 9 mm | Relief in the inside of both side walls where the 5/8 clamp's screw heads pass. 0.6 mm leaves 1 mm of wall in front of the PD board's zip-tie tunnel. |
| `clamp_head_y_mid` | 2.8 mm | 5/8 clamp screw-head depth (as on the MG90S drive) |

## Folder layout

```
cad/step/   cam-rotation-n20.step and .f3d: snapshots of the Fusion design
cad/stl/    print files for the parts that differ from the MG90S drive
cad/fusion/ orient_stl.py for this drive's STLs; wires.py rebuilds the Wiring component
docs/       wiring.md, cut-list.csv, images/ (schematic, wiring render); assembly to come
```
