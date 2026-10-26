"""Rotate exported STLs (model frame) into print orientation, drop to z=0, center on x/y.
Usage: python3 drives/mg90s/cad/fusion/orient_stl.py  (reads cad/stl/raw/*.stl exported from Fusion, writes print-ready cad/stl/*.stl and updates the parts table in cad/stl/README.md)"""
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
    'N20_Mount':              (FLIP, 'flange (wall face) down'),
    'N20_Cartridge':          (FLIP, 'bottom flange down, motor plate up'),
    'N20_Control_Pod':        (SIDE + FLIP, 'front panel down, open back up'),
    'N20_PETG_Fit_Coupon':    (FLIP, 'flat side down, labels up'),
    # mount fit coupon pieces: always flipped, so each prints the way the mount and pod do, whichever face has more contact
    'N20_Mount_Fit_Coupon_Wall': ([rx(180)], 'mount fit coupon: wall top down, as the mount'),
    'N20_Mount_Fit_Coupon_Base': ([rx(180)], 'mount fit coupon: base inside face down, underside up as on the mount'),
    'N20_Mount_Fit_Coupon_Pod':  ([rx(180)], 'mount fit coupon: pod front panel down, as the pod'),
    'N20_Adapter_075':        (FLIP, 'D-bore end down, socket up'),
    'N20_Adapter_0625':       (FLIP, 'D-bore end down, socket up'),
    'N20_Adapter_025':        (FLIP, 'D-bore end down, socket up'),
    'N20_Adapter_Clamp_0625': (FLIP, '5/8 clamp, D-bore end down'),
    'N20_Adapter_Clamp_025':  ([rx(180)], '1/4 clamp, socket end down (more bed contact)'),
    'N20_Adapter_075_Sleeved': (FLIP, '3/4 sleeved, D-bore end down'),
    'N20_Clamp_0625_cap':     (SIDE, '5/8 clamp cap, split face down'),
    'N20_Clamp_025_cap':      (SIDE, '1/4 clamp cap, split face down'),
}

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
        f.write(b'Cam Rotation N20 print part'.ljust(80, b' ')); f.write(struct.pack('<I', len(tri))); f.write(rec.tobytes())

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
