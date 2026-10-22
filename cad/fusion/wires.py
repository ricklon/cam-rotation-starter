"""Cam Rotation Starter - point-to-point wiring (matches wiring page rev B, W1-W11, L1-L3, J1-J2, R1).
Rebuilds everything inside the Wiring component. Coordinates in mm, world frame.
U1 terminal: + entry at z 22.9, GND at z 17.9 (-X face of the terminal block).
XIAO (USB to +X, viewed from -Y): top row z 9.9 = D0..D6 from x 19.1 leftward; bottom row z -5.4 = 5V, GND, 3V3 ... from x 19.1.
"""
import adsk.core, adsk.fusion, math, traceback

COLORS = {
    'red': (200, 30, 30), 'black': (28, 28, 28), 'brown': (115, 62, 28), 'orange': (240, 130, 20),
    'white': (238, 238, 230), 'yellow': (240, 205, 40), 'violet': (130, 60, 180), 'green': (40, 150, 60),
    'blue': (40, 90, 205), 'gray': (120, 120, 120), 'tan': (214, 184, 130),
}

# name, color, diameter mm, waypoints (mm)
SERVO_TRUNK = ('Servo lead (3-wire, to groove)', 'brown', 1.8, [
    (-6.6, -3.0, -19.0), (-10.0, -9.0, -17.6), (-13.4, -15.5, -15.6), (-14.4, -18.8, -13.4),
    (-14.5, -19.2, -11.0), (-14.5, -19.2, -8.0)])
BUNDLE_TRUNK = ('Control bundle (6 x 30 AWG)', 'gray', 2.0, 'raw', [
    (-7.5, 25.0, -2.0), (-10.0, 22.8, -2.2), (-11.3, 20.7, -2.6), (-11.5, 19.4, -4.2), (-11.6, 19.4, -6.4),
    (-11.8, 19.3, -8.4), (-12.0, 18.0, -9.35), (-12.0, 5.0, -9.35), (-12.0, -15.0, -9.35), (-12.0, -18.2, -9.3),
    (-11.9, -19.2, -8.2), (-11.6, -19.2, -6.0), (-11.2, -19.2, 2.0), (-11.0, -19.2, 9.0), (-11.0, -20.4, 11.2),
    (-10.4, -21.9, 12.8), (-8.0, -22.3, 13.5), (-6.0, -22.3, 13.6)])

WIRES = [
    # --- servo to U1 / XIAO
    ('W1 servo red to U1 +', 'red', 0.9, [(-14.85, -19.2, -8.3), (-14.85, -19.2, -6.0), (-14.85, -19.2, 19.5),
        (-14.4, -22.6, 22.9), (-11.0, -23.4, 22.9), (-6.0, -23.6, 22.9)]),
    ('W2 servo brown to U1 GND', 'brown', 0.9, [(-14.0, -19.2, -8.3), (-14.0, -19.2, -6.0), (-14.0, -19.2, 15.0),
        (-13.6, -22.2, 17.9), (-11.0, -23.4, 17.9), (-6.0, -23.6, 17.9)]),
    ('W5 servo orange to R1 to D0', 'orange', 0.9, [(-14.5, -19.6, -8.4), (-13.6, -21.6, -9.8), (-10.0, -22.4, -10.0),
        (22.6, -22.4, -10.0), (25.0, -22.2, -8.6), (25.2, -22.0, -6.0), (25.2, -22.0, 9.0), (24.6, -22.0, 11.8),
        (21.6, -21.6, 12.2), (19.6, -21.2, 11.2), (19.1, -21.0, 9.9)]),
    # --- power: C1, D1, U1, XIAO
    ('L1 C1 + to U1 +', 'red', 0.8, [(-2.75, -24.75, -3.3), (-2.75, -25.4, -5.0), (-4.0, -26.2, -5.6), (-11.0, -26.2, -5.6),
        (-12.0, -25.2, -4.4), (-12.0, -25.2, 21.5), (-10.6, -24.8, 22.9), (-6.0, -24.4, 23.1)]),
    ('L2 C1 - to U1 GND', 'black', 0.8, [(-6.25, -24.75, -3.3), (-6.4, -25.2, -4.8), (-9.8, -25.2, -4.8),
        (-9.8, -25.2, 16.4), (-8.8, -25.0, 17.9), (-6.0, -24.6, 17.7)]),
    ('L3 D1 anode to U1 +', 'red', 0.8, [(-9.2, -22.1, -4.6), (-10.9, -22.6, -4.6), (-10.9, -25.2, -2.6),
        (-10.9, -25.2, 21.0), (-9.6, -24.8, 22.7), (-6.0, -24.6, 22.7)]),
    ('W3 D1 cathode to XIAO 5V', 'red', 0.8, [(0.4, -22.1, -4.6), (0.4, -22.0, -6.2), (1.2, -21.8, -7.6), (3.0, -21.6, -9.6),
        (17.4, -21.6, -9.6), (19.1, -21.4, -7.4), (19.1, -21.0, -5.4)]),
    ('W4 U1 GND to XIAO GND', 'black', 0.8, [(-6.0, -24.8, 17.9), (-8.0, -26.0, 17.9), (-9.0, -27.0, 16.5), (-9.0, -27.0, -7.0),
        (-8.0, -25.5, -8.9), (-4.0, -22.8, -8.9), (15.0, -22.8, -8.9), (16.7, -22.0, -6.6), (16.6, -21.0, -5.5)]),
    # --- XIAO end of the control bundle
    ('W10 XIAO D4 to SW1 B (rev)', 'blue', 0.6, [(-6.0, -21.9, 13.5), (-3.0, -21.5, 13.4), (7.2, -21.5, 13.4), (8.9, -21.2, 11.6), (8.9, -21.0, 9.9)]),
    ('W9 XIAO D3 to SW1 A (fwd)', 'green', 0.6, [(-6.0, -22.1, 13.9), (-3.0, -22.2, 13.4), (9.7, -22.2, 13.4), (11.4, -21.4, 11.6), (11.4, -21.0, 9.9)]),
    ('W7 XIAO D2 to RV1 wiper', 'yellow', 0.6, [(-6.0, -22.5, 13.9), (-3.0, -22.9, 13.4), (12.3, -22.9, 13.4), (14.0, -21.6, 11.6), (14.0, -21.0, 9.9)]),
    ('W6 XIAO D1 to S1 A', 'white', 0.6, [(-6.0, -22.7, 13.6), (-3.0, -23.6, 13.4), (14.8, -23.6, 13.4), (16.5, -21.8, 11.6), (16.5, -21.0, 9.9)]),
    ('W8 XIAO 3V3 to RV1 pin 3', 'violet', 0.6, [(-6.0, -22.6, 12.9), (-3.0, -23.2, 12.5), (-0.2, -23.2, 12.4), (0.25, -23.2, 10.5),
        (0.25, -23.2, -5.5), (1.4, -23.2, -7.2), (12.3, -23.2, -7.2), (14.0, -21.6, -6.0), (14.0, -21.0, -5.4)]),
    ('W11 XIAO GND to SW1 center', 'black', 0.6, [(-6.0, -23.0, 13.2), (-3.0, -23.9, 12.5), (-0.2, -23.9, 12.4), (0.25, -23.9, 10.5),
        (0.25, -23.9, -5.5), (1.4, -23.9, -7.9), (14.8, -23.9, -7.9), (16.3, -22.0, -6.2), (16.5, -21.0, -5.4)]),
]
# pod end of the same wires: SW1 lugs x -4 (z 1 rev, 5 common, 9 fwd); S1 lugs z 5 (x 8 = B, x 10 = A); RV1 pins x 0/2.5/5 (pin 1/2/3)
POD = [
    ('W10 pod end (SW1 B)', 'blue', 0.6, [(-7.2, 25.4, -1.6), (-5.6, 26.6, 0.0), (-4.0, 27.4, 1.0), (-4.0, 28.2, 1.0)]),
    ('W11 pod end (SW1 center)', 'black', 0.6, [(-7.6, 25.4, -1.4), (-6.8, 26.2, 2.0), (-5.0, 27.0, 4.6), (-4.0, 28.2, 5.0)]),
    ('W9 pod end (SW1 A)', 'green', 0.6, [(-8.0, 25.4, -1.4), (-8.0, 26.0, 3.0), (-6.4, 26.6, 7.6), (-4.0, 27.4, 9.0), (-4.0, 28.2, 9.0)]),
    # 12 mm stainless button S1 on the pod's outer face (terminals at x 3 = B/ground, x 11 = A/signal, tips at z 16)
    ('W6 button lead A (pod end)', 'white', 0.6, [(11.0, 32.75, 15.6), (11.0, 32.75, 17.6), (8.0, 28.0, 18.4), (1.5, 23.6, 16.0),
        (-2.4, 23.4, 10.0), (-2.6, 23.6, 3.0), (-5.2, 24.4, -0.6), (-7.0, 25.6, -2.4)]),
    ('W7 pod end (RV1 wiper)', 'yellow', 0.6, [(-6.8, 25.2, -2.6), (-2.0, 24.4, -3.0), (0.8, 23.4, -2.6), (0.8, 23.3, 0.0), (0.8, 23.3, 11.4),
        (0.8, 25.0, 12.3), (0.8, 30.0, 12.4), (1.0, 35.0, 12.3), (2.5, 37.2, 11.8)]),
    ('W8 pod end (RV1 pin 3)', 'violet', 0.6, [(-6.8, 24.8, -2.4), (-2.0, 23.8, -3.2), (1.7, 23.2, -2.8), (1.7, 23.2, 0.0), (1.7, 23.2, 12.0),
        (1.7, 25.0, 13.3), (1.7, 30.0, 13.4), (1.9, 35.6, 13.4), (5.0, 37.2, 13.0)]),
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
            [SERVO_TRUNK, BUNDLE_TRUNK], 'Trunks'), (WIRES, 'XIAO side'), (POD, 'Pod side')):
            sk = comp.sketches.add(comp.xYConstructionPlane)
            sk.name = 'Wire paths - ' + label
            for w in group:
                name, color, dia, pts = w[0], w[1], w[2], w[-1]
                try:
                    route(comp, sk, name, color, dia, pts, len(w) == 5); n += 1
                except Exception as e:
                    print('FAILED', name, str(e).splitlines()[0])
            sk.isVisible = False
        # R1 330 ohm in line on W5, under heat-shrink, on the vertical run at x 25.2
        bf = comp.features.baseFeatures.add(); bf.name = 'R1 330R in heat-shrink'
        bf.startEdit()
        cylinder(comp, bf, 'R1 330R', 'tan', (25.2, -22.0, 0.35), (25.2, -22.0, 3.65), 1.8)
        cylinder(comp, bf, 'R1 heat-shrink', 'black', (25.2, -22.0, -2.5), (25.2, -22.0, 6.5), 2.4, 0.45)
        bf.finishEdit()
        print('routed', n, 'wires + R1; bodies', comp.bRepBodies.count)
    except Exception:
        print(traceback.format_exc())
