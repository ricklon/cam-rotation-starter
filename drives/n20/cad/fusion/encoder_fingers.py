import adsk.core, adsk.fusion
# Run once on Cam Rotation N20 (it adds features; it does not replace them). Used for Fusion version 41.

# Encoder-board fingers on the cartridge's -Y arm, +Y hook lead-in, -Y end-cap hook removed.
# All numbers in mm (model frame = cartridge component frame, gearbox face at z 0).
F_IN, F_OUT = 6.3, 7.5        # finger |x|: 0.3 mm outside the 12 mm board
SLOT_IN = 5.8                 # slot splits the finger off the arm, |x| 5.8..6.3
SLOT_TOP = -15.0              # finger flexes from here
ARM_END = -24.8
Y0, Y1 = -6.8, -5.3           # the -Y arm's thickness
CATCH_Z = -26.6               # board back is 26.0..26.5 mm from the gearbox face (tilted)
TIP_X = 5.2                   # 0.8 mm over the board edge
TIP_Z = -26.8
F_END = -27.7                 # lead-in runs from the tip to the finger's outside at the bottom

def run(_context: str):
    app = adsk.core.Application.get()
    des = adsk.fusion.Design.cast(app.activeProduct)
    occ = [o for o in des.rootComponent.allOccurrences if o.name.startswith('N20_Cartridge')][0]
    c = occ.component
    body = c.bRepBodies.item(0)
    V = adsk.core.ValueInput.createByReal
    P = lambda x, y, z: adsk.core.Point3D.create(x / 10, y / 10, z / 10)
    tl = des.timeline
    tl.moveToEnd()

    def plane(base, off_mm, name):
        ci = c.constructionPlanes.createInput()
        ci.setByOffset(base, V(off_mm / 10))
        p = c.constructionPlanes.add(ci)
        p.name = name
        p.isLightBulbOn = False
        return p

    def poly(sk, pts):
        ls = sk.sketchCurves.sketchLines
        q = [sk.modelToSketchSpace(P(*p)) for p in pts]
        for i in range(len(q)):
            ls.addByTwoPoints(q[i], q[(i + 1) % len(q)])

    def profiles(sk):
        col = adsk.core.ObjectCollection.create()
        for p in sk.profiles:
            col.add(p)
        return col

    def extrude(sk, op, half_mm, name):
        ei = c.features.extrudeFeatures.createInput(profiles(sk), op)
        ei.setSymmetricExtent(V(half_mm / 10), False)
        if op != adsk.fusion.FeatureOperations.NewBodyFeatureOperation:
            ei.participantBodies = [body]
        e = c.features.extrudeFeatures.add(ei)
        e.name = name
        return e

    CUT = adsk.fusion.FeatureOperations.CutFeatureOperation
    JOIN = adsk.fusion.FeatureOperations.JoinFeatureOperation

    # 1. Remove the -Y end-cap hook (the one that jams on the encoder board); keep the arm.
    sk = c.sketches.add(c.yZConstructionPlane)
    sk.name = 'Encoder fingers: -Y end-cap hook removed'
    poly(sk, [(0, Y1, -23.9), (0, -3.9, -23.9), (0, -3.9, -24.9), (0, Y1, -24.9)])
    extrude(sk, CUT, 8.0, 'Encoder fingers: -Y end-cap hook removed')

    # 2. +Y hook lead-in: a 0.5 mm 45-degree chamfer on its inner lower edge (the board sits just below).
    sk = c.sketches.add(c.yZConstructionPlane)
    sk.name = 'Encoder fingers: +Y hook lead-in'
    poly(sk, [(0, 3.9, -24.25), (0, 4.65, -25.0), (0, 3.9, -25.0)])
    extrude(sk, CUT, 8.0, 'Encoder fingers: +Y hook lead-in')

    # 3. Slots that split each finger off the -Y arm.
    zmid = (SLOT_TOP + ARM_END - 0.2) / 2
    pz = plane(c.xYConstructionPlane, zmid, 'Encoder fingers: slot mid-plane')
    sk = c.sketches.add(pz)
    sk.name = 'Encoder fingers: slots'
    for s in (1, -1):
        poly(sk, [(s * SLOT_IN, -7.6, zmid), (s * F_IN, -7.6, zmid), (s * F_IN, -4.0, zmid), (s * SLOT_IN, -4.0, zmid)])
    extrude(sk, CUT, (SLOT_TOP - (ARM_END - 0.2)) / 2, 'Encoder fingers: slots')

    # 4. Fingers past the board's sides, with the catch and a lead-in, in x-z, across the arm's thickness.
    ym = (Y0 + Y1) / 2
    py = plane(c.xZConstructionPlane, ym, 'Encoder fingers: arm mid-plane')
    sk = c.sketches.add(py)
    sk.name = 'Encoder fingers'
    for s in (1, -1):
        poly(sk, [(s * F_OUT, ym, ARM_END + 0.2), (s * F_OUT, ym, F_END), (s * F_IN, ym, F_END),
                  (s * TIP_X, ym, TIP_Z), (s * TIP_X, ym, CATCH_Z), (s * F_IN, ym, CATCH_Z), (s * F_IN, ym, ARM_END + 0.2)])
    extrude(sk, JOIN, (Y1 - Y0) / 2, 'Encoder fingers')

    # 5. The +x finger's catch prints hanging (the cartridge prints -x face down): taper it at 45 degrees in x-y.
    zc = (CATCH_Z + F_END) / 2
    pc = plane(c.xYConstructionPlane, zc, 'Encoder fingers: catch mid-plane')
    sk = c.sketches.add(pc)
    sk.name = 'Encoder fingers: +x catch taper'
    poly(sk, [(TIP_X - 0.3, Y0 - 0.1, zc), (F_IN, Y0 + 1.1 + 0.05, zc), (F_IN, Y1 + 0.2, zc), (TIP_X - 0.3, Y1 + 0.2, zc)])
    extrude(sk, CUT, (CATCH_Z - F_END) / 2 + 0.1, 'Encoder fingers: +x catch taper')

    for i in range(tl.count - 12, tl.count):
        it = tl.item(i)
        print(i, it.name, it.healthState, it.errorOrWarningMessage)
    bb = body.boundingBox
    print('body', [round(v * 10, 2) for v in (bb.minPoint.x, bb.minPoint.y, bb.minPoint.z, bb.maxPoint.x, bb.maxPoint.y, bb.maxPoint.z)],
          'vol', round(body.volume * 1000, 1))
