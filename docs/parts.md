# Parts

Everything for one drive unit. The same list is in [`bom.csv`](bom.csv). How it all connects is in the [schematic](images/schematic.svg) and [wiring notes](wiring.md). Fasteners, with McMaster part numbers and STEP models, are detailed in [`hardware/README.md`](../hardware/README.md).

## Electronics

| Ref | Qty | Part | What to look for |
|---|---|---|---|
| U2 | 1 | Seeed Studio XIAO ESP32-C6 | Make sure it's the **C6**; the firmware is built for it. |
| U1 | 1 | USB-C PD trigger board with screw terminal | Must have a **5 V** setting. The mount's pocket fits a board about 28 × 11 mm. |
| M1 | 1 | MG90S servo, **360° continuous rotation**, metal gear | The plain MG90S is the 180° positional version and won't turn a cam. |
| D1 | 1 | 1N5817 Schottky diode (or SS14) | Through-hole (axial); its leads pass through the mount wall. |
| C1 | 1 | 470 µF electrolytic capacitor, ≥ 10 V | **8 mm diameter**, radial: it press-fits into a seat in the mount. For another size, change `c1_seat_dia` and reprint. |
| R1 | 1 | 330 Ω resistor | |
| — | 1 | USB-C charger that supports PD | Most phone and laptop chargers do. |

## Controls

| Ref | Qty | Part | Fit |
|---|---|---|---|
| S1 | 1 | 12 mm stainless momentary push button, 1 NO, IP65, pre-wired 150 mm leads (e.g. Amazon B09BKWMNJ9) | 12.2 mm hole in the pod's outer face; M12 × 0.75 thread, 13.8 mm nut |
| SW1 | 1 | MTS-103 mini toggle, **ON-OFF-ON**, 3 pins, 1/4" (6 mm) bushing | 6.2 mm hole in the pod's side face. Avoid 6-pin (MTS-203), ON-ON and ON-OFF versions. |
| RV1 | 1 | 9 mm potentiometer, 10 kΩ linear (B10K), M7 bushing, with knob | 7.2 mm hole in the pod's side face. 16 mm pots won't fit inside the pod. |

## Printed parts (PLA)

Print files and settings are in [`cad/stl`](../cad/stl/README.md).

| Part | Qty | Notes |
|---|---|---|
| `Servo_Mount` | 1 | Flange down, supports under the base |
| `Control_Pod` | 1 | Front panel down |
| `Adapter_075` / `Adapter_0625` / `Adapter_025` | 1 for your dowel | Spline end down |
| `Bushing_075` / `Bushing_0625` / `Bushing_025` | 1 to match | Flange down |
| `Panel_Fit_Coupon` | 1 | Test the button, switch and knob holes before printing the pod |
| `Fit_Test_Coupon` | 1 | Print first to check the spline fit |
| `Dowel_Gauge_075` / `_0625` / `_025` | 1 for your dowel | Find the right socket and bushing clearance for your dowels |
| Clamps, sleeved adapter, shims | optional | For sanded dowels; see [assembly](assembly.md#sanded-or-undersize-dowels) |

## Fasteners

| Where | Part | Qty |
|---|---|---|
| Mount to box wall | McMaster 98685A950, brass rounded-head wood screw, #14 × 3/4" | 2 |
| Under the wall screws | McMaster 98026A029, USS washer, 1/4" screw, 0.734" OD | 2 |
| Adapter to dowel (3/4", 5/8") | McMaster 90031A108, #4 × 3/8" flat-head wood screw | 2 |
| Adapter to dowel (1/4") | McMaster 92114A106, brass #4 × 1/4" flat-head wood screw | 2 |
| Pod to mount | M3 × 8 wafer or pan head (M3 × 6 also fits) | 4 |
| Sanded-dowel clamps | M3 × 10 flat head thread-forming screw for plastic (McMaster 90485A442) | 2 per clamp |
| Servo tabs and horn | the screws that come with the MG90S | 2 + 1 |

## Wire and small parts

- 30 AWG silicone or Kynar wire: white, yellow, violet, green, blue, black
- 22–26 AWG wire: red and black
- Heat-shrink assortment
- Hot glue or CA glue (a dab to hold the diode)
- Small zip ties, 2.5–3.5 mm wide
- Wooden dowels: 3/4", 5/8" or 1/4"
