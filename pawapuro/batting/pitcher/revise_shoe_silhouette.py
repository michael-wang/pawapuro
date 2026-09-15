"""Reshape only the existing v1B feet into one slender-silhouette review candidate.
No topology construction, animation edit, or official-asset replacement.
"""
import argparse,hashlib,json,sys
from pathlib import Path
import bpy
from mathutils import Vector
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
from revise_footprint import snapshot

HERE=Path(__file__).resolve().parent
SOURCE=HERE/'review/style-feet-v1b/pitcher.blend'
TARGET=HERE/'review/style-feet-v1c/pitcher.blend'
BASELINE='f54437abc9ddcf75474962c1b34d6c30e689144dd30adb806b7f03df1faa943b'

def bounds(points,ids):
    return [[min(points[i][k] for i in ids) for k in range(3)],
            [max(points[i][k] for i in ids) for k in range(3)]]

def main(out):
    assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==BASELINE
    assert not TARGET.exists(),'Refuse to overwrite a review candidate'
    official={e:hashlib.sha256((HERE/('pitcher.'+e)).read_bytes()).hexdigest() for e in ('blend','glb','toml')}
    bpy.ops.wm.open_mainfile(filepath=str(SOURCE));bpy.context.scene.frame_set(1)
    constants,positions,oldrows=snapshot()
    obj=bpy.data.objects['PitcherMesh'];rig=bpy.data.objects['PitcherRig'];expected=[p[:] for p in positions];feet={}
    # These are the existing eleven v1B rings, not newly generated geometry.
    upper_width=(.58,.58,.60,.63,.67,.72,.95,1,1,1,1)
    for name in ('foot_R','foot_L'):
        group=obj.vertex_groups[name].index
        ids=[v.index for v in obj.data.vertices if any(g.group==group and g.weight>0 for g in v.groups)]
        assert len(ids)==220 and ids==list(range(ids[0],ids[0]+220))
        assert all([(g.group,g.weight) for g in obj.data.vertices[i].groups if g.weight>0]==[(group,1.0)] for i in ids)
        anchor=rig.data.bones[name].head_local;front=max(positions[i][1] for i in ids)
        contact_ids=[i for i in ids if positions[i][2]==0.0]
        for ordinal,i in enumerate(ids):
            x,y,z=positions[i];ring=ordinal//20
            if i in contact_ids:continue  # Preserve the complete coplanar support patch.
            t=max(0,min(1,(y-.10)/(front-.10)))
            taper=1-.50*t
            heel=1-.12*max(0,-y/front) if ring<6 else 1
            px=anchor.x+(x-anchor.x)*upper_width[ring]*taper*heel
            py=y+.025*t*t
            expected[i]=list(Vector((px,py,z)));obj.data.vertices[i].co=expected[i]
        upper=ids[:120]
        feet[name]={'ids':ids,'anchor_blender':list(anchor),'contact_ids':contact_ids,
                    'bounds_before':bounds(positions,ids),'bounds_after':bounds(expected,ids),
                    'upper_bounds_before':bounds(positions,upper),'upper_bounds_after':bounds(expected,upper)}
        assert all(positions[i]==expected[i] for i in contact_ids)
    obj.data.update();bpy.context.scene.frame_set(1)
    after,actual,newrows=snapshot();assert constants==after,'Non-geometry data changed'
    footids={i for f in feet.values() for i in f['ids']};nonfoot=set(range(len(positions)))-footids
    assert all(positions[i]==actual[i] for i in nonfoot)
    contacts=json.loads(constants['contacts']);ground=[]
    for f,(old,new) in enumerate(zip(oldrows,newrows),1):
        assert old['bones']==new['bones'],'Evaluated transforms changed'
        assert all(old['points'][i]==new['points'][i] for i in nonfoot),'Non-foot mesh changed'
        for name,foot in feet.items():
            oldbottom=min(old['points'][i][2] for i in foot['ids']);bottom=min(new['points'][i][2] for i in foot['ids'])
            planted=any(a<=f<=b for a,b in contacts[name]);ground.append({'frame':f,'foot':name,'planted':planted,'before':oldbottom,'after':bottom})
            assert all(old['points'][i]==new['points'][i] for i in foot['contact_ids'])
    out.mkdir(parents=True,exist_ok=True);(out/'grounding.json').write_text(json.dumps(ground,indent=2))
    assert all(v['after']>=-1e-6 for v in ground),'Ground penetration'
    assert all(abs(v['after']-v['before'])<1e-7 for v in ground if v['planted'])
    assert all(v['after']>1e-6 for v in ground if not v['planted'])
    TARGET.parent.mkdir(parents=True,exist_ok=True);bpy.context.scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=str(TARGET));bpy.ops.wm.open_mainfile(filepath=str(TARGET))
    saved,savedpositions,savedrows=snapshot()
    assert saved==constants and savedpositions==expected and savedrows==newrows
    assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==BASELINE
    assert all(hashlib.sha256((HERE/('pitcher.'+e)).read_bytes()).hexdigest()==h for e,h in official.items())
    report={'source_sha256':BASELINE,'candidate_sha256':hashlib.sha256(TARGET.read_bytes()).hexdigest(),
            'feet':feet,'frames_compared':205,'keys_constants_bones_grip_nonfoot_exact':True,'topology_weights_colors_exact':True,
            'changed_foot_positions':sum(positions[i]!=expected[i] for i in footids),'contact_patch_exact':True,
            'planted_bottom_max_delta_m':max(abs(v['after']-v['before']) for v in ground if v['planted']),
            'airborne_min_bottom_m':min(v['after'] for v in ground if not v['planted']),
            'airborne_max_bottom_delta_m':max(abs(v['after']-v['before']) for v in ground if not v['planted'])}
    (out/'source-comparison.json').write_text(json.dumps(report,indent=2));print('V1C_SOURCE_PASS',{k:v for k,v in report.items() if k!='feet'})

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True)
    main(p.parse_args(sys.argv[sys.argv.index('--')+1:]).evidence)
