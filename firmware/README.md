# CamBox firmware

Firmware for the Cam Rotation Starter: an ESP32-C6 (a Seeed XIAO ESP32-C6 or an ESP32-C6 Super Mini, see [Board](#board)) runs a motor that turns a cam shaft: an MG90S continuous-rotation servo, or an N20 gearmotor through a DRV8833 driver (chosen at build time, see [Drive type](#drive-type)). It can be driven from a phone, from the panel controls, or by playing back stored **stories**: timelines of speed beats. The board makes its own Wi-Fi network and serves the web app itself, so no internet connection is needed.

- **Play tab:** shows what is playing and lets you drive the motor by hand, with a big Stop button and the story speed.
- **Stories tab:** a list of stories and a beat editor with a timeline preview. **Record by performing** turns slider moves into beats, and stories can be imported or exported as `.json` files.
- **Settings tab:** stop trim with live preview, motion tuning, gallery mode (autoplay, rest between loops) and Wi-Fi.

## Hardware

| Function | XIAO pin | XIAO GPIO | Super Mini GPIO | Notes |
|---|---|---|---|---|
| Servo signal (MG90S) | D0 | 0 | 18 | 50 Hz LEDC PWM. The servo takes 5 V from the PD board, not from the board. |
| DRV8833 IN3, forward (N20) | D10 | 18 | 18 | 20 kHz LEDC PWM. Channel B of the module. |
| DRV8833 IN4, reverse (N20) | D9 | 20 | 20 | 20 kHz LEDC PWM. |
| DRV8833 nSLEEP (N20) | D8 | 19 | 19 | Held low until the firmware runs, and after the motor has been stopped for `idle_release_ms`. |
| Button (to GND) | D1 | 1 | 1 | Internal pull-up. |
| Speed knob wiper | D2 | 2 | 4 | 10 k pot. ADC1, 12 dB attenuation. XIAO: pot between 3V3 and GND. |
| Speed knob low end | GND | — | 3 | Super Mini only: the pot runs 3V3 → pot → GPIO3, and the firmware drives GPIO3 low before reading the knob. The reading is inverted (`KNOB_REVERSED`) so clockwise is still faster. |
| Direction switch FWD (to GND) | D3 | 21 | 2 | ON-OFF-ON switch, common to GND. |
| Direction switch REV (to GND) | D4 | 22 | 0 | Centre position = both open = stop. |
| User LED | on board | 15 (active low) | 15 (active high, unconfirmed) | |
| RF switch enable / antenna select | on board | 3 / 14 | none | XIAO only: driven low for the on-board antenna. The Super Mini has a fixed antenna. |

The Super Mini pins avoid USB (GPIO12, 13), UART0 (GPIO16, 17), the in-package flash and the strapping pins GPIO5, 8, 9 and 15 as inputs. The knob uses GPIO4, which is a strapping pin (MTMS), because the pot's pins go straight into the strips of GPIO4 and GPIO3 on the N20 perfboard. That's safe: on the ESP32-C6, GPIO4 only selects the JTAG source at reset, and only if the JTAG_SEL_ENABLE eFuse is burned, which it isn't by default. Otherwise its level at boot is ignored, so the wiper's position doesn't matter. The driver pins are the same GPIOs on both boards; on the Super Mini they are on the edge with the 5V, GND and 3V3 pins, and the panel pins on the other edge.

The full point-to-point wire list for the MG90S drive (W1–W11, L1–L3, J1–J2) is in [`docs/wiring.md`](../drives/mg90s/docs/wiring.md); the N20's (on a perfboard with the Super Mini) is in [`drives/n20/docs/wiring.md`](../drives/n20/docs/wiring.md). Pins are defined in `main/board.h`.

## Panel controls

| Control | Action |
|---|---|
| Button, short press | Play or pause. If nothing is playing it starts the autoplay story, or the first story. |
| Button, double press | Next story. |
| Button, hold 2 s | Stop and go to manual mode. |
| Button held while powering up (5 s, LED flickers) | Reset the Wi-Fi settings to their defaults. (Bare XIAO build: hold BOOT 10 s while running instead.) |
| Knob | Story speed (master %) while a story plays; manual speed otherwise. It can be disabled in Settings. |
| Switch FWD / centre / REV | Sets direction for everything, including web commands. Centre stops the motor and pauses a story. If the switch isn't fitted, untick "Direction switch is wired" so the box always runs forward. The Super Mini board has no switch at all (`BOARD_HAS_DIR_SWITCH` 0 in `board.h`): it runs forward and the web app hides the setting. |

### Virtual panel

The Play tab has a **Virtual panel** with the same three controls, for a box whose button, knob or switch aren't fitted yet (or for testing from a phone):

| Virtual control | Does the same as |
|---|---|
| Button: tap, double-tap, hold 2 s | Button short, double, long press |
| Knob slider, 0–100 % | The knob. It works even when "Use the speed knob" is off. |
| Reverse / Stop / Forward | The direction switch. **Only active while "Direction switch is wired" is unticked**; a wired switch always wins. It starts at Forward after every boot and isn't saved. |

The WebSocket messages behind them are `{"t":"btn","v":"short"|"double"|"long"}`, `{"t":"dir","v":-1|0|1}` and `{"t":"knob","v":0..100}`.

LED: steady while a story plays, a short blink every 2 s when ready.

## Connecting

1. Power the box. Join the Wi-Fi network **CamBox-XXXX** (open by default; the last four characters come from the board's MAC address).
2. Your phone should open the app automatically (captive portal). If it doesn't, go to `http://192.168.4.1` or `http://cambox.local`.
3. Optional: in Settings → Wi-Fi, add your own network (or several: school, home, a phone hotspot) so the box joins the first one it can reach. Then use `http://cambox.local` from that network. Or do it over USB with the serial console (below).

## Serial console

The board's USB-C port carries the log and a command prompt (`cambox>`), so Wi-Fi and diagnostics work from a computer without the web app. Open the port at 115200 baud with any serial terminal, for example:

```sh
uvx --from pyserial pyserial-miniterm --eol CRLF COM3 115200    # or idf.py -p PORT monitor
```

| Command | What it does |
|---|---|
| `status` | Board, drive, uptime, free heap, access point, home network (IP, signal), knob, button, direction, motor speed and duty, player |
| `wifi`, `wifi list` | The saved networks (up to 4), numbered in the order they're tried. `*` marks the one joined now. |
| `wifi scan` | List the networks the board can hear. It's 2.4 GHz only, so a 5 GHz-only network won't appear. |
| `wifi add <name> [password]` | Save a network at the end of the list, or change the password of one already saved. Quote names with spaces: `wifi add "Dog Den" secret`. |
| `wifi use <n\|name>` | Make that network first and join it now |
| `wifi join <name> [password]` | `wifi add`, then `wifi use` |
| `wifi remove <n\|name>`, `wifi forget` | Remove one saved network, or all of them |
| `ap <name> [password]`, `ap default` | Rename the CamBox access point or give it a password (8+ characters). Then `restart`. |
| `get`, `get <name>` | Show settings (the same fields as `/api/settings`, no passwords) |
| `set <name> <value>` | Change and save one setting, clamped like the web app: `set slew 200`, `set knob_enabled false` |
| `motor <-100..100>`, `stop` | Run the motor by hand, or stop it and any story |
| `play <story>` | Play a story |
| `log none\|error\|warn\|info\|debug` | Log level until the next restart. `log warn` keeps the prompt quiet while you type. |
| `restart` | Restart (applies network changes) |
| `reset-network` | All Wi-Fi settings back to defaults, like holding the button at power-up |

Changes are saved at once. `wifi use` and `wifi join` switch networks straight away if the box started with at least one network saved; otherwise, and after `wifi forget`, `ap` or `reset-network`, `restart` to apply them. Everything else takes effect immediately.

## Build and flash

You need [ESP-IDF v5.3 or newer](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/get-started/) (tested with v5.4.1) and Node 18 or newer for the web app.

```sh
# 1. Web app → data/www (gzipped)
cd web
npm install
npm run build
cd ..

# 2. Firmware + storage image
idf.py set-target esp32c6        # first time only
idf.py build
idf.py -p /dev/cu.usbmodemXXXX flash monitor
```

`idf.py flash` writes the app and the `storage` partition. The storage partition is a LittleFS image made from `data/`, so it contains the web app and the starter stories. Note that **flashing the storage partition replaces any stories saved on the box**; export the ones you want to keep first. To update only the firmware and keep stories, run `idf.py -p PORT app-flash`.

On the first build, the component manager downloads `joltwallet/littlefs` and `espressif/mdns`. The littlefs component also creates a small Python venv with `littlefs-python` to build the image.

### Drive type

The default build drives the MG90S servo. For the N20 + DRV8833 drive, build with the `sdkconfig.n20` fragment, or set **CamBox → Drive type** in `idf.py menuconfig`:

```sh
idf.py -B build/n20 -D SDKCONFIG=build/n20/sdkconfig -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.n20" build
idf.py -B build/n20 -p PORT flash
```

The N20 build uses slow-decay PWM (the off phase brakes), so speed follows duty closely and the motor stops dead at zero. It has no stop trim: the Settings tab hides the trim and the µs range, and `trim_us`/`range_us` are ignored. After `idle_release_ms` at zero the driver sleeps and the motor turns freely.

`data/www` already holds an unbundled copy of the web app, so the box works even if you skip step 1. `npm run build` replaces it with the minified, gzipped build.

### Board

The default build is for the Seeed XIAO ESP32-C6. For the ESP32-C6 Super Mini, add the `sdkconfig.supermini` fragment, or set **CamBox → Board** in `idf.py menuconfig`. It combines with either drive type:

```sh
idf.py -B build/n20-supermini -D SDKCONFIG=build/n20-supermini/sdkconfig -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.n20;sdkconfig.supermini" build
idf.py -B build/n20-supermini -p PORT flash
```

A third choice, **XIAO ESP32-C6, bare**, is for a XIAO with nothing wired to it yet (add `sdkconfig.xiao-bare`, which combines with either drive type like the others). The on-board **BOOT** button (GPIO9) is the panel button, and there is no knob or direction switch: the web app hides those settings, and the Play tab's virtual panel stands in for them. Because GPIO9 is a strapping pin, holding it at power-up enters the USB bootloader, so on this build the Wi-Fi reset is **hold BOOT for 10 s while the box is running** (the 2 s hold stops playback first; the LED flickers, then the box restarts).

Only the pin map (`main/board.h`) and the antenna set-up differ. The boot log names both: `ready (n20 drive, supermini-c6 board)`. A build for the wrong board runs, but the motor and the panel controls won't respond.

### Developing the web app

```sh
cd web
CAMBOX=http://cambox.local npm run dev     # or CAMBOX=http://192.168.4.1
```

Vite serves the app on your computer and proxies `/api` and `/ws` to the box. `npm run check` type-checks `src/main.js` with TypeScript (JSDoc types, no build step).

**Why there is no service worker:** a service worker (true offline PWA) needs HTTPS, and the box serves plain HTTP on its own network. The app has a web manifest and icon, so "Add to Home Screen" works. The box itself is the "offline" copy.

## Stories

A **playlist** (`/data/playlists/<name>.json`) is an ordered list of up to 16 story names. Play one with `POST /api/play {"playlist":"<name>"}` or from the Stories tab. It plays each story once, in order, then starts again; stories missing from the box are skipped. The loop flag and rest settings don't apply inside a playlist. Double-press or `POST /api/next` skips to the next story, and Stop ends the playlist. In the editor, changing a story's or playlist's name before Save saves a copy and leaves the original alone.

Stories are stored as JSON files in `/data/stories/<name>.json`. Names are 1–31 characters from `A–Z a–z 0–9 _ -`. Each story holds up to 64 steps.

```json
{
  "name": "demo",
  "loop": true,
  "steps": [
    { "label": "Wake up",      "speed": 20,  "ms": 3000, "ramp": 300 },
    { "label": "Storm builds", "speed": 70,  "ms": 4000, "ramp": 800 },
    { "label": "Wind turns",   "speed": -40, "ms": 3000, "ramp": 600 }
  ]
}
```

- `speed`: −100 to 100 (percent of full speed, negative = reverse). It is scaled by the master/knob percentage and flipped by the direction switch.
- `ramp`: ms to glide from the previous speed (default 300).
- `ms`: ms to hold the speed after the ramp.

## HTTP API

| Method | Path | Body / result |
|---|---|---|
| GET | `/api/status` | Player status plus `net`, `drive` (`"mg90s"` or `"n20"`), `trim_us`, `free_heap`. The motor output is `pulse_us` (servo) or `duty` (N20, −100..100 %). |
| GET | `/api/settings` | Settings. Passwords are not returned; `ap_pass_set` / `sta_pass_set` say whether one is stored. |
| POST | `/api/settings` | Any subset of the settings fields. Returns the settings plus `restart_needed`. |
| POST | `/api/wifi/use` | `{"ssid":"school"}`: moves that saved network to the top of `networks` and joins it now. Returns the settings plus `restart_needed` (true if the box started with no network saved). |
| POST | `/api/restart` | Restarts the box. |
| GET | `/api/stories` | `["demo", …]` |
| GET / PUT / DELETE | `/api/stories/<name>` | Story JSON |
| GET | `/api/playlists` | `["both", …]` |
| GET / PUT / DELETE | `/api/playlists/<name>` | Playlist JSON: `{"name":"both","stories":["demo","fwd-pause-rev"]}` (1–16 story names) |
| POST | `/api/play` | `{"name":"demo"}` |
| POST | `/api/pause`, `/api/stop`, `/api/next` | |

**WebSocket `/ws`:** the box pushes status JSON every 200 ms. Messages the client can send:

| Message | Effect |
|---|---|
| `{"t":"speed","v":-100..100}` | Manual mode at that speed. |
| `{"t":"stop"}` | Stop. |
| `{"t":"master","v":0..100}` | Set the story speed. |
| `{"t":"trim","v":1300..1700}` | Preview a stop pulse. |
| `{"t":"trim_end"}` | End the preview. |

### Settings fields

| Field | Default | Meaning |
|---|---|---|
| `trim_us` | 1500 | Pulse at which this servo stands still. MG90S only. |
| `range_us` | 400 | Pulse offset at ±100 % speed. MG90S only. |
| `slew` | 150 | Maximum change in speed, in % per second (smoothing). |
| `idle_release_ms` | 1500 | Stop sending pulses after this long at zero, so the servo doesn't creep or hum. On the N20 the driver sleeps instead. 0 = never. |
| `knob_enabled`, `switch_installed` | true; `switch_installed` is false on the Super Mini | Whether the panel controls are fitted. On a board without a direction switch (`BOARD_HAS_DIR_SWITCH` 0 in `main/board.h`: the Super Mini, for the N20 rev C build) `switch_installed` is always false, whatever was saved or posted; GPIO2 and GPIO0 aren't set up or read, direction is forward, and `/api/settings` reports `has_dir_switch: false` so the web app hides the checkbox. On the XIAO, untick it if the switch isn't fitted: with the switch absent both pins read as its centre (stop) position. |
| `autoplay`, `autoplay_story` | false, "" | Gallery mode: play this story when powered on. |
| `rest_every`, `rest_ms` | 0, 5000 | Pause for `rest_ms` after every N loops. |
| `ap_ssid`, `ap_pass` | "" | The box's own network. Blank name = CamBox-XXXX; a password under 8 characters = open network. |
| `networks` | [] | Up to 4 networks to join as well, `[{"ssid","pass"}]`, tried in order (3 attempts each). The API never returns passwords, only `pass_set`; an entry sent without `pass` keeps its saved password. A single `sta_ssid` / `sta_pass` from an older save becomes the first entry. |
| `hostname` | cambox | mDNS name (`cambox.local`). |

## Layout

```
main/          firmware (C, ESP-IDF)
  app_main.c   start-up order, LED, Wi-Fi reset
  board.h      pin maps (XIAO ESP32-C6, ESP32-C6 Super Mini)
  servo.c      MG90S: LEDC PWM, slew, idle release, trim preview
  motor_drv8833.c  N20: two-channel LEDC PWM to the DRV8833, slew, sleep on idle
  controls.c   button / knob / switch → events
  player.c     manual + story playback, gallery rests
  storage.c    LittleFS, story files
  settings.c   NVS JSON blob
  net.c        AP (+ optional STA), mDNS; dns_server.c = captive DNS
  web.c        REST, WebSocket, static files (prefers .gz)
  console.c    serial commands on the USB-C port (esp_console REPL)
web/           Vite app (vanilla JS, no framework)
tools/pack_web.mjs   gzip web/dist → data/www
data/          LittleFS image contents: www/ and stories/
partitions.csv 4 MB: 1.9 MB app, 2 MB storage
```
