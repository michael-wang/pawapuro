"""Sample-specific A/B/C comparisons; each scope retains its own protection rules."""
import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

p=argparse.ArgumentParser()
p.add_argument('--scope',choices=('ready-lift','arm-glove','follow-through'),default='ready-lift')
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
                    keys.append((fc.data_path,fc.array_index,[(list(k.co),k.interpolation) for k in fc.keyframe_points if (k.co.x<=97 if a.scope=='follow-through' else k.co.x>=72)]))
    rows={}; containment=[]
    default_frame=s.frame_current
    for f in range(1,206):
        s.frame_set(f); dg=bpy.context.evaluated_depsgraph_get(); er=rig.evaluated_get(dg); eo=obj.evaluated_get(dg); em=eo.to_mesh()
        bones={b.name:flat(er.matrix_world@b.matrix) for b in er.pose.bones}
        points=[list(eo.matrix_world@v.co) for v in em.vertices];eo.to_mesh_clear()
        rows[f]={'bones':bones,'mesh':points,
                 'grip_binding':flat(er.pose.bones['hand_R'].matrix.inverted()@er.pose.bones['grip'].matrix)}
        inv_head=(er.matrix_world@er.pose.bones['head'].matrix@rig.data.bones['head'].matrix_local.inverted()).inverted()
        def head_q(point):
            v=B.inverted()@(inv_head@Vector(point))-Vector((0,1.8375,0))
            return sum((x/r)**2 for x,r in zip(v,(.60858,.58212,.55566)))
        rows[f]['left_arm_head_q']=min(head_q(points[i]) for i in range(2230,2362))
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

expected='18894d26d6b2bc29aa2ff2e2a58c3479cf17eb9e1361890f8c8b2e3c2dedadcb' if a.scope=='arm-glove' else '562f1237a3066c89eb0ccb88423e0b227c7c02861b1b014ed9f222be30a11be0'
if a.scope=='follow-through': expected='b3dcccd38ad7c216b18ccb69b4a664240051de0b988618f449737611748dd366'
assert hashlib.sha256(a.before.read_bytes()).hexdigest()==expected, 'Wrong baseline for selected scope'
before=snapshot(a.before);after=snapshot(a.after)

if a.scope=='follow-through':
    # All original A/B constants stay exact, including B's weights and rest roll.
    assert before[0]==after[0], 'Mesh/weights/rest/hierarchy/camera/timing/contacts changed'
    assert before[1]==after[1], 'Protected keys at or before release changed'
    def position(data,f,n):
        m=data[2][f]['bones'][n];return Vector((m[3],m[7],m[11]))
    matrix_error=max(abs(x-y) for f in range(1,98) for n in before[2][f]['bones'] for x,y in zip(before[2][f]['bones'][n],after[2][f]['bones'][n]))
    mesh_error=max((Vector(x)-Vector(y)).length for f in range(1,98) for x,y in zip(before[2][f]['mesh'],after[2][f]['mesh']))
    left_error=max(abs(x-y) for f in range(1,206) for x,y in zip(before[2][f]['bones']['foot_L'],after[2][f]['bones']['foot_L']))
    assert matrix_error<1e-6 and mesh_error<1e-6 and left_error<1e-6, 'Protected evaluated motion changed'
    assert after[4][0]<-.99999 and abs(after[4][2])<1e-5 and after[5]==1, 'Closed Ready/default frame changed'
    assert max(after[3])<.8, 'Clasp ball containment changed'
    direction=B.inverted()@(position(after,79,'hand_L')-position(after,79,'arm_L'))
    home_cos=-direction.z/math.hypot(direction.x,direction.z)
    assert home_cos>.98, 'B glove direction regressed'
    binding=max(abs(x-y) for f in range(1,206) for x,y in zip(before[2][f]['grip_binding'],after[2][f]['grip_binding']))
    assert binding<1e-5, 'Fixed grip binding changed'
    ratios={};before_ratios={}
    for label,data,target in [('before',before,before_ratios),('after',after,ratios)]:
        for f in range(1,206):
            rings=[]
            for ring in range(13):
                vs=[Vector(v) for v in data[2][f]['mesh'][2050+ring*12:2050+(ring+1)*12]]
                centre=sum(vs,Vector())/12
                radius=2.45*(.055*(1-ring/12)+.04*ring/12)
                rings.append(min((v-centre).length for v in vs)/radius)
            target[f]=rings
    assert min(min(v) for v in ratios.values())>.25, 'Right tube ring collapsed'
    # Common rigid carry should preserve every B ring radius, not merely pass a low threshold.
    radius_delta=max(abs(x-y)*2.45*(.055*(1-i/12)+.04*i/12) for f in ratios for i,(x,y) in enumerate(zip(ratios[f],before_ratios[f])))
    assert radius_delta<1e-6, 'B right-arm deformation regressed under carry'
    placement=Vector(before[0]['placement'])
    final={n:list(B.inverted()@position(after,205,n)+placement) for n in ('foot_R','foot_L')}
    assert final['foot_R'][2]<final['foot_L'][2], 'Right foot must land closer to home'
    # Existing inspect_motion checks evaluated sole support in/out of these intervals.
    landing=json.loads(after[0]['contacts'])['foot_R'][-1][0]
    slide=max((position(after,f,'foot_R')-position(after,landing,'foot_R')).length for f in range(landing,206))
    assert slide<1e-6, 'Right foot slides after landing'
    def yaw(data,f,n):
        flat=data[2][f]['bones'][n];m=Matrix([flat[i:i+4] for i in range(0,16,4)])
        rest=Matrix([data[0]['bones'][n][0][i:i+4] for i in range(0,16,4)])
        forward=B.inverted()@(m.to_3x3()@rest.to_3x3().inverted())@B@Vector((0,0,-1))
        return math.degrees(math.atan2(-forward.x,-forward.z))
    chest={f:yaw(after,f,'chest') for f in range(97,206)}
    assert min(chest.values())<chest[97]-10, 'No continued chest rotation after release'
    added=[f for f in range(1,206) if after[2][f]['left_arm_head_q']<1 and before[2][f]['left_arm_head_q']>=1]
    assert not added, 'New glove-arm/head proxy intrusion'
    boundary={label:{n:[{'frame':f,'speed_mps':(position(data,f,n)-position(data,f-1,n)).length*60} for f in range(94,106)] for n in ('chest','grip','foot_R')} for label,data in [('before',before),('after',after)]}
    report={'status':'PASS','scope':a.scope,'before_sha256':expected,'after_sha256':hashlib.sha256(a.after.read_bytes()).hexdigest(),
            'protected_frames':[1,97],'protected_keys_exact':True,'constants_including_weights_exact':True,
            'protected_bone_matrix_max_abs':matrix_error,'protected_mesh_max_m':mesh_error,'left_foot_matrix_max_abs':left_error,
            'matrix_and_mesh_tolerance':1e-6,'grip_binding_matrix_max_abs':binding,'clasp_q_max':max(after[3]),
            'ready_front_game':after[4],'glove_home_cosine_f79':home_cos,'right_ring_min_ratio':min(min(v) for v in ratios.values()),
            'right_ring_radius_max_delta_m':radius_delta,'right_final_contact_frame':landing,'final_feet_game_m':final,
            'right_minus_left_game_z_m':final['foot_R'][2]-final['foot_L'][2],'right_contact_slide_m':slide,
            'chest_release_yaw_deg':chest[97],'chest_min_yaw_deg':min(chest.values()),'chest_min_yaw_frame':min(chest,key=chest.get),
            'boundary_speeds':boundary,'normal_speed_visually_reviewed':False,'runtime_verified':False}
    a.output.write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps({k:v for k,v in report.items() if k!='boundary_speeds'},indent=2))
    sys.exit(0)

if a.scope=='arm-glove':
    def pos(data,f,n):
        m=data[2][f]['bones'][n];return Vector((m[3],m[7],m[11]))
    def drift(names,frames):
        return max(abs(x-y) for f in frames for n in names for x,y in zip(before[2][f]['bones'][n],after[2][f]['bones'][n]))
    body=('root','pelvis','chest','head','foot_R','foot_L')
    left=('arm_L','forearm_L','hand_L')
    right=('arm_R','forearm_R','hand_R')
    for k in before[0]:
        if k!='mesh':assert before[0][k]==after[0][k], 'Protected constant changed: '+k
    changed_weights=[]
    for i,((co,w),(co2,w2)) in enumerate(zip(before[0]['mesh'],after[0]['mesh'])):
        assert co==co2, 'Rest vertex changed'
        if w!=w2:
            assert 2050<=i<2206, 'Weights changed outside right tube'
            changed_weights.append(i)
    protected=drift(body,range(1,206))
    hand=drift(('hand_R','grip'),range(1,206))
    joints=max((pos(before,f,n)-pos(after,f,n)).length for f in range(1,206) for n in right)
    left_outside=drift(left,list(range(1,50))+list(range(97,206)))
    right_outside=drift(right,list(range(1,50))+list(range(110,206)))
    binding=max(abs(x-y) for f in range(1,206) for x,y in zip(before[2][f]['grip_binding'],after[2][f]['grip_binding']))
    assert protected<1e-6 and left_outside<1e-6 and right_outside<1e-6, 'Protected transforms changed'
    # TRS parent compensation is float32; 10 um matches the existing bone-length guard.
    assert max(hand,joints,binding)<1e-5, 'Right hand/grip/joint trajectory drift'
    left_groups={bpy.data.objects['PitcherMesh'].vertex_groups[n].index for n in left}
    left_vertices={i for i,(co,weights) in enumerate(before[0]['mesh']) if any(g in left_groups and w>0 for g,w in weights)}
    unrelated=0.;right_delta=0.;radius_min={};left_lengths=0.
    for f in range(1,206):
        for i,(x,y) in enumerate(zip(before[2][f]['mesh'],after[2][f]['mesh'])):
            d=(Vector(x)-Vector(y)).length
            if 2050<=i<2206:right_delta=max(right_delta,d)
            elif not (50<=f<=96 and i in left_vertices):unrelated=max(unrelated,d)
        for n,child in [('arm_L','forearm_L'),('forearm_L','hand_L')]:
            left_lengths=max(left_lengths,abs((pos(after,f,child)-pos(after,f,n)).length-before[0]['bones'][n][2]))
        ratios=[]
        for ring in range(13):
            vs=[Vector(v) for v in after[2][f]['mesh'][2050+ring*12:2050+(ring+1)*12]]
            centre=sum(vs,Vector())/12
            radius=2.45*(.055*(1-ring/12)+.04*ring/12)
            ratios.append(min((v-centre).length for v in vs)/radius)
        radius_min[f]=min(ratios)
    assert unrelated<1e-5 and left_lengths<1e-5, 'Unrelated mesh or left bone length changed'
    assert min(radius_min.values())>.25, 'Right tube ring collapsed'
    assert max(after[3])<.8 and after[5]==1, 'Clasp containment/default Ready changed'
    added_intrusions=[f for f in range(1,206) if after[2][f]['left_arm_head_q']<1 and before[2][f]['left_arm_head_q']>=1]
    assert not added_intrusions, 'New left arm/head proxy intrusion'
    directions={}
    for label,data in [('before',before),('after',after)]:
        directions[label]={}
        for f in (72,79,81):
            v=B.inverted()@(pos(data,f,'hand_L')-pos(data,f,'arm_L'))
            directions[label][f]={'shoulder_to_glove_game_m':list(v),'horizontal_home_cosine':-v.z/math.hypot(v.x,v.z),'angle_from_home_deg':math.degrees(math.atan2(v.x,-v.z))}
    assert directions['after'][79]['horizontal_home_cosine']>.98, 'Glove does not point home'
    boundaries={label:[{'frame':f,'glove_speed_mps':(pos(data,f,'hand_L')-pos(data,f-1,'hand_L')).length*60} for f in list(range(48,58))+list(range(90,101))] for label,data in [('before',before),('after',after)]}
    report={'status':'PASS','scope':a.scope,'before_sha256':expected,'after_sha256':hashlib.sha256(a.after.read_bytes()).hexdigest(),
            'body_head_feet_matrix_max_abs':protected,'right_hand_grip_matrix_max_abs':hand,'right_joint_position_max_m':joints,
            'grip_binding_matrix_max_abs':binding,'left_outside_50_96_matrix_max_abs':left_outside,'right_outside_50_109_matrix_max_abs':right_outside,
            'unrelated_mesh_max_m':unrelated,'right_tube_allowed_mesh_max_delta_m':right_delta,'changed_right_tube_weight_vertices':len(changed_weights),
            'left_bone_length_max_error_m':left_lengths,'right_ring_min_radius_ratio_all_frames':min(radius_min.values()),'right_ring_ratio_by_frame':radius_min,
            'clasp_ball_glove_q_max':max(after[3]),'new_left_arm_head_proxy_intrusions':added_intrusions,'glove_direction':directions,'boundary_speeds':boundaries,
            'protected_matrix_tolerance':1e-6,'compensated_transform_and_mesh_tolerance':1e-5,'normal_speed_visually_reviewed':False,'runtime_verified':False}
    a.output.write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps({k:v for k,v in report.items() if k not in ('boundary_speeds','right_ring_ratio_by_frame')},indent=2))
    sys.exit(0)

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
