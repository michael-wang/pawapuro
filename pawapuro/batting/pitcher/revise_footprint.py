"""Create the single v1A 1.25x planar-foot candidate; never replace the official asset.
Run in Blender with --background --python this_file -- --evidence build/style-feet-v1a.
"""
import argparse
import hashlib
import json
import sys
from pathlib import Path
import bpy
from mathutils import Vector

HERE=Path(__file__).resolve().parent
SOURCE=HERE/'pitcher.blend'
TARGET=HERE/'review/style-feet-v1a/pitcher.blend'
BASELINE='8b76ae4a40377fa07021ebdf18c98b3bd73dddf132154961fbcf7131c63c3bca'

def flat(m): return [x for row in m for x in row]
def snapshot():
    scene=bpy.context.scene; rig=bpy.data.objects['PitcherRig']; obj=bpy.data.objects['PitcherMesh']
    constants={'faces':[list(p.vertices) for p in obj.data.polygons],
        'colors':[[list(v.color) for v in a.data] for a in obj.data.color_attributes],
        'weights':[[(g.group,g.weight) for g in v.groups] for v in obj.data.vertices],
        'groups':[g.name for g in obj.vertex_groups],
        'bones':[(b.name,b.parent.name if b.parent else None,flat(b.matrix_local),b.length) for b in rig.data.bones],
        'objects':[(o.name,o.parent.name if o.parent else None,flat(o.matrix_world)) for o in bpy.data.objects],
        'other_meshes':{o.name:[list(v.co) for v in o.data.vertices] for o in bpy.data.objects if o.type=='MESH' and o!=obj},
        'cameras':[(o.name,o.data.lens,o.data.ortho_scale,o.data.shift_x,o.data.shift_y) for o in bpy.data.objects if o.type=='CAMERA'],
        'timeline':(scene.frame_start,scene.frame_end,scene.render.fps,scene.render.fps_base,[(m.name,m.frame) for m in scene.timeline_markers]),
        'contacts':scene['contact_intervals'],'placement':list(scene['placement_game_m'])}
    keys=[]
    for action in bpy.data.actions:
        for layer in action.layers:
            for strip in layer.strips:
                for bag in strip.channelbags:
                    for fc in bag.fcurves:
                        keys.append((action.name,fc.data_path,fc.array_index,fc.extrapolation,
                            [(list(k.co),k.interpolation,list(k.handle_left),list(k.handle_right),k.handle_left_type,k.handle_right_type) for k in fc.keyframe_points]))
    constants['keys']=keys
    rows=[]
    for f in range(1,206):
        scene.frame_set(f); dg=bpy.context.evaluated_depsgraph_get(); er=rig.evaluated_get(dg); eo=obj.evaluated_get(dg); em=eo.to_mesh()
        rows.append({'bones':[flat(er.matrix_world@b.matrix) for b in er.pose.bones],
                     'points':[list(eo.matrix_world@v.co) for v in em.vertices]})
        eo.to_mesh_clear()
    scene.frame_set(1)
    return constants,[list(v.co) for v in obj.data.vertices],rows

def main(evidence):
    assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==BASELINE,'Official baseline changed; inspect first'
    assert not TARGET.exists(),'Candidate already exists; do not overwrite'
    bpy.ops.wm.open_mainfile(filepath=str(SOURCE)); bpy.context.scene.frame_set(1)
    before,positions,rows=snapshot()
    obj=bpy.data.objects['PitcherMesh']; rig=bpy.data.objects['PitcherRig']
    assert not obj.data.shape_keys,'Unexpected shape keys'
    assert flat(obj.matrix_world)==flat(rig.matrix_world),'Unexpected object space'
    feet={}; expected=[p[:] for p in positions]
    for name in ('foot_R','foot_L'):
        group=obj.vertex_groups[name].index
        ids=[v.index for v in obj.data.vertices if any(g.group==group and g.weight>0 for g in v.groups)]
        assert len(ids)==220 and all([(g.group,g.weight) for g in obj.data.vertices[i].groups if g.weight>0]==[(group,1.0)] for i in ids),'Expected rigid feet'
        anchor=obj.matrix_world.inverted()@rig.matrix_world@rig.data.bones[name].head_local
        for i in ids:
            p=positions[i]; expected[i]=list(Vector((anchor.x+(p[0]-anchor.x)*1.25,anchor.y+(p[1]-anchor.y)*1.25,p[2])))
            obj.data.vertices[i].co=expected[i]
        feet[name]={'ids':ids,'anchor_blender':list(anchor),
            'size_before_blender':[max(positions[i][k] for i in ids)-min(positions[i][k] for i in ids) for k in range(3)],
            'size_after_blender':[max(expected[i][k] for i in ids)-min(expected[i][k] for i in ids) for k in range(3)]}
    obj.data.update(); bpy.context.scene.frame_set(1)
    TARGET.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(TARGET))
    bpy.ops.wm.open_mainfile(filepath=str(TARGET))
    after,actual,newrows=snapshot()
    assert before==after,'Non-geometry source/keys changed'
    assert actual==expected,'Saved mesh differs from the exact planar edit'
    foot_ids={i for foot in feet.values() for i in foot['ids']}
    nonfoot=[i for i in range(len(positions)) if i not in foot_ids]
    contacts=json.loads(before['contacts']); max_airborne_bottom_delta=0.0
    for f,(old,new) in enumerate(zip(rows,newrows),1):
        assert old['bones']==new['bones'],'Evaluated bone/grip transforms changed'
        assert all(old['points'][i]==new['points'][i] for i in nonfoot),'Non-foot deformation changed'
        for name,foot in feet.items():
            oldbottom=min(old['points'][i][2] for i in foot['ids']); bottom=min(new['points'][i][2] for i in foot['ids'])
            planted=any(first<=f<=last for first,last in contacts[name])
            assert bottom>=-1e-6,'Foot penetrates the support plane'
            if planted: assert abs(bottom-oldbottom)<1e-7,'Planted bottom changed'
            else: max_airborne_bottom_delta=max(max_airborne_bottom_delta,abs(bottom-oldbottom))
    assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==BASELINE
    report={'source_sha256':BASELINE,'candidate_sha256':hashlib.sha256(TARGET.read_bytes()).hexdigest(),
        'frames_compared':205,'keys_constants_bones_grip_nonfoot_exact':True,'source_vertical_exact':True,
        'planted_bottom_tolerance_m':1e-7,'max_airborne_bottom_delta_m':max_airborne_bottom_delta,'feet':feet}
    evidence.mkdir(parents=True,exist_ok=True)
    (evidence/'source-comparison.json').write_text(json.dumps(report,indent=2))
    print('V1A_SOURCE_PASS', {k:v for k,v in report.items() if k!='feet'})

if __name__=='__main__':
    parser=argparse.ArgumentParser(); parser.add_argument('--evidence',type=Path,required=True)
    args=parser.parse_args(sys.argv[sys.argv.index('--')+1:]); main(args.evidence)
