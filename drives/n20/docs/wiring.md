# Wiring: N20 drive

A USB-C PD trigger board set to 5 V powers a DRV8833 motor driver, which runs the N20 gearmotor. The same 5 V feeds an ESP32-C6 Super Mini through a diode. The Super Mini drives the DRV8833's channel B inputs from GPIO18 and GPIO20, and holds the driver asleep from GPIO19 until the firmware is running. All grounds are tied together.

From rev C the Super Mini, the DRV8833, the diode, the speed knob and the motor plug are soldered onto an ElectroCookie perfboard on the mount's enlarged +X flange ear, under a printed cover. The perfboard's hole map and build order are in [perfboard.md](perfboard.md).

- The PD board and its capacitor stay where they were.
- The motor unplugs at J4.
- The button and the direction switch stay in the pod and unplug at J3, as before.

## Schematic

[Open the schematic full size](images/schematic.svg) (SVG, rev C).

![Schematic, rev C. PD board 5 V goes to the DRV8833 VCC pin and, through diode D1, to the Super Mini's 5V pin; a 470 µF capacitor sits across 5 V. Super Mini GPIO18, GPIO20 and GPIO19 go to DRV8833 IN3, IN4 and EEP. DRV8833 OUT3 and OUT4 go through the 2-pin plug J4 to the N20 motor M1, with a 0.1 µF capacitor C2 across it. The 12 mm button S1 runs from GPIO1 to ground. The 10 kΩ knob RV1 sits between 3.3 V and ground with its wiper on GPIO2. Switch SW1 has its common to ground and its throws to GPIO3 and GPIO0. The button and switch wires pass through connector J3. U2, U3 and D1 are outlined as the perfboard.](images/schematic.svg)

## Connections

| From | To | Net |
|---|---|---|
| U1 OUT+ | perfboard 5V bus (row 8 left), C1 + | +5V |
| 5V bus | U3 VCC; D1 anode | +5V |
| D1 cathode | U2 5V pin | +5V, Super Mini side |
| U1 OUT− | perfboard GND bus (row 9 left), C1 − | GND |
| GND bus | U3 GND, U2 GND | GND |
| U2 GPIO18 | U3 IN3 | MOT_IN1 (forward) |
| U2 GPIO20 | U3 IN4 | MOT_IN2 (reverse) |
| U2 GPIO19 | U3 EEP (nSLEEP) | DRV_SLEEP |
| U3 OUT3, OUT4 | J4 → M1 terminals, C2 across them | motor |
| U2 3V3, GND | RV1 pins 3 and 1 | +3V3, GND |
| RV1 wiper | U2 GPIO2 | SPEED |
| U2 GPIO1, GPIO3, GPIO0, GND | J3 → the control pod | BTN, DIR_FWD, DIR_REV, GND |

## Parts

| Ref | Part | Note |
|---|---|---|
| U1 | USB-C PD trigger board | Set to 5 V. Sits on a 3.5 mm pad on the outside of the electronics-side wall. |
| U2 | ESP32-C6 Super Mini, 26 × 18 mm | On the perfboard, rows 12–17 (pins), USB-C toward the cover's −Y opening. Build the firmware for the Super Mini (see [Board](../../../firmware/README.md#board)). |
| U3 | DRV8833 module, 12-pin, 18.5 × 16 mm | On the perfboard, rows 1–6, component side out. Leave the **J1** jumper open. Channel B (IN3/IN4 → OUT3/OUT4) is used. |
| M1 | N20 gearmotor, 6 V, 100:1 | In the printed cartridge under the base, terminals pointing away from the base |
| C2 | Ceramic capacitor, 0.1 µF (104) | Soldered straight across the motor terminals |
| D1 | Schottky diode, 1 A, axial (1N5817) | Flat on the perfboard, B11 to B17, band at B17 |
| C1 | Electrolytic, 470 µF, ≥ 10 V, **8 × 13 mm** | Press-fits into the 14 mm-tall seat beside the old XIAO pocket, legs down |
| RV1 | RK097N 10 kΩ linear pot, 6 mm knurled shaft | On the perfboard, J8–J10, shaft through the cover's 7 mm hole; supplied knob cap |
| J3 | JST-PH 2.0 mm, 6 pins: B6B-PH-K-S header + PHR-6 housing | Header held in the pod; the mount harness ends in the housing. Pins 4 and 5 are unused from rev C. |
| J4 | JST-PH 2.0 mm, 2 pins, side entry: S2B-PH-K-S header + PHR-2 housing | Header on the perfboard at J2/J3; the motor wires end in the housing |
| S1, SW1 | Button, direction switch | As on the MG90S drive, wired to J3 |

The MG90S drive's R1 is not used. The pod's knob hole is left empty: RV1 moved to the perfboard.

**Building with a XIAO ESP32-C6 instead:** rev B wiring is in the git history (commit 3efb5c3, `drives/n20/docs/wiring.md`). The mount still has the XIAO pocket, the DRV8833 pocket and the diode channel, so a rev B build still fits. Build the firmware for the XIAO, the default.

![Straight-on view of the electronics side, the mount shown see-through: the PD board with C1 below it and L1, L2 down to the PD terminals; W1 and W2, red and black, leave the terminals, run round the end of the PD board and across to the cover at the right. The gray control harness comes up the face from the base tunnel and its wires join the run into the cover. The cover, with the perfboard inside, stands on the flange ear at the right, and the white and blue motor pair runs down behind the mount to the cartridge at the bottom.](images/wiring_electronics.png)

## The DRV8833 module's pins

The module has two rows of six pins. Seen from the back, where the labels are printed:

| One row | Other row |
|---|---|
| **IN4** · **IN3** · **GND** · **VCC** · IN2 · IN1 | **EEP** · OUT1 · OUT2 · **OUT3** · **OUT4** · ULT |

Bold pins are used. On the perfboard, component side up, these rows land in columns D and H, read the other way round: [perfboard.md](perfboard.md#placement) has the hole for each pin. Check against the labels on your module before soldering.

![The electronics side of the drive, seen from outside the box: the perfboard on its four bosses on the enlarged flange ear, cover removed. The DRV8833 and the Super Mini straddle the board's centre channel, D1 lies beside the Super Mini, and the speed pot RV1 hangs off the board's far edge. The red and black power wires come round the end of the PD board into the open side, and the panel harness comes in along the mount's face. The white and blue motor pair leaves J4 and runs back to the mount.](images/perfboard.png)

![The whole drive from below with the cover on: the gray printed cover over the perfboard on the enlarged flange ear, with the knob's cap standing out of it, the PD board and C1 on the electronics side, and the motor in its cartridge under the base with the motor pair running down to it.](images/perfboard_cover.png)

## J3: the control panel connector

The pod holds a 6-pin JST-PH header (B6B-PH-K-S) in a printed holder against its left wall, open toward the back. The mount-side harness ends in a PHR-6 housing that plugs into it from behind. To take the panel off, undo the four pod screws, pull the pod back about 15 mm and unplug J3.

| J3 pin | Signal | Mount side (from the perfboard) | Pod side |
|---|---|---|---|
| 1 | DIR_REV | W10 from I15 (GPIO0) | W10p to SW1 lug B |
| 2 | GND | W11 from A16 | W11p to SW1 centre lug |
| 3 | DIR_FWD | W9 from I12 (GPIO3) | W9p to SW1 lug A |
| 4 | — | unused (was SPEED) | — |
| 5 | — | unused (was +3V3) | — |
| 6 | BTN | W6 from I14 (GPIO1) | button S1 lead A, trimmed |

Pin 1 is the end nearest the pod's floor. Crimp the mount-side wires into the PHR-6 housing (or use a pre-crimped PH pigtail and solder it to the harness), and solder the pod-side wires to the header tails.

## J4 and the motor wire clamp

The motor plugs into J4, a side-entry JST-PH 2-pin header on the perfboard, so the motor and its cartridge can come off for transit or repair without desoldering. Crimp W14 into pin 1 and W15 into pin 2 of a PHR-2 housing.

At the motor, the wires are held right next to the terminals, so tugging or vibration loads the cartridge rather than the solder joints. This is the **wire clamp**: a 1.5 mm plate hanging 5.5 mm below the cartridge's +Y snap arm, with two slots for a **2.5 mm zip tie**.

1. Solder W14, W15 and C2 to the terminals.
2. Lead both wires toward the +Y arm, then along the inside face of the clamp plate toward the motor's round +X side, and up between the arms as before.
3. Put a zip tie in through one slot, out through the other, and round both wires on the plate's inside face. Pull it tight.

The tie bar sits about 5 mm from the terminals. Leave a little slack between the tie and each terminal, so the wire bends there and not at the joint.

![The bottom of the cartridge from the +Y side: below the black zip tie round the snap arms, the clamp plate hangs from the arm with two rectangular slots one above the other, and the blue motor wire shows through the lower slot. C2's tan body is just visible below.](images/motor_clamp.png)

![The underside of the mount, from below: the N20 in its cartridge on the dowel axis, with the black zip tie round its arms and C2 across its terminals. The empty DRV8833 pocket and diode channel of the XIAO build are beside it. The white and blue motor pair comes in under the base from the far corner, passes through anchor A2, and goes down the motor's side. The cover over the perfboard is at the left, on the flange ear.](images/wiring_under_base.png)

## Point-to-point wire list

Follow this table one wire at a time and tick each off. The colours match the 3D model. The links on the perfboard's underside are listed in [perfboard.md](perfboard.md#links-underside), and their cut lengths are in the cut list below.

| Wire | Colour | From | To | Notes |
|---|---|---|---|---|
| **Power harness, 22–26 AWG** | | | | |
| W1 | red | U1 + terminal | perfboard A8 (5V bus) | Round the −Y end of the PD board, along the mount's +X face, into the cover's open side. |
| W2 | black | U1 GND terminal | perfboard A9 (GND bus) | Beside W1. |
| L1 | red | C1 + (long leg) | U1 + terminal | |
| L2 | black | C1 − (stripe side) | U1 GND terminal | Reversed polarity damages the capacitor. |
| **Motor, 22–26 AWG** | | | | |
| W14 | white | J4 pin 1 (OUT3) | M1 terminal (either) | Out of the notch in the cover's +Y wall, down past the base's corner, under the base through anchor A2, down the motor's open side inside the arms' zip tie, then the clamp. If the motor runs backwards, swap the two pins in the PHR-2 housing. |
| W15 | blue | J4 pin 2 (OUT4) | M1 other terminal | Beside W14, then along the clamp's tie bar and behind the end cap, inside the snap hooks, to the far terminal. |
| C2 | — | M1 terminal | M1 other terminal | Solder its legs to the terminals with the wires. |
| **Control harness, mount side, 30 AWG: perfboard → cover's open side → along the −Y face → base tunnel → J3 housing** | | | | |
| W6 | white | perfboard I14 (GPIO1) | J3 pin 6 | |
| W9 | green | perfboard I12 (GPIO3) | J3 pin 3 | |
| W10 | blue | perfboard I15 (GPIO0) | J3 pin 1 | |
| W11 | black | perfboard A16 (GND) | J3 pin 2 | |
| **Pod side, 30 AWG** | | | | |
| W10p | blue | J3 pin 1 | SW1 lug B (reverse) | |
| W11p | black | J3 pin 2 | SW1 centre lug | Ground for the whole pod. |
| W9p | green | J3 pin 3 | SW1 lug A (forward) | |
| W6p | button lead | J3 pin 6 | S1 lead A | The button's own pre-wired lead, trimmed. |
| J1 | button lead | S1 lead B | SW1 centre lug | The button's other lead, trimmed. Button leads are interchangeable. |

Gone in rev C:

- **W7, W8, W7p, W8p and J2:** the knob is on the perfboard, wired by the links W19–W21.
- **The loose W4, W5, W12 and W13:** they are underside links now.
- **D1's long leads:** it sits on the board.

### Terminal check

When you're done, every terminal should hold exactly this many wires.

| Terminal | Wires landing here | Count |
|---|---|---|
| U1 + terminal | W1 · L1 | 2 |
| U1 GND terminal | W2 · L2 | 2 |
| Perfboard strip 8 left (5V bus) | W1 · W16 · W17 | 3 |
| Perfboard strip 9 left (GND bus) | W2 · W18 · W4 | 3 |
| Strip 3 left (U3 VCC) / 4 left (U3 GND) | W16 / W18 · W20 | 1 / 2 |
| Strip 5 left (IN3) / 6 left (IN4) / 6 right (EEP) | W5 / W12 / W13 | 1 each |
| Strip 11 left / 17 left | W17 · D1 anode / D1 cathode | 2 / 1 |
| Strips 12, 13, 14 left (GPIO18, 19, 20) | W5, W13, W12 | 1 each |
| Strip 15 left (3V3) / 16 left (GND) | W21 / W4 · W11 | 1 / 2 |
| Strips 8, 9, 10 right (RV1) | W20, W19, W21 | 1 each |
| Strips 12, 13, 14, 15 right (GPIO3, 2, 1, 0) | W9, W19, W6, W10 | 1 each |
| Strips 2, 3 right (OUT4, OUT3) | J4 pins | 1 each |
| J3 pins 1, 2, 3, 6 | one mount-side and one pod-side wire each | 1 + 1 |
| J3 pins 4, 5 | none | 0 |
| SW1 A / centre / B | W9p / W11p + J1 / W10p | 1 / 2 / 1 |
| M1 terminals | W14 + C2 leg / W15 + C2 leg | 2 each |

## Cut list

Lengths are measured along the routes in the 3D model, plus 15% slack and 5 mm per end for stripping, rounded up to 5 mm. The mount-side control wires include the 101 mm trunk from the mount's −Y face to J3. Also in [`cut-list.csv`](cut-list.csv).

| Wire | Colour | Gauge | Routed | **Cut** |
|---|---|---|---|---|
| W1 | red | 22–26 AWG | 88 mm | **115** |
| W2 | black | 22–26 AWG | 88 mm | **115** |
| L1 | red | 22–26 AWG | 49 mm | **70** |
| L2 | black | 22–26 AWG | 35 mm | **50** |
| W14 | white | 22–26 AWG | 138 mm | **170** |
| W15 | blue | 22–26 AWG | 145 mm | **180** |
| W6 | white | 30 AWG | 173 mm | **210** |
| W9 | green | 30 AWG | 175 mm | **215** |
| W10 | blue | 30 AWG | 172 mm | **210** |
| W11 | black | 30 AWG | 145 mm | **180** |
| W4 | black | 30 AWG solid | 20 mm | **35** |
| W5 | orange | 30 AWG solid | 18 mm | **35** |
| W12 | brown | 30 AWG solid | 21 mm | **35** |
| W13 | gray | 30 AWG solid | 39 mm | **55** |
| W16 | red | 30 AWG solid | 14 mm | **30** |
| W17 | red | 30 AWG solid | 8 mm | **20** |
| W18 | black | 30 AWG solid | 14 mm | **30** |
| W19 | yellow | 30 AWG solid | 11 mm | **25** |
| W20 | black | 30 AWG solid | 36 mm | **55** |
| W21 | violet | 30 AWG solid | 37 mm | **55** |
| W6p | button lead | pre-wired | 30 mm | **trim to 45** |
| W9p | green | 30 AWG | 27 mm | **45** |
| W10p | blue | 30 AWG | 29 mm | **45** |
| W11p | black | 30 AWG | 28 mm | **45** |
| J1 | button lead | pre-wired | 21 mm | **trim to 35** |

## Why it's wired this way

- The DRV8833 takes its motor supply straight from the PD board, through the perfboard's 5V bus, so motor current doesn't pass through the Super Mini. Its VCC pin is the chip's VM; the module makes its own logic supply.
- The Super Mini's 3.3 V outputs drive the DRV8833 inputs directly: they accept logic levels from about 2 V up.
- EEP is the chip's sleep input. Held low by GPIO19 until the firmware has started, the motor can't twitch at power-up. The module's J1 jumper would tie EEP high permanently, which is why it stays open.
- C2 across the motor terminals absorbs the brush noise, which otherwise couples into the knob reading and the Wi-Fi.
- D1, C1 and the single common ground do the same jobs as on the MG90S drive: see [why it's wired this way](../../mg90s/docs/wiring.md#why-its-wired-this-way) there. D1 also stops the Super Mini's USB port from back-feeding the motor supply when it's plugged into a computer.
- GPIO18, GPIO20 and GPIO19 are the same GPIOs the XIAO build uses, so the motor code doesn't change between boards. The panel pins, GPIO1, GPIO2, GPIO3 and GPIO0, avoid the ESP32-C6's strapping pins (GPIO4, 5, 8, 9, 15), USB (GPIO12, 13) and UART0 (GPIO16, 17). GPIO2 is an ADC1 pin for the knob.
- The motor plugs in at J4 and its wires are clamped at the cartridge, because the solder joints on the motor's tabs were getting stressed in transit when the wires could pull on them.
- J3 carries only the four low-current panel signals, so a small 2.0 mm JST-PH is enough.

## Bring-up

1. Set the PD board to 5 V and measure OUT+ to OUT− with a meter before wiring anything.
2. Before applying power, check each row of the terminal check table for continuity. Confirm that +5V, +3V3 and GND aren't connected to each other.
3. Flash the N20 build of the firmware for the Super Mini (see [Board](../../../firmware/README.md#board)). The XIAO build drives the wrong pins on a Super Mini, and the MG90S build leaves EEP low, so the motor never runs.
4. Plug in J3 and J4. Turn the knob up slowly with the switch on forward. If the motor turns the wrong way, swap the two pins in J4's housing. If the knob works backwards, swap W20 and W21 at the pot.
5. Check the controls: the button should start and pause a story, the knob should change speed, and the switch should run forward, stop in the centre, and reverse.
