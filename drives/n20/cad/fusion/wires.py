"""Cam Rotation N20 - point-to-point wiring, rev C (matches drives/n20/docs/wiring.md and docs/perfboard.md:
W1, W2, W4-W6, W9-W21, W6p, W9p-W11p, L1, L2, J1, C2, D1 leads).
Rebuilds everything inside the Wiring component. Coordinates in mm, world frame.
Perfboard (component Perfboard): on four supports on the mount's -Y face, parallel to it, rows along +X, column A at the top
  (toward the box wall), component side facing -Y. Board-local u (columns), v (rows), d (height off the component side) map to
  the world as x = 5.3 + v, y = -28.1 - d, z = 15 - u, with u = (index(c) - 5.5) * 2.54 (A..E = 0..4, F..J = 7..11) and
  v = (row - 9) * 2.54. The underside (solder side, toward the mount) is at y -26.5; links run there, on two layers.
  Wires from the mount (L1, L2, the panel harness) go in from the underside and are soldered on the component side.
U1, the PD board, sits over columns A-C, rows 9-17, on two 3 mm foam pads, USB-C at the +X end; its screw terminal is removed
  and its + / - pads take W1 / W2 down to the 5V (row 7) and GND (row 8) buses.
J4 (side-entry JST-PH 2) at J3/J2 opens toward -Z; the motor pair leaves through the notch in the cover's bottom wall and runs
  under the base's -Y edge to the motor's +x side, inside the arms' zip tie, then into the cartridge's wire clamp
  (the tie plate on the +y arm, inner face y 5.3, tie bar at z -28.3) and back along it to the tabs at x +/-3.1.
Control panel: JST-PH 6-pin header in the pod (pins at x -6.75, z 17.9..27.9, tails end at y 38.1); the mount harness ends in the plug
  at y 22.9 and runs through the base tunnel to the -Y face, then behind the perfboard to its holes.
"""
import adsk.core, adsk.fusion, math, traceback

COLORS = {
    'red': (200, 30, 30), 'black': (28, 28, 28), 'brown': (115, 62, 28), 'orange': (240, 130, 20),
    'white': (238, 238, 230), 'yellow': (240, 205, 40), 'violet': (130, 60, 180), 'green': (40, 150, 60),
    'blue': (40, 90, 205), 'gray': (120, 120, 120), 'tan': (214, 184, 130), 'tin': (195, 195, 200),
}

# name, color, diameter mm, waypoints (mm)
BUNDLE_TRUNK = ('Control harness to J3 (4 x 30 AWG)', 'gray', 1.6, 'raw', [
    (-6.75, 22.9, 23.0), (-6.75, 22.0, 21.6), (-6.9, 21.8, 18.0), (-7.0, 21.8, 4.0), (-7.6, 21.8, 0.0), (-10.0, 21.8, -2.2),
    (-11.3, 20.7, -2.6), (-11.5, 19.4, -4.2), (-11.6, 19.4, -6.4),
    (-11.8, 19.3, -8.4), (-12.0, 18.0, -9.35), (-12.0, 5.0, -9.35), (-12.0, -15.0, -9.35), (-12.0, -18.2, -9.3),
    (-11.9, -19.2, -8.2), (-11.6, -19.2, -6.0), (-11.2, -19.2, 2.0), (-11.0, -19.2, 9.0), (-11.0, -20.4, 11.2),
    (-10.4, -21.9, 12.6), (-8.0, -22.3, 13.1), (-6.0, -22.3, 13.2)])

# perfboard helpers
COLS = {'A': 0, 'B': 1, 'C': 2, 'D': 3, 'E': 4, 'F': 7, 'G': 8, 'H': 9, 'I': 10, 'J': 11}
def cu(c, du=0.0): return (COLS[c] - 5.5) * 2.54 + du     # column -> u
def rv(r): return (r - 9) * 2.54                         # row -> v
def W(u, v, d): return (5.3 + v, -28.1 - d, 15.0 - u)     # board-local -> world
def hole(c, r, d=0.0): return W(cu(c), rv(r), d)          # component-side joint
def back(c, r, d=-2.4): return W(cu(c), rv(r), d)         # just behind the underside, for wires going in from the back
def link(pts, layer=0):
    """underside link: [(col, row) | (u, row)] -> waypoints on the solder side (+0.6 mm per layer for links that cross)"""
    d = -1.6 - 0.35 - 0.6 * layer
    return [W(cu(p[0]) if isinstance(p[0], str) else p[0], rv(p[1]), d) for p in pts]
def lane(c, du): return cu(c, du)
def L4(a, u, b, layer=0):
    """hole a -> lane u (run along the rows) -> hole b"""
    up = -1 if b[1] < a[1] else 1
    return link([a, (u, a[1] + 0.35 * up), (u, b[1] - 0.35 * up), b], layer)

PD_P, PD_N = W(-16.5, 0.3, 4.7), W(-11.0, 0.3, 4.7)       # U1's + and - pads (screw terminal removed)

WIRES = [
    # --- power: U1's pads down to the buses (5V row 7, GND row 8); C1 from its seat in behind to C7 / B4
    ('W1 U1 + to 5V bus (A7)', 'red', 0.8, [PD_P, W(-16.0, -1.8, 5.2), W(-14.6, -4.2, 2.6), hole('A', 7, 0.9), hole('A', 7, -0.8)]),
    ('W2 U1 GND to GND bus (A8)', 'black', 0.8, [PD_N, W(-12.0, -1.6, 5.2), W(-13.4, -2.4, 2.6), hole('A', 8, 0.9), hole('A', 8, -0.8)]),
    ('L1 C1 + to 5V bus (C7)', 'red', 0.8, [(-2.75, -19.78, -3.1), (-2.75, -21.6, -3.15), (-2.4, -23.6, -3.6), (0.22, -24.8, -1.0),
        (0.22, -24.8, 22.3), back('C', 7, -1.0), hole('C', 7, 0.4)]),
    ('L2 C1 - to GND (B4)', 'black', 0.8, [(-6.25, -19.78, -3.1), (-6.3, -21.6, -3.2), (-6.6, -23.6, -3.8), (-8.4, -24.8, -1.0),
        (-8.4, -24.8, 24.8), back('B', 4, -1.0), hole('B', 4, 0.4)]),
    # --- D1 standing at A3 (anode down), band up; its cathode lead bends over and down into A2
    ('D1 anode lead to A3', 'tin', 0.6, [hole('A', 3, 1.0), hole('A', 3, -0.8)]),
    ('D1 cathode lead to A2', 'tin', 0.6, [hole('A', 3, 6.2), hole('A', 3, 7.0), W(cu('A'), rv(2.5), 7.0), hole('A', 2, 6.0), hole('A', 2, -0.8)]),
    # --- motor pair: J4 (side entry, mouth toward -Z) -> notch in the cover's bottom wall -> under the base's -Y edge
    #     -> the motor's +x side inside the arms' zip tie -> the wire clamp -> the tabs
    ('W14 J4 pin 1 (OUT3) to motor +', 'white', 0.8, [W(26.4, rv(3), 2.4), W(29.0, rv(3), 2.4), (-9.9, -29.0, -15.0), (-2.0, -22.0, -15.0),
        (8.8, -14.0, -15.0), (9.0, -7.0, -15.2), (8.2, -1.0, -15.8), (6.7, 1.2, -17.0), (6.7, 1.2, -24.0), (6.7, 2.6, -26.6), (6.4, 4.4, -27.9),
        (3.6, 4.4, -27.9), (3.1, 1.8, -27.4), (3.1, 0.4, -26.8)]),
    ('W15 J4 pin 2 (OUT4) to motor -', 'blue', 0.8, [W(26.4, rv(2), 2.4), W(29.6, rv(2), 2.4), (-11.3, -29.6, -15.8), (-2.8, -22.0, -15.9),
        (9.8, -13.6, -15.9), (10.0, -6.4, -16.0), (9.0, -0.4, -16.6), (6.7, 2.6, -17.8), (6.7, 2.6, -24.0), (6.9, 3.4, -27.0), (6.2, 4.4, -28.9),
        (-2.6, 4.4, -28.9), (-3.1, 2.2, -28.4), (-3.1, 0.4, -26.8)]),
    # --- control harness, mount side: trunk end on the -Y face, behind the perfboard, in from the underside
    ('W10 GPIO0 (I15) to J3 pin 1 (rev)', 'blue', 0.6, [(-6.0, -21.9, 13.5), (-3.0, -22.0, 13.4), (8.0, -22.6, 9.8), (17.0, -23.6, 5.4), (19.4, -24.6, 4.2), back('I', 15, -1.0), hole('I', 15, 0.4)]),
    ('W9 GPIO3 (I12) to J3 pin 3 (fwd)', 'green', 0.6, [(-6.0, -22.1, 13.9), (-3.0, -22.6, 13.0), (4.0, -23.4, 9.0), (9.6, -23.8, 5.2), (11.8, -24.8, 4.2), back('I', 12, -1.0), hole('I', 12, 0.4)]),
    ('W6 GPIO1 (I14) to J3 pin 6 (button)', 'white', 0.6, [(-6.0, -22.7, 13.6), (-3.0, -23.2, 12.6), (6.0, -23.8, 8.4), (14.6, -24.0, 5.0), (16.9, -25.0, 4.2), back('I', 14, -1.0), hole('I', 14, 0.4)]),
    ('W11 GND (A4) to J3 pin 2', 'black', 0.6, [(-6.0, -23.0, 13.2), (-6.8, -24.9, 13.6), (-6.6, -25.0, 27.4),
        back('A', 4, -1.0), hole('A', 4, 0.4)]),
]

# underside links on the perfboard (docs/perfboard.md); lanes match the hole map
LINKS = [
    ('W16 5V bus (B7) to DRV VCC (C3)', 'red', 0.6, L4(('B', 7), lane('B', 0.6), ('C', 3))),
    ('W18 GND bus (B8) to DRV GND (C4)', 'black', 0.6, L4(('B', 8), lane('B', 1.27), ('C', 4))),
    ('W4 GND bus (C8) to Super Mini GND (C16)', 'black', 0.6, L4(('C', 8), lane('B', 0.6), ('C', 16))),
    ('W17 D1 cathode (B2) to Super Mini 5V (C17)', 'red', 0.6, L4(('B', 2), lane('B', 1.9), ('C', 17), 1)),
    ('W5 GPIO18 (C12) to DRV IN3 (C5)', 'orange', 0.6, L4(('C', 12), lane('C', 0.6), ('C', 5))),
    ('W12 GPIO20 (C14) to DRV IN4 (C6)', 'brown', 0.6, L4(('C', 14), lane('C', 1.27), ('C', 6))),
    ('W13 GPIO19 (C13) to DRV EEP (J6)', 'gray', 0.6, link([('C', 13), (lane('C', 1.9), 12.6), (lane('C', 1.9), 6.5), (cu('J'), 6.5), ('J', 6)])),
    ('W19 RV1 wiper (I9) to GPIO2 (I13)', 'yellow', 0.6, L4(('I', 9), lane('I', 1.0), ('I', 13))),
    ('W20 GND (A16) to RV1 pin 1 (I8)', 'black', 0.6, link([('A', 16), (lane('A', -0.9), 15.65), (lane('A', -0.9), 7.25), (lane('I', 0.5), 7.25), ('I', 8)], 1)),
    ('W21 3V3 (A15) to RV1 pin 3 (I10)', 'violet', 0.6, link([('A', 15), (lane('A', 0.7), 14.5), (lane('A', 0.7), 10.6), (lane('I', -0.6), 10.6), ('I', 10)], 1)),
]

# pod side: from the JST-PH header tails (x -6.75, y 38.1; pin 1 at z 17.9 .. pin 6 at z 27.9) to the controls.
# Pin order: 1 REV (W10), 2 GND (W11), 3 FWD (W9), 4 and 5 unused (rev C: RV1 is on the perfboard), 6 BTN (W6).
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
    # 12 mm stainless button S1: its own pre-wired lead A goes to JST pin 6 over the pod's empty knob hole
    ('W6p button lead A to J3 pin 6', 'white', 0.6, [tail(6), (-6.75, 39.4, 27.9), (-5.0, 39.2, 28.8), (-1.0, 36.0, 28.8), (8.0, 33.6, 28.0), (11.0, 32.75, 22.0), (11.0, 32.75, 16.3)]),
    ('J1 button lead B to SW1 center', 'black', 0.6, [(3.0, 32.75, 15.6), (3.0, 32.5, 17.6), (0.6, 27.8, 17.4), (-1.4, 25.6, 13.0),
        (-2.2, 25.8, 8.0), (-3.4, 27.2, 5.4), (-4.0, 28.2, 5.0)]),
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
            [BUNDLE_TRUNK], 'Trunks'), (WIRES, 'Mount side'), (LINKS, 'Perfboard links'), (POD, 'Pod side')):
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
