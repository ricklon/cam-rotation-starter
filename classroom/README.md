# In the classroom

The Cam Rotation Starter turns a wooden shaft through the wall of a box. Students fit cams to the shaft inside the box and attach found objects to followers that ride on the cams. As the shaft turns, the objects rise, fall, nod and spin, and together they tell a story. The drive unit sits on the outside of the box, so the inside is entirely the students' stage.

## Three ways to drive it

| Control | Who it's for | What it does |
|---|---|---|
| **Button** (outer face) | Anyone, including gallery visitors | Short press: play or pause. Double press: next story. Hold 2 s: stop. |
| **Knob and switch** (side face) | Students testing their mechanism | The knob sets the speed; the switch sets forward, stop (centre) or reverse. |
| **Phone or laptop** | Students writing stories | Join the box's Wi-Fi (CamBox-XXXX) and open the app. |

## Stories

A story is a list of **beats**. Each beat says how fast to turn, for how long, and how quickly to get there:

```json
{ "label": "Storm builds", "speed": 70, "ms": 4000, "ramp": 2500 }
```

- **speed**: −100 to 100. Negative turns backwards; 0 stops.
- **ms**: how long to hold that speed, in milliseconds (4000 = 4 seconds).
- **ramp**: how long to glide there from the previous beat. Long ramps feel calm; short ramps feel sudden.

The story speed (the knob, or the slider in the app) scales every beat, so one story can be played slow or fast without rewriting it.

### Ways to make one

- **Write it** in the app's Stories tab: add beats, name them after what happens in the story, and watch the timeline preview.
- **Perform it**: press Record and move the slider the way you'd turn a crank by hand. Each time you settle on a speed, it becomes a beat.
- **Share it**: export a story as a `.json` file and import it on another box.

## Example stories

The files in [`stories/`](stories) import straight into the app:

| Story | Feel | Length |
|---|---|---|
| [`storm.json`](stories/storm.json) | Wakes up, builds, stops dead, reverses, rests | 22 s, loops |
| [`tide.json`](stories/tide.json) | Slow forward and back, like water | 22 s, loops |
| [`heartbeat.json`](stories/heartbeat.json) | Two quick pulses and a rest | 2 s, loops |

## Gallery mode

For an exhibition, set a story to play automatically when the box is powered on, and optionally rest for a few seconds every so many loops (Settings → Gallery mode). Visitors can still pause and restart it with the button.

## Tips

- Set the **stop trim** first (Settings → Stop trim) so the shaft stands still at speed 0.
- Small servos are gentle. Light objects, short followers and smooth cams work best; if the shaft stalls, slow the story down or lighten the load.
- The direction switch's centre position stops everything, whatever is playing. It's the safe position for adjusting the mechanism.
