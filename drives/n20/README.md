# N20 drive

A version of the drive built around a 6 V N20 metal gearmotor and a DRV8833 motor driver, in place of the MG90S servo. The N20 is smaller, quieter and turns more smoothly at low speed than a modified servo, and you choose its speed and torque by picking a gear ratio.

**Status: planning.** The parts and wiring plan below are settled. The mount, the dowel adapter and the firmware motor driver are not designed or written yet.

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
| M1 | 1 | N20 gearmotor, **6 V**, no encoder | Pick the gear ratio for the speed you want. Typical 6 V no-load speeds are about 300 rpm at 100:1, 100 rpm at 298:1 and 30 rpm at 1000:1; at 5 V expect about 80% of that. 298:1 is closest to the MG90S's top speed (about 100 rpm) with more torque. |
| U3 | 1 | DRV8833 dual motor driver board | Only channel A is used. Motor supply 2.7–10.8 V, about 1.5 A per channel. |
| — | 2 | M1.6 × 3 or 4 mm screws | Hold the N20 by its gearbox face. Usually included with the motor. |
| — | 1 | 0.1 µF ceramic capacitor | Solder across the N20's terminals to cut electrical noise. |

## Wiring plan

| From | To | Why |
|---|---|---|
| PD board + (5 V) | C1 +, DRV8833 VM, D1 anode | Motor supply; the capacitor buffers start-up current |
| PD board GND | C1 −, DRV8833 GND, XIAO GND | Common ground |
| D1 cathode | XIAO 5V | Same as the MG90S drive |
| XIAO D0 (GPIO0) | DRV8833 AIN1 | PWM forward |
| XIAO D5 (GPIO23) | DRV8833 AIN2 | PWM reverse |
| XIAO D8 (GPIO19) | DRV8833 nSLEEP (EEP) | Keeps the driver asleep until the firmware is running, so the motor can't twitch at power-up. Some boards pull nSLEEP high on board; on those this wire is optional. |
| DRV8833 AOUT1, AOUT2 | N20 terminals | Swap them if forward runs backwards |

D1–D4 keep their MG90S jobs (button, knob, switch). D0 changes from the servo signal to AIN1.

The schematic and wire list will be drawn once the mount fixes where the driver board sits.

## Firmware plan

The firmware will choose the motor output at build time, so both drives build from the one `firmware/` folder:

- a `Drive type` choice in `idf.py menuconfig` (MG90S servo or N20 + DRV8833)
- `servo.c` kept for the MG90S; a new `motor_drv8833.c` with the same speed interface, using two LEDC PWM channels at about 20 kHz (above hearing) in slow-decay mode
- the stop trim setting hidden for the N20, which stops dead at zero

## Still to decide

- Which DRV8833 board you have, so the mount gets the right pocket and screw holes. Common ones are the small red module (often sold as HW-627), Adafruit's #3297 and Pololu's #2130, and they differ in size and pin order.
- The N20 gear ratio.
- Whether the N20 lines up on the dowel axis directly, which is simplest, or drives through a gear pair for more torque.

## Folder layout

```
cad/     Fusion exports and STLs for this drive (once designed)
docs/    parts, wiring, assembly for this drive (once designed)
```
