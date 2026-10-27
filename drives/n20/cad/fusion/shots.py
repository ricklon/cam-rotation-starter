"""Renders for drives/n20/docs/images. Run inside Fusion with the Cam Rotation N20 design active:
set CAM_REPO to the checkout (CAM_DRIVE is set here), then exec this file and call run_shots()."""
import os
REPO = os.environ.get('CAM_REPO', os.path.expanduser('~/cam-rotation-starter'))
os.environ['CAM_DRIVE'] = os.path.join(REPO, 'drives', 'n20')
exec(open(os.path.join(REPO, 'drives', 'mg90s', 'cad', 'fusion', 'photo.py')).read(), globals())

ALWAYS_OFF = ('Box_Wall', 'Dowel_075', 'Dowel_025', 'Dowel_0625', 'Fit_Test_Coupon', 'N20_Fit_Coupon', 'PETG_Fit_Coupon',
              'Panel_Fit_Coupon', 'Dowel_Fit_Gauge')
CART = ('N20_Cartridge', 'N20 DC MOTOR', 'HW_Arm_Zip_Tie')
POD = ('Control_Pod', 'SW1_Dir_Switch', 'RV1_Speed_Pot', 'S1_12mm_Button', 'J3_JST_PH6')
SHOTS = {
    'wiring_electronics': dict(hide=ALWAYS_OFF, opacity={'Servo_Mount': 0.7}, eye=(5, -400, 2), target=(5, -20, 2), ortho=True, extent=64),
    'wiring_under_base': dict(hide=ALWAYS_OFF + ('Control_Pod',), eye=(70, -90, -320), target=(4, -2, -12), up=(0, 1, 0), ortho=True, extent=66),
    'cartridge_exploded': dict(hide=ALWAYS_OFF + ('Wiring', 'Control_Pod', 'HW_N20_Screws') + POD, opacity={'Servo_Mount': 0.35},
        move={'N20_Cartridge': (0, 0, -32), 'N20 DC MOTOR': (0, 0, -32), 'HW_Arm_Zip_Tie': (0, 0, -32)},
        eye=(170, -220, -40), target=(0, 0, -12), ortho=True, extent=130),
    'cartridge': dict(show=CART, eye=(60, -150, -30), target=(0, 0, -11), ortho=True, extent=50),
    'latch_key': dict(show=('N20_Socket_Coupon', 'N20_Cartridge', 'N20_Latch_Key'), move={'N20_Latch_Key': (0, 0, 14)},
        eye=(-260, -70, 150), target=(-3, 0, 1), ortho=True, extent=44),
    'clamp_0625_caphead': dict(show=('Adapter_Clamp_0625', 'HW_Clamp0625_Screws'), eye=(80, 220, 90), target=(0, 3, 17), ortho=True, extent=42),
    'pod_jst': dict(show=POD + ('Wiring',), wires=('p J3 pin', 'p button', 'J1 ', 'J2 '), opacity={'Control_Pod': 0.45}, eye=(-90, -160, 70), target=(1, 32, 13), ortho=True, extent=46),
    'petg_fit_coupon': dict(show=('PETG_Fit_Coupon',), eye=(200, 0, 300), target=(200, 0, -7), up=(0, 1, 0), ortho=True, extent=108),
}


def run_shots(names=None):
    root = d.rootComponent
    out = []
    for n in (names or list(SHOTS)):
        kw = dict(SHOTS[n])
        show = kw.pop('show', None)
        wires = kw.pop('wires', None)
        saved = [(o, o.isLightBulbOn) for o in root.occurrences]
        wsaved = []
        if wires:
            w = [o for o in root.occurrences if o.component.name == 'Wiring'][0]
            wsaved = [(b, b.isLightBulbOn) for b in w.component.bRepBodies]
            for b, _ in wsaved:
                b.isLightBulbOn = any(k in b.name for k in wires)
        try:
            if show:
                for o in root.occurrences:
                    o.isLightBulbOn = o.component.name in show
            out.append(shoot(n, w=1600, h=1000, **kw))
        finally:
            for o, v in saved:
                o.isLightBulbOn = v
            for b, v in wsaved:
                b.isLightBulbOn = v
    return '\n'.join(l for l in '\n'.join(out).split('\n') if 'saved' in l or 'Trace' in l or 'Error' in l)
