# Wiring: N20 drive

A USB-C PD trigger board set to 5 V powers a DRV8833 motor driver, which runs the N20 gearmotor, and feeds the XIAO ESP32-C6 through a diode. The XIAO drives the DRV8833's inputs from D0 and D5 and holds it asleep from D8 until the firmware is running. The control pod (button, speed knob, direction switch) is wired exactly as on the [MG90S drive](../../mg90s/docs/wiring.md). All grounds are tied together.

## Schematic

[Open the schematic full size](images/schematic.svg) (SVG, rev A).

![Schematic: PD board 5 V to the DRV8833 VCC pin and, through diode D1, to the XIAO 5V pin; 470 µF capacitor across 5 V; XIAO D0, D5 and D8 to DRV8833 IN1, IN2 and EEP; DRV8833 OUT1 and OUT2 to the N20 motor M1 with a 0.1 µF capacitor C2 across it; 12 mm button S1 from D1 to ground; 10 kΩ knob RV1 between 3.3 V and ground with its wiper on D2; switch SW1 common to ground, throws to D3 and D4.](images/schematic.svg)

## Connections

| From | To | Net |
|---|---|---|
| U1 OUT+ | U3 VCC, C1 +, D1 anode | +5V |
| D1 cathode | U2 5V pin | +5V |
| U1 OUT− | U3 GND, C1 −, U2 GND | GND |
| U2 D0 (GPIO0) | U3 IN1 | MOT_IN1 |
| U2 D5 (GPIO23) | U3 IN2 | MOT_IN2 |
| U2 D8 (GPIO19) | U3 EEP (nSLEEP) | DRV_SLEEP |
| U3 OUT1, OUT2 | M1 terminals, C2 across them | motor |
| U2 D1–D4, 3V3, GND | the control pod | as on the MG90S drive |

## Parts

| Ref | Part | Note |
|---|---|---|
| U1 | USB-C PD trigger board | Set to 5 V. Sits on a 3.5 mm pad on the outside of the electronics-side wall, with room under it for its bottom-side parts. |
| U2 | Seeed XIAO ESP32-C6 | In the mount's side pocket, which has a 2.5 mm relief under both pin rows for wire ends and solder |
| U3 | DRV8833 module, 12-pin, 18.5 × 16 mm | In the pocket under the base, component side out. Leave the **J1** jumper open. Only channel 1 is used. |
| M1 | N20 gearmotor, 6 V, 100:1 | On the dowel axis; terminals point away from the base |
| C2 | Ceramic capacitor, 0.1 µF (104) | Soldered straight across the motor terminals |
| D1 | Schottky diode, 1 A (1N5817 or SS14) | As on the MG90S drive |
| C1 | Electrolytic, 470 µF, ≥ 10 V, 8 mm | As on the MG90S drive |
| S1, RV1, SW1 | Button, speed knob, direction switch | As on the MG90S drive |

The MG90S drive's R1 is not used.

## The DRV8833 module's pins

The module has two rows of six pins. Seen from the back, where the labels are printed:

| Row toward the XIAO side | Row toward the pod side |
|---|---|
| IN4 · IN3 · **GND** · **VCC** · **IN2** · **IN1** | **EEP** · **OUT1** · **OUT2** · OUT3 · OUT4 · ULT |

Fit the module with the IN row toward the wall that carries the XIAO. Bold pins are used; IN3, IN4, OUT3, OUT4 and ULT stay empty. Solder the wires from the component side. The pocket has a 2.5 mm relief under each pin row, so the wire ends and solder on the back have room; trim them to under 2.5 mm and put the foam tape on the strip between the rows.

![The underside of the mount: the N20 hangs from its sleeve on the dowel axis, and the DRV8833 module is taped under the base beside it. Red and black power wires come down the electronics-side wall and run under the base to the module; orange, brown and gray signal wires come from the XIAO to the module; white and blue wires run from the module round the motor to its terminals, with the tan C2 capacitor across them.](images/wiring_under_base.png)

## Point-to-point wire list

Follow this table one wire at a time and tick each off. The colours match the 3D model. The XIAO's power pin is printed 5V; it's the board's VIN.

| Wire | Colour | From | To | Notes |
|---|---|---|---|---|
| **Power harness, 22–26 AWG** | | | | |
| W1 | red | U1 + terminal | U3 VCC | Down the groove in the electronics-side wall, then under the base. |
| W2 | black | U1 GND terminal | U3 GND | Beside W1. |
| L1 | red | C1 + (long leg) | U1 + terminal | As on the MG90S drive. |
| L2 | black | C1 − (stripe side) | U1 GND terminal | Reversed polarity damages the capacitor. |
| L3 | red | D1 anode (plain end) | U1 + terminal | Solder where the lead comes through the wall; heat-shrink the joint. |
| W3 | red | D1 cathode (banded end) | XIAO 5V (VIN) | Solder where the lead comes through the wall; heat-shrink the joint. |
| W4 | black | U1 GND terminal | XIAO GND | |
| **Motor driver signals, 30 AWG** | | | | |
| W5 | orange | XIAO D0 | U3 IN1 | Round the end of the XIAO with the USB socket, then under the base. |
| W12 | brown | XIAO D5 | U3 IN2 | Over the top of the XIAO, outside the control bundle. |
| W13 | gray | XIAO D8 | U3 EEP | From the XIAO's bottom row, under the base between the motor and the module. |
| **Motor, 22–26 AWG** | | | | |
| W14 | white | U3 OUT1 | M1 terminal (either) | If the motor runs backwards, swap W14 and W15 at the motor. |
| W15 | blue | U3 OUT2 | M1 other terminal | Round the far side of the motor. |
| C2 | — | M1 terminal | M1 other terminal | Solder its legs to the terminals with the wires. |
| **Control bundle, 30 AWG** | | | | |
| W6–W11, J1, J2 | | | | Exactly as on the [MG90S drive](../../mg90s/docs/wiring.md#point-to-point-wire-list). |

### Terminal check

When you're done, every terminal should hold exactly this many wires.

| Terminal | Wires landing here | Count |
|---|---|---|
| U1 + terminal | W1 · L1 C1 + · L3 D1 anode | 3 |
| U1 GND terminal | W2 · L2 C1 − · W4 to XIAO | 3 |
| XIAO 5V (VIN) | W3 from D1 cathode | 1 |
| XIAO GND | W4 from U1 · W11 to pod | 2 |
| XIAO D0 / D5 / D8 | W5 / W12 / W13 | 1 each |
| XIAO 3V3, D1–D4 | as on the MG90S drive | 1 each |
| U3 VCC / GND | W1 / W2 | 1 each |
| U3 IN1 / IN2 / EEP | W5 / W12 / W13 | 1 each |
| U3 OUT1 / OUT2 | W14 / W15 | 1 each |
| M1 terminals | W14 + C2 leg / W15 + C2 leg | 2 each |

## Cut list

Lengths measured along the routes in the 3D model, plus 15% slack and 5 mm per end for stripping, rounded up to 5 mm. Also in [`cut-list.csv`](cut-list.csv).

| Wire | Colour | Gauge | Routed | **Cut** |
|---|---|---|---|---|
| W1 | red | 22–26 AWG | 89 mm | **115** |
| W2 | black | 22–26 AWG | 83 mm | **110** |
| L1 | red | 22–26 AWG | 50 mm | **70** |
| L2 | black | 22–26 AWG | 36 mm | **55** |
| L3 | red | 22–26 AWG | 37 mm | **55** |
| W3 | red | 22–26 AWG | 25 mm | **40** |
| W4 | black | 22–26 AWG | 59 mm | **80** |
| W5 | orange | 30 AWG | 45 mm | **65** |
| W12 | brown | 30 AWG | 66 mm | **90** |
| W13 | gray | 30 AWG | 44 mm | **65** |
| W14 | white | 22–26 AWG | 26 mm | **40** |
| W15 | blue | 22–26 AWG | 37 mm | **55** |
| W6–W11, J1, J2 | | | | as on the [MG90S drive](../../mg90s/docs/wiring.md#cut-list) |

## Why it's wired this way

- The DRV8833 takes its motor supply straight from the PD board, so motor current doesn't pass through the XIAO. Its VCC pin is the chip's VM; the module makes its own logic supply.
- The XIAO's 3.3 V outputs drive the DRV8833 inputs directly: they accept logic levels from about 2 V up.
- EEP is the chip's sleep input. Held low by D8 until the firmware has started, the motor can't twitch at power-up. The module's J1 jumper would tie EEP high permanently, which is why it stays open.
- C2 across the motor terminals absorbs the brush noise, which otherwise couples into the knob reading and the Wi-Fi.
- D1, C1 and the single common ground do the same jobs as on the MG90S drive: see [why it's wired this way](../../mg90s/docs/wiring.md#why-its-wired-this-way) there.
- D5 (GPIO23) and D8 (GPIO19) are not boot-strapping pins on the ESP32-C6, so they're safe for the driver.

## Bring-up

1. Set the PD board to 5 V and measure OUT+ to OUT− with a meter before wiring anything.
2. Before applying power, check each row of the terminal check table for continuity, and confirm that + and GND are not connected to each other.
3. Flash the N20 build of the firmware (see [Drive type](../../../firmware/README.md#drive-type)). The MG90S build would leave EEP low and the motor would never run.
4. Turn the knob up slowly with the switch on forward. If the motor turns the wrong way, swap W14 and W15 at the motor.
5. Check the controls: the button should start and pause a story, the knob should change speed, and the switch should run forward, stop in the centre, and reverse.
