"""One v1B shoe-shape candidate from accepted v1A; source and motion stay intact."""
import argparse, hashlib, json, math, sys
from pathlib import Path
import bpy
from mathutils import Vector, Matrix
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
from revise_footprint import snapshot

HERE=Path(__file__).resolve().parent
SOURCE=HERE/'pitcher.blend'
TARGET=HERE/'review/style-feet-v1b/pitcher.blend'
BASELINE='3fbd587e767c4a154a702bdeeca742d871b7ed16a1647e283c76746e00d1fb5a'

def main(out):
    assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==BASELINE
    assert not TARGET.exists(),'Do not overwrite an existing review candidate'
    bpy.ops.wm.open_mainfile(filepath=str(SOURCE));bpy.context.scene.frame_set(1)
    constants,positions,oldrows=snapshot()
    obj=bpy.data.objects['PitcherMesh'];rig=bpy.data.objects['PitcherRig']
    expected=[p[:] for p in positions];feet={}
    # Existing 11 latitude rings, 20 vertices each (including repeated poles).
    # +Y in the identity foot rest basis is front: GLB -Z, game -Z.
    radii=(0,.34,.60,.82,.94,.97,1,1,.75,.4,0)
    heights=(1,.96,.83,.66,.49,.32,.20,0,0,0,0)
    for name in ('foot_R','foot_L'):
        group=obj.vertex_groups[name].index
        ids=[v.index for v in obj.data.vertices if any(g.group==group and g.weight>0 for g in v.groups)]
        assert len(ids)==220 and ids==list(range(ids[0],ids[0]+220))
        assert all([(g.group,g.weight) for g in obj.data.vertices[i].groups if g.weight>0]==[(group,1.0)] for i in ids)
        anchor=rig.data.bones[name].head_local
        assert rig.data.bones[name].matrix_local.to_3x3()==Matrix.Identity(3)
        width=(max(positions[i][0] for i in ids)-min(positions[i][0] for i in ids))/2
        length=max(positions[i][1] for i in ids);height=max(positions[i][2] for i in ids)
        for ordinal,i in enumerate(ids):
            ring,segment=divmod(ordinal,20);theta=segment*2*math.pi/20
            cx,sy=-math.cos(theta),-math.sin(theta)
            # Round forefoot; squarer, narrower rear contour. Sole is geometry only.
            x=math.copysign(abs(cx)**(.55 if sy<0 else .85),cx)*(1-.22*max(-sy,0))
            y=math.copysign(abs(sy)**(.55 if sy<0 else 1),sy)
            radius=radii[ring];px=anchor.x+width*radius*x
            py=length*radius*y-.14*(1-radius) if ring<7 else length*radius*y
            # A modest rigid toe spring preserves the existing toe-down lift clearance.
            spring=.046*(max(0,py-.16)/(length-.16))**1.35
            pz=spring+height*heights[ring]*(1-.40*max(sy,0)*radius)
            expected[i]=list(Vector((px,py,pz)));obj.data.vertices[i].co=expected[i]
        feet[name]={'ids':ids,'anchor_blender':list(anchor),'bounds_before':[[min(positions[i][k] for i in ids) for k in range(3)],[max(positions[i][k] for i in ids) for k in range(3)]], 'bounds_after':[[min(expected[i][k] for i in ids) for k in range(3)],[max(expected[i][k] for i in ids) for k in range(3)]]}
    obj.data.update();bpy.context.scene.frame_set(1)
    newconstants,actual,newrows=snapshot()
    assert constants==newconstants,'Source constants/animation changed'
    footids={i for foot in feet.values() for i in foot['ids']};nonfoot=set(range(len(positions)))-footids
    assert all(positions[i]==actual[i] for i in nonfoot)
    contacts=json.loads(constants['contacts']);ground=[]
    for f,(old,new) in enumerate(zip(oldrows,newrows),1):
        assert old['bones']==new['bones'],'Evaluated bones/grip changed'
        assert all(old['points'][i]==new['points'][i] for i in nonfoot),'Non-foot evaluated mesh changed'
        for name,foot in feet.items():
            bottom=min(new['points'][i][2] for i in foot['ids']);oldbottom=min(old['points'][i][2] for i in foot['ids'])
            planted=any(a<=f<=b for a,b in contacts[name]);ground.append({'frame':f,'foot':name,'planted':planted,'before':oldbottom,'after':bottom})
    out.mkdir(parents=True,exist_ok=True);(out/'grounding.json').write_text(json.dumps(ground,indent=2))
    assert all(v['after']>=-1e-6 for v in ground),f"Ground penetration: {min(ground,key=lambda v:v['after'])}"
    assert all(abs(v['after']-v['before'])<1e-7 for v in ground if v['planted'])
    assert all(v['after']>1e-6 for v in ground if not v['planted'])
    for foot in feet.values():
        size=lambda b:[b[1][k]-b[0][k] for k in range(3)]
        assert all(abs(a/b-1)<1e-6 for a,b in zip(size(foot['bounds_before']),size(foot['bounds_after'])))
    TARGET.parent.mkdir(parents=True,exist_ok=True);bpy.context.scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=str(TARGET));bpy.ops.wm.open_mainfile(filepath=str(TARGET))
    saved,savedpositions,savedrows=snapshot()
    assert constants==saved and savedpositions==expected and savedrows==newrows,'Saved candidate differs'
    assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==BASELINE
    report={'source_sha256':BASELINE,'candidate_sha256':hashlib.sha256(TARGET.read_bytes()).hexdigest(),'frames_compared':205,'keys_constants_bones_grip_nonfoot_exact':True,'feet':feet,'foot_vertex_count':440,'nonfoot_vertex_count':len(nonfoot),'topology_weights_colors_exact':True,'planted_bottom_max_delta_m':max(abs(v['after']-v['before']) for v in ground if v['planted']),'airborne_min_bottom_m':min(v['after'] for v in ground if not v['planted']),'airborne_max_bottom_delta_m':max(abs(v['after']-v['before']) for v in ground if not v['planted'])}
    (out/'source-comparison.json').write_text(json.dumps(report,indent=2));print('V1B_SOURCE_PASS',{k:v for k,v in report.items() if k!='feet'})

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True);main(p.parse_args(sys.argv[sys.argv.index('--')+1:]).evidence)
