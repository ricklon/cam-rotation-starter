# Perfboard: N20 drive electronics

From rev C the electronics are soldered onto an **ElectroCookie mini solderable breadboard** (1/4 size, 38.1 × 50.8 mm) in place of the boards wired loose in their pockets. The perfboard carries:

- the ESP32-C6 Super Mini (U2)
- the DRV8833 module (U3)
- the diode D1
- the speed knob RV1, an RK097N pot that now sits here rather than in the pod
- the motor plug J4.

The PD board (U1) and C1 stay on the mount. The motor plugs into J4, and the button and switch in the pod connect through J3 as before. The board sits on the mount's enlarged flange ear under a printed cover, and the knob's shaft comes out through the cover (see [Where it goes](#where-it-goes)). The schematic and the full wire list are in [wiring.md](wiring.md).

![Hole map of the perfboard, seen from the component side with row 1 at the top. The DRV8833 straddles the centre channel in rows 1 to 6, with the J4 motor header beside it at J2 and J3. The Super Mini straddles the channel in rows 12 to 17, with its USB-C port past row 17. The speed pot RV1 has its pins at J8 to J10, and its body overhangs the right-hand edge. Short links on the underside join the ground bus (row 9), the 5 V bus (row 8), the driver signals and the pot. D1 lies along column B from row 11 to row 17.](images/perfboard.svg)

## The board

| | |
|---|---|
| Size | 38.1 × 50.8 mm (1.5 × 2.0 in), 1.6 mm FR4, plated through-holes |
| Holes | 0.1" (2.54 mm) grid, rows 1–17 |
| Strips | Each row is two strips of five joined holes, **A–E** and **F–J**, either side of a 0.3" centre channel. There are no power rails. |
| Mounting | Two Ø3.2 mm (M3) holes in the channel at rows 1 and 17, 40.6 mm apart. Four Ø2.2 mm (M2) corner holes at 31.8 × 44.5 mm. The drive uses the four **M2** holes; the M3 holes end up under the modules. |

Columns run A to J left to right with the component side up and row 1 at the top. Hole positions from the board's centre are x = (column index − 5.5) × 2.54 mm (A = 0 … E = 4, F = 7 … J = 11) and y = (row − 9) × 2.54 mm.

## Placement

Both modules straddle the channel with their two pin rows in columns **D** and **H**, 0.6" apart. That leaves holes A, B and C free in each left-hand strip, and I and J in each right-hand strip. Fit header pins **only where this table lists them**, and pull the others out of the strip with pliers. A strip with no module pin in it is then free to use as a bus. This also leaves out the Super Mini's two unused strapping pins (GPIO8, GPIO9).

| Part | Where | Pins |
|---|---|---|
| U3 DRV8833 module, component side up | Rows 1–6 (pins used in rows 2–6), IN row in column D, OUT row in column H | D3 VCC · D4 GND · D5 IN3 · D6 IN4 (a 4-pin strip); H2 OUT4 · H3 OUT3 · H6 EEP (a 5-pin strip with the rows 4 and 5 pins pulled) |
| U2 ESP32-C6 Super Mini, component side up, USB-C past row 17 | Rows 8–17 (pins), bottom-edge pins in column D, top-edge pins in column H | D12 GPIO18 · D13 GPIO19 · D14 GPIO20 · D15 3V3 · D16 GND · D17 5V (a 6-pin strip); H12 GPIO3 · H13 GPIO2 · H14 GPIO1 · H15 GPIO0 (a 4-pin strip) |
| D1 1N5817, lying flat | Anode B11, cathode (banded end) B17 | Its own leads, bent to 15.24 mm (6 holes) |
| J4 JST-PH 2-pin header, **side entry** (S2B-PH-K-S) | J3 (pin 1, OUT3) and J2 (pin 2, OUT4), mouth toward the right-hand edge. A top-entry plug would stand too tall for the cover. | The PH pins are 2.0 mm apart. Splay them 0.27 mm each to reach the 2.54 mm holes; they go in with a light push. |
| RV1 RK097N 10 kΩ linear pot, shaft up | Pins in J8 (pin 1, GND end), J9 (pin 2, wiper), J10 (pin 3, 3V3 end); the body points off the right-hand edge | The pins are 2.5 mm apart, which goes straight into the 2.54 mm holes. Snip off the two mounting tabs if yours has them: they're wider than the 1 mm holes. The panel nut holds the pot instead. |

With the DRV8833 held component side up, the pin rows read, top to bottom: **IN1, IN2, VCC, GND, IN3, IN4** on the left and **ULT, OUT4, OUT3, OUT2, OUT1, EEP** on the right. That is the label order printed on the back, upside down. **Check it against the labels on your module before soldering**: if yours differs, keep the same nets and move the header pins and links to suit.

**The Super Mini's USB-C port** ends about 3 mm inside the board edge past row 17. The module sits on the header spacers about 2.5 mm above the perfboard, so a plug's overmould clears the perfboard by about 2 mm.

**The knob's direction:** seen from the shaft end with the pins facing you, turning clockwise moves the wiper toward pin 3. With pin 3 on 3V3, clockwise is faster, as on the old pod knob. If yours runs the other way, swap the W20 and W21 links at J8 and J10.

**The pot hangs off the board.** Its pins are at the front of its base, and the 9.5 mm deep base runs from about 2 mm to 11.5 mm right of the pin row. About 6.5 mm of it is past the board edge, and the shaft is about 6.75 mm right of column J. The pins alone don't carry the knob's load: the nut on the cover does.

## Strips used

| Strip | Net | Holes |
|---|---|---|
| 3 left | +5V (DRV VCC) | D3 pin · C3 link from the 5V bus |
| 4 left | GND (DRV GND) | D4 pin · C4 link from the GND bus · A4 link to RV1's GND end |
| 5 left | MOT_IN1 (IN3) | D5 pin · C5 link from GPIO18 |
| 6 left | MOT_IN2 (IN4) | D6 pin · C6 link from GPIO20 |
| 2 right | motor (OUT4) | H2 pin · J2 J4 pin 2 |
| 3 right | motor (OUT3) | H3 pin · J3 J4 pin 1 |
| 6 right | DRV_SLEEP (EEP) | H6 pin · J6 link from GPIO19 |
| **8 left** | **+5V bus** | A8 **W1** from U1 + · B8 link to C3 · C8 link to C11 |
| **9 left** | **GND bus** | A9 **W2** from U1 GND · B9 link to C4 · C9 link to C16 |
| 11 left | +5V to D1 | B11 D1 anode · C11 link from C8 |
| 12 left | GPIO18 | D12 pin · C12 link to C5 |
| 13 left | GPIO19 | D13 pin · C13 link to J6 |
| 14 left | GPIO20 | D14 pin · C14 link to C6 |
| 15 left | +3V3 | D15 pin · A15 link to RV1's 3V3 end |
| 16 left | GND | D16 pin · A16 **W11** to J3 pin 2 · C16 link from C9 |
| 17 left | +5V, Super Mini side | D17 pin · B17 D1 cathode |
| 8 right | GND, RV1 end | J8 RV1 pin 1 · I8 link from A4 |
| 9 right | SPEED, RV1 wiper | J9 RV1 pin 2 · I9 link to I13 |
| 10 right | +3V3, RV1 end | J10 RV1 pin 3 · I10 link from A15 |
| 12 right | DIR_FWD (GPIO3) | H12 pin · I12 **W9** to J3 pin 3 |
| 13 right | SPEED (GPIO2) | H13 pin · I13 link from RV1's wiper |
| 14 right | BTN (GPIO1) | H14 pin · I14 **W6** to J3 pin 6 |
| 15 right | DIR_REV (GPIO0) | H15 pin · I15 **W10** to J3 pin 1 |

Strips 1 left and right, 2 left, 4 and 5 right, 7, 10 left, 11 right and 16–17 right are unused. Keep the corner holes A1, J1, A17 and J17 empty: the M2 screw heads cover their pads.

## Links (underside)

Ten short links of insulated 30 AWG solid wire run on the underside (solder side), each from hole to hole. They take over the jobs of the old loose wires W4, W5, W12, W13 and of the pod-side knob wires, and add the power links.

| Link | Colour | From | To | Job | Cut |
|---|---|---|---|---|---|
| W16 | red | B8 (5V bus) | C3 (DRV VCC) | +5V to the motor driver | 30 mm |
| W17 | red | C8 (5V bus) | C11 (D1 anode) | +5V to D1 | 20 mm |
| W18 | black | B9 (GND bus) | C4 (DRV GND) | GND to the motor driver | 30 mm |
| W4 | black | C9 (GND bus) | C16 (Super Mini GND) | GND to the Super Mini | 35 mm |
| W5 | orange | C12 (GPIO18) | C5 (IN3) | MOT_IN1, forward PWM | 35 mm |
| W12 | brown | C14 (GPIO20) | C6 (IN4) | MOT_IN2, reverse PWM | 35 mm |
| W13 | gray | C13 (GPIO19) | J6 (EEP) | DRV_SLEEP | 55 mm |
| W19 | yellow | I9 (RV1 wiper) | I13 (GPIO2) | SPEED | 25 mm |
| W20 | black | A4 (GND) | I8 (RV1 pin 1) | GND end of the knob | 55 mm |
| W21 | violet | A15 (3V3) | I10 (RV1 pin 3) | 3V3 end of the knob, 3.3 V only | 55 mm |

The cut lengths come from the routes in the 3D model, worked out the same way as every wire in the [cut list](cut-list.csv); trim each one to fit on the board. W20 and W21 cross under both modules; keep them flat against the board.

## Where it goes

The perfboard mounts on the mount's **+X flange ear**, which is enlarged to carry it. The board is parallel to the box wall with its components facing away from the wall, rows running along Y and row 17 (the USB-C end) toward the electronics side (−Y).

- **Ear bosses:** four Ø5 mm bosses, 9 mm tall, at the board's M2 holes. They leave room under the board for the links, and 2.5 mm over the head of the wall bolt through this ear, which goes in before the board.
- **Cover:** `N20_Electronics_Cover`, printed. Its 2 mm top sits **9.0 mm** off the board's component side, the height of RV1's base, so the pot's base bears on the cover from inside. Four 9.0 mm posts stack between the cover and the board, and the walls stop 0.5 mm short of the ear.
- **Screws:** four **M2 × 20** pan-head screws go through the cover posts and the board into Ø1.6 mm pilots in the bosses, about 7 mm deep.
- **Openings:** a 13 mm gap in the cover's −Y wall leaves the Super Mini's USB-C port reachable. A 6 mm notch in the +Y wall lets the motor pair out, and the side facing the mount is open for the power and panel wires.
- **Knob:** RV1's bushing comes through a **Ø7 mm** hole in the cover, and its nut and washer clamp the 2 mm cover top.
- **Shaft:** it stands 10 mm above the pot's base (if the listing measures it from there), so 8 mm clears the cover and about 6 mm clears the nut, for the supplied cap (15.6 × 17 mm) to push on. Leave the washer off if the cap won't seat.

Everything else on the board is under 9 mm tall: the Super Mini with its USB-C port about 6.7 mm, the DRV8833 about 5.1 mm, J4 4.8 mm, D1 2.7 mm. The panel wires run under the modules, between them and the cover.

If you mount the board behind a thicker panel instead, the panel can be at most about 3 mm thick where the nut goes, or the nut won't catch the thread. Counterbore a thicker panel from the inside to 3 mm round the pot, and keep the board's component side 9.0 mm from the panel's inside face.

## Build order

1. Pull the unused pins out of the header strips (see the placement table). Fit the strips to the Super Mini and the DRV8833 first, using the perfboard as the jig so they come out square, and solder them to the modules only.
2. Solder D1 flat along column B, banded end at B17.
3. Solder the ten links on the underside, shortest first: W17, W19, W16, W18, W4, W5, W12, W13, W20, W21.
4. Solder J4 and RV1, and the off-board wires (W1, W2, W6, W9, W10, W11) from the component side.
5. Solder both modules onto the perfboard last, so their bodies don't hide the holes you still need.
6. Before power: check every row of the [terminal check](wiring.md#terminal-check) for continuity. Check that +5V, +3V3 and GND are not shorted to each other or to the strip next door.
