# Cam Rotation Starter: fasteners

Every fastener in the Fusion model, with its source and fit. McMaster STEP files are in [`step/`](step).

| Where | Part | Spec | Qty | Source | Fit in the model |
|---|---|---|---|---|---|
| Mount to box wall | McMaster **98685A950** | Brass Phillips rounded head wood screw, #14 × 3/4" (head 0.457" dia × 0.17") | 2 | McMaster | Through the 7 mm ear holes; 10.9 mm into a 1/2" wall, tip 1.8 mm short of the inside face. Pilot 7/64" softwood, 9/64" hardwood. |
| Under the wall screws | McMaster **98026A029** | USS flat washer, 1/4" screw, 0.312" ID × 0.734" OD × 0.08" | 2 | McMaster | Flush under each ear. |
| 3/4" and 5/8" adapters to dowel | McMaster **90031A108** | Zinc Phillips flat head wood screw, #4 × 3/8" | 2 per adapter | McMaster | Countersunk 3.1 mm holes on opposite sides at 11 and 19 mm up; about 6.3 mm into the dowel. |
| 1/4" adapter to dowel | McMaster **92114A106** | Brass Phillips flat head wood screw, #4 × 1/4" | 2 | McMaster | Same holes; reaches the dowel's centre line. |
| N20 drive: adapters and clamps to the motor shaft | **M3 × 5 set screw** (ISO 4029, cup point) | 1.5 mm hex socket | 1 per adapter | Any | Radial, 2.8 mm above the D-bore end, at right angles to the flat: threads into a 2.5 mm pilot from the bore out to 5.6 mm radius, reached through a 3.4 mm hole. Its top sits below the surface on every adapter. |
| N20 drive: sanded-dowel clamps | **M3 × 8 socket cap screw** (ISO 4762), thread-forming or plain | Head 5.5 mm dia × 3 mm | 2 per clamp | Any | Through a 3.4 mm hole in the cap; the head sits flat in a 6.0 mm counterbore 2.0 mm above the split. Threads 5.25 mm into a 2.5 mm pilot, 6.5 mm deep, in the fixed half. Not M3 × 10: on the 5/8 clamp the tip would come out of the side. |
| N20 drive: cartridge to mount (optional since the snap fit) | **M3 × 8 socket cap screw** | as above | 2 | Any | Up through 3.4 mm holes in the cartridge's 2.4 mm flange into 2.5 mm pilots in the 6.9 mm floor (5.6 mm of thread). |
| N20 drive: motor to cartridge (optional since the snap fit) | **M1.6 × 4 pan head** + **M1.6 split lock washer** | Head about 3.0 × 1.0 mm; washer about 3.2 mm OD × 0.5 mm | 2 + 2 | Any (often with the motor) | Through 1.9 mm holes in the 3 mm cartridge plate, heads and washers flush in 3.8 mm × 1.5 mm counterbores; 2 mm into the gearbox face's threaded holes, 9 mm apart. |
| MG90S drive: sanded-dowel clamps (5/8", 1/4") | McMaster **90485A442** ([STEP](step/90485A442_M3x10_Flat_Head_Thread_Forming.step)) | M3 × 10 flat head thread-forming screw for plastic, 90° head 5.47 mm dia | 2 per clamp | McMaster | Through a 3.4 mm hole in the cap into a 5.8 mm countersink (1 mm below the surface on the 5/8 clamp); forms its thread in a 2.5 mm pilot, 7.5 mm deep, in the fixed half (5–6.5 mm of engagement). |
| Control pod to mount | M3 × 8 wafer head | Machine screw, self-taps into the printed pilot | 4 | Your Positron printer kit (M3 × 6 also fits) | 2.6 mm pilots, 5.6 mm deep, 0.45 mm floor. |
| Servo tabs | MG90S stock screws | Self-tapping, about M2 × 6–8 | 2 | Comes with servo | Placeholder bodies. |
| Adapter to servo spline | MG90S stock horn screw | **Self-tapping M2 × 4, pan head about 4 × 1.4 mm** (typical; not published by Tower Pro) | 1 | Comes with servo | Modeled as *Horn_Screw_M2x4*: head in the 4.5 mm counterbore, about 2.3 mm into the spline (5–6 threads). |

## Check the horn screw when the servo arrives

Tower Pro doesn't publish the horn screw size, and clones vary (M2 × 4 is common, M1.7 × 3 turns up too). Measure it with calipers and update these Fusion parameters; the model rebuilds:

- `hs_thread`: thread diameter (2.0 mm)
- `hs_len`: length under the head (4.0 mm)
- `hs_head_dia`, `hs_head_h`: head size (4.0 × 1.4 mm)

If it's shorter than about 4 mm under the head, it will only catch 1–2 threads of the spline; use an M2 × 5 pan-head screw instead. Driving it needs a long, thin #0 or #1 Phillips, because the head sits at the bottom of the 18 mm socket.

## Panel parts

- **S1 push button:** 12 mm stainless momentary, 1 NO, IP65, pre-wired with 150 mm leads (Amazon B09BKWMNJ9). It mounts in a 12.2 mm hole in the pod's outer face (the face that looks straight out from the box wall), with its nut inside the pod. The nut has only 0.2 mm to spare against the side wall, so line its flats up with the walls and tighten it by hand before screwing the pod to the mount.

- **J3 panel connector (N20 drive):** JST-PH 2.0 mm, 6 pins: a B6B-PH-K-S header held in the pod, and a PHR-6 housing on the mount harness. Pin 1 is nearest the pod's floor; the pinout is in [the N20 wiring page](../drives/n20/docs/wiring.md#j3-the-control-panel-connector).

## Not fasteners, but in the build

- 2–4 small zip ties (2.5–3.5 mm wide) hold the PD board and XIAO in their pockets. Not modeled.
- Nuts and washers for the pot, switch and button come with those parts and are modeled.
