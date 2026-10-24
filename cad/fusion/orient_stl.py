"""Rotate exported STLs (model frame) into print orientation, drop to z=0, center on x/y.
Usage: python3 cad/fusion/orient_stl.py  (reads cad/stl/raw/*.stl exported from Fusion, writes print-ready cad/stl/*.stl and updates the parts table in cad/stl/README.md)"""
import numpy as np, struct, os, sys
BASE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(BASE, '..', '..'))
RAW = os.path.join(REPO, 'cad', 'stl', 'raw'); OUT = os.path.join(REPO, 'cad', 'stl')

def rx(deg):
    t = np.radians(deg); c, s = np.cos(t), np.sin(t)
    return np.array([[1, 0, 0], [0, c, -s], [0, s, c]])

# part -> (rotation, note). Model frame: z up toward the box wall, +Y toward the control pod.
# part -> (candidate rotations, note). The candidate with the largest flat contact on the bed wins,
# so this works whatever frame the STL was exported in.
FLIP = [np.eye(3), rx(180)]
SIDE = [rx(90), rx(-90)]
PLAN = {
    'Servo_Mount':     (FLIP, 'flange (wall face) down'),
    'Control_Pod':     (SIDE + FLIP, 'front panel down, open back up'),
    'Adapter_075':     (FLIP, 'spline end down, socket up'),
    'Adapter_0625':    (FLIP, 'spline end down, socket up'),
    'Adapter_025':     (FLIP, 'spline end down, socket up'),
    'Bushing_075':     (FLIP, 'flange down'),
    'Bushing_0625':    (FLIP, 'flange down'),
    'Bushing_025':     (FLIP, 'flange down'),
    'Fit_Test_Coupon': (FLIP, 'flat side down'),
    'Adapter_Clamp_0625': (FLIP, '5/8 clamp, spline end down'),
    # caps: SIDE only, so the split face stays down (the end face has about as much area)
    'Clamp_0625_cap':  (SIDE, '5/8 clamp cap, split face down'),
    'Adapter_Clamp_025': (FLIP, '1/4 clamp, spline end down'),
    'Clamp_025_cap':   (SIDE, '1/4 clamp cap, split face down'),
    'Adapter_075_Sleeved': (FLIP, '3/4 sleeved, spline end down'),
    'Panel_Fit_Coupon': (SIDE + FLIP, 'front panel down, like the pod'),
    'Dowel_Gauge_075': (FLIP, '3/4 dowel gauge strip, flat'),
    'Dowel_Gauge_0625': (FLIP, '5/8 dowel gauge strip, flat'),
    'Dowel_Gauge_025': (FLIP, '1/4 dowel gauge strip, flat'),
}
for _n, _d in (('19p0', '19.0'), ('18p5', '18.5'), ('18p0', '18.0'), ('17p5', '17.5')):
    PLAN['Shim_075_for_%smm' % _n] = (FLIP, 'shim for %s mm dowel, on end' % _d)

def read(p):
    b = open(p, 'rb').read(); n = struct.unpack('<I', b[80:84])[0]
    a = np.frombuffer(b[84:84 + n * 50], dtype=np.dtype([('n', '<f4', 3), ('v', '<f4', (3, 3)), ('a', '<u2')]))
    return a['v'].astype(np.float64)

def write(p, tri):
    n = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    n /= np.maximum(np.linalg.norm(n, axis=1, keepdims=True), 1e-12)
    rec = np.zeros(len(tri), dtype=np.dtype([('n', '<f4', 3), ('v', '<f4', (3, 3)), ('a', '<u2')]))
    rec['n'] = n; rec['v'] = tri
    with open(p, 'wb') as f:
        f.write(b'Cam Rotation Starter print test'.ljust(80, b' ')); f.write(struct.pack('<I', len(tri))); f.write(rec.tobytes())

def bed_area(tri, tol=0.05):
    """area of downward-facing triangles lying on z=0 (contact patch with the bed)"""
    cr = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    flat = (np.abs(tri[:, :, 2]).max(axis=1) < tol) & (cr[:, 2] < 0)
    return 0.5 * np.linalg.norm(cr[flat], axis=1).sum()

lines = ['| Part | Orientation | Size on bed (mm) | Bed contact (mm²) |', '|---|---|---|---|']
def place(tri, R):
    t = tri @ R.T
    mn, mx = t.reshape(-1, 3).min(0), t.reshape(-1, 3).max(0)
    return t - np.array([(mn[0] + mx[0]) / 2, (mn[1] + mx[1]) / 2, mn[2]])

for name, (cands, note) in PLAN.items():
    src = os.path.join(RAW, name + '.stl')
    if not os.path.exists(src): print('missing', name); continue
    tri = read(src)
    # Fusion exports STL in mm (volumes checked against the model)
    tri = max((place(tri, R) for R in cands), key=bed_area)
    sz = tri.reshape(-1, 3).max(0) - tri.reshape(-1, 3).min(0)
    write(os.path.join(OUT, name + '.stl'), tri)
    lines.append('| %s | %s | %.1f × %.1f × %.1f | %.0f |' % (name, note, sz[0], sz[1], sz[2], bed_area(tri)))
readme = os.path.join(OUT, 'README.md')
table = '<!-- parts -->\n' + '\n'.join(lines) + '\n<!-- /parts -->'
txt = open(readme).read() if os.path.exists(readme) else '# Print test parts\n\nExported from Fusion, oriented for printing, z=0 is the bed.\n\n<!-- parts -->\n<!-- /parts -->\n'
if '<!-- parts -->' not in txt:  # first run of the new format: replace the old table block
    import re
    txt = re.sub(r'\| Part \|.*?\n(?=\n)', '<!-- parts -->\n<!-- /parts -->\n', txt, count=1, flags=re.S)
a, b = txt.index('<!-- parts -->'), txt.index('<!-- /parts -->') + len('<!-- /parts -->')
open(readme, 'w').write(txt[:a] + table + txt[b:])
print('\n'.join(lines))
