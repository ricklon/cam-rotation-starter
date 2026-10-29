import adsk.core, adsk.fusion, json, os
BASE = r'C:\Users\rickr\AppData\Local\Temp\claude\D--OneDrive-Documents-GitHub-cam-rotation-starter\1521e62a-56d0-44ea-9b68-dde3664e7231\scratchpad\sc\v22\purchased'
def run(_context: str):
    app = adsk.core.Application.get()
    des = adsk.fusion.Design.cast(app.activeDocument.products.itemByProductType('DesignProductType'))
    em = des.exportManager
    pl = json.load(open(os.path.join(BASE, 'placements.json')))
    for o in des.rootComponent.occurrences:
        if o.name == 'U3_DRV8833:1':
            c = o.component
            was = [(b, b.isLightBulbOn) for b in c.bRepBodies]; ow = o.isLightBulbOn
            o.isLightBulbOn = True
            for b, _ in was: b.isLightBulbOn = True
            path = os.path.join(BASE, 'U3_DRV8833_1.step')
            try:
                em.execute(em.createSTEPExportOptions(path, c))
            finally:
                for b, on in was: b.isLightBulbOn = on
                o.isLightBulbOn = ow
            pl[o.name] = {'path': path, 'component': c.name, 'matrix': o.transform2.asArray()}
            print('exported', [b.name for b, _ in was])
    json.dump(pl, open(os.path.join(BASE, 'placements.json'), 'w'), indent=1)
