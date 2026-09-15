"""Explicit S0.1 -> S0.2A local FK edit; never called by export."""
import argparse
import hashlib
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args(sys.argv[sys.argv.index('--')+1:])
assert not args.output.exists(), 'Refusing to replace an existing candidate'
scene = bpy.context.scene
assert scene.get('motion_revision') == 'S0.1'
assert (scene.frame_start, scene.frame_end, scene.timeline_markers['release'].frame) == (1,205,97)
rig = bpy.data.objects['PitcherRig']
B = Matrix(((-1,0,0),(0,0,-1),(0,1,0)))
Bi = B.inverted()
h = scene['blockout_proportion_scale_m']
rest = {b.name:b.matrix_local.copy() for b in rig.data.bones}
baseline = {}
for f in range(1,73):
    scene.frame_set(f)
    baseline[f] = {b.name:b.matrix.copy() for b in rig.pose.bones}

def smooth(t):
    t = max(0,min(1,t))
    return t*t*(3-2*t)

def rotation(yaw):
    return Matrix.Rotation(math.radians(yaw),3,'Y')

def body(name, pos, rot):
    return Matrix.Translation(B@pos) @ (B@rot@Bi@rest[name].to_3x3()).to_4x4()

def limb(name,start,end,rot,old,w):
    direction=(B@(end-start)).normalized()
    # Pose roll follows the turned rest orientation, then rejoins old stride roll.
    base=(B@rot@Bi@rest[name].to_3x3()).to_quaternion()
    q=(base@Vector((0,1,0))).rotation_difference(direction)@base
    previous=old[name].to_quaternion()
    previous=(previous@Vector((0,1,0))).rotation_difference(direction)@previous
    return Matrix.LocRotScale(B@start,q.slerp(previous,1-w),Vector((1,1,1)))

def deform(m,name):
    return Bi@(m[name].to_3x3()@rest[name].to_3x3().inverted())@B

grip_local = rest['hand_R'].inverted()@rest['grip']

for f in range(1,72):
    scene.frame_set(f)
    old = baseline[f]
    w = 1-smooth((f-49)/23)
    lift = smooth((f-17)/32)
    rot = rotation(90+20*lift).to_quaternion().slerp(deform(old,'chest').to_quaternion(),1-w).to_matrix()
    prot = rotation(80+18*lift).to_quaternion().slerp(deform(old,'pelvis').to_quaternion(),1-w).to_matrix()
    p = Bi@old['pelvis'].translation
    chest = p+rot@Vector((0,.23*h,0))
    head = chest+rot@Vector((0,.18*h,0))
    m = {k:v.copy() for k,v in old.items()}
    m['pelvis'] = body('pelvis',p,prot)
    m['chest'] = body('chest',chest,rot)
    m['head'] = body('head',head,deform(old,'head'))
    for side,sign,u,v in [('R',-1,(.35,-.60,-.7194),(.645,.484,-.591)),
                          ('L',1,(-.40,-.20,-.8944),(-.60,.20,-.7746))]:
        upper,fore,hand = [n+'_'+side for n in ('arm','forearm','hand')]
        shoulder = chest+rot@Vector((sign*.18*h,-.06*h,0))
        ou = (Bi@(old[fore].translation-old[upper].translation)).normalized()
        ov = (Bi@(old[hand].translation-old[fore].translation)).normalized()
        u = (rot@Vector(u).normalized()).slerp(ou,1-w)
        v = (rot@Vector(v).normalized()).slerp(ov,1-w)
        elbow = shoulder+u*rig.data.bones[upper].length
        wrist = elbow+v*rig.data.bones[fore].length
        m[upper] = limb(upper,shoulder,elbow,rot,old,w)
        m[fore] = limb(fore,elbow,wrist,rot,old,w)
        hr = rot.to_quaternion().slerp(deform(old,hand).to_quaternion(),1-w).to_matrix()
        m[hand] = body(hand,wrist,hr)
    m['grip'] = m['hand_R']@grip_local
    # The airborne path gathers below the glove in the turning body's space.
    glove = Bi@m['hand_L'].translation
    target = Vector((glove.x,1.035,glove.z))
    start = Vector((.05,.099225,-.42))
    gathered = start.lerp(target,lift)
    foot = gathered.lerp(Bi@old['foot_L'].translation,1-w)
    fr = rotation(90+20*lift).to_quaternion().slerp(deform(old,'foot_L').to_quaternion(),1-w).to_matrix()
    m['foot_L'] = body('foot_L',foot,fr)
    for bone in rig.data.bones:
        # Root, planted right shoe and grip binding are never rewritten.
        if bone.name in ('root','foot_R','grip'): continue
        kw = {'parent_matrix':m[bone.parent.name], 'parent_matrix_local':rest[bone.parent.name]} if bone.parent else {}
        basis = bone.convert_local_to_pose(m[bone.name],rest[bone.name],invert=True,**kw)
        loc,q,scale = basis.decompose()
        # Anchor each key to the existing sign convention, including frame 72.
        if q.dot(rig.pose.bones[bone.name].rotation_quaternion)<0: q.negate()
        pose = rig.pose.bones[bone.name]
        pose.location,pose.rotation_quaternion,pose.scale = loc,q,scale
        for channel in ('location','rotation_quaternion','scale'):
            pose.keyframe_insert(channel,frame=f,group=bone.name)
for layer in rig.animation_data.action.layers:
    for strip in layer.strips:
        for bag in strip.channelbags:
            for fc in bag.fcurves:
                for key in fc.keyframe_points:
                    if key.co.x<72: key.interpolation='LINEAR'
scene['motion_revision']='S0.2A'
scene['revision_source_sha256']=hashlib.sha256(Path(bpy.data.filepath).read_bytes()).hexdigest()
scene['preview_notice']='S0.2A LOCAL CANDIDATE | HUMAN REVIEW PENDING'
scene.frame_set(1)
bpy.ops.wm.save_as_mainfile(filepath=str(args.output.resolve()))
print('S02A_CANDIDATE',args.output)
