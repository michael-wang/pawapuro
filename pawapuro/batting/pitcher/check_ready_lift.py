"""S0.2A-specific comparison against the preserved ed3d8be S0.1 source."""
import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

p=argparse.ArgumentParser()
p.add_argument('--before',type=Path,required=True)
p.add_argument('--after',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args(sys.argv[sys.argv.index('--')+1:])
B=Matrix(((-1,0,0),(0,0,-1),(0,1,0)))

def snapshot(path):
    bpy.ops.wm.open_mainfile(filepath=str(path.resolve()))
    s=bpy.context.scene; rig=bpy.data.objects['PitcherRig']; obj=bpy.data.objects['PitcherMesh']
    def flat(m): return [v for row in m for v in row]
    constants={'mesh':[(list(v.co),[(g.group,g.weight) for g in v.groups]) for v in obj.data.vertices],
               'faces':[list(p.vertices) for p in obj.data.polygons],
               'colors':[[list(c.color) for c in attr.data] for attr in obj.data.color_attributes],
               'bones':{b.name:(flat(b.matrix_local),b.parent.name if b.parent else None,b.length) for b in rig.data.bones},
               'objects':{o.name:flat(o.matrix_world) for o in (rig,obj)},
               'cameras':{o.name:(flat(o.matrix_world),o.data.lens,o.data.ortho_scale,o.data.shift_x,o.data.shift_y) for o in bpy.data.objects if o.type=='CAMERA'},
               'placement':list(s['placement_game_m']),'scale':s['blockout_proportion_scale_m'],
               'contacts':s['contact_intervals'],'timeline':(s.frame_start,s.frame_end,s.render.fps,s.render.fps_base,[(m.name,m.frame) for m in s.timeline_markers]),
               'ball':[(o.name,list(o.scale),[list(v.co) for v in o.data.vertices]) for o in bpy.data.objects if o.name.startswith('HeldBall_')]}
    keys=[]
    for layer in rig.animation_data.action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for fc in bag.fcurves:
                    keys.append((fc.data_path,fc.array_index,[(list(k.co),k.interpolation) for k in fc.keyframe_points if k.co.x>=72]))
    rows={}; containment=[]
    default_frame=s.frame_current
    for f in range(1,206):
        s.frame_set(f); dg=bpy.context.evaluated_depsgraph_get(); er=rig.evaluated_get(dg); eo=obj.evaluated_get(dg); em=eo.to_mesh()
        bones={b.name:flat(er.matrix_world@b.matrix) for b in er.pose.bones}
        points=[list(eo.matrix_world@v.co) for v in em.vertices];eo.to_mesh_clear()
        rows[f]={'bones':bones,'mesh':points}
        if f<=49:
            hand=er.pose.bones['hand_L']; inv=(hand.matrix@hand.bone.matrix_local.inverted()).inverted()
            centre=er.pose.bones['grip'].matrix.translation
            qmax=0
            # Ball surface well inside the analytic glove ellipsoid; renders also required.
            for i in range(12):
                for j in range(24):
                    theta=math.pi*(i+.5)/12; phi=2*math.pi*j/24
                    pt=centre+Vector((math.sin(theta)*math.cos(phi),math.sin(theta)*math.sin(phi),math.cos(theta)))*.085
                    local=B.inverted()@(inv@pt)-Vector((1.12,1.28,-.22))
                    qmax=max(qmax,sum((v/r)**2 for v,r in zip(local,(.2205,.245,.1715))))
            containment.append(qmax)
    s.frame_set(1)
    chest=rig.pose.bones['chest']; rot=B.inverted()@(chest.matrix.to_3x3()@chest.bone.matrix_local.to_3x3().inverted())@B
    return constants,keys,rows,containment,list(rot@Vector((0,0,-1))),default_frame

assert hashlib.sha256(a.before.read_bytes()).hexdigest()=='562f1237a3066c89eb0ccb88423e0b227c7c02861b1b014ed9f222be30a11be0', 'Before must be ed3d8be S0.1'
before=snapshot(a.before);after=snapshot(a.after)
assert before[0]==after[0], 'Mesh/rest/weights/hierarchy/cameras/contact/ball contract changed'
assert before[1]==after[1], 'Keys at frame 72 or later changed'
matrix_error=max(abs(x-y) for f in range(72,206) for n in before[2][f]['bones'] for x,y in zip(before[2][f]['bones'][n],after[2][f]['bones'][n]))
mesh_error=max((Vector(x)-Vector(y)).length for f in range(72,206) for x,y in zip(before[2][f]['mesh'],after[2][f]['mesh']))
assert matrix_error<1e-6 and mesh_error<1e-6, 'Protected evaluated animation changed'
assert after[4][0]<-.99999 and abs(after[4][2])<1e-5, 'Ready does not face game -X'
assert after[5]==1, 'Saved default is not Ready frame 1'
assert max(after[3])<.8, 'Held ball not contained with margin inside glove during clasp'
def pos(rows,f,name):
    m=rows[f]['bones'][name];return Vector((m[3],m[7],m[11]))
boundary={label:{name:[{'frame':f,'speed_mps':(pos(data[2],f,name)-pos(data[2],f-1,name)).length*60} for f in range(66,77)] for name in ('chest','grip','foot_L')} for label,data in [('before',before),('after',after)]}
report={'status':'PASS','before_sha256':hashlib.sha256(a.before.read_bytes()).hexdigest(),
        'after_sha256':hashlib.sha256(a.after.read_bytes()).hexdigest(),'protected_frames':[72,205],
        'all_13_bones_every_frame_matrix_max_abs':matrix_error,'all_2362_vertices_every_frame_max_m':mesh_error,
        'matrix_tolerance':1e-6,'mesh_tolerance_m':1e-6,'protected_keys_exact':True,'constant_data_exact':True,
        'ready_front_game':after[4],'saved_frame':after[5], 'clasp_ball_glove_q_max':max(after[3]),
        'boundary_speeds':boundary,'normal_speed_visually_reviewed':False,'runtime_verified':False}
a.output.write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in report.items() if k!='boundary_speeds'},indent=2))
