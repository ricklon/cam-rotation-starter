# Cam Rotation Starter

A small servo drive that turns a wooden camshaft through the wall of a box, so students can build cam-driven story animations with found objects inside. It also automates hand-cranked cam pieces. You program a "story" of speeds and pauses from a phone, then play it with a button, a speed knob and a direction switch, with no phone, laptop or internet needed.

![X-ray view of the drive unit: a 3D-printed mount with the servo, the adapter that grips the dowel, the electronics on one side and the control pod on the other.](docs/images/xray_minusY.png)

Made by Rick Anderson and Sonia Yaco.

## What it is

- **Mount:** a 3D-printed bracket that screws to the outside of a box wall. The camshaft (a 3/4", 5/8" or 1/4" wooden dowel) passes through the wall into a printed adapter on the servo, so nothing has to be reached from inside the box.
- **Drive:** an MG90S continuous-rotation servo, powered from any USB-C charger through a PD trigger board.
- **Brain:** a Seeed XIAO ESP32-C6 running ESP-IDF firmware. It makes its own Wi-Fi network and serves a web app for live control, story editing (including record-by-performing) and gallery mode.
- **Controls:** a 12 mm stainless button (play, pause, next), a speed knob, and a forward / stop / reverse switch.

## Start here

| I want to… | Go to |
|---|---|
| See what to buy | [docs/parts.md](docs/parts.md) · [docs/bom.csv](docs/bom.csv) |
| Print the parts | [cad/stl](cad/stl/README.md) |
| Wire it | [docs/wiring.md](docs/wiring.md) · [cut list](docs/cut-list.csv) |
| Build it | [docs/assembly.md](docs/assembly.md) |
| Flash the firmware | [firmware/README.md](firmware/README.md) |
| Use it with students | [classroom/README.md](classroom/README.md) |
| Change the design | [cad/README.md](cad/README.md) |
| Look up a screw | [hardware/README.md](hardware/README.md) |

## Quick start

1. **Print** `Panel_Fit_Coupon` and `Fit_Test_Coupon` first to check the controls and the servo spline, then the mount, pod, and the adapter and bushing for your dowel size.
2. **Wire** the electronics from the [wire list](docs/wiring.md). Set the PD board to 5 V and check it with a meter before connecting anything.
3. **Flash** the firmware:
   ```sh
   cd firmware/web && npm install && npm run build && cd ..
   idf.py set-target esp32c6 && idf.py build
   idf.py -p /dev/cu.usbmodemXXXX flash
   ```
4. **Assemble** the unit on the bench, slide it onto the shaft (28 mm sticking out of the wall), and screw it down.
5. **Connect** to the `CamBox-XXXX` Wi-Fi network, set the stop trim, and play the demo story.

## Repository layout

```
cad/          Fusion design (STEP + .f3d), print-ready STLs, design scripts
docs/         parts, wiring, cut list, assembly guide, images
firmware/     ESP-IDF firmware and the Vite web app it serves
hardware/     fastener list and McMaster STEP models
classroom/    how stories work, example stories, teaching notes
```

## Status

The design, firmware and documentation are complete and the firmware compiles with ESP-IDF v5.4.1. The first physical build is in progress; expect fit adjustments after test prints. Please open an issue with print-fit reports or build problems.

## License

- **Hardware** (everything outside `firmware/`: CAD, STL and STEP files, documentation and images): [CERN Open Hardware Licence v2, Permissive](LICENSE) (CERN-OHL-P-2.0).
- **Firmware** (`firmware/`): [Apache License 2.0](firmware/LICENSE).

Copies of both are in [`LICENSES/`](LICENSES). McMaster-Carr STEP models in `hardware/step/` are McMaster's and are included for reference.
