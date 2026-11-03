"""Cam Rotation N20 - fit RV1 and C3 as built (rev C, 2026-11-03), and the electronics cover around them.

RV1 (an RK097N-style 9 mm pot, M7 bushing) stands on its pins in J10-J12 with its shaft parallel to the board, pointing off
the column-J edge; C3 (470 uF) stands upright over I7/I8. Every size comes from the user parameters below (rv1_*, c3_*,
cover_*): change one in Fusion (Modify > Change Parameters), then re-run this script to rebuild

  - the RV1 and C3 bodies in the Perfboard component's base feature, and
  - the cover's 'RVC ...' features: the J-edge wall moved in to just past RV1's face, with the knob hole in it at the
    shaft height, the motor-wire notch in that wall, a notch in the top plate for RV1's body and a hole for C3's top.

Run inside Fusion (or via the Fusion MCP server with exec(open(...).read()); then run(None)). Board frame as in wires.py:
u across the columns, v along the rows, d up from the component side; world = (5.3 - v, -28.1 - d, 15 - u) in mm (row 1 toward +X, row 17 and the USB-C ports toward -X).
"""
import adsk.core, adsk.fusion, math

PARAMS = [  # name, value, comment. 'measured' values came from the built board (2026-11-03); 'assumed' ones are still estimates.
    ('rv1_axis_h', '5 mm', 'RV1 shaft centre above the board top; measured'),
    ('rv1_bush_past_edge', '5 mm', 'RV1 threaded bushing past the board J edge; measured'),
    ('rv1_bush_len', '5 mm', 'RV1 M7 threaded bushing length from the body face; assumed (sets where the body face is drawn)'),
    ('rv1_body_w', '10 mm', 'RV1 body width along the board edge; assumed, measure'),
    ('rv1_body_d', '6.5 mm', 'RV1 body depth from its face back over the board; assumed, measure'),
    ('rv1_body_h', '12.5 mm', 'RV1 body height above the board top; assumed, measure'),
    ('rv1_shaft_len', '15 mm', 'RV1 shaft length from the body face; assumed, measure'),
    ('rv1_nut_t', '2.5 mm', 'RV1 M7 nut plus washer thickness; assumed'),
    ('rv1_panel_hole', '7.2 mm', 'Knob hole in the cover wall for the M7 bushing'),
    ('rv1_face_gap', '0.3 mm', 'Gap from the board edge (or RV1 face, if further out) to the cover wall'),
    ('cover_knob_wall', '2 mm', 'Cover J-edge wall thickness'),
    ('cover_nut_recess_dia', '11.5 mm', 'Recess round the knob hole on the outside of the wall, for the nut and washer'),
    ('cover_nut_recess', '0.8 mm', 'Recess depth: the wall is cover_knob_wall minus this at the nut, so the nut gets more thread'),
    ('c3_u', '14.4 mm', 'C3 centre across the columns; assumed: leads in I7/I8 bent about 3 mm toward the J edge, clear of the modules'),
    ('c3_v', '-4.0 mm', 'C3 centre along the rows; assumed (between rows 7 and 8)'),
    ('c3_dia', '8 mm', 'C3 can diameter; measured (same part as C1)'),
    ('c3_h', '13 mm', 'C3 can height; measured (same part as C1)'),
    ('c3_standoff', '1.5 mm', 'C3 can bottom above the board top; assumed, as C1'),
    ('cover_part_clear', '0.5 mm', 'Clearance round parts that pass through the cover top'),
]
RETIRED = ('rv1_face_past_edge',)   # replaced by the measured rv1_bush_past_edge
TOP_IN, TOP_OUT, WALL_BOT, HALF_V = 9.0, 11.0, -1.5, 27.6      # cover plate d 9..11, walls down to d -1.5, outline v +/-27.6
BOARD_EDGE_J, RV1_V, J_U = 19.05, 5.08, 13.97                   # board J edge u, RV1 centre (J11) v, column J u
NOTCH_V = (-20.0, -13.0)                                        # motor-wire notch (J4 region, rows 1-4)

app = adsk.core.Application.get()
d = adsk.fusion.Design.cast(app.activeProduct)
tm = adsk.fusion.TemporaryBRepManager.get()


def P(u, v, dd):
    return adsk.core.Point3D.create((5.3 - v) / 10, (-28.1 - dd) / 10, (15 - u) / 10)


def box(u0, u1, v0, v1, d0, d1):
    a, b = P(u0, v0, d0), P(u1, v1, d1)
    c = adsk.core.Point3D.create((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2)
    ob = adsk.core.OrientedBoundingBox3D.create(c, adsk.core.Vector3D.create(1, 0, 0), adsk.core.Vector3D.create(0, 1, 0),
                                                abs(b.x - a.x), abs(b.y - a.y), abs(b.z - a.z))
    return tm.createBox(ob)


def cyl_u(u0, u1, v, dd, r):      # axis across the columns (the knob shaft)
    return tm.createCylinderOrCone(P(u0, v, dd), r / 10, P(u1, v, dd), r / 10)


def cyl_d(u, v, d0, d1, r):       # axis up from the board
    return tm.createCylinderOrCone(P(u, v, d0), r / 10, P(u, v, d1), r / 10)


def mm(name):
    return d.userParameters.itemByName(name).value * 10


def ensure_params():
    for name, val, note in PARAMS:
        if not d.userParameters.itemByName(name):
            d.userParameters.add(name, adsk.core.ValueInput.createByString(val), 'mm', note)
    for name in RETIRED:
        p = d.userParameters.itemByName(name)
        if p: p.deleteMe()


def comp(name):
    return [c for c in d.allComponents if c.name == name][0]


def occ_of(c):
    return [o for o in d.rootComponent.allOccurrences if o.component == c][0]


def to_comp(body, c):
    inv = occ_of(c).transform2.copy(); inv.invert()
    tm.transform(body, inv)
    return body


def to_perfboard(body):
    """world -> the Perfboard component's own frame (x = 46 + u, y = v, z = 23.6 - d, in mm). Its occurrence is nested, so
    transform2 (relative to the parent) can't be inverted for this."""
    m = adsk.core.Matrix3D.create()
    m.setWithArray([0, 0, -1, 6.1,  -1, 0, 0, 0.53,  0, 1, 0, 5.17,  0, 0, 0, 1])
    tm.transform(body, m)
    return body


def rebuild_parts(g):
    """RV1 and C3 bodies in the Perfboard base feature that holds them."""
    pb = comp('Perfboard')
    bf = [f for f in pb.features.baseFeatures if any(b.name.startswith('RV1') for b in f.bodies)][0]
    looks = {b.name: b.appearance for b in bf.bodies if b.name.startswith(('RV1', 'C3'))}
    look = lambda *names: next((looks[n] for n in names if n in looks), None)
    metal = look('RV1 bushing', 'RV1 bushing M7')
    bf.startEdit()
    for b in [b for b in bf.bodies if b.name.startswith(('RV1', 'C3'))]:
        b.deleteMe()
    face, ax = g['face'], g['axis_h']
    parts = [
        ('RV1 body (standing, shaft parallel to the board)', look('RV1 RK097N base', 'RV1 body (standing, shaft parallel to the board)'), box(face - g['body_d'], face, RV1_V - g['body_w'] / 2, RV1_V + g['body_w'] / 2, 0.0, g['body_h'])),
        ('RV1 bushing M7', metal, cyl_u(face, face + g['bush_len'], RV1_V, ax, 3.5)),
        ('RV1 shaft', look('RV1 shaft') or metal, cyl_u(face + g['bush_len'], face + g['shaft_len'], RV1_V, ax, 3.0)),
        ('RV1 nut and washer', look('RV1 nut', 'RV1 nut and washer') or metal, cyl_u(g['nut_at'], g['nut_at'] + g['nut_t'], RV1_V, ax, 5.2)),
        ('RV1 knob cap', look('RV1 knob cap'), cyl_u(g['nut_at'] + g['nut_t'] + 1.0, g['nut_at'] + g['nut_t'] + 18.0, RV1_V, ax, 7.8)),
        ('C3 470uF hold-up (standing)', look('C3 470uF hold-up (lying)', 'C3 470uF hold-up (standing)'), cyl_d(g['c3_u'], g['c3_v'], g['c3_off'], g['c3_off'] + g['c3_h'], g['c3_dia'] / 2)),
    ]
    for r, row in enumerate((10, 11, 12)):
        parts.append(('RV1 pin %d' % (r + 1), metal, cyl_d(J_U, (row - 9) * 2.54, -2.5, 0.0, 0.4)))
    for name, appearance, body in parts:
        nb = pb.bRepBodies.add(to_perfboard(body), bf)
        nb.name = name
        if appearance: nb.appearance = appearance
    bf.finishEdit()
    d.timeline.moveToEnd()      # editing a base feature leaves the marker rolled back to it


def rebuild_cover(g):
    cov = comp('N20_Electronics_Cover')
    target = [b for b in cov.bRepBodies if b.name == 'N20_Electronics_Cover'][0]
    for f in [f for f in cov.features if f.name.startswith('RVC ')][::-1]:
        f.deleteMe()
    win, wout = g['win'], g['wall_out']
    clr = g['clear']
    steps = [
        ('RVC trim past the new J wall', adsk.fusion.FeatureOperations.CutFeatureOperation,
         [box(win, 45.0, -40.0, 40.0, -5.0, 15.0)]),
        ('RVC J-edge wall at RV1 face', adsk.fusion.FeatureOperations.JoinFeatureOperation,
         [box(win, wout, -HALF_V, HALF_V, WALL_BOT, TOP_OUT)]),
        ('RVC knob hole, motor notch, RV1 and C3 openings', adsk.fusion.FeatureOperations.CutFeatureOperation,
         [cyl_u(win - 1.0, wout + 1.0, RV1_V, g['axis_h'], g['panel_hole'] / 2),
          cyl_u(g['nut_at'], wout + 1.0, RV1_V, g['axis_h'], g['recess_dia'] / 2),
          box(win - 1.0, wout + 1.0, NOTCH_V[0], NOTCH_V[1], -3.4, 11.6),
          box(g['face'] - g['body_d'] - clr, win + 0.01, RV1_V - g['body_w'] / 2 - clr, RV1_V + g['body_w'] / 2 + clr, TOP_IN - 1.0, TOP_OUT + 1.0),
          cyl_d(g['c3_u'], g['c3_v'], TOP_IN - 1.0, TOP_OUT + 1.0, g['c3_dia'] / 2 + clr)]),
    ]
    for name, op, tools in steps:
        bf = cov.features.baseFeatures.add(); bf.name = name + ' (tools)'
        bf.startEdit()
        for t in tools:
            cov.bRepBodies.add(to_comp(t, cov), bf)
        bf.finishEdit()
        coll = adsk.core.ObjectCollection.create()
        for b in bf.bodies: coll.add(b)
        ci = cov.features.combineFeatures.createInput(target, coll)
        ci.operation = op; ci.isKeepToolBodies = False
        cf = cov.features.combineFeatures.add(ci); cf.name = name
        target = cf.bodies.item(0) if cf.bodies.count else target


def run(_context):
    ensure_params()
    tip = BOARD_EDGE_J + mm('rv1_bush_past_edge')                # end of the threaded bushing (measured)
    g = dict(face=tip - mm('rv1_bush_len'), axis_h=mm('rv1_axis_h'), body_w=mm('rv1_body_w'),
             body_d=mm('rv1_body_d'), body_h=mm('rv1_body_h'), bush_len=mm('rv1_bush_len'), shaft_len=mm('rv1_shaft_len'),
             nut_t=mm('rv1_nut_t'), panel_hole=mm('rv1_panel_hole'), gap=mm('rv1_face_gap'), wall=mm('cover_knob_wall'),
             c3_u=mm('c3_u'), c3_v=mm('c3_v'), c3_dia=mm('c3_dia'), c3_h=mm('c3_h'), c3_off=mm('c3_standoff'),
             clear=mm('cover_part_clear'), recess_dia=mm('cover_nut_recess_dia'), recess=mm('cover_nut_recess'))
    g['win'] = max(BOARD_EDGE_J, g['face']) + g['gap']         # never over the board
    g['wall_out'] = g['win'] + g['wall']
    g['nut_at'] = g['wall_out'] - g['recess']                  # the nut seats in the recess round the hole
    thread = tip - g['nut_at']
    spare = thread - g['nut_t']
    rebuild_parts(g)
    rebuild_cover(g)
    print('RV1 face u %.2f, bushing tip u %.2f, wall u %.2f..%.2f (%.2f at the nut), shaft axis d %.2f; thread past the wall %.2f mm for a %.2f mm nut and washer%s' % (
        g['face'], tip, g['win'], g['wall_out'], g['wall'] - g['recess'], g['axis_h'], thread, g['nut_t'],
        '' if spare >= 0 else "  ** too short: the nut won't get a full thread"))
