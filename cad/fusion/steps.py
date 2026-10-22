import os
REPO=os.environ.get('CAM_REPO', os.path.expanduser('~/cam-rotation-starter'))
exec(open(os.path.join(REPO,'cad','fusion','photo.py')).read(), globals())
ALT=('Adapter_025','Dowel_025','Bushing_025','Fit_Test_Coupon')
ORDER=[ # build order
 ('Servo_Mount',),
 ('Bushing_075',),
 ('MicroServoMG90S','MG90S_Spline','HW_Servo_Tab_Screws'),
 ('Adapter_075','HW_Horn_Screw'),
 ('usbc_pd_w_switches','XIAO-ESP32S3 v2','D1_1N5817','C1_470uF_10V'),
 ('Control_Pod','RV1_Speed_Pot','SW1_Dir_Switch','S1_Push_Button','HW_Pod_Screws','Wiring'),
 ('Dowel_075',),
 ('Box_Wall','HW_Wall_Screws_Washers','HW_Set_Screw'),
]
ALL=[n for g in ORDER for n in g]
def upto(k):
    shown=[n for g in ORDER[:k+1] for n in g]
    return tuple(n for n in ALL if n not in shown)+ALT
ELEC=('usbc_pd_w_switches','XIAO-ESP32S3 v2','D1_1N5817','C1_470uF_10V')
CTRL=('RV1_Speed_Pot','SW1_Dir_Switch','S1_Push_Button')
SHOTS={
 'xray_plusY': dict(hide=ALT+('Box_Wall',), opacity={'Servo_Mount':0.3,'Control_Pod':0.3}, eye=(260,330,90), target=(3,8,22), up=(0,0,1), ortho=True, extent=120),
 'xray_minusY': dict(hide=ALT+('Box_Wall',), opacity={'Servo_Mount':0.3,'Control_Pod':0.3}, eye=(220,-340,90), target=(3,-8,20), up=(0,0,1), ortho=True, extent=120),
 'exploded_full': dict(hide=ALT+('Wiring',), opacity={'Servo_Mount':0.85},
     move={'MicroServoMG90S':(0,0,-55),'MG90S_Spline':(0,0,-55),'Adapter_075':(70,0,0),'Bushing_075':(-75,0,0),
           'usbc_pd_w_switches':(0,-45,0),'XIAO-ESP32S3 v2':(0,-45,0),'D1_1N5817':(0,-45,0),'C1_470uF_10V':(0,-45,0),
           'Control_Pod':(0,45,0),'RV1_Speed_Pot':(0,80,0),'SW1_Dir_Switch':(0,80,0),'S1_Push_Button':(0,80,0),
           'Dowel_075':(0,0,45),'Box_Wall':(0,0,75),'HW_Servo_Tab_Screws':(0,0,22),'HW_Horn_Screw':(70,0,30),'HW_Set_Screw':(70,22,0),'HW_Pod_Screws':(0,62,0),'HW_Wall_Screws_Washers':(0,0,-30)},
     eye=(400,-420,200), target=(0,0,40), up=(0,0,1), ortho=True, extent=290),
 'step1_bushing': dict(hide=upto(1), move={'Bushing_075':(-55,0,-14)}, eye=(-230,-150,-90), target=(-15,0,22), up=(0,0,1), extent=200),
 'step2_servo': dict(hide=upto(2), move={'MicroServoMG90S':(0,0,-45),'MG90S_Spline':(0,0,-45),'HW_Servo_Tab_Screws':(0,0,18)}, eye=(170,-230,-150), target=(5,0,-5), up=(0,0,1), extent=230),
 'step3_adapter': dict(hide=upto(3), move={'Adapter_075':(55,0,0),'HW_Horn_Screw':(55,0,26)}, eye=(250,-140,60), target=(20,0,12), up=(0,0,1), extent=220),
 'step4_electronics': dict(hide=upto(4), move={'usbc_pd_w_switches':(0,-30,0),'XIAO-ESP32S3 v2':(0,-30,0),'D1_1N5817':(0,-30,0),'C1_470uF_10V':(0,-30,0)}, eye=(280,-190,70), target=(5,-25,10), up=(0,0,1), extent=230),
 'step5_pod': dict(hide=upto(5), move={'Control_Pod':(0,35,0),'RV1_Speed_Pot':(0,65,0),'SW1_Dir_Switch':(0,65,0),'S1_Push_Button':(0,65,0),'HW_Pod_Screws':(0,55,0)}, hide_extra=('Wiring',), eye=(170,290,60), target=(3,45,12), up=(0,0,1), extent=240),
 'step6_slide_on': dict(hide=ALT+('HW_Wall_Screws_Washers','HW_Set_Screw'), move={'Box_Wall':(0,0,75),'Dowel_075':(0,0,75)}, eye=(330,-380,250), target=(0,0,70), up=(0,0,1), ortho=True, extent=270),
 'step7_secure': dict(hide=ALT, move={'HW_Wall_Screws_Washers':(0,0,-18)}, opacity={'Box_Wall':0.55}, eye=(330,-250,-120), target=(8,0,28), up=(0,0,1), ortho=True, extent=150),
}
def run_shots(names):
    out=[]
    for n in names:
        kw=dict(SHOTS[n]); extra=kw.pop('hide_extra',())
        kw['hide']=tuple(kw.get('hide',()))+tuple(extra)
        out.append(shoot(n, w=1600, h=1000, **kw))
    return '\n'.join(out)
