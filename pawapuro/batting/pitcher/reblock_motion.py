"""Explicit S0.1 revision of a loaded S0 source; save to a NEW candidate only.

No export path calls this script. The saved .blend remains editable truth.
Directions key an ordinary fixed-length FK chain, not a target/IK solver.
"""
import argparse
import hashlib
import json
import math
import sys
import tomllib
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:])
if args.output.exists():
    raise RuntimeError('Candidate already exists; refusing to replace authored work')
scene = bpy.context.scene
rig = bpy.data.objects['PitcherRig']
if scene.frame_end != 151 or scene.timeline_markers['release'].frame != 91:
    raise RuntimeError('This one-time revision expects the reviewed S0 source')
source_hash = hashlib.sha256(Path(bpy.data.filepath).read_bytes()).hexdigest()
B = Matrix(((-1, 0, 0), (0, 0, -1), (0, 1, 0)))
h = scene['blockout_proportion_scale_m']
foot_y = .045*h*.9


def curve(keys, frame):
    """Shape-preserving Hermite; monotone interior knots retain nonzero velocity."""
    keys = sorted(keys.items())
    if frame <= keys[0][0]: return keys[0][1]
    if frame >= keys[-1][0]: return keys[-1][1]
    if isinstance(keys[0][1], (tuple, list)):
        return Vector([curve({f: v[a] for f, v in keys}, frame) for a in range(len(keys[0][1]))])
    def slope(i):
        if i == 0 or i == len(keys)-1: return 0.0
        x0, y0 = keys[i-1]; x1, y1 = keys[i]; x2, y2 = keys[i+1]
        d0, d1 = (y1-y0)/(x1-x0), (y2-y1)/(x2-x1)
        if d0*d1 <= 0: return 0.0
        w0, w1 = 2*(x2-x1)+(x1-x0), (x2-x1)+2*(x1-x0)
        return (w0+w1)/(w0/d0+w1/d1)
    i = next(i for i in range(len(keys)-1) if keys[i][0] <= frame < keys[i+1][0])
    x0, y0 = keys[i]; x1, y1 = keys[i+1]; d = x1-x0; t = (frame-x0)/d
    return (2*t**3-3*t*t+1)*y0+(t**3-2*t*t+t)*d*slope(i)+(-2*t**3+3*t*t)*y1+(t**3-t*t)*d*slope(i+1)


def direction(keys, frame):
    # Angular FK curves avoid speed spikes from normalizing a near-zero chord.
    if frame <= 70: return Vector(curve(keys,frame)).normalized()
    azimuth, elevation = {}, {}
    previous = None
    for f, value in sorted(keys.items()):
        v = Vector(value).normalized()
        angle = math.degrees(math.atan2(v.x, -v.z))
        if previous is not None:
            while angle-previous > 180: angle -= 360
            while angle-previous < -180: angle += 360
        azimuth[f] = angle; previous = angle
        elevation[f] = math.degrees(math.asin(v.y))
    az, el = math.radians(curve(azimuth,frame)), math.radians(curve(elevation,frame))
    return Vector((math.sin(az)*math.cos(el),math.sin(el),-math.cos(az)*math.cos(el)))


def rotation(yaw=0, lean=0):
    return Matrix.Rotation(math.radians(yaw), 3, 'Y') @ Matrix.Rotation(math.radians(-lean), 3, 'X')


def body(name, pos, yaw=0, lean=0):
    return Matrix.Translation(B@Vector(pos)) @ (B@rotation(yaw, lean)@B.inverted()@rig.data.bones[name].matrix_local.to_3x3()).to_4x4()


def limb(start, end):
    return Matrix.LocRotScale(B@start, (B@(end-start)).to_track_quat('Y','Z'), Vector((1,1,1)))


lengths = {}
for side in ('R', 'L'):
    lengths[side] = [rig.data.bones[f'{name}_{side}'].length for name in ('arm', 'forearm')]

# Positive game Y yaw carries the right shoulder BACK; opening decreases yaw.
# The separate extrema/knots express pelvis -> chest -> arm, not scaled copies.
pelvis_yaw = {1:-5.6, 25:15, 49:45, 60:47, 72:28, 79:8, 88:-26, 100:-45, 124:-55, 161:-35, 205:-12}
chest_yaw = {1:-8, 25:24, 49:70, 60:74, 72:66, 79:56, 87:34, 92:4, 97:-25, 105:-51, 124:-68, 153:-57, 179:-32, 205:-18}
chest_lean = {1:0, 49:-7, 65:-3, 79:7, 90:17, 97:28, 111:48, 125:52, 149:39, 175:19, 205:8}
head_yaw = {1:-1.44, 34:4, 63:7, 86:3, 101:-5, 129:-20, 159:-16, 205:-5}
head_lean = {1:0, 62:-2, 90:3, 103:11, 127:26, 162:17, 205:3}

# A single authored release construction places the entire body, without moving
# grip independently or snapping the hand to a target on the marker frame.
release_frame = 97
release_rot = rotation(-25, 28)
upper_release_world = Vector((-.65,.70,-.30)).normalized()
fore_release_world = Vector((.25,.08,-.965)).normalized()
upper_release = release_rot.inverted()@upper_release_world
fore_release = release_rot.inverted()@fore_release_world
grip_rest = rig.data.bones['hand_R'].matrix_local.inverted()@rig.data.bones['grip'].matrix_local
staging = tomllib.loads((Path(__file__).resolve().parent.parent/'staging.toml').read_text(encoding='utf-8'))
release_hand = Vector([a-b for a,b in zip(staging['release']['position_m'],scene['placement_game_m'])])-Vector((0,.035,-.13))
release_shoulder = release_hand-upper_release_world*lengths['R'][0]-fore_release_world*lengths['R'][1]
release_pelvis = release_shoulder-release_rot@Vector((-.18*h,.17*h,0))
pelvis_pos = {1:(0,.833,0), 25:(-.08,.84,.03), 49:(-.16,.96,.03), 63:(-.17,.92,-.05), 79:(-.22,.85,-.33),
              97:tuple(release_pelvis), 115:(-.15,.83,-.84), 137:(-.03,.77,-1.10), 159:(-.04,.82,-1.34), 184:(-.06,.87,-1.50), 205:(-.06,.85,-1.53)}
upper = {1:(0,-.40,-.916), 30:(.05,-.50,-.86), 49:(.10,-.50,-.86), 70:(-.70,-.12,.70),
         79:(-.94,.25,.23), 87:(-.88,.42,.22), 92:(-.83,.55,.08), 97:tuple(upper_release),
         107:(-.3,.2,-.93), 112:(.35,-.48,-.80), 126:(.55,-.75,-.35), 149:(.36,-.90,-.24), 178:(.02,-.95,-.31), 205:(-.1,-.92,-.38)}
fore = {1:(.8,.4,-.447), 30:(.82,.15,-.55), 49:(.83,.15,-.54), 70:(-.3,.92,.25),
        79:(-.05,.83,.55), 87:(-.20,.38,.90), 92:(-.30,.67,.68), 97:tuple(fore_release),
        107:(.05,-.5,-.86), 112:(.80,-.53,-.27), 126:(.80,-.50,.33), 149:(.73,-.46,.50), 178:(.56,-.23,-.80), 205:(.58,.05,-.81)}
front_pos = {1:(.37,foot_y,-.07), 17:(.37,foot_y,-.07), 32:(.29,.52,-.13), 49:(.19,.99,-.28),
             59:(.24,.86,-.56), 70:(.33,.49,-1.16), 79:(.3675,foot_y,-1.666), 205:(.3675,foot_y,-1.666)}
rear_lift = {1:0, 85:0, 96:.12, 113:.62, 128:.77, 148:.40, 164:.065, 171:0, 205:0}
rear_z = {1:0, 96:0, 111:.12, 130:-.30, 150:-1.04, 171:-1.42, 205:-1.42}
rear_x = {1:-.1715, 96:-.1715, 120:-.32, 151:-.52, 171:-.51, 205:-.51}
rear_lean = {1:0, 85:0, 101:32, 127:52, 150:25, 171:0, 205:0}
rear_yaw = {1:0, 85:0, 113:-35, 144:-42, 171:-16, 205:-16}

rig.animation_data_clear()
for old in list(bpy.data.actions):
    if old.name == 'pitch_R' and old.users == 0: bpy.data.actions.remove(old)
scene.frame_end = 205
scene.timeline_markers['release'].frame = release_frame
scene['key_poses'] = '1:ready;49:coil;72:stride;79:front_contact;90:torso_opening;94:acceleration;108:early_follow;139:rear_follow;171:rear_contact;205:balanced'
scene['contact_intervals'] = json.dumps({'foot_R':[[1,85],[171,205]], 'foot_L':[[1,17],[79,205]]})
scene['motion_revision'] = 'S0.1'
scene['revision_source_sha256'] = source_hash
scene['preview_notice'] = 'S0.1 AUTHORING CANDIDATE | TIMING NOT VIDEO-VERIFIED'
for f in range(1, scene.frame_end+1):
    scene.frame_set(f)
    p = Vector(curve(pelvis_pos,f)); yaw = curve(chest_yaw,f); lean = curve(chest_lean,f); rot = rotation(yaw,lean)
    chest = p+rot@Vector((0,.23*h,0)); head = chest+rot@Vector((0,.18*h,0))
    m = {'root':rig.data.bones['root'].matrix_local.copy(), 'pelvis':body('pelvis',p,curve(pelvis_yaw,f),lean*.25),
         'chest':body('chest',chest,yaw,lean), 'head':body('head',head,curve(head_yaw,f),curve(head_lean,f))}
    for side,sign in (('R',-1),('L',1)):
        shoulder = chest+rot@Vector((sign*.18*h,-.06*h,0))
        if side == 'R':
            u, v = direction(upper,f), direction(fore,f)
        else:
            u = Vector(curve({1:(.1,-.25,-.96),49:(-.10,-.50,-.86),70:(.18,.08,-.98),79:(.20,0,-.98),97:(-.10,-.85,-.52),125:(-.25,-.94,-.22),205:(.1,-.75,-.65)},f)).normalized()
            v = Vector(curve({1:(-.82,.47,-.32),49:(-.83,.15,-.54),70:(-.05,.32,-.95),79:(-.2,.25,-.95),97:(-.8,.5,-.33),125:(-.75,.62,-.22),205:(-.75,.55,-.36)},f)).normalized()
        elbow = shoulder+rot@u*lengths[side][0]; hand = elbow+rot@v*lengths[side][1]
        m[f'arm_{side}']=limb(shoulder,elbow); m[f'forearm_{side}']=limb(elbow,hand)
        m[f'hand_{side}']=body(f'hand_{side}',hand)
    m['grip']=m['hand_R']@grip_rest
    m['foot_L']=body('foot_L',curve(front_pos,f))
    fl = curve(rear_lean,f)
    # Raise the rigid flat shoe above its rotated ellipsoid support extent.
    support = math.hypot(foot_y*math.cos(math.radians(fl)), .10*h*1.9*math.sin(math.radians(fl)))
    m['foot_R']=body('foot_R',(curve(rear_x,f),support+curve(rear_lift,f),curve(rear_z,f)),curve(rear_yaw,f),fl)
    for bone in rig.data.bones:
        kw = {'parent_matrix':m[bone.parent.name], 'parent_matrix_local':bone.parent.matrix_local} if bone.parent else {}
        basis=bone.convert_local_to_pose(m[bone.name],bone.matrix_local,invert=True,**kw)
        pose=rig.pose.bones[bone.name]; pose.rotation_mode='QUATERNION'
        loc, q, scale=basis.decompose()
        if f > 1 and pose.rotation_quaternion.dot(q)<0: q.negate()
        pose.location,pose.rotation_quaternion,pose.scale=loc,q,scale
        for channel in ('location','rotation_quaternion','scale'): pose.keyframe_insert(channel,frame=f,group=bone.name)
action=rig.animation_data.action; action.name='pitch_R'
for layer in action.layers:
    for strip in layer.strips:
        for bag in strip.channelbags:
            for fc in bag.fcurves:
                for key in fc.keyframe_points: key.interpolation='LINEAR'
for obj in bpy.data.objects:
    if obj.name.startswith('HeldBall_'):
        obj.animation_data_clear()
        # Authoring-only visibility reads the marker, not a second set of keys.
        for prop in ('hide_render','hide_viewport'):
            driver=obj.driver_add(prop).driver
            variable=driver.variables.new(); variable.name='release_frame'
            variable.type='SINGLE_PROP'; variable.targets[0].id_type='SCENE'
            variable.targets[0].id=scene
            variable.targets[0].data_path='timeline_markers["release"].frame'
            driver.expression='frame > release_frame'
scene.frame_set(49)
bpy.ops.wm.save_as_mainfile(filepath=str(args.output.resolve()))
print('S01_CANDIDATE',args.output,'release',release_frame,'end',scene.frame_end,'fixed lengths',lengths)
