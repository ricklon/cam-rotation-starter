"""Cam Rotation N20 - point-to-point wiring (matches drives/n20/docs/wiring.md: W1, W2, W4-W15, W6p-W11p, L1, L2, J1, J2, C2, D1 leads).
Rebuilds everything inside the Wiring component. Coordinates in mm, world frame.
U1 terminal: + entry at z 22.9, GND at z 17.9 (-X face of the terminal block, x -6), about y -27 (the board sits on a 3.5 mm pad).
XIAO (USB to +X, viewed from -Y): top row z 9.9 = D0..D6 from x 19.1 leftward; bottom row z -5.4 = 5V, GND, 3V3, D10, D9, D8, D7 from x 19.1.
DRV8833 module under the base, component side down (z -11.0), pins on a 2.54 mm pitch from x 11.8:
  IN row  (y -7.62, toward the XIAO side): IN4 11.8, IN3 14.34, GND 16.88, VCC 19.42, IN2 21.96, IN1 24.5
  OUT row (y +7.62): EEP 11.8, OUT1 14.34, OUT2 16.88, OUT3 19.42, OUT4 21.96, ULT 24.5
  Channel B is used (IN3/IN4 -> OUT3/OUT4): IN3, IN4, GND and VCC sit under the XIAO's D10, D9, GND and 5V pads.
D1 lies in a channel under the base at x 19.42 (axis z -10.6, cathode toward the wall); its own leads run to the XIAO 5V pad and DRV VCC.
N20 terminals at x +/-3.1, y 0, tabs down to z -26.9. The N20 sits in a cartridge under the base; its snap arms run down the
motor's flats (y +/-5.3..6.8) to z -26.6, so the motor wires stay at |y| < 4.5 below the flange.
Control panel: JST-PH 6-pin header in the pod (pins at x -6.75, z 17.9..27.9, tails end at y 38.1); the mount harness ends in the plug at y 22.9.
"""
import adsk.core, adsk.fusion, math, traceback

COLORS = {
    'red': (200, 30, 30), 'black': (28, 28, 28), 'brown': (115, 62, 28), 'orange': (240, 130, 20),
    'white': (238, 238, 230), 'yellow': (240, 205, 40), 'violet': (130, 60, 180), 'green': (40, 150, 60),
    'blue': (40, 90, 205), 'gray': (120, 120, 120), 'tan': (214, 184, 130), 'tin': (195, 195, 200),
}

# name, color, diameter mm, waypoints (mm)
BUNDLE_TRUNK = ('Control harness to J3 (6 x 30 AWG)', 'gray', 2.0, 'raw', [
    (-6.75, 22.9, 23.0), (-6.75, 22.0, 21.6), (-6.9, 21.8, 18.0), (-7.0, 21.8, 4.0), (-7.6, 21.8, 0.0), (-10.0, 21.8, -2.2),
    (-11.3, 20.7, -2.6), (-11.5, 19.4, -4.2), (-11.6, 19.4, -6.4),
    (-11.8, 19.3, -8.4), (-12.0, 18.0, -9.35), (-12.0, 5.0, -9.35), (-12.0, -15.0, -9.35), (-12.0, -18.2, -9.3),
    (-11.9, -19.2, -8.2), (-11.6, -19.2, -6.0), (-11.2, -19.2, 2.0), (-11.0, -19.2, 9.0), (-11.0, -20.4, 11.2),
    (-10.4, -21.9, 12.6), (-8.0, -22.3, 13.1), (-6.0, -22.3, 13.2)])

# XIAO pad / DRV pin helpers
def xb(x): return (x, -21.0, -5.4)          # XIAO bottom-row pad
def drv_in(x): return (x, -7.62, -11.3)     # DRV IN-row joint, component side
def drv_out(x): return (x, 7.62, -11.3)     # DRV OUT-row joint

def drop(x, zlane, ylane=-21.3):
    """from a XIAO bottom-row pad straight down the wall face and round its bottom edge"""
    return [xb(x), (x, ylane, -6.6), (x, ylane, -10.0), (x, -21.0, zlane), (x, -19.5, zlane)]

WIRES = [
    # --- power: U1 down the groove, along under the -Y wall, up to the DRV8833's GND and VCC
    ('W1 U1 + to DRV VCC', 'red', 0.8, [(-6.0, -27.1, 22.9), (-11.0, -23.4, 22.9), (-13.4, -22.6, 22.9), (-13.8, -19.2, 19.5),
        (-13.8, -19.2, -8.0), (-13.8, -19.2, -11.2), (-12.6, -18.2, -11.3), (17.2, -18.2, -11.3), (17.4, -16.0, -11.4),
        (17.4, -10.4, -11.8), (18.6, -8.6, -11.6), drv_in(19.42)]),
    ('W2 U1 GND to DRV GND', 'black', 0.8, [(-6.0, -27.1, 17.9), (-11.0, -23.4, 17.9), (-15.2, -22.2, 17.9), (-15.2, -20.4, 16.0), (-14.9, -19.2, 14.0),
        (-14.9, -19.2, -8.0), (-14.9, -18.4, -11.3), (-13.6, -16.8, -11.3), (15.0, -16.8, -11.3), (15.4, -14.6, -11.4),
        (15.6, -10.4, -11.8), (16.4, -8.6, -11.6), drv_in(16.88)]),
    ('W4 XIAO GND to DRV GND', 'black', 0.8, [xb(16.56), (16.56, -21.4, -6.8), (16.56, -21.4, -9.6), (16.56, -21.1, -11.6), (16.56, -19.4, -12.6), (16.56, -16.0, -12.6), (16.7, -10.2, -12.4), (16.88, -8.6, -11.8), drv_in(16.88)]),
    ('L1 C1 + to U1 +', 'red', 0.8, [(-2.75, -19.78, -3.1), (-2.75, -21.6, -3.15), (-2.75, -23.4, -3.6), (-2.75, -25.4, -5.0), (-4.0, -26.2, -5.6), (-11.0, -26.2, -5.6),
        (-12.0, -25.2, -4.4), (-12.0, -25.2, 21.5), (-10.6, -24.8, 22.9), (-6.0, -27.9, 23.1)]),
    ('L2 C1 - to U1 GND', 'black', 0.8, [(-6.25, -19.78, -3.1), (-6.3, -21.6, -3.15), (-6.3, -23.4, -3.8), (-6.4, -25.2, -4.8), (-9.6, -25.2, -4.8),
        (-9.6, -25.2, 16.4), (-8.8, -25.0, 17.9), (-6.0, -28.1, 17.7)]),
    # --- D1's own leads: cathode up the wall face to the XIAO 5V pad, anode along the channel to DRV VCC
    ('D1 cathode lead to XIAO 5V', 'tin', 0.8, [(19.42, -21.2, -10.6), (19.4, -21.3, -9.6), (19.1, -21.25, -6.6), (19.1, -21.0, -5.4)]),
    ('D1 anode lead to DRV VCC', 'tin', 0.8, [(19.42, -11.6, -10.6), (19.42, -10.4, -11.3), (19.42, -9.0, -11.5), drv_in(19.42)]),
    # --- XIAO bottom row to the DRV8833: straight down and under
    ('W5 XIAO D10 to DRV IN3', 'orange', 0.6, drop(11.48, -12.4) + [(11.6, -16.0, -12.4), (13.6, -11.6, -12.2), (14.34, -9.0, -11.8), drv_in(14.34)]),
    ('W12 XIAO D9 to DRV IN4', 'brown', 0.6, drop(8.94, -12.4) + [(9.2, -16.0, -12.4), (11.2, -11.6, -12.2), (11.8, -9.0, -11.8), drv_in(11.8)]),
    ('W13 XIAO D8 to DRV EEP', 'gray', 0.6, drop(6.4, -12.4) + [(8.6, -16.4, -12.4), (9.0, -14.0, -11.4), (9.0, 6.4, -11.2), (10.6, 7.6, -11.6), drv_out(11.8)]),
    # --- DRV8833 outputs (channel B) to the N20 terminals: through anchor A2, then down the motor's +x side between the cartridge's snap arms,
    #     inside the arms' zip tie (x 7.5) and clear of the can (x 6 at most)
    ('W14 DRV OUT3 to motor +', 'white', 0.8, [drv_out(19.42), (19.0, 9.0, -12.6), (15.0, 10.0, -13.8), (10.4, 6.0, -14.6),
        (6.7, 1.2, -16.0), (6.7, 1.2, -24.0), (6.4, 0.8, -25.8), (5.0, 0.2, -26.5), (3.5, 0.0, -26.4)]),
    ('W15 DRV OUT4 to motor -', 'blue', 0.8, [drv_out(21.96), (21.4, 9.6, -12.8), (16.0, 11.4, -14.6), (11.4, 6.6, -15.4),
        (6.7, 2.6, -16.8), (6.7, 2.6, -24.0), (6.8, 3.2, -26.2), (0.0, 3.4, -26.3), (-3.5, 3.0, -26.3), (-3.5, 0.0, -26.4)]),
    # --- XIAO end of the control harness (it runs in the bundle to the J3 housing at the pod)
    ('W10 XIAO D4 to J3 pin 1 (rev)', 'blue', 0.6, [(-6.0, -21.9, 13.5), (-3.0, -21.5, 13.4), (7.2, -21.5, 13.4), (8.9, -21.2, 11.6), (8.9, -21.0, 9.9)]),
    ('W9 XIAO D3 to J3 pin 3 (fwd)', 'green', 0.6, [(-6.0, -22.1, 13.9), (-3.0, -22.2, 13.4), (9.7, -22.2, 13.4), (11.4, -21.4, 11.6), (11.4, -21.0, 9.9)]),
    ('W7 XIAO D2 to J3 pin 4 (speed)', 'yellow', 0.6, [(-6.0, -22.5, 13.9), (-3.0, -22.9, 13.4), (12.3, -22.9, 13.4), (14.0, -21.6, 11.6), (14.0, -21.0, 9.9)]),
    ('W6 XIAO D1 to J3 pin 6 (button)', 'white', 0.6, [(-6.0, -22.7, 13.6), (-3.0, -23.6, 13.4), (14.8, -23.6, 13.4), (16.5, -21.8, 11.6), (16.5, -21.0, 9.9)]),
    ('W8 XIAO 3V3 to J3 pin 5', 'violet', 0.6, [(-6.0, -22.6, 12.9), (-3.0, -23.2, 12.5), (-0.2, -23.2, 12.4), (0.25, -23.2, 10.5),
        (0.25, -23.2, -5.5), (1.4, -23.2, -7.2), (12.3, -23.2, -7.2), (14.0, -21.6, -6.0), (14.0, -21.0, -5.4)]),
    ('W11 XIAO GND to J3 pin 2', 'black', 0.6, [(-6.0, -23.0, 13.2), (-3.0, -23.9, 12.5), (-0.2, -23.9, 12.4), (0.25, -23.9, 10.5),
        (0.25, -23.9, -5.5), (1.4, -23.9, -7.9), (14.8, -23.9, -7.9), (16.3, -22.0, -6.2), (16.5, -21.0, -5.4)]),
]

# pod side: from the JST-PH header tails (x -6.75, y 38.1; pin 1 at z 17.9 .. pin 6 at z 27.9) to the controls.
# Pin order: 1 REV (W10), 2 GND (W11), 3 FWD (W9), 4 SPEED (W7), 5 3V3 (W8), 6 BTN (W6).
def tail(n): return (-6.75, 38.1, 17.9 + 2.0 * (n - 1))
def sw1_run(n, ylane, xlane, zrun, zlug):
    """pin tail -> drop behind the header at y ylane -> sidestep to lane x xlane -> forward over SW1 at height zrun -> down in front of the lugs"""
    z0 = 17.9 + 2.0 * (n - 1)
    pts = [tail(n), (-6.75, ylane, z0 - 1.0), (-6.75, ylane, zrun + 1.9), (xlane, ylane - 0.4, zrun + 0.5), (xlane, ylane - 3.8, zrun),
           (xlane, 30.0, zrun), (xlane, 27.0, zrun - 1.0)]
    if zlug < 8:
        pts += [(xlane, 27.0, zlug + 2.4), (-4.6, 27.0, zlug + 0.6)]
    else:
        pts += [(-4.4, 27.2, zlug + 0.6)]
    return pts + [(-4.0, 28.2, zlug)]
POD = [
    ('W10p J3 pin 1 to SW1 B (rev)', 'blue', 0.6, sw1_run(1, 38.6, -7.9, 12.4, 1.0)),
    ('W11p J3 pin 2 to SW1 center', 'black', 0.6, sw1_run(2, 39.8, -5.9, 13.1, 5.0)),
    ('W9p J3 pin 3 to SW1 A (fwd)', 'green', 0.6, sw1_run(3, 40.9, -5.1, 13.8, 9.0)),
    ('W7p J3 pin 4 to RV1 wiper', 'yellow', 0.6, [tail(4), (-6.75, 41.6, 23.9), (-6.75, 41.6, 14.6), (-4.5, 41.6, 14.2), (2.5, 40.8, 12.4), (2.5, 38.6, 11.8), (2.5, 37.2, 11.8)]),
    ('W8p J3 pin 5 to RV1 pin 3', 'violet', 0.6, [tail(5), (-6.75, 42.25, 25.9), (-6.75, 42.25, 15.4), (-4.5, 42.2, 15.0), (5.0, 41.6, 13.2), (5.0, 38.6, 13.0), (5.0, 37.2, 13.0)]),
    # 12 mm stainless button S1: its own pre-wired lead A goes to JST pin 6 over the top of the pot
    ('W6p button lead A to J3 pin 6', 'white', 0.6, [tail(6), (-6.75, 39.4, 27.9), (-5.0, 39.2, 28.8), (-1.0, 36.0, 28.8), (8.0, 33.6, 28.0), (11.0, 32.75, 22.0), (11.0, 32.75, 16.3)]),
    ('J1 button lead B to SW1 center', 'black', 0.6, [(3.0, 32.75, 15.6), (3.0, 32.5, 17.6), (0.6, 27.8, 17.4), (-1.4, 25.6, 13.0),
        (-2.2, 25.8, 8.0), (-3.4, 27.2, 5.4), (-4.0, 28.2, 5.0)]),
    ('J2 SW1 center to RV1 pin 1', 'black', 0.6, [(-3.8, 27.6, 5.6), (-3.8, 26.9, 8.0), (-3.4, 26.6, 11.0), (-2.2, 28.4, 12.9),
        (-1.6, 33.0, 13.0), (-1.0, 35.8, 13.3), (0.0, 37.2, 13.6)]),
]

app = adsk.core.Application.get()
d = adsk.fusion.Design.cast(app.activeProduct)
P = adsk.core.Point3D.create


def appearance(name):
    full = 'Wire - ' + name.title()
    a = d.appearances.itemByName(full)
    if a:
        return a
    base = d.appearances.itemByName('Plastic - Matte (Red)')
    a = d.appearances.addByCopy(base, full)
    r, g, b = COLORS[name]
    for i in range(a.appearanceProperties.count):
        p = a.appearanceProperties.item(i)
        if isinstance(p, adsk.core.ColorProperty) and p.id in ('opaque_albedo', 'generic_diffuse'):
            p.value = adsk.core.Color.create(r, g, b, 255)
    return a


def smooth(pts, k=1.0):
    """Round each corner: replace it by points k mm before/after it, so the fitted spline stays in its lane."""
    out = [pts[0]]
    for i in range(1, len(pts) - 1):
        a, b, c = pts[i - 1], pts[i], pts[i + 1]
        def towards(p, q, dist):
            v = [q[j] - p[j] for j in range(3)]; n = math.sqrt(sum(x * x for x in v)) or 1
            dist = min(dist, n / 3)
            return tuple(p[j] + v[j] / n * dist for j in range(3))
        out += [towards(b, a, k), towards(b, c, k)]
    out.append(pts[-1])
    return out


def route(comp, sk, name, color, dia, pts, raw=False):
    col = adsk.core.ObjectCollection.create()
    for p in (pts if raw else smooth(pts, max(1.2, 1.8 * dia))):
        col.add(sk.modelToSketchSpace(P(p[0] / 10, p[1] / 10, p[2] / 10)))
    sp = sk.sketchCurves.sketchFittedSplines.add(col)
    path = comp.features.createPath(sp, False)
    pi = comp.features.pipeFeatures.createInput(path, adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
    pi.sectionSize = adsk.core.ValueInput.createByReal(dia / 10)
    f = comp.features.pipeFeatures.add(pi)
    f.name = name
    for b in f.bodies:
        b.name = name
        b.appearance = appearance(color)
    return f


def cylinder(comp, bf, name, color, p1, p2, dia, opacity=1.0):
    t = adsk.fusion.TemporaryBRepManager.get()
    body = t.createCylinderOrCone(P(*[v / 10 for v in p1]), dia / 20, P(*[v / 10 for v in p2]), dia / 20)
    b = comp.bRepBodies.add(body, bf)
    b.name = name
    b.appearance = appearance(color)
    b.opacity = opacity
    return b


def run(context):
    try:
        root = d.rootComponent
        occ = [o for o in root.occurrences if o.component.name == 'Wiring'][0]
        comp = occ.component
        # clear old wiring
        for f in list(comp.features.pipeFeatures):
            f.deleteMe()
        for f in list(comp.features.baseFeatures):
            f.deleteMe()
        for s in list(comp.sketches):
            s.deleteMe()
        for b in list(comp.bRepBodies):
            b.deleteMe()
        n = 0
        for group, label in ((
            [BUNDLE_TRUNK], 'Trunks'), (WIRES, 'XIAO side'), (POD, 'Pod side')):
            sk = comp.sketches.add(comp.xYConstructionPlane)
            sk.name = 'Wire paths - ' + label
            for w in group:
                name, color, dia, pts = w[0], w[1], w[2], w[-1]
                try:
                    route(comp, sk, name, color, dia, pts, len(w) == 5); n += 1
                except Exception as e:
                    print('FAILED', name, str(e).splitlines()[0])
            sk.isVisible = False
        # C2 0.1 uF ceramic across the N20 terminals, below the motor
        bf = comp.features.baseFeatures.add(); bf.name = 'C2 100nF across the motor'
        bf.startEdit()
        cylinder(comp, bf, 'C2 100nF', 'tan', (0.0, -1.0, -28.8), (0.0, 1.0, -28.8), 4.0)
        cylinder(comp, bf, 'C2 lead +', 'tin', (3.1, 0.0, -26.9), (1.2, 0.0, -28.6), 0.5)
        cylinder(comp, bf, 'C2 lead -', 'tin', (-3.1, 0.0, -26.9), (-1.2, 0.0, -28.6), 0.5)
        bf.finishEdit()
        print('routed', n, 'wires + C2; bodies', comp.bRepBodies.count)
    except Exception:
        print(traceback.format_exc())
