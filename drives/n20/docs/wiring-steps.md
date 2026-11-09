# Step-by-step wiring: N20 perfboard (rev C)

One wire, link or part per step, in the [build order](perfboard.md#build-order). Each picture shows everything already done in grey and the current step in its real colour, with its end holes circled and named. The [hole map](images/perfboard.svg) and the [wire list](wiring.md#point-to-point-wire-list) show the whole board at once; this page is the same board taken one step at a time. The wire IDs, colours and cut lengths come from the [cut list](cut-list.csv).

**Reading the pictures**

- Columns **A–E** and **F–J**, rows **1–17**. Every hole in a row's half is joined to the other four in that half: a **strip**. The strips that the current step joins are drawn yellow.
- **Component side** pictures show the board as in the hole map, column A on the left.
- **Hole names are the component side's.** This board's underside letters are **mirrored**: the hole called A2 here is printed **J2** underneath (A↔J, B↔I, C↔H, D↔G, E↔F; the row numbers are the same). Wherever you work from the underside, go by the printed underside name given in brackets.
- **Underside** pictures show the board as you see it turned over, labelled with the letters printed on the underside, so they read A–J left to right just like the board in your hand. Each circled end shows its printed name large and its component-side name small.
- Checks use the component-side names: probe from the component side, or use the printed names in brackets.
- A **check** at the end of each step says what should connect and what must not. Test each one on the meter's **continuity setting**: connected means it beeps (or reads near 0 Ω). The few checks that need the diode-test or resistance setting say so.

## Before you start

- **Wire:** 30 AWG stranded, in red (+5 V), black (GND), green, blue, white and yellow. Only W24–W29 differ: they're the motor cable's own wires, in its own colours.
- **Tin every stripped end** before it goes through a hole, so the strands don't splay.
- **Laying a link:** solder one end, press the wire flat along its lane and pinch the corners square with tweezers, hold the corners down with Kapton tape, then solder the far end. Keep it off the holes it crosses, and don't anchor wires through spare holes: every hole is part of a strip. Once the step's check passes, a dab of hot glue can hold it for good.
- **Prepare the modules** (build step 1): desolder U1's screw terminal and set its switches to 5 V. Pull the unused pins out of the header strips: the DRV8833 keeps D3–D6 and H2, H3, H6 (**H4 and H5 must come out**: the encoder's GND and 3V3 use those strips); the Super Mini keeps D12–D17 and H11–H15 (H10, GPIO5, stays out). Fit the strips to the modules with the perfboard as a jig, and solder them **to the modules only**.

## On the bench: D1 and the links

### Step 1: D1, the 1N5817 diode, standing

![Step 1: The perfboard seen from the component side, column A on the left. D1 drawn in colour, anode A3, cathode A2, its holes circled.](images/steps-wiring/step-01-D1.svg)

![Which end of the 1N5817 is which. Its body is black with a grey band at one end. The band end is the cathode, minus, and goes to A2; the plain end is the anode, plus, and goes to A3. In the schematic symbol, a triangle pointing at a bar, the bar is the band. Standing on the board, seen from the side: the body stands in A3 with the band at the top, and the top lead bends over and down into A2.](images/steps-wiring/d1-polarity.svg)

**Do this:**

1. The banded end is the cathode (−); the plain end is the anode (+). See the sketch.
2. On top: stand the anode lead in **A3** (printed J3 underneath), band at the top, and bend the top lead over and down into **A2** (printed J2 underneath).
3. Turn the board over (underside up) and solder each lead at printed **J3** (top A3), printed **J2** (top A2). Trim the leads.
4. Check, as below.

- **Part:** D1
- **Holes:** anode A3, cathode A2
- **Side:** component side
- **What it does:** D1 joins the 5 V that feeds the motor driver (strip 3 left) to the Super Mini's 5 V (strip 2 left), one way only.
- **How:** Stand it in A3 with the banded (cathode) end at the top, bend the top lead over and down into A2, and solder both from the underside.
- **Check:** Diode test, red probe on A3 and black on A2: about 0.2–0.3 V. Reversed: open.

**The twelve links** go on the underside: the ten for the drive, shortest first, then the two for the encoder. Turn the board over: from here to step 13 the pictures show the underside as you see it, with the mirrored letters printed there; each hole is given by both names.

### Step 2: W16, red, B7 → C3

![Step 2: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the earlier step drawn in grey. W16, red, drawn bold from B7 to C3 (printed I7 and H3 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-02-W16.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **I7** (top B7): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **H3** (top C3): push it in and solder it.
5. Check, as below.

- **Wire:** W16, red, 30 AWG stranded
- **Cut:** 25 mm
- **From → to:** B7 → C3 (printed I7 → H3 on the underside)
- **Side:** underside (link)
- **What it does:** Takes +5 V from the bus (row 7) to the motor driver's VCC strip and D1's anode.
- **Check:** B7 connects to A3 (D1's anode). B7 must not connect to B8 (the ground bus).

### Step 3: W18, black, B8 → C4

![Step 3: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 2 earlier steps drawn in grey. W18, black, drawn bold from B8 to C4 (printed I8 and H4 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-03-W18.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **I8** (top B8): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **H4** (top C4): push it in and solder it.
5. Check, as below.

- **Wire:** W18, black, 30 AWG stranded
- **Cut:** 25 mm
- **From → to:** B8 → C4 (printed I8 → H4 on the underside)
- **Side:** underside (link)
- **What it does:** Takes ground from the bus (row 8) to the motor driver's GND strip.
- **Check:** B8 connects to A4. B8 must not connect to B7, and C4 must not connect to C3.

### Step 4: W5, green, C12 → C5

![Step 4: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 3 earlier steps drawn in grey. W5, green, drawn bold from C12 to C5 (printed H12 and H5 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-04-W5.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **H12** (top C12): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **H5** (top C5): push it in and solder it.
5. Check, as below.

- **Wire:** W5, green, 30 AWG stranded
- **Cut:** 35 mm
- **From → to:** C12 → C5 (printed H12 → H5 on the underside)
- **Side:** underside (link)
- **What it does:** MOT_IN1: the Super Mini's GPIO18 to the driver's IN3, the forward PWM.
- **Check:** C12 connects to A5. It should not touch C13 or C11 next door.

### Step 5: W12, blue, C14 → C6

![Step 5: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 4 earlier steps drawn in grey. W12, blue, drawn bold from C14 to C6 (printed H14 and H6 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-05-W12.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **H14** (top C14): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **H6** (top C6): push it in and solder it.
5. Check, as below.

- **Wire:** W12, blue, 30 AWG stranded
- **Cut:** 35 mm
- **From → to:** C14 → C6 (printed H14 → H6 on the underside)
- **Side:** underside (link)
- **What it does:** MOT_IN2: GPIO20 to the driver's IN4, the reverse PWM.
- **Check:** C14 connects to A6. It should not touch C13, C15 or W5.

### Step 6: W4, black, C8 → C16

![Step 6: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 5 earlier steps drawn in grey. W4, black, drawn bold from C8 to C16 (printed H8 and H16 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-06-W4.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **H8** (top C8): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **H16** (top C16): push it in and solder it.
5. Check, as below.

- **Wire:** W4, black, 30 AWG stranded
- **Cut:** 40 mm
- **From → to:** C8 → C16 (printed H8 → H16 on the underside)
- **Side:** underside (link)
- **What it does:** Ground from the bus to the Super Mini's GND strip.
- **Check:** C16 connects to B8. C16 must not connect to C15 (3V3) or C17 (5V).

### Step 7: W22, red, C2 → H7

![Step 7: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 6 earlier steps drawn in grey. W22, red, drawn bold from C2 to H7 (printed H2 and C7 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-07-W22.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **H2** (top C2): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **C7** (top H7): push it in and solder it.
5. Check, as below.

- **Wire:** W22, red, 30 AWG stranded
- **Cut:** 50 mm
- **From → to:** C2 → H7 (printed H2 → C7 on the underside)
- **Side:** underside (link)
- **What it does:** The Super Mini's 5 V, after D1, across the channel to the strip where C3's + lead will go.
- **Check:** H7 connects to A2. H7 must not connect to H8 or H6.

### Step 8: W21, yellow, A15 → I10

![Step 8: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 7 earlier steps drawn in grey. W21, yellow, drawn bold from A15 to I10 (printed J15 and B10 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-08-W21.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **J15** (top A15): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **B10** (top I10): push it in and solder it.
5. Check, as below.

- **Wire:** W21, yellow, 30 AWG stranded
- **Cut:** 55 mm
- **From → to:** A15 → I10 (printed J15 → B10 on the underside)
- **Side:** underside (link)
- **What it does:** The Super Mini's 3.3 V to the strip where the knob's pin 1 will go. Yellow, not red: this is 3.3 V only.
- **Check:** A15 connects to J10. I10 must not connect to I11 (the wiper strip), and A15 must not connect to A16.

### Step 9: W13, white, C13 → J6

![Step 9: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 8 earlier steps drawn in grey. W13, white, drawn bold from C13 to J6 (printed H13 and A6 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-09-W13.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **H13** (top C13): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **A6** (top J6): push it in and solder it.
5. Check, as below.

- **Wire:** W13, white, 30 AWG stranded
- **Cut:** 55 mm
- **From → to:** C13 → J6 (printed H13 → A6 on the underside)
- **Side:** underside (link)
- **What it does:** DRV_SLEEP: GPIO19 to the driver's EEP pin, which wakes it.
- **Check:** C13 connects to J6. It should not touch C12, C14, or W22 where they cross.

### Step 10: W17, red, B2 → C17

![Step 10: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 9 earlier steps drawn in grey. W17, red, drawn bold from B2 to C17 (printed I2 and H17 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-10-W17.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **I2** (top B2): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **H17** (top C17): push it in and solder it.
5. Check, as below.

- **Wire:** W17, red, 30 AWG stranded
- **Cut:** 60 mm
- **From → to:** B2 → C17 (printed I2 → H17 on the underside)
- **Side:** underside (link)
- **What it does:** The Super Mini's 5 V: from D1's cathode strip down the board to the 5V pin's strip.
- **Check:** C17 connects to A2 and to H7. C17 must not connect to C16 (GND).

### Step 11: W23, black, A16 → H8

![Step 11: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 10 earlier steps drawn in grey. W23, black, drawn bold from A16 to H8 (printed J16 and C8 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-11-W23.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **J16** (top A16): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **C8** (top H8): push it in and solder it.
5. Check, as below.

- **Wire:** W23, black, 30 AWG stranded
- **Cut:** 65 mm
- **From → to:** A16 → H8 (printed J16 → C8 on the underside)
- **Side:** underside (link)
- **What it does:** Ground to the strip where C3's − lead will go.
- **Check:** H8 connects to B8. H8 must not connect to H7 (5 V) or H9.

### Step 12: W30, black, I4 → E4

![Step 12: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 11 earlier steps drawn in grey. W30, black, drawn bold from I4 to E4 (printed B4 and F4 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-12-W30.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **B4** (top I4): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **F4** (top E4): push it in and solder it.
5. Check, as below.

- **Wire:** W30, black, 30 AWG stranded
- **Cut:** 30 mm
- **From → to:** I4 → E4 (printed B4 → F4 on the underside)
- **Side:** underside (link)
- **What it does:** Ground for the encoder: from strip 4 right, where its blue wire (W26) will land, across to the driver's GND strip. **The DRV8833's H4 pin (OUT2) must be pulled**, or this would ground a driver output.
- **Check:** I4 connects to B8. I4 must not connect to I3 (OUT3) or I5.

### Step 13: W31, yellow, I5 → H10

![Step 13: The perfboard seen from the underside, as you see it turned over, its columns labelled with the mirrored letters printed there, with the 12 earlier steps drawn in grey. W31, yellow, drawn bold from I5 to H10 (printed B5 and C10 on the underside), both ends circled and labelled with both names.](images/steps-wiring/step-13-W31.svg)

**Do this:**

1. Turn the board over (underside up).
2. Start at hole printed **B5** (top I5): push the tinned end in and solder it.
3. Lay the wire flat along the drawn path, between the existing joints; pinch the corners and hold them with Kapton tape.
4. End at hole printed **C10** (top H10): push it in and solder it.
5. Check, as below.

- **Wire:** W31, yellow, 30 AWG stranded
- **Cut:** 30 mm
- **From → to:** I5 → H10 (printed B5 → C10 on the underside)
- **Side:** underside (link)
- **What it does:** The encoder's 3.3 V supply: from strip 5 right, where its black VCC wire (W27) will land, to the knob's 3V3 strip (row 10 right). Yellow: 3.3 V only. **The DRV8833's H5 pin (OUT1) must be pulled.** H10 has no Super Mini pin, so its joint ends up under the module, like H8's.
- **Check:** I5 connects to A15 (3V3). I5 must not connect to I4 or I6, and H10 must not connect to H9 or H11 (GPIO4).

## On the bench: C2, RV1 and C3

### Step 14: C2, the 100 nF capacitor across the motor

![Step 14: The perfboard seen from the component side, column A on the left, with the 13 earlier steps drawn in grey. C2 drawn in colour, legs in I3 and I2, its holes circled.](images/steps-wiring/step-14-C2.svg)

**Do this:**

1. A 104 ceramic: either way round.
2. On top: its legs in **I3** (printed B3 underneath) and **I2** (printed B2 underneath), standing as low as it goes.
3. Turn the board over (underside up) and solder each lead at printed **B3** (top I3), printed **B2** (top I2). Trim the leads.
4. Check, as below.

- **Part:** C2
- **Holes:** legs in I3 and I2
- **Side:** component side
- **What it does:** Absorbs the motor's brush noise, across OUT3 (row 3 right) and OUT4 (row 2 right). With the encoder motor it lives here, not on the motor's tabs: the encoder board covers them.
- **How:** A 104 ceramic with 2.54 mm leg spacing. Stand it on its legs as low as it goes, so it stays under the cover. Solder from the underside.
- **Check:** I3 connects to H3 and I2 to H2; I3 must not connect to I2 (a meter on resistance reads open).

### Step 15: RV1, the 10 kΩ speed knob

![Step 15: The perfboard seen from the component side, column A on the left, with the 14 earlier steps drawn in grey. RV1 drawn in colour, pins J10, J11, J12, its holes circled.](images/steps-wiring/step-15-RV1.svg)

**Do this:**

1. Snip off its two mounting tabs if it has them. Shaft up, body off the right-hand edge.
2. On top: its three pins in **J10** (printed A10 underneath), **J11** (printed A11 underneath) and **J12** (printed A12 underneath).
3. Turn the board over (underside up) and solder each lead at printed **A10** (top J10), printed **A11** (top J11), printed **A12** (top J12). Trim the leads.
4. Check, as below.

- **Part:** RV1
- **Holes:** pins J10, J11, J12
- **Side:** component side
- **What it does:** The knob: pin 1 in the 3V3 strip (fed by W21), the wiper in the GPIO4 strip, pin 3 in the GPIO3 strip.
- **How:** Snip off its two mounting tabs if it has them. Shaft up, body off the right-hand edge. Solder from the underside.
- **Check:** J10 to J12 reads about 10 kΩ. J10 connects to A15. J11 to J10 changes as you turn the shaft.

### Step 16: C3, the 470 µF hold-up capacitor, lying down

![Step 16: The perfboard seen from the component side, column A on the left, with the 15 earlier steps drawn in grey. C3 drawn in colour, + I7, − I8, its holes circled.](images/steps-wiring/step-16-C3.svg)

![Which lead of C3 is which. The 470 µF capacitor is a blue can with a light stripe printed down one side, marked with minus signs. The lead on the stripe side is minus and goes to I8; the other lead, usually the longer one, is plus and goes to I7. Lying on the board, the stripe faces row 8.](images/steps-wiring/c3-polarity.svg)

**Do this:**

1. The striped side is −; the other lead is +. See the sketch.
2. Bend both leads down 90° about 1 mm from the can. On top: + in **I7** (printed B7 underneath), − (stripe) in **I8** (printed B8 underneath); lay it flat, its end past the right-hand edge.
3. Turn the board over (underside up) and solder each lead at printed **B7** (top I7), printed **B8** (top I8). Trim the leads.
4. Check, as below.

- **Part:** C3
- **Holes:** + I7, − I8
- **Side:** component side
- **What it does:** Keeps the Super Mini running through a motor start or stall.
- **How:** Bend both leads down 90° about 1 mm from the can. **+ lead in I7, the striped (−) side toward I8.** Lay it flat, its end past the right-hand edge.
- **Check:** The stripe faces row 8. I7 connects to A2, I8 connects to B8, and I7 must not connect to I8 (a meter on resistance shows it charging, then open).

## On the bench: the modules and U1

### Step 17: U3, the DRV8833 module

![Step 17: The perfboard seen from the component side, column A on the left, with the 16 earlier steps drawn in grey. U3 drawn in colour, pins D3–D6, H2, H3, H6.](images/steps-wiring/step-17-U3.svg)

**Do this:**

1. Component side up. Check the labels on your module: VCC, GND, IN3, IN4 down one row; OUT4, OUT3, EEP on the other (see perfboard.md).
2. On top: its pins in **D3–D6** (printed G3–G6 underneath) and **H2, H3, H6** (printed C2, C3, C6 underneath).
3. Turn the board over (underside up) and solder each pin at printed **G3–G6** (top D3–D6) and **C2, C3, C6** (top H2, H3, H6).
4. Check, as below.

- **Part:** U3
- **Holes:** pins D3–D6, H2, H3, H6
- **Side:** component side
- **What it does:** The motor driver: VCC, GND, IN3 and IN4 down column D, OUT4, OUT3 and EEP in column H.
- **How:** **Check the labels on your module first** (see perfboard.md). Only these seven pins are fitted. Component side up, solder from the underside.
- **Check:** D3 (VCC) must not connect to D4 (GND). VCC connects to B7, GND to B8, H6 (EEP) to C13.

### Step 18: U2, the ESP32-C6 Super Mini

![Step 18: The perfboard seen from the component side, column A on the left, with the 17 earlier steps drawn in grey. U2 drawn in colour, pins D12–D17, H11–H15.](images/steps-wiring/step-18-U2.svg)

**Do this:**

1. Component side up, USB-C past row 17.
2. On top: its pins in **D12–D17** (printed G12–G17 underneath) and **H11–H15** (printed C11–C15 underneath). H10 stays empty.
3. Turn the board over (underside up) and solder each pin at printed **G12–G17** (top D12–D17) and **C11–C15** (top H11–H15).
4. Check, as below.

- **Part:** U2
- **Holes:** pins D12–D17, H11–H15
- **Side:** component side
- **What it does:** The controller: GPIO18–20, 3V3, GND and 5V down column D; GPIO4 to GPIO0 in column H. H10 (GPIO5) stays out.
- **How:** Component side up, USB-C past row 17. Solder from the underside.
- **Check:** The 5V, 3V3 and GND pins must not connect to each other. 5V connects to A2, GND to B8, 3V3 to J10.

### Step 19: U1, the PD trigger board, on foam pads

![Step 19: The perfboard seen from the component side, column A on the left, with the 18 earlier steps drawn in grey. U1 drawn in colour, over A–C, rows 9–17.](images/steps-wiring/step-19-U1.svg)

**Do this:**

1. Its screw terminal is off and its switches are set to 5 V (Before you start).
2. On top: stick it down on its two foam pads over columns A–C, rows 9–17, USB-C past row 17 beside the Super Mini's, output pads toward row 8. Nothing goes into holes.
3. Find + and − by the marking printed on your PD board, then measure them with a charger plugged in (the check).
4. Check, as below.

- **Part:** U1
- **Holes:** over A–C, rows 9–17
- **Side:** component side
- **What it does:** The USB-C power input. It isn't soldered into holes: it sits on two 3 mm foam pads over the Super Mini's left-hand strips.
- **How:** Screw terminal desoldered and switches set to 5 V (Before you start). Stick it down with its USB-C past row 17, beside the Super Mini's, and its two output pads toward row 8. **Find + and − by the marking printed on your own PD board** (+ / − or VBUS / GND), not by this picture: boards differ. On ours, − (GND) is the outer pad, toward the column A edge, and + (VBUS) the inner one, toward the channel, as drawn.
- **Check:** Before soldering W1 and W2: plug a USB-C charger into U1 and measure across its two pads on DC volts. The pad that reads +5 V against the other is + (W1, red); the other is − (W2, black). Unplug it again. Its USB-C sits about 1.4 mm past the board edge, and it lies flat.

### Step 20: W1, red, U1 + → A7

![Step 20: The perfboard seen from the component side, column A on the left, with the 19 earlier steps drawn in grey. W1, red, drawn bold from U1 + to A7, both ends circled.](images/steps-wiring/step-20-W1.svg)

**Do this:**

1. Component side up; this one stays on top.
2. Start at U1's + (VBUS) pad, found by the marking on your board: solder the end to it.
3. Run the wire between A8 and B8, clear of W2.
4. End at hole **A7**: push it in and solder it on top.
5. Check, as below.

- **Wire:** W1, red, 30 AWG stranded
- **Cut:** 25 mm
- **From → to:** U1 + → A7
- **Side:** component side
- **What it does:** U1's 5 V output, from its + (VBUS) pad, down to the +5V bus (row 7 left). On our board + is the inner pad; go by the marking on yours. Run it between A8 and B8, so it doesn't cross W2.
- **Check:** Before soldering, make sure this is the pad marked + (or VBUS), the one that read +5 V in step 17. After: U1's + pad connects to B7 and A3. It must not connect to U1's − pad.

### Step 21: W2, black, U1 − → A8

![Step 21: The perfboard seen from the component side, column A on the left, with the 20 earlier steps drawn in grey. W2, black, drawn bold from U1 − to A8, both ends circled.](images/steps-wiring/step-21-W2.svg)

**Do this:**

1. Component side up; this one stays on top.
2. Start at U1's − (GND) pad, found by the marking on your board: solder the end to it.
3. Run the wire straight to A8.
4. End at hole **A8**: push it in and solder it on top.
5. Check, as below.

- **Wire:** W2, black, 30 AWG stranded
- **Cut:** 20 mm
- **From → to:** U1 − → A8
- **Side:** component side
- **What it does:** U1's ground, from its − (GND) pad, down to the GND bus (row 8 left). On our board − is the outer pad; go by the marking on yours.
- **Check:** Before soldering, make sure this is the pad marked − (or GND). After: U1's − pad connects to B8 and the Super Mini's GND. U1 + must not connect to U1 −.

## On the drive: the button wires (W11, W6), then C1

With the board on the drive and the button in its pod mount. The button's two wires (W11, W6) run straight from it, through the base tunnel and behind the board: push each **up from the underside** and solder it on the component side. There's no direction switch and no panel connector, so the pod stays wired to the board. Both go in before C1: it stands over A4, the hole W11 uses.

![The perfboard on the drive, seen edge-on. The mount is at the bottom; its -Y face holds the board on supports, with a gap between them. The board's underside faces the mount and its component side faces out, where you solder. The button's two wires come from the pod through the base tunnel, run up the mount's face in the gap behind the board, and each comes up through its own hole to be soldered on top. C1 stands on the board and its top comes through the cover. Nothing wraps round the board's edge.](images/steps-wiring/on-the-drive.svg)

### Step 22: W11, black, button lead B → A4

![Step 22: The perfboard seen from the component side, column A on the left, with the 21 earlier steps drawn in grey. Hole A4 (printed J4 on the underside) is marked with a dot in a ring where W11, black, comes up from below; a dashed line shows it running behind the board to the left edge, on to the button (lead B). Below, a side view shows the board edge-on over the mount: the wire runs along the gap behind the board, up through the hole, and is soldered on the component side with its stub trimmed.](images/steps-wiring/step-22-W11.svg)

**Do this:**

1. Start at the button (lead B): the button's own lead, or 30 AWG spliced to it in the pod (see [the button](wiring.md#the-button-s1)).
2. Run the wire behind the board, between the board and the mount.
3. Push the other end up through the hole printed **J4** on the underside (top A4).
4. Turn to the top and solder it at **A4**. Trim the stub.
5. Check, as below.

- **Wire:** W11, black, 30 AWG stranded (or the button's own lead)
- **Cut:** 185 mm
- **From → to:** button lead B → A4 (push it in at printed J4 on the underside)
- **Side:** in from the underside, soldered on the component side
- **What it does:** The button's ground side, straight from the button to GND (strip 4 left). Solder it before C1: C1 stands over A4.
- **Check:** The button's lead B connects to B8. It must not connect to I14. Keep the joint at A4 low and neat: C1 stands just above it.

### Step 23: W6, white, button lead A → I14

![Step 23: The perfboard seen from the component side, column A on the left, with the 22 earlier steps drawn in grey. Hole I14 (printed B14 on the underside) is marked with a dot in a ring where W6, white, comes up from below; a dashed line shows it running behind the board to the right edge, on to the button (lead A). Below, a side view shows the board edge-on over the mount: the wire runs along the gap behind the board, up through the hole, and is soldered on the component side with its stub trimmed.](images/steps-wiring/step-23-W6.svg)

**Do this:**

1. Start at the button (lead A): the button's own lead, or 30 AWG spliced to it in the pod (see [the button](wiring.md#the-button-s1)).
2. Run the wire behind the board, between the board and the mount.
3. Push the other end up through the hole printed **B14** on the underside (top I14).
4. Turn to the top and solder it at **I14**. Trim the stub.
5. Check, as below.

- **Wire:** W6, white, 30 AWG stranded (or the button's own lead)
- **Cut:** 185 mm
- **From → to:** button lead A → I14 (push it in at printed B14 on the underside)
- **Side:** in from the underside, soldered on the component side
- **What it does:** BTN: the button's other lead, straight to GPIO1 (strip 14 right). Pressing the button joins GPIO1 to ground.
- **Check:** The button's lead A connects to H14 (GPIO1), and to B8 only while you hold the button down. I14 must not connect to I13 or I15.

### Step 24: C1, the 470 µF bulk capacitor, standing

![Step 24: The perfboard seen from the component side, column A on the left, with the 23 earlier steps drawn in grey. C1 drawn in colour, + C7, − B4, its holes circled.](images/steps-wiring/step-24-C1.svg)

![Which leg of C1 is which, and how it stands. The 470 µF can is blue with a light stripe down one side, marked with minus signs. The leg on the stripe side is minus and goes to B4; the other leg is plus, the longer one on a new part, and goes to C7. Seen from the side, the can stands 1.5 mm off the board with both legs sleeved and bent out to their holes.](images/steps-wiring/c1-polarity.svg)

**Do this:**

1. The striped side is −; the long leg is +. See the sketch. Slip heat-shrink over both legs.
2. On top: stand it 1.5 mm up over A4–C6, stripe toward row 4; bend the + leg out to **C7** (printed H7 underneath) and the − leg to **B4** (printed I4 underneath).
3. Turn the board over (underside up) and solder each lead at printed **H7** (top C7), printed **I4** (top B4). Trim the leads.
4. Check, as below.

- **Part:** C1
- **Holes:** + C7, − B4
- **Side:** component side
- **What it does:** Steadies the 5 V bus when the motor starts. It stands over columns A–C, rows 4–7, between D1 and W1/W2, and its top comes through the cover's 9 mm hole.
- **How:** **+ is the long leg, − is the striped side** (see the sketch). Slip heat-shrink over both leads: they cross the pads of rows 5 and 6 (IN3, IN4) on the way out. Stand the can 1.5 mm off the board over A4–C6, stripe toward row 4, bend the + lead out to C7 and the − lead to B4, and solder them from the underside.
- **Check:** The stripe faces row 4. C7 connects to A7, B4 connects to B8, and C7 must not connect to B4 (on resistance it shows the capacitor charging, then open). Neither lead touches the pads of rows 5 or 6: A5 and A6 must not connect to A7 or B8.

## The motor cable

The encoder motor comes with its own 6-wire cable, plugged into the 6-pin socket on the encoder board: that plug is **J4**, where the motor and its cartridge unplug. The cable's other end is cut to length and soldered into the board. All six wires come in through the notch in the cover's bottom wall. They keep the cable's own colours, which don't follow this board's: **black is the encoder's 3.3 V supply, blue is ground and red is a motor lead.** Routing is in [wiring.md](wiring.md#j4-the-motor-cable). Plug the cable into the motor first, so you can check each wire with a meter from the plug end.

### Step 25: W24, white, J4 pin 1 (M1) → J3

![Step 25: The perfboard seen from the component side, column A on the left, with the 24 earlier steps drawn in grey. W24, white, drawn bold from the cover's notch at the right-hand edge into hole J3, circled; it is the encoder plug's (J4's) pin 1, M1.](images/steps-wiring/step-25-W24.svg)

**Do this:**

1. Find the cable's white wire, the encoder plug's pin 1, M1. Measure 70 mm from the back of the plug, cut it there, strip 2 mm and tin it.
2. Bring it in through the notch in the cover's bottom wall, along the right-hand edge.
3. On top: push it into **J3** (printed A3 underneath).
4. Turn the board over (underside up) and solder it at printed **A3** (top J3). Trim the stub.
5. Check, as below.

- **Wire:** W24, white, the motor cable's own wire
- **Cut:** 70 mm, measured from the back of the encoder plug
- **From → to:** J4 pin 1 (M1) → J3
- **Side:** in from the component side through the cover's notch, soldered underneath
- **What it does:** Motor lead M1, to the driver's OUT3. If the motor runs backwards, swap W24 and W25.
- **Check:** With the cable plugged into the motor, J3 to J2 reads the motor's winding, a few ohms or more: 0 Ω means a short, at C2 or between the wires.

### Step 26: W25, red, J4 pin 6 (M2) → J2

![Step 26: The perfboard seen from the component side, column A on the left, with the 25 earlier steps drawn in grey. W25, red, drawn bold from the cover's notch at the right-hand edge into hole J2, circled; it is the encoder plug's (J4's) pin 6, M2.](images/steps-wiring/step-26-W25.svg)

**Do this:**

1. Find the cable's red wire, the encoder plug's pin 6, M2. Measure 70 mm from the back of the plug, cut it there, strip 2 mm and tin it.
2. Bring it in through the notch in the cover's bottom wall, along the right-hand edge.
3. On top: push it into **J2** (printed A2 underneath).
4. Turn the board over (underside up) and solder it at printed **A2** (top J2). Trim the stub.
5. Check, as below.

- **Wire:** W25, red, the motor cable's own wire
- **Cut:** 70 mm, measured from the back of the encoder plug
- **From → to:** J4 pin 6 (M2) → J2
- **Side:** in from the component side through the cover's notch, soldered underneath
- **What it does:** Motor lead M2, to the driver's OUT4. Red here is a motor lead, not +5 V.
- **Check:** J2 connects to H2 and to C2's leg in I2. J2 must not connect to J3 except through the motor.

### Step 27: W26, blue, J4 pin 2 (GND) → J4

![Step 27: The perfboard seen from the component side, column A on the left, with the 26 earlier steps drawn in grey. W26, blue, drawn bold from the cover's notch at the right-hand edge into hole J4, circled; it is the encoder plug's (J4's) pin 2, GND.](images/steps-wiring/step-27-W26.svg)

**Do this:**

1. Find the cable's blue wire, the encoder plug's pin 2, GND. Measure 70 mm from the back of the plug, cut it there, strip 2 mm and tin it.
2. Bring it in through the notch in the cover's bottom wall, along the right-hand edge.
3. On top: push it into **J4** (printed A4 underneath).
4. Turn the board over (underside up) and solder it at printed **A4** (top J4). Trim the stub.
5. Check, as below.

- **Wire:** W26, blue, the motor cable's own wire
- **Cut:** 70 mm, measured from the back of the encoder plug
- **From → to:** J4 pin 2 (GND) → J4
- **Side:** in from the component side through the cover's notch, soldered underneath
- **What it does:** The encoder's ground, into strip 4 right, which W30 joins to GND. Blue is ground on this cable.
- **Check:** Hole J4 connects to B8 (GND). It must not connect to J3 or J5.

### Step 28: W27, black, J4 pin 5 (VCC) → J5

![Step 28: The perfboard seen from the component side, column A on the left, with the 27 earlier steps drawn in grey. W27, black, drawn bold from the cover's notch at the right-hand edge into hole J5, circled; it is the encoder plug's (J4's) pin 5, VCC.](images/steps-wiring/step-28-W27.svg)

**Do this:**

1. Find the cable's black wire, the encoder plug's pin 5, VCC. Measure 70 mm from the back of the plug, cut it there, strip 2 mm and tin it.
2. Bring it in through the notch in the cover's bottom wall, along the right-hand edge.
3. On top: push it into **J5** (printed A5 underneath).
4. Turn the board over (underside up) and solder it at printed **A5** (top J5). Trim the stub.
5. Check, as below.

- **Wire:** W27, black, the motor cable's own wire
- **Cut:** 70 mm, measured from the back of the encoder plug
- **From → to:** J4 pin 5 (VCC) → J5
- **Side:** in from the component side through the cover's notch, soldered underneath
- **What it does:** The encoder's supply, into strip 5 right, which W31 joins to 3V3. **Black is the supply on this cable, and it must be 3.3 V**: the encoder pulls its outputs up to it, and the ESP32-C6 isn't 5 V tolerant.
- **Check:** J5 connects to A15 (3V3). J5 must not connect to J4 (GND), J6 (EEP) or anything on 5 V (A7, A2).

### Step 29: W28, green, J4 pin 3 (C1) → I15

![Step 29: The perfboard seen from the component side, column A on the left, with the 28 earlier steps drawn in grey. W28, green, drawn bold from the cover's notch at the right-hand edge, over the DRV8833 and the Super Mini, into hole I15, circled; it is the encoder plug's (J4's) pin 3, C1.](images/steps-wiring/step-29-W28.svg)

**Do this:**

1. Find the cable's green wire, the encoder plug's pin 3, C1. Measure 110 mm from the back of the plug, cut it there, strip 2 mm and tin it.
2. From the notch, run it over the DRV8833 and the Super Mini (clear of C3 and RV1), keeping it low: the cover sits 9 mm above the board.
3. On top: push it into **I15** (printed B15 underneath).
4. Turn the board over (underside up) and solder it at printed **B15** (top I15). Trim the stub.
5. Check, as below.

- **Wire:** W28, green, the motor cable's own wire
- **Cut:** 110 mm, measured from the back of the encoder plug
- **From → to:** J4 pin 3 (C1) → I15
- **Side:** in from the component side through the cover's notch, soldered underneath
- **What it does:** Encoder channel A (C1) to GPIO0. From the notch it runs over the DRV8833 and the Super Mini, under the cover, and down into I15 beside the Super Mini.
- **Check:** I15 connects to H15 (GPIO0). I15 must not connect to I14 (the button) or I16.

### Step 30: W29, yellow, J4 pin 4 (C2) → I13

![Step 30: The perfboard seen from the component side, column A on the left, with the 29 earlier steps drawn in grey. W29, yellow, drawn bold from the cover's notch at the right-hand edge, over the DRV8833 and the Super Mini, into hole I13, circled; it is the encoder plug's (J4's) pin 4, C2.](images/steps-wiring/step-30-W29.svg)

**Do this:**

1. Find the cable's yellow wire, the encoder plug's pin 4, C2. Measure 105 mm from the back of the plug, cut it there, strip 2 mm and tin it.
2. From the notch, run it over the DRV8833 and the Super Mini (clear of C3 and RV1), keeping it low: the cover sits 9 mm above the board.
3. On top: push it into **I13** (printed B13 underneath).
4. Turn the board over (underside up) and solder it at printed **B13** (top I13). Trim the stub.
5. Check, as below.

- **Wire:** W29, yellow, the motor cable's own wire
- **Cut:** 105 mm, measured from the back of the encoder plug
- **From → to:** J4 pin 4 (C2) → I13
- **Side:** in from the component side through the cover's notch, soldered underneath
- **What it does:** Encoder channel B (C2) to GPIO2, beside W28 over the modules, down into I13. Yellow here is a signal, not 3.3 V.
- **Check:** I13 connects to H13 (GPIO2). I13 must not connect to I12 (GPIO3) or I14.

## Last checks

1. Go through the [terminal check](wiring.md#terminal-check): every terminal should hold exactly the wires it lists.
2. **+5V, +3V3 and GND must not connect to each other**, or to the strip next door. Check B7 to B8, A15 to A16, C17 to C16, and J4 to J5 (the encoder's GND and 3V3).
3. Then follow [bring-up](wiring.md#bring-up) before the first power-on.

The encoder runs on **3.3 V only**: with the board powered and the motor cable unplugged at the motor, check that J5 reads 3.3 V to J4.

The button's wires are covered in [wiring.md](wiring.md#the-button-s1): use its own leads if they reach, or splice in the pod. The pod no longer unplugs from the drive.

_Generated by [make_steps.mjs](images/steps-wiring/make_steps.mjs) from the same hole map as [perfboard.svg](images/perfboard.svg); the wire IDs, colours and cut lengths are checked against [cut-list.csv](cut-list.csv) when it runs._
