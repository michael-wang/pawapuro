"""S0.2B local authoring edit of saved S0.2A; never called by export."""
import argparse
import hashlib
import sys
import time
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

p=argparse.ArgumentParser()
p.add_argument('--output',type=Path,required=True)
p.add_argument('--strategy',choices=('roll','roll-weights'),required=True)
args=p.parse_args(sys.argv[sys.argv.index('--')+1:])
started=time.perf_counter()
assert not args.output.exists(), 'Refusing to overwrite a candidate'
source=Path(bpy.data.filepath)
assert hashlib.sha256(source.read_bytes()).hexdigest()=='18894d26d6b2bc29aa2ff2e2a58c3479cf17eb9e1361890f8c8b2e3c2dedadcb'
s=bpy.context.scene; rig=bpy.data.objects['PitcherRig']; mesh=bpy.data.objects['PitcherMesh']
assert s.get('motion_revision')=='S0.2A'
rest={b.name:b.matrix_local.copy() for b in rig.data.bones}
poses={}
for f in range(1,206):
    s.frame_set(f); poses[f]={b.name:b.matrix.copy() for b in rig.pose.bones}

def smooth(t):
    t=max(0,min(1,t));return t*t*(3-2*t)

def aligned(name,reference,current):
    # Minimum swing from a neighbouring skin frame; the bone's axis stays fixed.
    base=(reference@rest[name].to_3x3()).to_quaternion()
    direction=current.to_quaternion()@Vector((0,1,0))
    return (base@Vector((0,1,0))).rotation_difference(direction)@base

for f in range(50,110):
    s.frame_set(f); old=poses[f]; out={n:m.copy() for n,m in old.items()}
    amount=smooth((f-49)/16)*(1-smooth((f-90)/20))
    hand_deform=old['hand_R'].to_3x3()@rest['hand_R'].to_3x3().inverted()
    fore=aligned('forearm_R',hand_deform,old['forearm_R'])
    fore_deform=fore.to_matrix()@rest['forearm_R'].to_3x3().inverted()
    upper=aligned('arm_R',fore_deform,old['arm_R'])
    for name,q in [('arm_R',upper),('forearm_R',fore)]:
        out[name]=Matrix.LocRotScale(old[name].translation,old[name].to_quaternion().slerp(q,amount),old[name].to_scale())
    # Explicit child compensation preserves the complete hand world transform.
    for name in ('arm_R','forearm_R','hand_R'):
        b=rig.data.bones[name]
        basis=b.convert_local_to_pose(out[name],rest[name],invert=True,parent_matrix=out[b.parent.name],parent_matrix_local=rest[b.parent.name])
        loc,q,scale=basis.decompose();pb=rig.pose.bones[name]
        if q.dot(pb.rotation_quaternion)<0:q.negate()
        pb.location,pb.rotation_quaternion,pb.scale=loc,q,scale
        for channel in ('location','rotation_quaternion','scale'):pb.keyframe_insert(channel,frame=f,group=name)

if args.strategy=='roll-weights':
    # Blend near the elbow, not along entire segments. The spherical hand covers
    # the tube end, so it need not counter-twist the forearm surface.
    fore_weights=[0,0,0,0,.1,.35,.65,.9,1,1,1,1,1]
    indices=list(range(2050,2206))
    for name in ('arm_R','forearm_R','hand_R'):mesh.vertex_groups[name].remove(indices)
    for ring,w in enumerate(fore_weights):
        vertices=list(range(2050+ring*12,2050+(ring+1)*12))
        if w<1:mesh.vertex_groups['arm_R'].add(vertices,1-w,'REPLACE')
        if w>0:mesh.vertex_groups['forearm_R'].add(vertices,w,'REPLACE')
for layer in rig.animation_data.action.layers:
    for strip in layer.strips:
        for bag in strip.channelbags:
            for fc in bag.fcurves:
                for k in fc.keyframe_points:
                    if 50<=k.co.x<110:k.interpolation='LINEAR'
s['motion_revision']='S0.2B'
s['revision_source_sha256']=hashlib.sha256(source.read_bytes()).hexdigest()
s['preview_notice']='S0.2B ARM CANDIDATE | HUMAN REVIEW PENDING'
s.frame_set(1)
bpy.ops.wm.save_as_mainfile(filepath=str(args.output.resolve()))
print('S02B_LOCAL_EDIT_SECONDS',time.perf_counter()-started,args.strategy)
