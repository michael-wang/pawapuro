"""S0.2C: local post-release edit of official S0.2B; never used by export."""
import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

p = argparse.ArgumentParser()
p.add_argument('--output', type=Path, required=True)
a = p.parse_args(sys.argv[sys.argv.index('--')+1:])
source = Path(bpy.data.filepath)
expected = 'b3dcccd38ad7c216b18ccb69b4a664240051de0b988618f449737611748dd366'
assert hashlib.sha256(source.read_bytes()).hexdigest() == expected, 'Requires official S0.2B'
assert not a.output.exists(), 'Do not overwrite a candidate'
s = bpy.context.scene
rig = bpy.data.objects['PitcherRig']
assert s['motion_revision'] == 'S0.2B'
assert (s.frame_start, s.frame_end, s.timeline_markers['release'].frame) == (1, 205, 97)
B = Matrix(((-1, 0, 0), (0, 0, -1), (0, 1, 0)))
rest = {b.name: b.matrix_local.copy() for b in rig.data.bones}
poses = {}
for f in range(97, 206):
    s.frame_set(f)
    poses[f] = {b.name: b.matrix.copy() for b in rig.pose.bones}


def curve(knots, f):
    """Monotone Hermite offsets with zero initial slope, not per-pose easing."""
    x = sorted(knots)
    if f <= x[0]: return knots[x[0]]
    if f >= x[-1]: return knots[x[-1]]
    slopes = [(knots[v]-knots[u])/(v-u) for u, v in zip(x, x[1:])]
    tangent = [0.0]
    for u, v in zip(slopes, slopes[1:]):
        tangent.append(2*u*v/(u+v) if u*v > 0 else 0.0)
    tangent.append(0.0)
    i = next(i for i in range(len(x)-1) if x[i] <= f < x[i+1])
    h = x[i+1]-x[i]; t = (f-x[i])/h
    return ((2*t**3-3*t*t+1)*knots[x[i]] + (t**3-2*t*t+t)*h*tangent[i]
            + (-2*t**3+3*t*t)*knots[x[i+1]] + (t**3-t*t)*h*tangent[i+1])


def game_rotation(name, old):
    return B.inverted() @ (old[name].to_3x3() @ rest[name].to_3x3().inverted()) @ B


def rotated(name, old, yaw, lean):
    base = game_rotation(name, old)
    result = Matrix.Rotation(math.radians(yaw), 3, 'Y') @ base @ Matrix.Rotation(math.radians(-lean), 3, 'X')
    return B @ result @ B.inverted() @ rest[name].to_3x3()


for f in range(98, 206):
    s.frame_set(f)
    old = poses[f]
    out = {n: m.copy() for n, m in old.items()}
    # Pelvis leads the added turn; chest carries on while the rear foot swings.
    py = curve({97:0, 112:-13, 130:-34, 149:-39, 176:-35, 205:-32}, f)
    cy = curve({97:0, 115:-12, 137:-43, 157:-51, 182:-49, 205:-47}, f)
    lean = curve({97:0, 116:3, 135:5, 157:3, 183:1, 205:0}, f)
    shift = B @ Vector((curve({97:0, 129:-.10, 152:-.09, 183:.01, 205:.01}, f),
                        curve({97:0, 136:-.02, 172:-.015, 205:0}, f),
                        curve({97:0, 119:-.045, 146:-.16, 176:-.27, 205:-.30}, f)))
    pelvis = old['pelvis'].translation + shift
    out['pelvis'] = Matrix.Translation(pelvis) @ rotated('pelvis', old, py, lean*.25).to_4x4()
    chest_rotation = rotated('chest', old, cy, lean)
    delta = chest_rotation @ old['chest'].to_3x3().inverted()
    chest = pelvis + delta @ (old['chest'].translation-old['pelvis'].translation)
    out['chest'] = Matrix.Translation(chest) @ chest_rotation.to_4x4()
    carry = Matrix.Translation(chest) @ delta.to_4x4() @ Matrix.Translation(-old['chest'].translation)
    # A shared rigid carry preserves B's arm deformation and local follow-through.
    for n in ('arm_R', 'forearm_R', 'hand_R', 'grip', 'arm_L', 'forearm_L', 'hand_L'):
        out[n] = carry @ old[n]
    head_yaw = curve({97:0, 121:-5, 146:-18, 177:-25, 205:-28}, f)
    head_pos = carry @ old['head'].translation
    out['head'] = Matrix.Translation(head_pos) @ rotated('head', old, head_yaw, lean*.4).to_4x4()
    # The accepted lift/sole rotation is retained; forward travel overlaps body turn.
    # Both offsets settle before the existing final contact, so there is no slide.
    out['foot_R'].translation += B @ Vector((curve({97:0, 126:.02, 149:.08, 171:.10}, f), 0,
                                             curve({97:0, 116:-.035, 138:-.25, 159:-.55, 171:-.62}, f)))
    # Keep root/left foot keys untouched. Convert only changed world poses to TRS.
    for name in ('pelvis', 'chest', 'head', 'arm_R', 'forearm_R', 'hand_R',
                 'arm_L', 'forearm_L', 'hand_L', 'foot_R'):
        b = rig.data.bones[name]
        basis = b.convert_local_to_pose(out[name], rest[name], invert=True,
                                       parent_matrix=out[b.parent.name], parent_matrix_local=rest[b.parent.name])
        loc, q, scale = basis.decompose()
        pb = rig.pose.bones[name]
        if pb.rotation_quaternion.dot(q) < 0: q.negate()
        pb.location, pb.rotation_quaternion, pb.scale = loc, q, scale
        for channel in ('location', 'rotation_quaternion', 'scale'):
            pb.keyframe_insert(channel, frame=f, group=name)
for layer in rig.animation_data.action.layers:
    for strip in layer.strips:
        for bag in strip.channelbags:
            for fc in bag.fcurves:
                for k in fc.keyframe_points:
                    if k.co.x > 97: k.interpolation = 'LINEAR'
s['motion_revision'] = 'S0.2C'
s['revision_source_sha256'] = expected
s['preview_notice'] = 'S0.2C CANDIDATE | HUMAN REVIEW PENDING'
# The actual foot contact remains 171; do not create a second event schedule.
assert json.loads(s['contact_intervals'])['foot_R'] == [[1,85],[171,205]]
s['key_poses'] = '1:ready;49:coil;72:stride;79:front_contact;90:torso_opening;94:acceleration;108:early_follow;137:continued_turn;151:rear_follow;171:rear_contact;205:balanced'
s.frame_set(1)
a.output.parent.mkdir(parents=True, exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=str(a.output.resolve()))
assert hashlib.sha256(source.read_bytes()).hexdigest() == expected
print('S02C_SAVED', a.output)
