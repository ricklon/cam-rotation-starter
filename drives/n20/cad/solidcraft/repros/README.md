# SolidCraft 0.26.1: boolean and chamfer repros

Found while rebuilding the N20 motor mount of the Cam Rotation Starter, an open-hardware project, from its Fusion history: the mount and adapters for an N20 gearmotor. The design files are CERN-OHL-P-2.0.

Each `.json` file is a standalone `solidcraft-cli` script with no external files, except that 06 has a STEP file to compare against. Units are millimetres. Run one with:

```
solidcraft-cli run 01_pd_relief_cut_no_result.json --results --json-out out.json
```

Repros 01–04 rebuild the mount up to the failing feature (250–310 commands, about 100 s each), then run that feature. They measure the body before and after it. The geometry up to that point matches Fusion's: the volume before each failing feature agrees with Fusion's to within 0.5 mm³ out of about 47,000 mm³.

Checked on solidcraft-cli 0.26.1, Windows 11, 2026-10-29.

| File | What happens | Expected |
|---|---|---|
| `01_pd_relief_cut_no_result.json` | A 29.3 × 9.3 mm pocket, 1.9 mm deep, cut from the wall's outer face (plane y = −20.675, cut toward −y) into a pad joined on that face, which already has a seat cut 1.9–3.5 mm down. Fails: `boolean Cut: the operation failed: no result`. A plain `solid.box` cut of the same region fails the same way; a box cut elsewhere on the body works. | Fusion: 47450.92 mm³ after the cut. |
| `02_coaxial_slot_implausible_volume.json` | A Ø1.1 mm hole cut along the axis of an existing Ø2.9 mm channel, 0.2 mm below a flat face. Fails: `boolean Cut: implausible result volume 47464.26` (the body was 47433 mm³, so the cut would have added material). Cutting the slot first and the channel second fails the same way. | Fusion: 47432.28 mm³. |
| `03_socket_cut_no_result.json` | A 15.4 × 13.4 mm rectangular pocket cut 13.4 mm up through several layers that were joined earlier (faces at z = −10.4, −6.4, −3.5, 0 and 2). Fails: `no result`. | Fusion: 52047.18 mm³. |
| `03b_socket_as_box_no_result.json` | The same pocket as one `solid.box` cut. Fails the same way. | Same as 03. |
| `03c_socket_one_layer_passes.json` | One layer of that pocket (z −3.5 to 0) as a box cut. **Passes.** Every single layer passes alone. | — |
| `03d_socket_two_layers_second_fails.json` | The pocket as two box cuts, z −10.9 to −3.5 and then −3.5 to 2.5. The first passes; the second, which meets the first one's face, fails. Overlapping the second into the first by 0.05 mm fails too. | Same as 03. |
| `04_pd_pad_join_coincident_faces.json` | Earlier in the same mount, rebuilt in a different order (the socket cut from the original body, then the N20 parts joined in pre-cut): a 28.7 × 12.1 × 3.5 mm pad joined onto the wall face. Fails: `boolean Union: no result; coincident faces were left in place: pushing them apart would change the result; planar boolean beside curved faces: a piece in several parts`. The same join passes in the original order. | Adds 1215.4 mm³, the pad's volume. |
| `05_taper_cut_ending_on_hole_wall.json` | Small standalone case (6 commands): a 10 mm cube with a Ø4 hole, and a 45° taper cut (countersink) from a Ø6 circle on the top face, 1 mm deep, so its bottom circle lies exactly on the hole wall. Fails: `no result`. | 1000 − 40π − (19π/3 − 4π) ≈ 867.0 mm³ (cube, less the hole, less the part of the countersink outside the hole). |
| `06_countersink_chamfer_volume.json` + `06_fusion_Adapter_025.step` | Not an error, but a geometry difference: a 1.3 mm chamfer on the edges where two Ø3.1 mm cross holes meet the Ø12.6 mm outer cylinder (a saddle-shaped edge). SolidCraft removes 37.10 mm³; Fusion removes 21.32 mm³. Every other step of this part agrees within 0.5 mm³ (volumes after each feature are measured in the script). The Fusion STEP sits 4.2 mm up in z in the assembly: `matrix: [1,0,0,0, 0,1,0,0, 0,0,1,4.2]`. | Fusion: 2269.08 mm³ after the chamfer. Final part: 2244.85 mm³; SolidCraft gives 2228.87 mm³. |
| `07_clamp_build_panics.json` | Builds and gives the right volume, but prints three Rust panics to stderr during the build: `truck-geotrait-0.4.0/src/algo/curve.rs:78:5: tolerance must be no less than 1e-6`. | No panics. |

## Workarounds that got these through

These may help find the cause:

- **01:** cut the relief straight after the pad is joined, before the ledge and the seat.
- **02:** extrude the channel and the slot into one new tool body, then a single `solid.combine` cut.
- **03:** none found while the layers are joined. Cutting the socket from the body before the layers are joined works; that order is what leads to 04.
- **05:** stop the taper 0.1 mm past the hole wall (the extra tip lies inside the hole).

## What 0.26.1 fixed since 0.20

Small standalone tests of these 0.20 failures now pass:
- joining a ring onto a body whose bore matches the ring's inner face
- a cut that splits a body into two
- a 45° chamfer round a rounded-rectangle flange, picked at edge midpoints
- a D-shaped fill whose arc lies on an existing bore wall
