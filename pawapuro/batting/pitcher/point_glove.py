"""Second S0.2B edit: only left-arm frames 50-96 after the arm strategy review."""
import argparse
import hashlib
import sys
import time
from pathlib import Path

import bpy
from mathutils import Matrix,Vector

p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True)
a=p.parse_args(sys.argv[sys.argv.index('--')+1:]);start=time.perf_counter()
assert not a.output.exists(), 'Refusing to overwrite a candidate'
s=bpy.context.scene;rig=bpy.data.objects['PitcherRig']
assert s.get('motion_revision')=='S0.2B' and not s.get('glove_revision')
B=Matrix(((-1,0,0),(0,0,-1),(0,1,0)))
rest={b.name:b.matrix_local.copy() for b in rig.data.bones}
def smooth(t):
    t=max(0,min(1,t));return t*t*(3-2*t)
def align(name,deform,direction):
    q=(deform@rest[name].to_3x3()).to_quaternion()
    return (q@Vector((0,1,0))).rotation_difference(direction)@q
for f in range(50,97):
    s.frame_set(f);old={b.name:b.matrix.copy() for b in rig.pose.bones};out={n:m.copy() for n,m in old.items()}
    amount=smooth((f-49)/23)*(1-smooth((f-81)/16))
    shoulder=old['arm_L'].translation
    upper=(old['forearm_L'].translation-shoulder).normalized().slerp((B@Vector((.08,-.10,-.992))).normalized(),amount)
    fore=(old['hand_L'].translation-old['forearm_L'].translation).normalized().slerp((B@Vector((-.05,.32,-.946))).normalized(),amount)
    elbow=shoulder+upper*rig.data.bones['arm_L'].length
    hand=elbow+fore*rig.data.bones['forearm_L'].length
    hand_deform=old['hand_L'].to_3x3()@rest['hand_L'].to_3x3().inverted()
    fq=align('forearm_L',hand_deform,fore)
    uq=align('arm_L',fq.to_matrix()@rest['forearm_L'].to_3x3().inverted(),upper)
    for name,position,direction,target in [('arm_L',shoulder,upper,uq),('forearm_L',elbow,fore,fq)]:
        oldq=old[name].to_quaternion()
        adjusted=(oldq@Vector((0,1,0))).rotation_difference(direction)@oldq
        out[name]=Matrix.LocRotScale(position,adjusted.slerp(target,amount),old[name].to_scale())
    out['hand_L'].translation=hand
    for name in ('arm_L','forearm_L','hand_L'):
        b=rig.data.bones[name]
        basis=b.convert_local_to_pose(out[name],rest[name],invert=True,parent_matrix=out[b.parent.name],parent_matrix_local=rest[b.parent.name])
        loc,q,scale=basis.decompose();pb=rig.pose.bones[name]
        if q.dot(pb.rotation_quaternion)<0:q.negate()
        pb.location,pb.rotation_quaternion,pb.scale=loc,q,scale
        for channel in ('location','rotation_quaternion','scale'):pb.keyframe_insert(channel,frame=f,group=name)
for layer in rig.animation_data.action.layers:
    for strip in layer.strips:
        for bag in strip.channelbags:
            for fc in bag.fcurves:
                for k in fc.keyframe_points:
                    if 50<=k.co.x<97:k.interpolation='LINEAR'
s['glove_revision']='S0.2B'
s['glove_source_sha256']=hashlib.sha256(Path(bpy.data.filepath).read_bytes()).hexdigest()
s['preview_notice']='S0.2B ARM / GLOVE CANDIDATE | HUMAN REVIEW PENDING'
s.frame_set(1);bpy.ops.wm.save_as_mainfile(filepath=str(a.output.resolve()))
print('S02B_GLOVE_EDIT_SECONDS',time.perf_counter()-start)
