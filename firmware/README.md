# CamBox firmware

Firmware for the Cam Rotation Starter: a Seeed XIAO ESP32-C6 runs a continuous-rotation servo that turns a cam shaft. It can be driven from a phone, from the panel controls, or by playing back stored **stories**: timelines of speed beats. The board makes its own Wi-Fi network and serves the web app itself, so no internet connection is needed.

- **Play tab:** shows what is playing and lets you drive the servo by hand, with a big Stop button and the story speed.
- **Stories tab:** a list of stories and a beat editor with a timeline preview. **Record by performing** turns slider moves into beats, and stories can be imported or exported as `.json` files.
- **Settings tab:** stop trim with live preview, motion tuning, gallery mode (autoplay, rest between loops) and Wi-Fi.

## Hardware

| Function | XIAO pin | GPIO | Notes |
|---|---|---|---|
| Servo signal | D0 | 0 | 50 Hz LEDC PWM. The servo takes 5 V from the PD board, not from the XIAO. |
| Button (to GND) | D1 | 1 | Internal pull-up. |
| Speed knob wiper | D2 | 2 | 10 k pot between 3V3 and GND. ADC, 12 dB attenuation. |
| Direction switch FWD (to GND) | D3 | 21 | ON-OFF-ON switch, common to GND. |
| Direction switch REV (to GND) | D4 | 22 | Centre position = both open = stop. |
| User LED | on board | 15 | Active low. |
| RF switch enable / antenna select | on board | 3 / 14 | Driven low: on-board antenna. |

The full point-to-point wire list (W1–W11, L1–L3, J1–J2) is in [`docs/wiring.md`](../docs/wiring.md). Pins are defined in `main/board.h`.

## Panel controls

| Control | Action |
|---|---|
| Button, short press | Play or pause. If nothing is playing it starts the autoplay story, or the first story. |
| Button, double press | Next story. |
| Button, hold 2 s | Stop and go to manual mode. |
| Button held while powering up (5 s, LED flickers) | Reset the Wi-Fi settings to their defaults. |
| Knob | Story speed (master %) while a story plays; manual speed otherwise. It can be disabled in Settings. |
| Switch FWD / centre / REV | Sets direction for everything, including web commands. Centre stops the motor and pauses a story. If the switch isn't fitted, untick "Direction switch is wired" so the box always runs forward. |

LED: steady while a story plays, a short blink every 2 s when ready.

## Connecting

1. Power the box. Join the Wi-Fi network **CamBox-XXXX** (open by default; the last four characters come from the board's MAC address).
2. Your phone should open the app automatically (captive portal). If it doesn't, go to `http://192.168.4.1` or `http://cambox.local`.
3. Optional: in Settings → Wi-Fi, enter your own network so the box joins it too. Then use `http://cambox.local` from that network.

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

`data/www` already holds an unbundled copy of the web app, so the box works even if you skip step 1. `npm run build` replaces it with the minified, gzipped build.

### Developing the web app

```sh
cd web
CAMBOX=http://cambox.local npm run dev     # or CAMBOX=http://192.168.4.1
```

Vite serves the app on your computer and proxies `/api` and `/ws` to the box. `npm run check` type-checks `src/main.js` with TypeScript (JSDoc types, no build step).

**Why there is no service worker:** a service worker (true offline PWA) needs HTTPS, and the box serves plain HTTP on its own network. The app has a web manifest and icon, so "Add to Home Screen" works. The box itself is the "offline" copy.

## Stories

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
| GET | `/api/status` | Player status plus `net`, `trim_us`, `free_heap` |
| GET | `/api/settings` | Settings. Passwords are not returned; `ap_pass_set` / `sta_pass_set` say whether one is stored. |
| POST | `/api/settings` | Any subset of the settings fields. Returns the settings plus `restart_needed`. |
| POST | `/api/restart` | Restarts the box. |
| GET | `/api/stories` | `["demo", …]` |
| GET / PUT / DELETE | `/api/stories/<name>` | Story JSON |
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
| `trim_us` | 1500 | Pulse at which this servo stands still. |
| `range_us` | 400 | Pulse offset at ±100 % speed. |
| `slew` | 150 | Maximum change in speed, in % per second (smoothing). |
| `idle_release_ms` | 1500 | Stop sending pulses after this long at zero, so the servo doesn't creep or hum. 0 = never. |
| `knob_enabled`, `switch_installed` | true | Whether the panel controls are fitted. |
| `autoplay`, `autoplay_story` | false, "" | Gallery mode: play this story when powered on. |
| `rest_every`, `rest_ms` | 0, 5000 | Pause for `rest_ms` after every N loops. |
| `ap_ssid`, `ap_pass` | "" | The box's own network. Blank name = CamBox-XXXX; a password under 8 characters = open network. |
| `sta_ssid`, `sta_pass` | "" | Optional network to join as well. |
| `hostname` | cambox | mDNS name (`cambox.local`). |

## Layout

```
main/          firmware (C, ESP-IDF)
  app_main.c   start-up order, LED, Wi-Fi reset
  board.h      pin map
  servo.c      LEDC PWM, slew, idle release, trim preview
  controls.c   button / knob / switch → events
  player.c     manual + story playback, gallery rests
  storage.c    LittleFS, story files
  settings.c   NVS JSON blob
  net.c        AP (+ optional STA), mDNS; dns_server.c = captive DNS
  web.c        REST, WebSocket, static files (prefers .gz)
web/           Vite app (vanilla JS, no framework)
tools/pack_web.mjs   gzip web/dist → data/www
data/          LittleFS image contents: www/ and stories/
partitions.csv 4 MB: 1.9 MB app, 2 MB storage
```
