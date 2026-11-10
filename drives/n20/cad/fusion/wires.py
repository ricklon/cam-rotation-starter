"""Cam Rotation N20 - point-to-point wiring, rev C with the Hall encoder motor (matches drives/n20/docs/wiring.md and
docs/perfboard.md: W1, W2, W4-W6, W11-W13, W16-W18, W21-W31, C2, and the D1, C1 and C3 leads).
Rebuilds everything inside the Wiring component. Coordinates in mm, world frame.
Perfboard (component Perfboard): on four supports on the mount's -Y face, parallel to it, rows along -X (row 1 at +X), column A at the top
  (toward the box wall), component side facing -Y. Board-local u (columns), v (rows), d (height off the component side) map to
  the world as x = 5.3 - v, y = -28.1 - d, z = 15 - u, with u = (index(c) - 5.5) * 2.54 (A..E = 0..4, F..J = 7..11) and
  v = (row - 9) * 2.54. The underside (solder side, toward the mount) is at y -26.5; links run there, on two layers.
  Wires from the mount (the panel harness) go in from the underside and are soldered on the component side.
C1, the 470 uF bulk capacitor on the 5 V bus, stands on the board over columns A-C, rows 4-7 (between D1 and W1/W2), its top through
  a 9 mm hole in the cover; sleeved leads to C7 (+) and B4 (-). It stands over A4, so W11 is soldered there before C1 goes on.
C3, the 470 uF hold-up capacitor for the Super Mini, stands upright over I7/I8 as built (cover_rv1.py sizes and places it);
  its + lead goes into I7 (strip fed from D1's cathode by W22 at H7) and its - lead into I8 (GND, W23 at H8).
U1, the PD board, sits over columns A-C, rows 9-17, on two 3 mm foam pads, USB-C at the -X end; its screw terminal is removed
  and its + / - pads take W1 / W2 down to the 5V (row 7) and GND (row 8) buses. The - pad is the outer one (column A side),
  the + pad the inner one (channel side), as on the owner's board; other PD boards may differ, so go by the board's own marking.
J4 is the 6-pin socket on the encoder board (N20_Encoder), its plug end at y -14.5 on the motor's -Y side. Seen from the
  components with the plug at the bottom, the pins run M1, GND, C1, C2, VCC, M2 left to right: x +5 to -5 at 2 mm pitch.
  The motor's own 6-wire cable (W24-W29, about 0.9 mm) leaves the plug toward -Y, rises past the base's -Y edge and comes in
  through the notch in the cover's bottom wall (x 18.3-25.3). W24-W27 land in J3, J2, J4 and J5 along the board's J edge;
  W28 and W29 climb over the DRV8833 and the Super Mini, under the cover, and come down into I15 (GPIO0) and I13 (GPIO2).
  Each goes in from the component side and is soldered underneath. The links W30 (I4 -> E4, GND) and W31 (I5 -> H10, 3V3)
  carry the encoder's ground and supply on from strips 4 and 5 right. C2 stands on the perfboard at I2/I3.
  The plain N20's 2-pin header (the Perfboard body 'J4 JST-PH 2 side entry') is hidden; W14, W15 and the wire clamp are unused.
Controls: the knob RV1 is on the perfboard and the 12 mm button S1 is in the pod (no direction switch, no panel connector). Two wires run
  straight from the button's leads, back through the pod where the J3 connector used to be, along the base tunnel to the -Y face, then
  behind the perfboard and up through their holes: W6 (lead A to I14, GPIO1) and W11 (lead B to A4, GND). The pod no longer unplugs.
"""
import adsk.core, adsk.fusion, math, traceback

COLORS = {
    'red': (200, 30, 30), 'black': (28, 28, 28), 'brown': (115, 62, 28), 'orange': (240, 130, 20),
    'white': (238, 238, 230), 'yellow': (240, 205, 40), 'violet': (130, 60, 180), 'green': (40, 150, 60),
    'blue': (40, 90, 205), 'gray': (120, 120, 120), 'tan': (214, 184, 130), 'tin': (195, 195, 200),
}

# name, color, diameter mm, waypoints (mm). All wire is 30 AWG stranded, about 0.6 mm over the insulation, except the motor's
# factory leads W14/W15 (assumed 26 AWG, 1.0 mm).
# the route the button pair shares, from where the J3 plug was (y 22.9) through the base tunnel to the -Y face; each wire runs
# 0.35 mm to one side of it in x and y, so they lie side by side wherever it turns
TRUNK = [
    (-6.75, 22.9, 23.0), (-6.75, 22.0, 21.6), (-6.9, 21.8, 18.0), (-7.0, 21.8, 4.0), (-7.6, 21.8, 0.0), (-10.0, 21.8, -2.2),
    (-11.3, 20.7, -2.6), (-11.5, 19.4, -4.2), (-11.6, 19.4, -6.4),
    (-11.8, 19.3, -8.4), (-12.0, 18.0, -9.35), (-12.0, 5.0, -9.35), (-12.0, -15.0, -9.35), (-12.0, -18.2, -9.3),
    (-11.9, -19.2, -8.2), (-11.6, -19.2, -6.0), (-11.2, -19.2, 2.0), (-11.0, -19.2, 9.0), (-11.0, -20.4, 11.2),
    (-10.4, -21.9, 12.6), (-8.0, -22.3, 13.1), (-6.0, -22.3, 13.2)]
def trunk(dx): return [(x + dx, y + dx, z) for x, y, z in TRUNK[:-1]]

# perfboard helpers
COLS = {'A': 0, 'B': 1, 'C': 2, 'D': 3, 'E': 4, 'F': 7, 'G': 8, 'H': 9, 'I': 10, 'J': 11}
def cu(c, du=0.0): return (COLS[c] - 5.5) * 2.54 + du     # column -> u
def rv(r): return (r - 9) * 2.54                         # row -> v
def W(u, v, d): return (5.3 - v, -28.1 - d, 15.0 - u)     # board-local -> world
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

PD_P, PD_N = W(-11.0, 0.3, 4.7), W(-16.5, 0.3, 4.7)       # U1's + (inner, channel side) and - (outer, column A side) pads

# the motor cable: (name, colour, J4 pin x at the plug, x where it passes the cover's notch, path on the board).
# At the notch (x 18.3-25.3, 1.04 mm pitch) the wires are in the order of where they land: M2 (J2) at the +x end, then M1
# (J3), the two signals, GND (J4) and VCC (J5). That isn't the plug's order, and between the plug and the base's edge the
# cable runs mostly along +x, so spacing in x doesn't keep the wires apart there: the six are stacked 1.1 mm apart across
# the run instead (STACK, in that same notch order), as a loose bundle would twist to.
PLUG_Y, PLUG_Z = -14.5, -27.75                            # the plug's -Y face; wires leave at mid-height of the housing
CABLE_D = 0.9                                             # cable wire over the insulation (gauge unconfirmed)
OVER_D = 7.4                                              # over the modules: clear of C2 (6.4) and the USB-C (6.7), under the cover (9.0)
STACK = (0.0, 0.81, 0.58)                                 # across the run from the plug to the base's edge, in y-z
def into(c, r):                                           # down into a hole from the component side, through to the underside
    return [hole(c, r, 1.0), hole(c, r, -1.8)]
def over(lane_u, vn, c, r):                               # up over C2, the DRV8833 and the Super Mini, down beside the Super Mini
    return [W(18.0, vn, 5.0), W(14.0, vn, OVER_D), W(lane_u, vn, OVER_D), W(lane_u, rv(r), OVER_D), W(cu(c), rv(r), OVER_D)] + into(c, r)
CABLE = [
    ('W24 J4 pin 1 (M1) to J3 (OUT3)', 'white', 5.0, 23.36, lambda vn: [W(17.8, vn + 0.6, 1.8), W(16.0, rv(3), 1.6)] + into('J', 3)),
    ('W25 J4 pin 6 (M2) to J2 (OUT4)', 'red', -5.0, 24.4, lambda vn: [W(16.0, rv(2), 1.6)] + into('J', 2)),
    ('W26 J4 pin 2 (GND) to J4 (encoder GND strip)', 'blue', 3.0, 20.24, lambda vn: [W(17.0, rv(4) - 0.7, 2.8), W(15.6, rv(4), 2.0)] + into('J', 4)),
    ('W27 J4 pin 5 (VCC) to J5 (encoder 3V3 strip)', 'black', -3.0, 19.2, lambda vn: [W(17.3, vn + 2.2, 1.6), W(15.6, rv(5), 1.5)] + into('J', 5)),
    ('W28 J4 pin 3 (C1) to GPIO0 (I15)', 'green', 1.0, 22.32, lambda vn: over(4.2, vn, 'I', 15)),
    ('W29 J4 pin 4 (C2) to GPIO2 (I13)', 'yellow', -1.0, 21.28, lambda vn: over(5.8, vn, 'I', 13)),
]
def cable(name, color, xp, xn, board):
    vn = 5.3 - xn                                         # board v where it passes the notch
    s = 1.1 * ((25.3 + 18.3) / 2 - xn) / 1.04             # stack offset: notch order, centred
    o = lambda p: (p[0], p[1] + s * STACK[1], p[2] + s * STACK[2])
    pts = [(xp, PLUG_Y, PLUG_Z), (xp, PLUG_Y - 4.0, PLUG_Z), o(((xp + xn) / 2, -24.0, -22.0)), o((xn, -29.0, -15.0)),
           (xn, -30.5, -9.0), W(19.0, vn, 2.4)] + board(vn)
    return (name, color, CABLE_D, pts)

WIRES = [
    # --- power: U1's pads down to the buses (5V row 7, GND row 8)
    # W2 runs from the outer - pad straight to A8; W1 from the inner + pad passes between A8 and B8 to A7, so they don't cross
    ('W1 U1 + to 5V bus (A7)', 'red', 0.6, [PD_P, W(-11.4, -1.6, 5.2), W(-12.4, -3.4, 3.4), W(-13.3, -4.6, 1.8), hole('A', 7, 0.9), hole('A', 7, -0.8)]),
    ('W2 U1 GND to GND bus (A8)', 'black', 0.6, [PD_N, W(-16.0, -1.2, 5.2), W(-15.0, -2.4, 2.6), hole('A', 8, 0.9), hole('A', 8, -0.8)]),
    # --- C1 standing on the board (axis at u -13.5, v -9.6, base 1.5 mm up, top through the cover's 9 mm hole); its leads are
    # sleeved and bent out to C7 (+5V bus) and B4 (GND), crossing the IN3/IN4 strips' pads in rows 5-6
    ('C1 + lead to C7 (sleeved)', 'tin', 0.6, [W(-11.75, -9.6, 2.4), W(-10.4, -7.4, 1.6), W(cu('C'), rv(7), 1.6), hole('C', 7, -0.8)]),
    ('C1 - lead to B4 (sleeved)', 'tin', 0.6, [W(-15.25, -9.6, 2.4), W(-13.4, -11.4, 1.6), W(cu('B'), rv(4), 1.6), hole('B', 4, -0.8)]),
    # --- D1 standing at A3 (anode down), band up; its cathode lead bends over and down into A2
    ('D1 anode lead to A3', 'tin', 0.6, [hole('A', 3, 1.0), hole('A', 3, -0.8)]),
    ('D1 cathode lead to A2', 'tin', 0.6, [hole('A', 3, 6.2), hole('A', 3, 7.0), W(cu('A'), rv(2.5), 7.0), hole('A', 2, 6.0), hole('A', 2, -0.8)]),
    # --- C3 standing over I7 / I8 as built: an 8 x 13 mm can like C1, 1.5 mm off the board, its centre (cover_rv1.py's c3_u / c3_v,
    #     assumed 14.4 / -4.0) about 3 mm toward the J edge, so its leads bend back to the holes; stripe (-) toward I8
    ('C3 + lead to I7', 'tin', 0.6, [W(14.4, -5.75, 2.0), W(13.0, -5.5, 1.0), W(cu('I'), rv(7), 0.8), hole('I', 7, -0.8)]),
    ('C3 - lead to I8', 'tin', 0.6, [W(14.4, -2.25, 2.0), W(13.0, -2.4, 1.0), W(cu('I'), rv(8), 0.8), hole('I', 8, -0.8)]),
    # --- the motor cable: J4 at the encoder -> past the base's -Y edge -> the cover's notch -> the perfboard, from the front
] + [cable(*c) for c in CABLE] + [
    # --- the button pair: S1's leads -> back through the pod -> base tunnel -> -Y face -> behind the perfboard, in from the underside
    ('W6 button lead A to GPIO1 (I14)', 'white', 0.6, [(11.0, 32.75, 16.3), (11.0, 32.75, 22.0), (8.0, 33.6, 28.0), (-1.0, 36.0, 28.8),
        (-5.0, 33.0, 27.0), (-6.4, 26.0, 24.0)] + trunk(0.35) + [(-6.0, -22.7, 13.6), (-6.6, -23.6, 9.0), (-7.2, -24.4, 6.0),
        back('I', 14, -1.0), hole('I', 14, 0.4)]),
    ('W11 button lead B to GND (A4)', 'black', 0.6, [(3.0, 32.75, 15.6), (3.0, 32.5, 17.6), (1.0, 31.0, 21.0), (-4.0, 28.0, 23.0),
        (-7.0, 25.5, 23.0)] + trunk(-0.35) + [(-6.0, -23.0, 13.2), (-4.0, -24.6, 14.4), (14.0, -24.8, 24.0), (17.0, -25.0, 27.4), back('A', 4, -1.0), hole('A', 4, 0.4)]),
]

# underside links on the perfboard (docs/perfboard.md); lanes match the hole map
LINKS = [
    ('W16 5V bus (B7) to DRV VCC (C3)', 'red', 0.6, L4(('B', 7), lane('B', 0.6), ('C', 3))),
    ('W18 GND bus (B8) to DRV GND (C4)', 'black', 0.6, L4(('B', 8), lane('B', 1.27), ('C', 4))),
    ('W4 GND bus (C8) to Super Mini GND (C16)', 'black', 0.6, L4(('C', 8), lane('B', 0.6), ('C', 16))),
    ('W17 D1 cathode (B2) to Super Mini 5V (C17)', 'red', 0.6, L4(('B', 2), lane('B', 1.9), ('C', 17), 1)),
    ('W5 GPIO18 (C12) to DRV IN3 (C5)', 'green', 0.6, L4(('C', 12), lane('C', 0.6), ('C', 5))),
    ('W12 GPIO20 (C14) to DRV IN4 (C6)', 'blue', 0.6, L4(('C', 14), lane('C', 1.27), ('C', 6))),
    ('W13 GPIO19 (C13) to DRV EEP (J6)', 'white', 0.6, link([('C', 13), (lane('C', 1.9), 12.6), (lane('C', 1.9), 6.5), (cu('J'), 6.5), ('J', 6)])),
    ('W22 D1 cathode (C2) to C3 + strip (H7)', 'red', 0.6, link([('C', 2), (cu('C'), 1.6), (lane('I', -0.7), 1.6), (lane('I', -0.7), 6.6), ('H', 7)], 1)),
    ('W23 GND (A16) to C3 - strip (H8)', 'black', 0.6, link([('A', 16), (lane('A', -0.9), 15.65), (lane('A', -0.9), 7.4), (lane('H', -0.9), 7.4), ('H', 8)], 1)),
    ('W21 3V3 (A15) to RV1 3V3 end (I10)', 'yellow', 0.6, link([('A', 15), (lane('A', 0.7), 14.5), (lane('A', 0.7), 10.6), (lane('I', -0.6), 10.6), ('I', 10)], 1)),
    # the encoder's ground and 3V3, from the strips its cable lands in (4 and 5 right); H4 and H5 have no DRV8833 pins
    ('W30 encoder GND (I4) to DRV GND (E4)', 'black', 0.6, link([('I', 4), (cu('I'), 4.45), (cu('E'), 4.45), ('E', 4)])),
    ('W31 encoder 3V3 (I5) to 3V3 (H10)', 'yellow', 0.6, link([('I', 5), (lane('H', 0.9), 5.4), (lane('H', 0.9), 9.6), ('H', 10)], 2)),
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
            WIRES, 'Mount side'), (LINKS, 'Perfboard links')):
            sk = comp.sketches.add(comp.xYConstructionPlane)
            sk.name = 'Wire paths - ' + label
            for w in group:
                name, color, dia, pts = w[0], w[1], w[2], w[-1]
                try:
                    route(comp, sk, name, color, dia, pts, len(w) == 5); n += 1
                except Exception as e:
                    print('FAILED', name, str(e).splitlines()[0])
            sk.isVisible = False
        # C2 0.1 uF ceramic standing on the perfboard in I2/I3 (OUT4/OUT3): a 5 x 2 mm disc, edge-on to the J edge, its
        # legs bent 0.8 mm toward the J edge so it clears the DRV8833 module (u <= 10.5)
        bf = comp.features.baseFeatures.add(); bf.name = 'C2 100nF across the motor (perfboard I2/I3)'
        bf.startEdit()
        cu2, vm = cu('I', 0.8), (rv(2) + rv(3)) / 2
        cylinder(comp, bf, 'C2 100nF', 'tan', W(cu2 - 1.0, vm, 3.9), W(cu2 + 1.0, vm, 3.9), 5.0)
        for r in (2, 3):
            cylinder(comp, bf, 'C2 lead ' + ('OUT4' if r == 2 else 'OUT3'), 'tin', hole('I', r, -0.8), W(cu('I'), rv(r), 1.0), 0.5)
            cylinder(comp, bf, 'C2 lead bend ' + str(r), 'tin', W(cu('I'), rv(r), 1.0), W(cu2, vm + (rv(r) - vm) * 0.5, 2.2), 0.5)
        bf.finishEdit()
        # the plain N20's 2-pin JST-PH header isn't fitted with the encoder motor: hide it
        for o in root.allOccurrences:
            if o.component.name == 'Perfboard':
                for b in o.bRepBodies:
                    if b.name.startswith('J4 JST-PH 2'):
                        b.isLightBulbOn = False
        print('routed', n, 'wires + C2; bodies', comp.bRepBodies.count)
    except Exception:
        print(traceback.format_exc())
