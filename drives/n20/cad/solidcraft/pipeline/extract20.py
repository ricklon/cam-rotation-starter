# Walks the N20 design's timeline and writes the print parts' features as neutral JSON (mm, world space,
# using each component's final occurrence placement). Run in Fusion via fusion.js.
import adsk.core, adsk.fusion, json, math, traceback

OUT = r'C:\Users\rickr\AppData\Local\Temp\claude\D--OneDrive-Documents-GitHub-cam-rotation-starter\1521e62a-56d0-44ea-9b68-dde3664e7231\scratchpad\sc\v22\__COMP__.json'
COMPS = ['__COMP__']

def mm(v): return v * 10.0

def xf(m, p):
    q = p.copy(); q.transformBy(m); return [mm(q.x), mm(q.y), mm(q.z)]

def xv(m, v):
    w = v.copy(); w.transformBy(m); w.normalize(); return [w.x, w.y, w.z]

def curve2d(g):
    """Sketch-space Curve3D -> neutral 2D curve (mm)."""
    t = g.objectType.split('::')[-1]
    if t == 'Line3D':
        return {'t': 'line', 'p': [[mm(g.startPoint.x), mm(g.startPoint.y)], [mm(g.endPoint.x), mm(g.endPoint.y)]]}
    if t == 'Circle3D':
        return {'t': 'circle', 'c': [mm(g.center.x), mm(g.center.y)], 'r': mm(g.radius)}
    ev = g.evaluator
    ok, p0, p1 = ev.getParameterExtents()
    pts = []
    n = 2 if t == 'Arc3D' else 24
    for i in range(n + 1):
        ok, p = ev.getPointAtParameter(p0 + (p1 - p0) * i / n)
        pts.append([mm(p.x), mm(p.y)])
    if t == 'Arc3D':
        return {'t': 'arc', 's': pts[0], 'm': pts[1], 'e': pts[2]}
    return {'t': 'spline', 'pts': pts, 'src': t}

def loops_of_profile(prof):
    out = []
    for lp in prof.profileLoops:
        cs = []
        for pc in lp.profileCurves:
            cs.append(curve2d(pc.geometry))
        out.append({'outer': lp.isOuter, 'curves': cs})
    return out

def body_names(coll):
    return [b.name for b in coll] if coll else []

def edge_points(edges, m):
    pts = []
    for e in edges:
        # component-space geometry is stable over the timeline; the occurrence moves (snapshots), so use the final one
        if e.assemblyContext:
            e = e.nativeObject
        w = m
        ev = e.evaluator
        ok, a, b = ev.getParameterExtents()
        ok, p = ev.getPointAtParameter(a + (b - a) * 0.37)  # off-centre: clear of seams
        pts.append({'p': xf(w, p), 'body': e.body.name})
    return pts

def plane_dist(entity, origin_c, normal_c):
    """Signed distance (cm, component space) from the sketch plane to an extent entity, along the sketch normal."""
    if getattr(entity, 'assemblyContext', None):
        entity = entity.nativeObject
    t = entity.objectType.split('::')[-1]
    if t == 'BRepFace':
        g = entity.geometry
        if g.objectType.split('::')[-1] != 'Plane':
            ok, pt = entity.evaluator.getPointAtParameter(adsk.core.Point2D.create(0.5, 0.5))
            p = entity.pointOnFace
        else:
            p = g.origin
            # nearest point of the plane on the sketch normal line
            n = g.normal
            d = (p.x - origin_c.x) * n.x + (p.y - origin_c.y) * n.y + (p.z - origin_c.z) * n.z
            den = normal_c.x * n.x + normal_c.y * n.y + normal_c.z * n.z
            return d / den
    elif t == 'ConstructionPlane':
        g = entity.geometry
        n = g.normal; p = g.origin
        d = (p.x - origin_c.x) * n.x + (p.y - origin_c.y) * n.y + (p.z - origin_c.z) * n.z
        den = normal_c.x * n.x + normal_c.y * n.y + normal_c.z * n.z
        return d / den
    elif t == 'BRepVertex':
        p = entity.geometry
    elif t == 'SketchPoint':
        p = entity.worldGeometry
    else:
        raise Exception('extent entity ' + t)
    return (p.x - origin_c.x) * normal_c.x + (p.y - origin_c.y) * normal_c.y + (p.z - origin_c.z) * normal_c.z

def extent(ed, origin_c, normal_c):
    t = ed.objectType.split('::')[-1]
    if t == 'DistanceExtentDefinition':
        return {'k': 'dist', 'd': mm(ed.distance.value)}
    if t == 'ThroughAllExtentDefinition':
        return {'k': 'all', 'pos': ed.isPositiveDirection}
    if t == 'ToEntityExtentDefinition':
        d = plane_dist(ed.entity, origin_c, normal_c) + (ed.offset.value if ed.offset else 0)
        return {'k': 'dist', 'd': mm(d), 'to': True}
    if t == 'SymmetricExtentDefinition':
        d = mm(ed.distance.value)
        return {'k': 'sym', 'total': d if ed.isFullLength else 2 * d}
    raise Exception('extent ' + t)

def run(_context: str):
    app = adsk.core.Application.get()
    for d in app.documents:
        if d.name.startswith('Cam Rotation N20'):
            d.activate()
    des = adsk.fusion.Design.cast(app.activeDocument.products.itemByProductType('DesignProductType'))
    root = des.rootComponent
    tl = des.timeline
    ops, warns = [], []
    occT = {}
    for o in root.allOccurrences:
        if o.component.name in COMPS and o.component.name not in occT:
            occT[o.component.name] = o.transform2.copy()
    comp = [c for c in des.allComponents if c.name == COMPS[0]][0]
    idx = []
    for coll in ('extrudeFeatures', 'chamferFeatures', 'filletFeatures', 'combineFeatures', 'baseFeatures', 'moveFeatures', 'mirrorFeatures', 'revolveFeatures', 'holeFeatures', 'splitBodyFeatures'):
        try:
            fc = getattr(comp.features, coll)
            for k in range(fc.count):
                ti = fc.item(k).timelineObject
                if ti: idx.append(ti.index)
        except Exception as ex:
            warns.append('collect %s %s' % (coll, ex))
    idx.sort()
    try:
        for i in idx:
            it = tl.item(i)
            if it.isSuppressed or it.isGroup:
                continue
            e = it.entity
            if e is None:
                continue
            t = e.objectType.split('::')[-1]
            if t not in ('ExtrudeFeature', 'ChamferFeature', 'FilletFeature', 'CombineFeature', 'MoveFeature',
                         'SplitBodyFeature', 'MirrorFeature', 'RectangularPatternFeature', 'CircularPatternFeature',
                         'HoleFeature', 'RevolveFeature', 'ShellFeature', 'RemoveFeature', 'BaseFeature'):
                continue
            try:
                cname = e.parentComponent.name
            except Exception:
                continue
            if cname not in COMPS:
                continue
            m = occT[cname]
            op = {'i': i, 'name': it.name, 'type': t, 'comp': cname}
            if t == 'ExtrudeFeature':
                it.rollTo(True)
                profs = e.profile
                plist = []
                if profs.objectType.split('::')[-1] == 'ObjectCollection':
                    plist = [profs.item(k) for k in range(profs.count)]
                else:
                    plist = [profs]
                sk = None
                plist = [p.nativeObject if p.assemblyContext else p for p in plist]
                for p in plist:
                    pt = p.objectType.split('::')[-1]
                    if pt != 'Profile':
                        raise Exception('profile type ' + pt)
                    sk = p.parentSketch
                    if sk.assemblyContext:
                        sk = sk.nativeObject
                st = sk.transform  # sketch -> component space
                full = st.copy(); full.transformBy(m)
                o = adsk.core.Point3D.create(0, 0, 0)
                op['plane'] = {'origin': xf(full, o), 'x_dir': xv(full, adsk.core.Vector3D.create(1, 0, 0)),
                               'y_dir': xv(full, adsk.core.Vector3D.create(0, 1, 0))}
                oc = o.copy(); oc.transformBy(st)
                nc = adsk.core.Vector3D.create(0, 0, 1); nc.transformBy(st); nc.normalize()
                op['sketch'] = sk.name
                op['profiles'] = [loops_of_profile(p) for p in plist]
                op['operation'] = ['join', 'cut', 'intersect', 'new', 'newcomp'][e.operation]
                op['extentType'] = e.extentType
                op['e1'] = extent(e.extentOne, oc, nc)
                if e.extentType == 1 and e.extentTwo:
                    op['e2'] = extent(e.extentTwo, oc, nc)
                if e.extentType == 2:
                    op['e1'] = extent(e.extentOne, oc, nc)
                sx = e.startExtent
                stt = sx.objectType.split('::')[-1]
                if stt == 'OffsetStartDefinition':
                    op['start_offset'] = mm(sx.offset.value)
                elif stt != 'ProfilePlaneStartDefinition':
                    if stt == 'EntityStartDefinition':
                        op['start_offset'] = mm(plane_dist(sx.entity, oc, nc) + (sx.offset.value if sx.offset else 0))
                    else:
                        warns.append('%d start %s' % (i, stt))
                ta = e.taperAngleOne
                if ta and abs(ta.value) > 1e-9:
                    op['taper'] = math.degrees(ta.value)
                op['participants'] = body_names(e.participantBodies)
                tl.moveToEnd()
                op['bodies'] = body_names(e.bodies)
            elif t in ('ChamferFeature', 'FilletFeature'):
                it.rollTo(True)
                sets = []
                for es in e.edgeSets:
                    est = es.objectType.split('::')[-1]
                    s = {'type': est}
                    edges = es.edges if hasattr(es, 'edges') else es.entities
                    el = []
                    for k in range(edges.count):
                        x = edges.item(k)
                        xt = x.objectType.split('::')[-1]
                        if xt == 'BRepEdge':
                            el.append(x)
                        elif xt == 'BRepFace':
                            for ed in x.edges: el.append(ed)
                            s['from_face'] = True
                        else:
                            warns.append('%d edge entity %s' % (i, xt))
                    s['edges'] = edge_points(el, m)
                    if est == 'EqualDistanceChamferEdgeSet':
                        s['d'] = mm(es.distance.value)
                    elif est == 'TwoDistancesChamferEdgeSet':
                        s['d'] = mm(es.distanceOne.value); s['d2'] = mm(es.distanceTwo.value)
                    elif est == 'DistanceAndAngleChamferEdgeSet':
                        s['d'] = mm(es.distance.value); s['angle'] = math.degrees(es.angle.value)
                    elif est == 'ConstantRadiusFilletEdgeSet':
                        s['r'] = mm(es.radius.value)
                    else:
                        warns.append('%d edgeset %s' % (i, est))
                    sets.append(s)
                op['sets'] = sets
                tl.moveToEnd()
            elif t == 'CombineFeature':
                it.rollTo(True)
                op['target'] = e.targetBody.name
                op['tools'] = [b.name for b in e.toolBodies]
                op['operation'] = ['join', 'cut', 'intersect'][e.operation]
                op['keep'] = e.isKeepToolBodies
                tl.moveToEnd()
            elif t == 'BaseFeature':
                it.rollTo(False)
                tb = adsk.fusion.TemporaryBRepManager.get()
                files = []
                for b in e.bodies:
                    cp = tb.copy(b)
                    tb.transform(cp, m)
                    fn = OUT.replace('.json', '_base_%d_%s.smt' % (i, ''.join(ch if ch.isalnum() else '_' for ch in b.name)))
                    tb.exportToFile([cp], fn)
                    files.append({'body': b.name, 'file': fn})
                op['base_bodies'] = files
                tl.moveToEnd()
            else:
                warns.append('%d %s %s unhandled' % (i, t, it.name))
            # volumes of this component's bodies right after the feature
            it.rollTo(False)
            op['vols'] = {b.name: b.volume * 1000 for b in e.parentComponent.bRepBodies if b.isSolid}
            tl.moveToEnd()
            ops.append(op)
    except Exception:
        warns.append(traceback.format_exc())
    finally:
        tl.moveToEnd()
    finals = {}
    for o in root.allOccurrences:
        if o.component.name in COMPS:
            for b in o.bRepBodies:
                x = b.boundingBox
                finals[b.name] = {'vol': b.volume * 1000, 'faces': b.faces.count,
                                  'bbox': [[mm(x.minPoint.x), mm(x.minPoint.y), mm(x.minPoint.z)], [mm(x.maxPoint.x), mm(x.maxPoint.y), mm(x.maxPoint.z)]]}
    json.dump({'ops': ops, 'warns': warns, 'finals': finals}, open(OUT, 'w'), indent=1)
    print('ops', len(ops), 'warns', warns)
