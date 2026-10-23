import adsk.core, adsk.fusion, traceback, math
app=adsk.core.Application.get(); d=adsk.fusion.Design.cast(app.activeProduct)
import os
# Fusion runs these scripts with exec(), so set REPO to your checkout of the repository.
REPO=os.environ.get('CAM_REPO', os.path.expanduser('~/cam-rotation-starter'))
# this drive's folder inside the repo (scripts, docs and images live under drives/<name>/)
DRIVE=os.environ.get('CAM_DRIVE', os.path.join(REPO,'drives','mg90s'))
OUT=os.path.join(DRIVE,'docs','images')+'/'
P=adsk.core.Point3D.create; Vc=adsk.core.Vector3D.create
def occs():
    return {o.component.name:o for o in d.rootComponent.occurrences}
def shoot(name, hide=(), opacity=None, move=None, eye=(0,0,0), target=(0,0,0), up=(0,0,1), ortho=False, w=1800, h=1100, extent=None, save=True):
    """move: {comp: (dx,dy,dz) mm}; opacity: {comp: 0..1}; eye/target in mm"""
    # names match every top-level occurrence of that component (imported hardware often has several)
    ALLO=list(d.rootComponent.occurrences); log=[]
    named=lambda n:[o for o in ALLO if o.component.name==n]
    state=[(o,o.isLightBulbOn,[(b,b.opacity) for b in o.bRepBodies],o.transform2.copy()) for o in ALLO]
    try:
        for n in hide:
            for o in named(n): o.isLightBulbOn=False
        for n,v in (opacity or {}).items():
            for o in named(n):
                for b in o.bRepBodies: b.opacity=v
        for n,(dx,dy,dz) in (move or {}).items():
            for o in named(n):
                m=o.transform2.copy(); t=m.translation; t.add(Vc(dx/10,dy/10,dz/10)); m.translation=t; o.transform2=m
        for c in d.allComponents:
            for attr in ('isJointsFolderLightBulbOn','isJointOriginsFolderLightBulbOn','isConstructionFolderLightBulbOn','isSketchFolderLightBulbOn','isOriginFolderLightBulbOn'):
                try: setattr(c,attr,False)
                except: pass
        vp=app.activeViewport; cam=vp.camera
        cam.isSmoothTransition=False
        cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType if ortho else adsk.core.CameraTypes.PerspectiveCameraType
        vp.camera=cam; adsk.doEvents(); cam=vp.camera; cam.isSmoothTransition=False
        cam.eye=P(*[v/10 for v in eye]); cam.target=P(*[v/10 for v in target]); cam.upVector=Vc(*up)
        cam.isFitView=False
        if extent: cam.viewExtents=extent/10
        vp.camera=cam; adsk.doEvents()
        if extent and ortho:
            cam=vp.camera; cam.isSmoothTransition=False; cam.viewExtents=extent/10; vp.camera=cam
        vp.refresh(); adsk.doEvents()
        log.append('extents now %.2f'%vp.camera.viewExtents)
        if save:
            op=adsk.core.SaveImageFileOptions.create(OUT+name+'.png'); op.width=w; op.height=h; op.isAntiAliased=True; op.isBackgroundTransparent=True
            log.append('%s saved=%s'%(name,vp.saveAsImageFileWithOptions(op)))
    except: log.append(traceback.format_exc())
    finally:
        if save:
            for o,lb,ops,tr in state:
                o.transform2=tr; o.isLightBulbOn=lb
                for b,v in ops: b.opacity=v
            log.append('restored; pending snapshot=%s'%d.snapshots.hasPendingSnapshot)
    return '\n'.join(log)
CFG25=('Adapter_025','Dowel_025','Bushing_025','Fit_Test_Coupon')
