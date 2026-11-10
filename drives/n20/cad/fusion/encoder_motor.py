"""Model the encoder on the back of the MECCANIXITY N20 encoder gearmotor (B0F8NGVNCH).

Run inside Fusion with *Cam Rotation N20* active (or via the Fusion MCP server:
exec(open(...).read()) then run(None)). It (re)builds the component `N20_Encoder`
at the world origin, next to the existing `N20 DC MOTOR` occurrence, so the
encoder can be checked against the cartridge, mount, perfboard and wiring.

World frame (mm): motor axis on Z, gearbox face at z 0, shaft toward +Z; the
motor's 12 mm side runs along X and its 10 mm flats face +Y and -Y; the
perfboard is on the -Y side. The connector end of the encoder board points -Y,
toward the perfboard.

Measured on the part (calipers and photos, 2026-11-08):
  shaft tip to back of magnet cap 40.5; gearbox face to back of cap 30.5
  board 18 x 12 x 1, flush with the motor's +Y flat, so it reaches 13 past the
  axis on the -Y side; plug overhangs the board end by 1.5
  connector pins, left to right from the component side: M1 white, GND blue,
  C1 green, C2 yellow, VCC black, M2 red
  gearbox face to the board's front face about 25
The cap diameter and the housing depth are estimates from photos; re-measure
and change the constants below.
"""
import adsk.core, adsk.fusion

MM = 0.1  # Fusion API lengths are cm

CAP_BACK_Z = -30.5      # back of the magnet cap (gearbox face to cap = 30.5)
BOARD_THK = 1.0
BOARD_FRONT_Z = -25.0   # gearbox face to board front face, calipers, about 25 mm
BOARD_W = 12.0          # along X
BOARD_Y_MAX = 5.0       # flush with the +Y flat
BOARD_Y_MIN = -13.0     # 18 mm long
BOARD_END_R = 4.0       # rounded corners at the magnet (+Y) end
CAP_DIA = 10.0          # magnet cap, on the motor axis
PLUG_Y_MIN = -14.5      # plug end, 1.5 past the board
PLUG_Y_MAX = -9.5       # housing footprint on the board, about 3.5 + overhang
PLUG_W = 11.0           # along X
PLUG_DEPTH = 3.5        # behind the board's back face
WIRE_KEEPOUT = 6.0      # room for the six wires to leave the plug and bend
WIRE_W = 9.0
WIRE_H = 2.5

NAME = 'N20_Encoder'


def _pt(x, y, z=0):
    return adsk.core.Point3D.create(x * MM, y * MM, z * MM)


def _plane(comp, z):
    ip = comp.constructionPlanes.createInput()
    ip.setByOffset(comp.xYConstructionPlane, adsk.core.ValueInput.createByReal(z * MM))
    pl = comp.constructionPlanes.add(ip)
    return pl


def _extrude(comp, prof, depth, name):
    # extrude toward -Z by `depth` mm
    ei = comp.features.extrudeFeatures.createInput(prof, adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
    ei.setOneSideExtent(adsk.fusion.DistanceExtentDefinition.create(adsk.core.ValueInput.createByReal(depth * MM)),
                        adsk.fusion.ExtentDirections.NegativeExtentDirection)
    f = comp.features.extrudeFeatures.add(ei)
    f.name = name
    b = f.bodies.item(0)
    b.name = name
    return f, b


def _rect(sk, x0, y0, x1, y1):
    sk.sketchCurves.sketchLines.addTwoPointRectangle(_pt(x0, y0), _pt(x1, y1))


def run(_context):
    app = adsk.core.Application.get()
    design = adsk.fusion.Design.cast(app.activeProduct)
    root = design.rootComponent

    old = root.occurrences.itemByName(NAME + ':1')
    if old:
        old.deleteMe()
    occ = root.occurrences.addNewComponent(adsk.core.Matrix3D.create())
    comp = occ.component
    comp.name = NAME

    # board
    pl = _plane(comp, BOARD_FRONT_Z)
    pl.name = 'Encoder board front'
    sk = comp.sketches.add(pl)
    sk.name = 'Encoder board'
    _rect(sk, -BOARD_W / 2, BOARD_Y_MIN, BOARD_W / 2, BOARD_Y_MAX)
    f, board = _extrude(comp, sk.profiles.item(0), BOARD_THK, 'Encoder board 18x12x1')
    # round the two corners at the magnet end
    edges = adsk.core.ObjectCollection.create()
    for e in board.edges:
        a, b = e.startVertex.geometry, e.endVertex.geometry
        if abs(a.x - b.x) < 1e-6 and abs(a.y - b.y) < 1e-6 and a.y > (BOARD_Y_MAX - 0.01) * MM \
                and abs(abs(a.x) - BOARD_W / 2 * MM) < 1e-6:
            edges.add(e)
    fi = comp.features.filletFeatures.createInput()
    fi.edgeSetInputs.addConstantRadiusEdgeSet(edges, adsk.core.ValueInput.createByReal(BOARD_END_R * MM), True)
    comp.features.filletFeatures.add(fi).name = 'Encoder board corners'

    back_z = BOARD_FRONT_Z - BOARD_THK
    pl2 = _plane(comp, back_z)
    pl2.name = 'Encoder board back'

    # magnet cap
    sk = comp.sketches.add(pl2)
    sk.name = 'Magnet cap'
    sk.sketchCurves.sketchCircles.addByCenterRadius(_pt(0, 0), CAP_DIA / 2 * MM)
    _extrude(comp, sk.profiles.item(0), back_z - CAP_BACK_Z, 'Magnet cap')

    # connector housing with the plug in it
    sk = comp.sketches.add(pl2)
    sk.name = 'Encoder plug'
    _rect(sk, -PLUG_W / 2, PLUG_Y_MIN, PLUG_W / 2, PLUG_Y_MAX)
    _extrude(comp, sk.profiles.item(0), PLUG_DEPTH, 'Encoder plug (6-pin, with housing)')

    # wire exit keep-out: the six wires leave the plug toward -Y
    wz = back_z - PLUG_DEPTH / 2 + WIRE_H / 2
    pl3 = _plane(comp, wz)
    pl3.name = 'Encoder wire exit'
    sk = comp.sketches.add(pl3)
    sk.name = 'Encoder wire keep-out'
    _rect(sk, -WIRE_W / 2, PLUG_Y_MIN - WIRE_KEEPOUT, WIRE_W / 2, PLUG_Y_MIN)
    _extrude(comp, sk.profiles.item(0), WIRE_H, 'Wire exit keep-out')

    for pl in (comp.constructionPlanes):
        pl.isLightBulbOn = False
    print('built', NAME, [b.name for b in comp.bRepBodies])
