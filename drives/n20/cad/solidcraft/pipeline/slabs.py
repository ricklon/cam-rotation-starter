# Each prismatic tool body (planar faces only) -> slabs along the one axis all its faces are parallel or
# perpendicular to: the section outline of each slab as line segments (mm, world), for native extrudes.
import adsk.core, adsk.fusion, os, json
D = r'C:\Users\rickr\AppData\Local\Temp\claude\D--OneDrive-Documents-GitHub-cam-rotation-starter\1521e62a-56d0-44ea-9b68-dde3664e7231\scratchpad\sc\v22'
AX = {'X': (1, 0, 0), 'Y': (0, 1, 0), 'Z': (0, 0, 1)}

def run(_context: str):
    tb = adsk.fusion.TemporaryBRepManager.get()
    out = {}
    for f in sorted(os.listdir(D)):
        if not f.endswith('.smt'):
            continue
        b = tb.createFromFile(os.path.join(D, f)).item(0)
        res = None
        for name, a in AX.items():
            hs, ok = set(), True
            for fc in b.faces:
                n = fc.geometry.normal
                dot = n.x * a[0] + n.y * a[1] + n.z * a[2]
                if abs(abs(dot) - 1) < 1e-6:
                    o = fc.geometry.origin
                    hs.add(round((o.x * a[0] + o.y * a[1] + o.z * a[2]) * 10, 5))
                elif abs(dot) > 1e-6:
                    ok = False
                    break
            if ok:
                res = (name, a, sorted(hs))
                break
        if not res:
            print(f, 'not prismatic on X/Y/Z')
            continue
        name, a, hs = res
        slabs = []
        for h0, h1 in zip(hs, hs[1:]):
            mid = (h0 + h1) / 20.0  # cm
            pl = adsk.core.Plane.create(adsk.core.Point3D.create(a[0] * mid, a[1] * mid, a[2] * mid), adsk.core.Vector3D.create(*a))
            w = tb.planeIntersection(b, pl)
            segs = []
            if w:
                for e in w.edges:
                    p, q = e.startVertex.geometry, e.endVertex.geometry
                    segs.append([[round(p.x * 10, 5), round(p.y * 10, 5), round(p.z * 10, 5)], [round(q.x * 10, 5), round(q.y * 10, 5), round(q.z * 10, 5)]])
            slabs.append({'h0': h0, 'h1': h1, 'segs': segs})
        out[f] = {'axis': name, 'vol': b.volume * 1000, 'slabs': slabs}
        print(f, name, 'heights', hs, 'segs per slab', [len(s['segs']) for s in slabs])
    json.dump(out, open(os.path.join(D, 'slabs.json'), 'w'), indent=1)
