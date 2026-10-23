# Cam Rotation Starter

A small motor drive that turns a wooden camshaft through the wall of a box, so students can build cam-driven story animations with found objects inside. It also automates hand-cranked cam pieces. You program a "story" of speeds and pauses from a phone, then play it with a button, a speed knob and a direction switch, with no phone, laptop or internet needed.

![X-ray view of the MG90S drive unit: a 3D-printed mount with the servo, the adapter that grips the dowel, the electronics on one side and the control pod on the other.](drives/mg90s/docs/images/xray_minusY.png)

Made by Rick Anderson and Sonia Yaco.

## Pick a drive

The drive comes in more than one version. They share the brain, the controls, the firmware's story player and web app, and the dowel bushings and gauges. What differs is the motor, the mount it sits in, the adapter that couples it to the dowel, and the wiring.

| Drive | Motor | Status | Start here |
|---|---|---|---|
| **MG90S** | MG90S continuous-rotation micro servo | Design complete; first build in progress. Tagged [`v1.0-mg90s`](https://github.com/ricklon/cam-rotation-starter/releases/tag/v1.0-mg90s). | [drives/mg90s](drives/mg90s/README.md) |
| **N20** | 6 V N20 metal gearmotor (no encoder) with a DRV8833 driver | Planning: parts and wiring plan written, mount not designed yet. | [drives/n20](drives/n20/README.md) |

Both run from any USB-C charger through a PD trigger board set to 5 V, on a Seeed XIAO ESP32-C6.

## What every version has

- **Mount:** a 3D-printed bracket that screws to the outside of a box wall. The camshaft (a 3/4", 5/8" or 1/4" wooden dowel) passes through the wall into a printed adapter on the motor, so nothing has to be reached from inside the box.
- **Brain:** a Seeed XIAO ESP32-C6 running ESP-IDF firmware. It makes its own Wi-Fi network and serves a web app for live control, story editing (including record-by-performing) and gallery mode.
- **Controls:** a 12 mm stainless button (play, pause, next), a speed knob, and a forward / stop / reverse switch.

## Shared parts of the repository

| I want to… | Go to |
|---|---|
| Flash the firmware | [firmware/README.md](firmware/README.md) |
| Use it with students | [classroom/README.md](classroom/README.md) |
| Look up a screw | [hardware/README.md](hardware/README.md) |

## Repository layout

```
drives/
  mg90s/      MG90S servo drive: cad/ (Fusion exports, STLs, scripts) and docs/ (parts, wiring, assembly, images)
  n20/        N20 gearmotor drive: same layout, being designed
firmware/     ESP-IDF firmware and the Vite web app it serves (shared by all drives)
hardware/     fastener list and McMaster STEP models (shared)
classroom/    how stories work, example stories, teaching notes (shared)
```

Each drive folder is self-contained: its `cad/` and `docs/` sit side by side, so the scripts and links inside it work the same way for every drive.

## Versions

- Each drive's releases are tagged `vMAJOR.MINOR-<drive>`, for example `v1.0-mg90s`.
- The Fusion design for each drive is its own document in the owner's Fusion project (MovementStudy › Cam Rotation Tool). Each drive's `cad/README.md` names the document and the Fusion version a tag was cut from.

## License

- **Hardware** (everything outside `firmware/`: CAD, STL and STEP files, documentation and images): [CERN Open Hardware Licence v2, Permissive](LICENSE) (CERN-OHL-P-2.0).
- **Firmware** (`firmware/`): [Apache License 2.0](firmware/LICENSE).

Copies of both are in [`LICENSES/`](LICENSES). McMaster-Carr STEP models in `hardware/step/` are McMaster's and are included for reference.
