# STEP exports from Cam Rotation N20: each print part (for the comparison) into v22/fusion_ref, and every other
# visible top-level occurrence (bought parts, wiring, hardware) into v22/purchased, with placements.json in each.
import adsk.core, adsk.fusion, json, os

BASE = r'C:\Users\rickr\AppData\Local\Temp\claude\D--OneDrive-Documents-GitHub-cam-rotation-starter\1521e62a-56d0-44ea-9b68-dde3664e7231\scratchpad\sc\v22'
PRINT = ['Servo_Mount', 'N20_Cartridge', 'N20_Latch_Key', 'Adapter_075', 'Adapter_0625', 'Adapter_025',
         'Adapter_075_Sleeved', 'Adapter_Clamp_0625', 'Adapter_Clamp_025', 'Control_Pod']

def export(em, occ, path):
    c = occ.component
    was = [(b, b.isLightBulbOn) for b in c.bRepBodies]
    occ_was = occ.isLightBulbOn
    occ.isLightBulbOn = True
    for b, _ in was:
        b.isLightBulbOn = True
    try:
        em.execute(em.createSTEPExportOptions(path, c))
    finally:
        for b, on in was:
            b.isLightBulbOn = on
        occ.isLightBulbOn = occ_was
    return [b.name for b, _ in was]

def run(_context: str):
    app = adsk.core.Application.get()
    for d in app.documents:
        if d.name.startswith('Cam Rotation N20'):
            d.activate()
    des = adsk.fusion.Design.cast(app.activeDocument.products.itemByProductType('DesignProductType'))
    em = des.exportManager
    ref, buy = {}, {}
    for sub in ('fusion_ref', 'purchased'):
        os.makedirs(os.path.join(BASE, sub), exist_ok=True)
    for o in des.rootComponent.allOccurrences:
        c = o.component
        if c.name in PRINT and c.name not in ref:
            path = os.path.join(BASE, 'fusion_ref', c.name + '.step')
            ref[c.name] = {'path': path, 'matrix': o.transform2.asArray(), 'bodies': export(em, o, path)}
    for o in des.rootComponent.occurrences:
        c = o.component
        if c.name in PRINT or not o.isLightBulbOn:
            continue
        safe = ''.join(ch if ch.isalnum() or ch in '-_' else '_' for ch in o.name)
        path = os.path.join(BASE, 'purchased', safe + '.step')
        export(em, o, path)
        buy[o.name] = {'path': path, 'component': c.name, 'matrix': o.transform2.asArray()}
    json.dump(ref, open(os.path.join(BASE, 'fusion_ref', 'placements.json'), 'w'), indent=1)
    json.dump(buy, open(os.path.join(BASE, 'purchased', 'placements.json'), 'w'), indent=1)
    print('ref', list(ref))
    print('purchased', list(buy))
