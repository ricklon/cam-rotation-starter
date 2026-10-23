# MG90S drive

The original Cam Rotation Starter drive: an MG90S continuous-rotation micro servo in a printed mount, coupled to the dowel through a printed adapter that presses onto the servo spline.

![X-ray view of the drive unit: a 3D-printed mount with the servo, the adapter that grips the dowel, the electronics on one side and the control pod on the other.](docs/images/xray_minusY.png)

- **Release:** [`v1.0-mg90s`](https://github.com/ricklon/cam-rotation-starter/releases/tag/v1.0-mg90s)
- **Fusion design:** *Cam Rotation Starter* in MovementStudy › Cam Rotation Tool

## Start here

| I want to… | Go to |
|---|---|
| See what to buy | [docs/parts.md](docs/parts.md) · [docs/bom.csv](docs/bom.csv) |
| See the circuit | [schematic](docs/images/schematic.svg) · [wiring notes](docs/wiring.md) |
| Print the parts | [cad/stl](cad/stl/README.md) |
| Wire it | [schematic](docs/images/schematic.svg) · [docs/wiring.md](docs/wiring.md) · [cut list](docs/cut-list.csv) |
| Build it | [docs/assembly.md](docs/assembly.md) |
| Flash the firmware | [firmware/README.md](../../firmware/README.md) |
| Change the design | [cad/README.md](cad/README.md) |
| Look up a screw | [hardware/README.md](../../hardware/README.md) |

## Quick start

1. **Print** the test pieces first: `Panel_Fit_Coupon` (button, switch and knob holes), `Fit_Test_Coupon` (servo spline) and the `Dowel_Gauge` strip for your dowel size. Then print the mount, pod, and the adapter and bushing for your dowel. See the [print order](cad/stl/README.md#print-order-for-a-fit-test).
2. **Wire** the electronics from the [schematic](docs/images/schematic.svg) and [wire list](docs/wiring.md). Set the PD board to 5 V and check it with a meter before connecting anything.
3. **Flash** the firmware (from the repository root):
   ```sh
   cd firmware/web && npm install && npm run build && cd ..
   idf.py set-target esp32c6 && idf.py build
   idf.py -p /dev/cu.usbmodemXXXX flash
   ```
4. **Assemble** the unit on the bench, slide it onto the shaft (28 mm sticking out of the wall), and screw it down.
5. **Connect** to the `CamBox-XXXX` Wi-Fi network, set the stop trim, and play the demo story.

## Folder layout

```
cad/     Fusion exports (STEP + .f3d), print-ready STLs, Fusion and STL scripts
docs/    parts, BOM, wiring, cut list, assembly guide, rendered images and schematic
```
