# Wiring: N20 drive

A USB-C PD trigger board set to 5 V powers a DRV8833 motor driver, which runs the N20 gearmotor. The same 5 V feeds an ESP32-C6 Super Mini through a diode. The Super Mini drives the DRV8833's channel B inputs from GPIO18 and GPIO20, and holds the driver asleep from GPIO19 until the firmware is running. The motor's Hall encoder runs on the Super Mini's 3.3 V and its two channels go to GPIO0 and GPIO2. All grounds are tied together.

**Encoder motor (2026-11-08):** this page is for the MECCANIXITY 6 V 1:150 N20 with a magnetic Hall encoder (see [the drive's README](../README.md#encoder-motor-2026-11-08)). Its 6-wire cable plugs into the encoder board, and that plug is now **J4**; the cable's other end is soldered into the perfboard. The 2-pin header at J2/J3, the motor's factory leads W14 and W15, and the wire clamp aren't used. For the plain N20 without an encoder, see this page in the git history (commit beb5a56).

From rev C all of this is on one ElectroCookie perfboard: the PD board, the Super Mini, the DRV8833, the diode, the capacitors, the speed knob and the motor cable's six wires. The board is on the mount's electronics-side (−Y) face, under a printed cover that forms the mount's outer wall. Its hole map and build order are in [perfboard.md](perfboard.md).

- C1 stands on the board, its top through a 9 mm hole in the cover. Its seat in the mount and the wires L1 and L2 are gone.
- The motor unplugs at J4, the 6-pin socket on its encoder board.
- The button stays in its mount in the pod, and its two wires (W6, W11) run straight to the perfboard. There's no panel connector any more, so **the pod no longer unplugs** from the drive.
- There's no direction switch (it never arrived): the motor runs forward.
- Both USB-C ports, the PD board's for power and the Super Mini's for programming, are at the −X end of the drive.

## Schematic

[Open the schematic full size](images/schematic.svg) (SVG, rev C.6).

![Schematic, rev C.6. PD board 5 V goes to the DRV8833 VCC pin and, through diode D1, to the Super Mini's 5V pin; a 470 µF capacitor C1 sits across 5 V, and a second 470 µF capacitor C3 sits from D1's cathode to ground. Super Mini GPIO18, GPIO20 and GPIO19 go to DRV8833 IN3, IN4 and EEP. DRV8833 OUT3 and OUT4 go, with a 0.1 µF capacitor C2 across them on the perfboard, through the 6-pin plug J4 to the N20 motor M1's leads M1 and M2. The same plug carries the Hall encoder ENC1: its VCC from the Super Mini's 3.3 V, its GND, channel A (C1) to GPIO0 and channel B (C2) to GPIO2. The 12 mm button S1 runs from GPIO1 to ground. The 10 kΩ knob RV1 sits between 3.3 V and GPIO3, which the firmware drives low, with its wiper on GPIO4. U1, U2, U3, D1, C1, C2 and C3 are outlined as the perfboard.](images/schematic.svg)

## Connections

| From | To | Net |
|---|---|---|
| U1 + pad | perfboard 5V bus (row 7 left) | +5V |
| C1 + | 5V bus (C7) | +5V |
| 5V bus | U3 VCC; D1 anode | +5V |
| D1 cathode | U2 5V pin; C3 + | +5V, Super Mini side |
| C3 − | GND | GND |
| U1 − pad, C1 − | perfboard GND bus (row 8 left), DRV GND strip | GND |
| GND bus | U3 GND, U2 GND | GND |
| U2 GPIO18 | U3 IN3 | MOT_IN1 (forward) |
| U2 GPIO20 | U3 IN4 | MOT_IN2 (reverse) |
| U2 GPIO19 | U3 EEP (nSLEEP) | DRV_SLEEP |
| U3 OUT3, OUT4 | C2 across them; J4 pins 1 (M1) and 6 (M2) → motor (W24, W25) | motor |
| U2 3V3 | RV1 pin 1; J4 pin 5 (VCC) → encoder (W31, W27) | +3V3 |
| GND (U3 GND strip) | J4 pin 2 (GND) → encoder (W30, W26) | GND |
| U2 GPIO0 | J4 pin 3 (C1) → encoder channel A (W28) | ENC_A |
| U2 GPIO2 | J4 pin 4 (C2) → encoder channel B (W29) | ENC_B |
| U2 GPIO3, driven low by the firmware | RV1 pin 3 | KNOB_LO, the knob's ground |
| RV1 wiper | U2 GPIO4 | SPEED |
| U2 GPIO1, GND | S1, the button in the pod (W6, W11) | BTN, GND |

## Parts

| Ref | Part | Note |
|---|---|---|
| U1 | USB-C PD trigger board, set to 5 V | On the perfboard over columns A–C, on two 3 mm foam pads, USB-C at the −X end. **Desolder its screw terminal**: its pads take W1 and W2 instead, and with the terminal it's too tall for the cover. |
| U2 | ESP32-C6 Super Mini, 26 × 18 mm | On the perfboard, rows 12–17 (pins), USB-C at the −X end beside U1's. Build the firmware for the Super Mini (see [Board](../../../firmware/README.md#board)). |
| U3 | DRV8833 module, 12-pin, 18.5 × 16 mm | On the perfboard, rows 1–6, component side out. Leave the **J1** jumper open. Channel B (IN3/IN4 → OUT3/OUT4) is used. |
| M1 | N20 gearmotor with magnetic Hall encoder, 6 V, 1:150 (MECCANIXITY, Amazon B0F8NGVNCH) | In the printed cartridge under the base, encoder pointing away from the base, its plug toward −Y. 7 pulses per motor turn per channel, so 1050 per output turn, 4200 counting every edge. |
| ENC1 | The encoder board on M1 | Part of the motor. Powered at **3.3 V**: its outputs C1 and C2 seem to be pulled up to its VCC, and the ESP32-C6's inputs aren't 5 V tolerant. |
| J4 | 6-pin socket on the encoder board, and the motor's supplied 6-wire cable | The motor unplugs here. Pins, left to right with the pins at the bottom and the components facing you: 1 M1 (white), 2 GND (blue), 3 C1 (green), 4 C2 (yellow), 5 VCC (black), 6 M2 (red). The cable's far end is cut to length and soldered into the perfboard (W24–W29). |
| C2 | Ceramic capacitor, 0.1 µF (104), 2.54 mm leg spacing | On the perfboard, standing in I3 and I2, across OUT3 and OUT4. The encoder board covers the motor's terminals. |
| D1 | Schottky diode, 1 A, axial (1N5817) | Standing on the perfboard: anode in A3, band up, cathode lead bent down into A2 |
| C3 | Electrolytic, 470 µF, ≥ 6.3 V (10 V preferred), at most **8 mm across and 11.5 mm long** | The Super Mini's hold-up capacitor. Lying on its side on the perfboard over columns I–J, rows 5–8, its end past the board's edge inside the cover; + lead in I7, − lead (stripe side) in I8. See [placement](perfboard.md#placement). Not to be confused with hole C3 on the board. |
| C1 | Electrolytic, 470 µF, ≥ 10 V, **8 × 13 mm** | Standing on the perfboard over columns A–C, rows 4–7, between D1 and W1/W2, 1.5 mm off the board; its top comes through a 9 mm hole in the cover. + lead (long leg) to C7, − lead (stripe side) to B4, both **sleeved** with heat-shrink because they cross the IN3/IN4 pads. See [placement](perfboard.md#placement). |
| RV1 | RK097N 10 kΩ linear pot, 6 mm knurled shaft | On the perfboard, standing on its pins in J10–J12 (wiper and low end straight into the GPIO4 and GPIO3 strips), shaft parallel to the board and out through the Ø7.2 mm hole in the cover's J-edge wall, where its nut clamps; supplied knob cap. See [placement](perfboard.md#placement). |
| S1 | 12 mm push button, pre-wired leads | In its mount in the pod, as on the MG90S drive; wired straight to the perfboard by W6 and W11 |

The MG90S drive's R1 is not used. The pod's knob hole is left empty: RV1 moved to the perfboard.

**Building with a XIAO ESP32-C6 instead:** rev B wiring is in the git history (commit 3efb5c3, `drives/n20/docs/wiring.md`). It needs the rev B mount (Fusion version 22): the rev C mount still has the XIAO pocket, but the perfboard's supports block the XIAO's USB-C port.

## The DRV8833 module's pins

The module has two rows of six pins. Seen from the back, where the labels are printed:

| One row | Other row |
|---|---|
| **IN4** · **IN3** · **GND** · **VCC** · IN2 · IN1 | **EEP** · OUT1 · OUT2 · **OUT3** · **OUT4** · ULT |

Bold pins are used. On the perfboard, component side up, these rows land in columns D and H, read the other way round: [perfboard.md](perfboard.md#placement) has the hole for each pin. Check against the labels on your module before soldering.

![The electronics side of the drive with the cover off: the perfboard stands on its supports on the mount's -Y face. The PD board lies over the top-left corner of the perfboard with short red and black jumpers down to the buses, and its USB-C port and the Super Mini's both face left. The DRV8833 is at the right, with D1 and the capacitor C1 standing above it and the tan disc of C2 standing beside it. The speed pot RV1 and the hold-up capacitor C3 stand along the bottom edge. The motor cable's six wires come up from the encoder below into the board's right-hand edge: red, white, blue and black go straight into the edge holes, and the green and yellow pair loop over the DRV8833 and the Super Mini to holes beside the Super Mini.](images/perfboard.png)

![The whole drive from the electronics side with the cover on: the gray printed cover forms the mount's outer wall, with the knob's cap and the top of C1 standing out of it. The motor cable's six coloured wires come out of the notch in the cover's bottom wall and run down to the plug on the encoder at the back of the motor, which hangs in its cartridge below the base.](images/perfboard_cover.png)

![Straight-on view of the electronics side with the cover off and the perfboard see-through. C1 stands near the top-right corner, its sleeved leads bent out to the board. The wires behind the board are visible: the button's two wires coming up from the base tunnel, and the underside links. The PD board's jumpers sit on the front. At the bottom right, C2 stands at the board's edge and the motor cable's six wires rise from below into the edge holes, the green and yellow pair arching left over the modules.](images/wiring_electronics.png)

## The button (S1)

The controls are the knob RV1, on the perfboard, and the 12 mm button S1 in its mount in the pod. The direction switch SW1 never arrived, so it, the 6-pin panel connector J3 and its holder in the pod are gone (Fusion version 30). The motor runs forward.

Two wires run straight from the button's leads to the perfboard, with no connector: **W6** (white) from lead A to I14 (GPIO1), and **W11** (black) from lead B to A4 (GND). Each goes back through the pod where J3 used to be, along the base tunnel to the mount's −Y face, behind the perfboard, and up through its hole from the underside, to be soldered on the component side.

Use the button's own pre-wired leads if they reach: W6 and W11 each need **185 mm**, measured from the button. If they're shorter, cut them to about 30 mm and splice 30 AWG on inside the pod (white to one lead, black to the other: the leads are interchangeable), with the two joints staggered and each under heat-shrink.

The pod no longer unplugs from the drive. To take it off, unsolder W6 and W11 at the board (or cut them and splice them back).

## J4: the motor cable

The encoder motor comes with a 6-wire cable that plugs into the 6-pin socket on its encoder board. That plug is **J4**: the motor and its cartridge come off for transit or repair by unplugging it, with no soldering at the motor. The encoder board covers the motor's terminals, so there are no factory leads to protect and the cartridge's wire clamp isn't used. C2 has moved to the perfboard (I2/I3).

The cable's far end is cut to length and soldered into the perfboard, six wires in six strips. They keep the cable's own colours, which don't follow this board's: **black is the encoder's 3.3 V supply, blue is its ground, and red is a motor lead**.

| J4 pin | Signal | Wire | Hole | Strip, and how it reaches its net |
|---|---|---|---|---|
| 1 | M1 | W24, white | J3 | 3 right: OUT3 (H3) |
| 6 | M2 | W25, red | J2 | 2 right: OUT4 (H2) |
| 2 | GND | W26, blue | J4 | 4 right, joined to the DRV8833's GND strip (4 left) by the link W30 (I4 → E4) |
| 5 | VCC, 3.3 V | W27, black | J5 | 5 right, joined to 3V3 (strip 10 right, the knob's) by the link W31 (I5 → H10) |
| 3 | C1, channel A | W28, green | I15 | 15 right: GPIO0 (H15) |
| 4 | C2, channel B | W29, yellow | I13 | 13 right: GPIO2 (H13) |

The hole J4 and the plug J4 share a name only by chance.

- **Why these holes:** no six neighbouring rows on the right-hand side are free (strips 6 to 8 right carry EEP and C3, 10 to 12 the knob, 14 the button), so a 6-pin header has nowhere to go. Only strips 4 and 5 right are free beside the notch, and no 3V3 or GND hole is free on the component side, so the encoder's supply comes in there and two short links carry it on. The signals land straight in the GPIO0 and GPIO2 strips, which have free holes beside the Super Mini.
- **Route:** from the plug at the encoder's −Y end, under the base's −Y edge and in through the notch in the cover's bottom wall, as the motor pair ran before. W24–W27 go straight into J2–J5 along the board's edge. W28 and W29 run on over the DRV8833 and the Super Mini, under the cover (it sits 9 mm above the board; the modules stand about 5 mm), and down into I15 and I13 beside the Super Mini, clear of C3 and RV1.
- **Soldering:** push each wire in from the component side and solder it underneath.
- **Strain relief:** the cable's plug latches into the encoder's socket. Leave a little slack at the plug, and if the cable pulls, tie it to the cartridge's arms with their zip tie.
- **In the model from Fusion version 39** (`cad/fusion/wires.py`), with the cable's wires taken as 0.9 mm over the insulation (unconfirmed). At the notch they lie side by side at 1.04 mm pitch, in the order they land (W25, W24, W28, W29, W26, W27 from the +X end), with 0.1–0.4 mm to spare in its 7 mm. Between the plug and the base's edge they cross over to get into that order. W28 and W29 run 7.4 mm above the board, over C2 and the Super Mini's USB-C, and 1 mm under the cover. If your cable's wires are thicker than 0.9 mm, check the notch.

![The underside of the mount, from below: the encoder motor in its cartridge on the dowel axis, with the black zip tie round its arms and the encoder's plug on the board at its back. The motor cable's six coloured wires leave the plug, run across under the base and up into the notch at the cover's bottom wall, where they spread out to the perfboard.](images/wiring_under_base.png)

## Point-to-point wire list

Every wire is **30 AWG stranded**, in six colours: red for +5V, black for GND, and green, blue, white and yellow for signals (yellow is also 3.3 V: W21 and W31).

The exception is the motor cable, **W24–W29**: the six wires of the cable that came with the motor, in its own colours, gauge unconfirmed. Here **black is the encoder's 3.3 V supply, blue is ground and red is a motor lead**: the cable runs only from the encoder's plug (J4) to its own six holes, so it can't be confused with the board's own wiring. Which motor lead goes to J3 and which to J2 only sets the spin direction.

- **Tin the stripped ends** before pushing them through the perfboard's holes, so the strands don't splay or break off.
- **Keep the underside links flat** against the board, and press each one down along its lane before soldering the next.
- **Current is fine.** 30 AWG is about 0.34 Ω per metre: about 30 mV drop on a 100 mm run at 1 A. The wires that carry motor current are W24 and W25 (the cable's motor leads), and on the 5 V and ground paths W1, W2, W16 and W18; W17 carries only the Super Mini's own current. Check your N20's stall current on its datasheet. The DRV8833 limits a channel to about 1.5 A, and 30 AWG handles a 1 A stall for the moment it lasts. If your motor stalls above 1 A, double up W1, W2, W16 and W18. The encoder draws a few mA through W27, W31 and W21.

Follow this table one wire at a time and tick each off. The [step-by-step wiring guide](wiring-steps.md) takes the perfboard one wire at a time, with a picture and a check for each. The colours match the 3D model. The links on the perfboard's underside are listed in [perfboard.md](perfboard.md#links-underside), and their cut lengths are in the cut list below. The button's wires (W6, W11) come from the mount and go into the perfboard **from the underside** and are soldered on the component side.

| Wire | Colour | From | To | Notes |
|---|---|---|---|---|
| **Power** | | | | |
| W1 | red | U1 + pad | perfboard A7 (5V bus) | Short jumper on the component side. On our PD board + is the inner pad, toward the channel; go by the marking on yours. Passes between A8 and B8, clear of W2. |
| W2 | black | U1 − pad | perfboard A8 (GND bus) | Short jumper on the component side. On our PD board − is the outer pad, toward the column A edge. |
| **The motor cable: J4 at the encoder → cover notch → perfboard (from the front)** | | | | |
| W24 | white (cable) | J4 pin 1, M1 | perfboard J3 (OUT3) | Out of the plug, under the base's −Y edge, in through the cover's bottom notch. If the motor runs backwards, swap W24 and W25 at J3/J2. |
| W25 | red (cable) | J4 pin 6, M2 | perfboard J2 (OUT4) | Beside W24. A motor lead, not +5 V. |
| W26 | blue (cable) | J4 pin 2, GND | perfboard J4 (strip 4 right) | The encoder's ground; W30 takes it on to GND. |
| W27 | black (cable) | J4 pin 5, VCC | perfboard J5 (strip 5 right) | The encoder's supply, **3.3 V only**; W31 takes it on to 3V3. |
| W28 | green (cable) | J4 pin 3, C1 | perfboard I15 (GPIO0) | Channel A. From the notch over the DRV8833 and the Super Mini, under the cover, down beside the Super Mini. |
| W29 | yellow (cable) | J4 pin 4, C2 | perfboard I13 (GPIO2) | Channel B, beside W28. |
| C2 | — | perfboard I3 (OUT3) | perfboard I2 (OUT4) | 100 nF, standing on its legs. |
| **The button: S1 in the pod → base tunnel → −Y face → perfboard (from behind)** | | | | |
| W6 | white | S1 lead A | perfboard I14 (GPIO1) | The button's own lead if it reaches, otherwise spliced in the pod. |
| W11 | black | S1 lead B | perfboard A4 (GND) | Beside W6. The button's leads are interchangeable. |

Gone in rev C:

- **W7, W8, W7p, W8p and J2:** the knob is on the perfboard. Its wiper and low end go straight into the GPIO4 and GPIO3 strips, and the link W21 brings 3V3 to its other end.
- **W19 and W20:** the old wiper and ground links, from before the knob moved to J10–J12.
- **The loose W4, W5, W12 and W13:** they are underside links now.
- **The long runs of W1 and W2:** the PD board is on the perfboard.
- **D1's long leads:** it stands on the board.

Gone with the encoder motor (2026-11-08):

- **W14 and W15**, the plain N20's factory leads, and the **2-pin JST-PH header** at J2/J3: the cable W24–W29 replaces them, and J4 is now the plug at the encoder.
- **The wire clamp** on the cartridge's +Y arm: nothing is soldered at the motor any more. It's still in the printed cartridge (Fusion version 38).

### Terminal check

When you're done, every terminal should hold exactly this many wires.

| Terminal | Wires landing here | Count |
|---|---|---|
| U1 + pad / − pad | W1 / W2 | 1 each |
| Perfboard strip 7 left (5V bus) | W1 · W16 · C1 + lead | 3 |
| Perfboard strip 8 left (GND bus) | W2 · W18 · W4 | 3 |
| Strip 2 left (D1 cathode) | D1 cathode lead · W17 · W22 | 3 |
| Strip 3 left (U3 VCC) | D1 anode lead · W16 | 2 |
| Strip 4 left (U3 GND) | W11 · C1 − lead · W18 · W30 | 4 |
| Strip 5 left (IN3) / 6 left (IN4) / 6 right (EEP) | W5 / W12 / W13 | 1 each |
| Strips 12, 13, 14 left (GPIO18, 19, 20) | W5, W13, W12 | 1 each |
| Strip 15 left (3V3) / 16 left (GND) / 17 left (5V) | W21 / W4 · W23 / W17 | 1 / 2 / 1 |
| Strips 7, 8 right (C3 + / −) | W22 · C3 + lead / W23 · C3 − lead | 2 each |
| Strip 10 right (3V3: RV1 and the encoder) | W21 · RV1 pin 1 · W31 | 3 |
| Strips 11, 12 right (GPIO4, GPIO3) | RV1 pin 2 (wiper), RV1 pin 3 | 1 each |
| Strips 13, 14, 15 right (GPIO2, GPIO1, GPIO0) | W29 / W6 / W28 | 1 each |
| Strips 2, 3 right (OUT4, OUT3) | W25 · C2 leg / W24 · C2 leg | 2 each |
| Strips 4, 5 right (encoder GND, 3V3) | W26 · W30 / W27 · W31 | 2 each |
| S1 leads A / B | W6 / W11 (or their splices) | 1 each |
| J4, the encoder's socket | the motor cable's plug | 6 pins |

## Cut list

Lengths are measured along the routes in the 3D model, plus 15% slack and 5 mm per end for stripping, rounded up to 5 mm. W6 and W11 are measured all the way from the button, through the pod and the base tunnel; W24–W29 from the back of the encoder plug. Also in [`cut-list.csv`](cut-list.csv).

| Wire | Colour | Gauge | Routed | **Cut** |
|---|---|---|---|---|
| W1 | red | 30 AWG stranded | 10 mm | **25** |
| W2 | black | 30 AWG stranded | 8 mm | **20** |
| W24 | white | motor cable wire (gauge unconfirmed) | 49 mm | **70**, from the plug |
| W25 | red | motor cable wire | 58 mm | **80**, from the plug |
| W26 | blue | motor cable wire | 47 mm | **65**, from the plug |
| W27 | black | motor cable wire | 51 mm | **70**, from the plug |
| W28 | green | motor cable wire | 105 mm | **135**, from the plug |
| W29 | yellow | motor cable wire | 96 mm | **125**, from the plug |
| W6 | white | 30 AWG stranded, or the button's own lead | 149 mm | **185**, from the button |
| W11 | black | 30 AWG stranded, or the button's own lead | 150 mm | **185**, from the button |
| W4 | black | 30 AWG stranded | 22 mm | **40** |
| W5 | green | 30 AWG stranded | 18 mm | **35** |
| W12 | blue | 30 AWG stranded | 21 mm | **35** |
| W13 | white | 30 AWG stranded | 39 mm | **55** |
| W16 | red | 30 AWG stranded | 11 mm | **25** |
| W17 | red | 30 AWG stranded | 39 mm | **60** |
| W18 | black | 30 AWG stranded | 11 mm | **25** |
| W21 | yellow | 30 AWG stranded | 37 mm | **55** |
| W22 | red | 30 AWG stranded | 35 mm | **50** |
| W23 | black | 30 AWG stranded | 46 mm | **65** |
| W30 | black | 30 AWG stranded | 17 mm | **30** |
| W31 | yellow | 30 AWG stranded | 14 mm | **30** |

## Why it's wired this way

- The DRV8833 takes its motor supply straight from the PD board, through the perfboard's 5V bus, so motor current doesn't pass through the Super Mini. Its VCC pin is the chip's VM; the module makes its own logic supply.
- The Super Mini's 3.3 V outputs drive the DRV8833 inputs directly: they accept logic levels from about 2 V up.
- EEP is the chip's sleep input. Held low by GPIO19 until the firmware has started, the motor can't twitch at power-up. The module's J1 jumper would tie EEP high permanently, which is why it stays open.
- C2 across the motor's leads absorbs the brush noise, which otherwise couples into the knob reading, the encoder inputs and the Wi-Fi. It's on the perfboard because the encoder board covers the motor's terminals; it's a little less effective there, at the far end of the cable.
- **C3** holds the Super Mini up through a motor start or stall. When the N20 starts or stalls, the 5 V bus sags. C1 sits on the bus and mostly feeds the motor. C3 sits after D1, so D1 stops it feeding the bus, and it can only discharge into the Super Mini. At about 300 mA (Wi-Fi transmitting), 470 µF covers a dip of about 1.7 ms before the Super Mini's 3.3 V regulator drops out; at 100 mA, about 5 ms.
- **Inrush at plug-in:** C1 and C3 together are 940 µF, all charged through the PD board when the USB-C cable goes in, and C3 also charges from the Super Mini's own USB port when it's plugged into a computer. The 1N5817 takes that surge comfortably. The USB specification allows only 10 µF on a device's VBUS at attach, so 940 µF is well over it. This PD trigger board hasn't been tested with that much capacitance, and neither has any particular charger. Most chargers ride through a few hundred µF, but a charger with tight overcurrent protection may cut out and retry. **Test it on the bench**: plug the drive in several times with the charger you'll use. If the charger cycles or the PD board fails to reach 5 V, use 220 µF for C3 (or C1).
- D1, C1 and the single common ground do the same jobs as on the MG90S drive: see [why it's wired this way](../../mg90s/docs/wiring.md#why-its-wired-this-way) there. D1 also stops the Super Mini's USB port from back-feeding the motor supply when it's plugged into a computer.
- GPIO18, GPIO20 and GPIO19 are the same GPIOs the XIAO build uses, so the motor code doesn't change between boards. The panel pins avoid USB (GPIO12, 13) and UART0 (GPIO16, 17).
- The knob sits with its pins straight in the strips of GPIO4 (wiper) and GPIO3 (low end), next to each other on the Super Mini's edge. The firmware drives GPIO3 low before it reads the knob, so GPIO3 is the knob's ground. Its other end gets 3V3 through W21.
- GPIO4 is an ADC1 pin and a strapping pin (MTMS). That's safe here: on the ESP32-C6 it only selects the JTAG source at reset, and only if the JTAG_SEL_ENABLE eFuse is burned, which it isn't by default. Otherwise its level at reset is ignored, so the knob's position at power-up doesn't matter.
- The pot's clockwise end is on GPIO3, so the firmware inverts the reading (`KNOB_REVERSED` in `firmware/main/board.h`): clockwise is still faster, and the web app's virtual knob still reads 0–100.
- The motor unplugs at the encoder's own socket (J4), so nothing is soldered at the motor. The old J4 header and wire clamp were there because the solder joints on the motor's tabs got stressed in transit when the wires could pull on them; the encoder board now carries those joints.
- **The encoder runs on 3.3 V.** Its outputs seem to be pulled up to its VCC on the encoder board, and the ESP32-C6's inputs aren't 5 V tolerant. A Hall encoder this small draws a few mA, well within the Super Mini's 3.3 V regulator. The firmware turns on the C6's internal pull-ups on GPIO0 and GPIO2 too, in case the board's pull-ups are weak or missing.
- **GPIO0 and GPIO2** were the direction switch's, which was dropped. Neither is a strapping pin on the ESP32-C6. GPIO0 is also the 32 kHz crystal's XTAL_32K_P pin, unused on the Super Mini. The C6's pulse counter (PCNT) counts both channels in hardware, so no edges are lost to Wi-Fi or the web server: at full speed each channel runs at about 1.75 kHz.
- **No direction switch.** The Super Mini build of the firmware has no direction-switch input (`BOARD_HAS_DIR_SWITCH` 0 in `firmware/main/board.h`): the motor always runs forward, and the web app hides the "Direction switch is wired" setting.

## Bring-up

1. Set the PD board to 5 V and measure its + to − pads with a meter before wiring anything.
2. Before applying power, check each row of the terminal check table for continuity, that C3's stripe (−) is in I8, and that C1's stripe (−) faces B4. Confirm that +5V, +3V3 and GND aren't connected to each other, and that J4 and J5 (the encoder's GND and 3V3) aren't connected to the DRV8833's OUT pins: H4 and H5 must have no pins.
3. Flash the N20 build of the firmware for the Super Mini (see [Board](../../../firmware/README.md#board)). The XIAO build drives the wrong pins on a Super Mini, and the MG90S build leaves EEP low, so the motor never runs.
4. With the motor unplugged at J4, power the board and measure J5 to J4: **3.3 V**. Anything near 5 V means W27 or W31 is on the wrong strip: don't plug the motor in until it's fixed.
5. Plug the cable into the motor (J4). On the serial console, `encoder watch` and turn the cam shaft slowly by hand: the count should change by about 4200 per turn, both ways. If it doesn't move, check W28 and W29 and the encoder's 3.3 V; if it moves only by ±1 back and forth, one channel is missing.
6. Turn the knob up slowly. If the motor turns the wrong way, swap W24 and W25 at J3 and J2. If the count runs backwards while the motor runs forward, set `ENC_REVERSED` to 1 in `firmware/main/board.h` and reflash (or swap W28 and W29). If the knob works backwards, set `KNOB_REVERSED` to 0 and reflash: the pot's pins are soldered straight into the strips.
7. Check the controls: the button should start and pause a story, and the knob should change speed. The web app's status shows the shaft's rpm and turns. There's no direction setting to check: the Super Mini build always runs forward.
