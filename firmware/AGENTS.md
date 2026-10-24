# AGENTS.md: notes for coding agents working on CamBox

Read README.md first. It covers the pins, panel behaviour, the API and the story format, and it is the user-facing contract. Keep it in sync with any change.

## Project shape
- ESP-IDF ≥ 5.3, target **esp32c6** (Seeed XIAO ESP32-C6, 4 MB flash). Plain C, no Arduino.
- Managed components: `joltwallet/littlefs` and `espressif/mdns` (declared in `main/idf_component.yml`).
- Web app in `web/`: Vite with vanilla ES modules, one `src/main.js` and one `src/style.css`. No framework, and keep it that way; the gzipped bundle must stay small (it ships in a 2 MB LittleFS partition and is served by a tiny HTTP server).
- `data/` becomes the LittleFS image (`littlefs_create_partition_image` in `main/CMakeLists.txt`). `npm run build` (in `web/`) regenerates `data/www` as `.gz` files via `tools/pack_web.mjs`.

## Invariants
- **Init order in `app_main.c` matters:** `player_init()` must run before `controls_init()`, because control callbacks call into the player.
- **The servo is continuous-rotation.** "Position" means nothing; speed is −100..100, and pulse = `trim_us + speed/100 * range_us`. Every command goes through `servo_set_target()`, which applies slew. Never write LEDC directly from elsewhere.
- **Two drive types, one interface.** `CONFIG_CAM_DRIVE_N20` (menuconfig → CamBox, or `sdkconfig.n20`) builds `motor_drv8833.c` instead of `servo.c`; both implement `servo.h`. Keep them in step: a change to slew or idle release in one belongs in the other. Build both before committing.
- **The direction switch is applied in the player** (`target = cmd * dir`). It affects web commands too, on purpose: the physical switch is the safety override.
- **Settings live in NVS as one JSON blob** (namespace `cambox`, key `cfg`). `settings_apply_json()` clamps every field. Add new fields in `settings.h`, in the defaults, and in both `settings_to_json` and `settings_apply_json`. Never return passwords from the API; only `*_pass_set` flags.
- **Story names** match `^[A-Za-z0-9_-]{1,31}$`. This is enforced in both `storage.c` and the UI. Stories are capped at 64 steps (`STORY_MAX_STEPS`) and 32 KB.
- **Static files:** `web.c` serves `<path>.gz` with `Content-Encoding: gzip` when it exists, otherwise `<path>`. Unknown paths fall back to `index.html`, which also catches captive-portal probes. Files under `/assets/` get immutable caching (Vite hashes their names).
- **No service worker:** the box serves plain HTTP. Don't add one.
- **UI must not use `alert/confirm/prompt`:** delete uses a two-tap confirm. Keep controls large enough to use one-handed on a phone.

## Checking work
```sh
idf.py build                          # firmware; there should be no warnings in main/
cd web && npm run check               # tsc --checkJs over src/main.js
cd web && npm run build               # bundle + gzip into ../data/www
```
For UI changes, run `npm run dev` with `CAMBOX=http://<box>` against real hardware, or mock `/api/*` and `/ws` (Playwright `page.route` + `page.route_web_socket` works well). Test at 390 px wide.

## Hardware context
The mechanical design (Fusion 360: "Cam Rotation Starter") mounts an MG90S-class 360° servo on the outside of a box wall. It drives 1/4" or 3/4" dowel camshafts through printed adapters. The power path is a USB-C PD trigger set to 5 V, then a Schottky diode and a bulk capacitor, then the servo and the XIAO's 5V pin. A long story at high speed can stall a small servo on a heavy cam, so be conservative about defaults that increase torque demand (e.g. a higher `slew`).

This project is for teaching students to tell stories with cams and found objects, and for automating Sonia Yaco's hand-cranked cam pieces. Simple, legible behaviour beats clever behaviour.
