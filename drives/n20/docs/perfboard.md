# Perfboard: N20 drive electronics

From rev C the electronics are soldered onto an **ElectroCookie mini solderable breadboard** (1/4 size, 38.1 × 50.8 mm) in place of the boards wired loose in their pockets. The perfboard carries:

- the USB-C PD trigger board (U1)
- the ESP32-C6 Super Mini (U2)
- the DRV8833 module (U3)
- the diode D1
- C3, a 470 µF hold-up capacitor for the Super Mini
- the speed knob RV1, an RK097N pot that now sits here rather than in the pod
- the motor plug J4.

Only C1 stays in its seat on the mount, wired to the board. The motor plugs into J4, and the button and switch in the pod connect through J3 as before. The board sits on the mount's electronics-side (−Y) face under a printed cover that forms the mount's outer wall, and the knob comes out of the cover sideways (see [Where it goes](#where-it-goes)). The schematic and the full wire list are in [wiring.md](wiring.md).

![Hole map of the perfboard, seen from the component side with row 1 at the top and column A at the left. The DRV8833 straddles the centre channel in rows 1 to 6, with the J4 motor header beside it at J2 and J3, and D1 stands at A3 with its cathode lead in A2. The Super Mini straddles the channel in rows 12 to 17, with its USB-C port past row 17. The PD trigger board U1 lies over columns A to C, rows 9 to 17, its USB-C port also past row 17, with short jumpers from its pads to the 5 V bus (row 7) and the ground bus (row 8). The speed pot RV1 has its pins at J10 to J12, in the strips of 3V3, GPIO4 and GPIO3, and its body overhangs the right-hand edge. C3 lies on its side over columns I and J, rows 5 to 8, with its leads in I7 and I8. Links on the underside join the buses, the driver signals, the pot's 3V3 end and C3.](images/perfboard.svg)

## The board

| | |
|---|---|
| Size | 38.1 × 50.8 mm (1.5 × 2.0 in), 1.6 mm FR4, plated through-holes |
| Holes | 0.1" (2.54 mm) grid, rows 1–17 |
| Strips | Each row is two strips of five joined holes, **A–E** and **F–J**, either side of a 0.3" centre channel. There are no power rails. |
| Mounting | Two Ø3.2 mm (M3) holes in the channel at rows 1 and 17, 40.6 mm apart. Four Ø2.2 mm (M2) corner holes at 31.8 × 44.5 mm. The drive uses three of the **M2** holes; the M3 holes end up under the modules. |

**The underside print is mirrored.** On this board the letters printed on the underside run A–J left to right when you turn it over like a page, so each hole's underside letter is the mirror of its component-side one: A↔J, B↔I, C↔H, D↔G, E↔F, with the same row number. A2 on top is printed J2 underneath. To check your own board, push a wire into top A2, turn the board over, and read the hole it comes out of. This page and the [wiring guide](wiring-steps.md) name holes by the **component side**; the guide gives the printed underside name in brackets wherever you work from underneath.

In this page the board is drawn component side up, row 1 at the top and column A at the left. On the drive, row 1 points to −X and column A toward the box wall, so the hole map's right-hand edge is the board's bottom edge on the drive. Hole positions from the board's centre are u = (column index − 5.5) × 2.54 mm across the columns (A = 0 … E = 4, F = 7 … J = 11) and v = (row − 9) × 2.54 mm along the rows.

## Placement

Both modules straddle the channel with their two pin rows in columns **D** and **H**, 0.6" apart. That leaves holes A, B and C free in each left-hand strip, and I and J in each right-hand strip. Fit header pins **only where this table lists them**, and pull the others out of the strip with pliers. A strip with no module pin in it is then free to use as a bus. This also leaves out the Super Mini's two unused strapping pins (GPIO8, GPIO9).

| Part | Where | Pins |
|---|---|---|
| U3 DRV8833 module, component side up | Rows 1–6 (pins used in rows 2–6), IN row in column D, OUT row in column H | D3 VCC · D4 GND · D5 IN3 · D6 IN4 (a 4-pin strip); H2 OUT4 · H3 OUT3 · H6 EEP (a 5-pin strip with the rows 4 and 5 pins pulled) |
| U2 ESP32-C6 Super Mini, component side up, USB-C past row 17 | Rows 8–17 (pins), bottom-edge pins in column D, top-edge pins in column H | D12 GPIO18 · D13 GPIO19 · D14 GPIO20 · D15 3V3 · D16 GND · D17 5V (a 6-pin strip); H11 GPIO4 · H12 GPIO3 · H13 GPIO2 · H14 GPIO1 · H15 GPIO0 (a 5-pin strip). **Leave H10 (GPIO5) out**: strip 10 right carries 3V3. |
| U1 USB-C PD trigger board, screw terminal removed | Over columns A–C, rows 9–17, USB-C past row 17 beside the Super Mini's, on two **3 mm double-sided foam pads** | None: its + and − pads take W1 and W2 down to A7 and A8. **Go by the + and − (or VBUS and GND) printed on your PD board**: on ours − is the outer pad, toward the column A edge, and + the inner one, toward the channel, as drawn; boards differ. W2 runs straight from − to A8, and W1 from + passes between A8 and B8 to A7, so the two don't cross. With a charger plugged in, the + pad reads +5 V against −: check it before soldering. Desolder its screw terminal first; with it, the board is too tall for the cover. Set its switches to 5 V before you stick it down. |
| D1 1N5817, standing | Anode lead in A3, body upright, banded end at the top; the cathode lead bends over and down into A2 | Its own leads |
| J4 JST-PH 2-pin header, **side entry** (S2B-PH-K-S) | J3 (pin 1, OUT3) and J2 (pin 2, OUT4), mouth toward the right-hand edge (down, on the drive). A top-entry plug would stand too tall for the cover. | The PH pins are 2.0 mm apart. Splay them 0.27 mm each to reach the 2.54 mm holes; they go in with a light push. |
| C3 470 µF electrolytic, lying on its side | Over columns I–J, rows 5–8, axis across the columns: its lead end sits just past column I and its far end about 5 mm past the right-hand edge, inside the cover, between J4 and RV1. Lay it on the board. | **+ lead in I7**, − lead (stripe side) in I8: bend both down 90° about 1 mm from the can. Any can up to 8 mm across and 11.5 mm long fits. Not to be confused with hole C3. |
| RV1 RK097N 10 kΩ linear pot, shaft up | Pins in J10 (pin 1, 3V3 end), J11 (pin 2, wiper, in the GPIO4 strip), J12 (pin 3, low end, in the GPIO3 strip); the body points off the right-hand edge (down, on the drive) | The pins are 2.5 mm apart, which goes straight into the 2.54 mm holes. Snip off the two mounting tabs if yours has them: they're wider than the 1 mm holes. The panel nut holds the pot instead. |

With the DRV8833 held component side up, the pin rows read, top to bottom: **IN1, IN2, VCC, GND, IN3, IN4** in column D and **ULT, OUT4, OUT3, OUT2, OUT1, EEP** in column H. That is the label order printed on the back, upside down. **Check it against the labels on your module before soldering**: if yours differs, keep the same nets and move the header pins and links to suit.

**Both USB-C ports** face the same end, past row 17. The Super Mini's port ends about 3 mm inside the board edge, and U1's about 1.4 mm outside it. Both plugs go in from that end of the drive (+X), through the open end of the cover.

**The knob's direction:** seen from the shaft end with the pins facing you, turning clockwise moves the wiper toward pin 3. Pin 3 is on GPIO3, the low end, so the firmware inverts the reading (`KNOB_REVERSED` in `firmware/main/board.h`) and clockwise is faster, as on the old pod knob. If yours runs the other way, set `KNOB_REVERSED` to 0.

**The pot hangs off the board.** Its pins are at the front of its base, and the 9.5 mm deep base runs from about 2 mm to 11.5 mm past the pin row, off the board's edge. The pins alone don't carry the knob's load: the nut on the cover does.

## Strips used

| Strip | Net | Holes |
|---|---|---|
| 2 left | D1 cathode | A2 D1 cathode lead · B2 link to C17 · C2 link to H7 |
| 3 left | +5V (DRV VCC) | D3 pin · A3 D1 anode · C3 link from the 5V bus |
| 4 left | GND (DRV GND) | D4 pin · A4 **W11** to J3 pin 2 · B4 **L2** from C1 − · C4 link from the GND bus |
| 5 left | MOT_IN1 (IN3) | D5 pin · C5 link from GPIO18 |
| 6 left | MOT_IN2 (IN4) | D6 pin · C6 link from GPIO20 |
| **7 left** | **+5V bus** | A7 **W1** from U1 + · B7 link to C3 · C7 **L1** from C1 + |
| **8 left** | **GND bus** | A8 **W2** from U1 − · B8 link to C4 · C8 link to C16 |
| 12 left | GPIO18 | D12 pin · C12 link to C5 |
| 13 left | GPIO19 | D13 pin · C13 link to J6 |
| 14 left | GPIO20 | D14 pin · C14 link to C6 |
| 15 left | +3V3 | D15 pin · A15 link to RV1's 3V3 end |
| 16 left | GND | D16 pin · C16 link from the GND bus · A16 link to H8 |
| 17 left | +5V, Super Mini side | D17 pin · C17 link from D1's cathode |
| 2 right | motor (OUT4) | H2 pin · J2 J4 pin 2 |
| 3 right | motor (OUT3) | H3 pin · J3 J4 pin 1 |
| 6 right | DRV_SLEEP (EEP) | H6 pin · J6 link from GPIO19 |
| 7 right | +5V, Super Mini side (C3 +) | I7 C3 + lead · H7 link from C2 |
| 8 right | GND (C3 −) | I8 C3 − lead · H8 link from A16. H8 (GPIO7) has no header pin. |
| 10 right | +3V3, RV1 end | J10 RV1 pin 1 · I10 link from A15. H10 (GPIO5) stays out. |
| 11 right | SPEED (GPIO4) | H11 pin · J11 RV1 pin 2 (wiper) |
| 12 right | KNOB_LO (GPIO3, driven low) | H12 pin · J12 RV1 pin 3 |
| 13 right | DIR_FWD (GPIO2) | H13 pin · I13 **W9** to J3 pin 3 |
| 14 right | BTN (GPIO1) | H14 pin · I14 **W6** to J3 pin 6 |
| 15 right | DIR_REV (GPIO0) | H15 pin · I15 **W10** to J3 pin 1 |

Under U1, rows 9–17 in columns A–C are used only from the underside: the link ends at A15, A16, C12–C17 and nothing on top. Strips 1 left and right, 4, 5 and 9 right, 9–11 left and 16–17 right are unused. Keep the corner holes A1, J1, A17 and J17 empty: the M2 screw heads and supports cover their pads.

**Wires from the mount go in from the underside.** The board faces away from the mount, so L1, L2 and the panel wires W6, W9, W10 and W11 come from behind it. Push each one through its hole from the underside and solder it on the component side. Only W1, W2 and the parts are soldered from the component side in the usual way.

## Links (underside)

Ten links of 30 AWG stranded wire, ends tinned, run on the underside (solder side), each from hole to hole. They take over the jobs of the old loose wires W4, W5, W12 and W13 and of the pod-side knob wires, and add the power links.

| Link | Colour | From | To | Job | Cut |
|---|---|---|---|---|---|
| W16 | red | B7 (5V bus) | C3 (DRV VCC) | +5V to the motor driver and D1 | 25 mm |
| W18 | black | B8 (GND bus) | C4 (DRV GND) | GND to the motor driver | 25 mm |
| W4 | black | C8 (GND bus) | C16 (Super Mini GND) | GND to the Super Mini | 40 mm |
| W17 | red | B2 (D1 cathode) | C17 (Super Mini 5V) | +5V to the Super Mini, through D1 | 60 mm |
| W5 | green | C12 (GPIO18) | C5 (IN3) | MOT_IN1, forward PWM | 35 mm |
| W12 | blue | C14 (GPIO20) | C6 (IN4) | MOT_IN2, reverse PWM | 35 mm |
| W13 | white | C13 (GPIO19) | J6 (EEP) | DRV_SLEEP | 55 mm |
| W21 | yellow | A15 (3V3) | I10 (RV1 pin 1) | 3V3 end of the knob, 3.3 V only | 55 mm |
| W22 | red | C2 (D1 cathode) | H7 (C3 +) | +5V, Super Mini side, to the hold-up capacitor | 50 mm |
| W23 | black | A16 (GND) | H8 (C3 −) | GND to the hold-up capacitor | 65 mm |

The cut lengths come from the routes in the 3D model, worked out the same way as every wire in the [cut list](cut-list.csv); trim each one to fit on the board. W17, W21, W22 and W23 cross other links. Keep every link flat against the board: stranded wire springs up more than solid.

**Laying a link:** solder one end, press the wire flat along its lane and pinch the corners square with tweezers, hold the corners down with Kapton tape, then solder the far end. Don't anchor a wire through a spare hole: every hole is part of a strip, so it would join that strip's net. Once the continuity check passes, a dab of hot glue can hold the link for good. W21 is yellow, not red, because red means 5 V on this board. The knob needs no wiper or ground link: its pins sit in the GPIO4 and GPIO3 strips.

## Where it goes

The perfboard mounts on the mount's **electronics-side (−Y) face**, parallel to it, components facing out. Its rows run along X, with row 17 (the USB-C end) toward +X, and column A toward the box wall. The pot's body hangs off the bottom edge, away from the wall.

- **Supports:** the board's underside sits 5.8 mm off the mount's face, clear of C1 and the old PD board's pad.
  - **Top corners:** two 5 × 5.8 mm posts that rise from the flange plane.
  - **Bottom corners:** two bosses with 45° gussets above them, so the mount still prints flange down without supports there.
  - **+X side:** the face ends at x 22.4, so a 3.3 mm deep rail joined to the flange and the base carries those supports.
  - The supports have Ø1.6 mm M2 pilots, 9 mm deep.
- **Cover:** `N20_Electronics_Cover`, printed, becomes the mount's outer −Y wall.
  - Its 2 mm top sits **9.0 mm** off the board's component side, the height of RV1's base, so the pot's base bears on the cover from inside.
  - Three 9.0 mm posts stack between the cover and the board. The fourth corner is under U1, so it has neither a post nor a screw.
  - Its walls stop at the board's underside plane. The side toward the box wall is open.
- **Screws:** three **M2 × 20** pan-head screws go through the cover posts and the board into the support pilots, about 7 mm deep. The fourth corner (A17, under U1) rests on its support without a screw.
- **Openings:**
  - The +X end is open over both USB-C ports, so both plugs and their overmoulds clear the cover.
  - A 7 mm notch in the bottom wall lets the motor pair out of J4.
- **Knob:** RV1's bushing comes through a **Ø7 mm** hole in the cover, and its nut and washer clamp the 2 mm cover top.
- **Shaft:** it stands 10 mm above the pot's base (if the listing measures it from there), so 8 mm clears the cover and about 6 mm clears the nut, for the supplied cap (15.6 × 17 mm) to push on. Leave the washer off if the cap won't seat.

Everything else on the board is under 9 mm tall:

- U1 with its USB-C port, on 3 mm pads: about 7.8 mm
- the Super Mini with its USB-C port: about 6.7 mm
- D1 standing: about 7 mm with its lead
- the DRV8833: about 5.1 mm
- J4: 4.8 mm
- C3 lying on its side: 8.1 mm for an 8 mm can.

## Build order

For one wire at a time, with a picture and a continuity check for each, follow the [step-by-step wiring guide](wiring-steps.md).

1. Desolder U1's screw terminal and set its switches to 5 V. Pull the unused pins out of the header strips (see the placement table). Fit the strips to the Super Mini and the DRV8833, using the perfboard as the jig so they come out square, and solder them to the modules only.
2. Stand D1 in A3 and A2, banded end up.
3. Solder the ten links on the underside, shortest first: W16, W18, W5, W12, W4, W22, W21, W13, W17, W23.
4. Solder J4, RV1 and C3. Lay C3 flat on the board with its + lead in I7 and its stripe toward I8.
5. Solder both modules onto the perfboard.
6. Stick U1 down on its two foam pads, USB-C past row 17 beside the Super Mini's, then solder W1 and W2 from its pads to A7 and A8.
7. On the drive, with C1 already in its seat: push L1, L2, W6, W9, W10 and W11 through their holes from the underside, and solder them on the component side.
8. Before power: check every row of the [terminal check](wiring.md#terminal-check) for continuity. Check that +5V, +3V3 and GND are not shorted to each other or to the strip next door.
