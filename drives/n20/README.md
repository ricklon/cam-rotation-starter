# N20 drive

A version of the drive built around a 6 V N20 metal gearmotor and a DRV8833 motor driver, in place of the MG90S servo. The N20 is smaller, quieter and turns more smoothly at low speed than a modified servo, and you choose its speed and torque by picking a gear ratio.

**Status: in progress.** The parts and wiring plan below are settled, and the firmware drives the N20 (untested on hardware). The mount and the dowel adapter are not designed yet.

- **Fusion design:** *Cam Rotation N20* in MovementStudy › Cam Rotation Tool, copied from *Cam Rotation Starter*. The N20 reference model is *N20 DC MOTOR*, copied into the same folder.

## What carries over from the MG90S drive

| Part | Reused as is? | Notes |
|---|---|---|
| XIAO ESP32-C6, USB-C PD trigger board (5 V), 470 µF capacitor, 1N5817 diode | Yes | Same power path: PD 5 V feeds the motor side; the XIAO gets 5 V through the diode. |
| Control pod, 12 mm button, speed knob, direction switch | Yes | Same pins as the MG90S drive (D1, D2, D3, D4). |
| Bushings, dowel gauges, shims | Yes | Print them from [`drives/mg90s/cad/stl`](../mg90s/cad/stl/README.md) until this drive has its own STL set. |
| Dowel adapters and clamps | No | Same dowel sockets, but a 3 mm D-shaft bore in place of the servo spline pocket. |
| Mount | No | New motor pocket: the N20 is 12 × 10 mm in section and 25 mm long, held by two M1.6 screws in the gearbox face. |
| 330 Ω signal resistor (R1) | Dropped | Not needed for a motor driver input. |
| Firmware | Shared | Same story player and web app; a DRV8833 motor output in place of the servo pulse (see below). |

## New parts

| Ref | Qty | Part | Notes |
|---|---|---|---|
| M1 | 1 | N20 gearmotor, **6 V, 100:1**, no encoder | Chosen for speed (see Decisions). Other ratios fit the same mount for the speed you want. Typical 6 V no-load speeds are about 300 rpm at 100:1, 100 rpm at 298:1 and 30 rpm at 1000:1; at 5 V expect about 80% of that. 298:1 is closest to the MG90S's top speed (about 100 rpm) with more torque. |
| U3 | 1 | DRV8833 dual motor driver module, 12-pin (two rows of six 0.1" pins: IN4 IN3 GND VCC IN2 IN1 / EEP OUT1–OUT4 ULT), sold in 5-packs (e.g. "EC Buying") | Only channel 1 (IN1/IN2 → OUT1/OUT2) is used. Motor supply 2.7–10.8 V, about 1.5 A per channel. Leave the J1 jumper open. |
| — | 2 | M1.6 × 3 or 4 mm screws | Hold the N20 by its gearbox face. Usually included with the motor. |
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

The schematic and wire list will be drawn once the mount fixes where the driver board sits.

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

## Still to decide

- Board thickness and the height of its tallest part (the tantalum capacitor). The pocket assumes a 1.6 mm board and about 2 mm of parts on top; a fit coupon will check it before the full mount is printed.

## Folder layout

```
cad/     Fusion exports and STLs for this drive (once designed)
docs/    parts, wiring, assembly for this drive (once designed)
```
