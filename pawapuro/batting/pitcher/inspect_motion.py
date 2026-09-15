"""Sample-specific evaluated motion evidence; no source mutation or runtime code."""
import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

parser=argparse.ArgumentParser()
parser.add_argument('--output',type=Path,required=True)
args=parser.parse_args(sys.argv[sys.argv.index('--')+1:])
scene=bpy.context.scene; rig=bpy.data.objects['PitcherRig']; obj=bpy.data.objects['PitcherMesh']
B=Matrix(((-1,0,0),(0,0,-1),(0,1,0)))
foot_indices={name:[v.index for v in obj.data.vertices if any(obj.vertex_groups[g.group].name==name and g.weight>.999 for g in v.groups)] for name in ('foot_R','foot_L')}
rows=[]
for frame in range(scene.frame_start,scene.frame_end+1):
    scene.frame_set(frame); dg=bpy.context.evaluated_depsgraph_get(); er=rig.evaluated_get(dg)
    row={'frame':frame,'bones':{}}
    if scene.get('motion_revision') in ('S0.1', 'S0.2A'):
        for held in bpy.data.objects:
            if held.name.startswith('HeldBall_'):
                assert held.hide_render==(frame>scene.timeline_markers['release'].frame), 'Held ball visibility differs from sole release marker'
    for name in ('pelvis','chest','head','arm_R','forearm_R','hand_R','grip','arm_L','hand_L','foot_R','foot_L'):
        bone=er.pose.bones[name]; matrix=er.matrix_world@bone.matrix
        rot=B.inverted()@(matrix.to_3x3()@bone.bone.matrix_local.to_3x3().inverted())@B
        forward=rot@Vector((0,0,-1)); point=B.inverted()@matrix.translation
        row['bones'][name]={'p':list(point),'yaw_deg':math.degrees(math.atan2(-forward.x,-forward.z))}
    points={name:Vector(v['p']) for name,v in row['bones'].items()}
    row['upper_length_m']=(points['forearm_R']-points['arm_R']).length
    row['forearm_length_m']=(points['hand_R']-points['forearm_R']).length
    row['grip_local'] = list((er.pose.bones['hand_R'].matrix.inverted()@er.pose.bones['grip'].matrix).translation)
    evaluated=obj.evaluated_get(dg); mesh=evaluated.to_mesh()
    row['foot_bottom_m']={name:min((evaluated.matrix_world@mesh.vertices[i].co).z for i in indices) for name,indices in foot_indices.items()}
    # Analytic head ellipsoid only: not a full character collision test.
    inv_head=(er.matrix_world@er.pose.bones['head'].matrix@rig.data.bones['head'].matrix_local.inverted()).inverted()
    def head_q(co):
        p=B.inverted()@(inv_head@co)-Vector((0,.75*2.45,0))
        return sum((a/b)**2 for a,b in zip(p,(.23*2.45*1.08,.22*2.45*1.08,.21*2.45*1.08)))
    row['arm_head_ellipsoid_min_q']=min(head_q(evaluated.matrix_world@mesh.vertices[i].co) for i in range(2050+24,2206))
    row['hand_head_ellipsoid_q']=head_q(er.matrix_world@er.pose.bones['hand_R'].matrix.translation)
    evaluated.to_mesh_clear(); rows.append(row)
for i,row in enumerate(rows):
    prev=rows[max(0,i-1)]
    row['grip_speed_mps']=(Vector(row['bones']['grip']['p'])-Vector(prev['bones']['grip']['p'])).length*60
    for name in ('pelvis','chest','head'):
        row[name+'_yaw_speed_dps']=(row['bones'][name]['yaw_deg']-prev['bones'][name]['yaw_deg'])*60
release=scene.timeline_markers['release'].frame
summary={'source':bpy.data.filepath,'frames':len(rows),'release_frame':release,
         'upper_length_range_m':[min(r['upper_length_m'] for r in rows),max(r['upper_length_m'] for r in rows)],
         'forearm_length_range_m':[min(r['forearm_length_m'] for r in rows),max(r['forearm_length_m'] for r in rows)],
         'peak_opening_frames':{name:min(rows,key=lambda r:r[name+'_yaw_speed_dps'])['frame'] for name in ('pelvis','chest')},
         'peak_grip_speed':max(rows,key=lambda r:r['grip_speed_mps'])['grip_speed_mps'],
         'peak_grip_frame':max(rows,key=lambda r:r['grip_speed_mps'])['frame'],
         'arm_head_proxy_intrusion_frames':[r['frame'] for r in rows if r['arm_head_ellipsoid_min_q']<1],
         'min_foot_bottom_m':{name:min(r['foot_bottom_m'][name] for r in rows) for name in foot_indices},
         'contacts':json.loads(scene.get('contact_intervals','{}')),
         'normal_speed_visually_reviewed':False,'reference_video_visually_reviewed':False,
         'proxy_notice':'Head ellipsoid excludes cap, shoulder seam and other character parts; screen overlap is a separate review.'}
if scene.get('motion_revision') in ('S0.1', 'S0.2A'):
    for key, bone in [('upper_length_m','arm_R'),('forearm_length_m','forearm_R')]:
        assert max(abs(r[key]-rig.data.bones[bone].length) for r in rows)<1e-5, 'FK length drift'
    assert not summary['arm_head_proxy_intrusion_frames'], 'Arm enters head ellipsoid proxy'
    assert min(summary['min_foot_bottom_m'].values())>=-1e-6, 'Foot penetrates ground'
    for name, intervals in summary['contacts'].items():
        for first, last in intervals:
            assert max(abs(rows[f-scene.frame_start]['foot_bottom_m'][name]) for f in range(first,last+1))<1e-6, 'Contact interval floats above ground'
        for row in rows:
            if not any(first<=row['frame']<=last for first,last in intervals):
                assert row['foot_bottom_m'][name]>1e-6, 'Foot unexpectedly contacts outside authored support interval'
    assert max((Vector(r['grip_local'])-Vector(rows[0]['grip_local'])).length for r in rows)<1e-6, 'Independent grip drift'
    assert summary['peak_opening_frames']['pelvis'] < summary['peak_opening_frames']['chest'] < release, 'Opening order'
    assert all(rows[f-scene.frame_start]['bones']['grip']['p'][2] < rows[f-scene.frame_start-1]['bones']['grip']['p'][2] for f in range(release-2,release+3)), 'Grip reverses throwing direction around release'
    summary['sample_checks']='PASS'
summary['release_speed_mps']=rows[release-scene.frame_start]['grip_speed_mps']
summary['preparation_peak_grip_speed_mps']=max(r['grip_speed_mps'] for r in rows if r['frame']<=summary['contacts'].get('foot_L',[[1,1],[73,151]])[-1][0])
args.output.parent.mkdir(parents=True,exist_ok=True)
args.output.write_text(json.dumps({'summary':summary,'frames':rows},indent=2),encoding='utf-8')
print(json.dumps(summary,indent=2))
