"""One concrete pre-export readability correction to the initial S0 source."""
import bpy,math,shutil
from pathlib import Path
from mathutils import Vector
s=bpy.context.scene;assert not s.get('readability_pass');src=Path(bpy.data.filepath);backup=src.parents[3]/'build/batter-authoring-s0/initial-blocking.blend';shutil.copy2(src,backup)
rig=bpy.data.objects['BatterRig'];names=[b.name for b in rig.data.bones]
for f in range(1,s.frame_end+1):
 s.frame_set(f);mats={n:rig.pose.bones[n].matrix.copy() for n in names}
 for side in ['R','L']:
  for j in [1,2]:mats[f'arm_{side}_{j}'].translation.z-=.09
 for n in names:
  b=rig.data.bones[n];p=rig.pose.bones[n];kw={'parent_matrix':mats[b.parent.name],'parent_matrix_local':b.parent.matrix_local} if b.parent else {};m=b.convert_local_to_pose(mats[n],b.matrix_local,invert=True,**kw);p.location,p.rotation_quaternion,p.scale=m.decompose()
  for ch in ['location','rotation_quaternion','scale']:p.keyframe_insert(ch,frame=f,group=n)
# Baked vertex-color form cues; no lighting/material feature or runtime change.
for name in ['BatterMesh','Bat']:
 obj=bpy.data.objects[name];mesh=obj.data;mesh.update();colors=mesh.color_attributes['Color'];light=Vector((-.4,-.5,.8)).normalized()
 for v in mesh.vertices:
  c=colors.data[v.index].color;factor=.68+.32*max(0,v.normal.dot(light));colors.data[v.index].color=(c[0]*factor,c[1]*factor,c[2]*factor,c[3])
# A narrow jersey front placket makes chest rotation readable without arrows.
colors=bpy.data.objects['BatterMesh'].data.color_attributes['Color']
for ring in range(6):
 i=ring*24+12;colors.data[i].color=(.84,.82,.88,1)
cam=bpy.data.objects['Review_ThreeQuarter'];origin=Vector(s['placement_game_m']);B=__import__('mathutils').Matrix(((-1,0,0),(0,0,-1),(0,1,0)));cam.location=B@(Vector((-2.8,2.7,-3.4))-origin);cam.rotation_euler=(B@(Vector((1.05,1.03,.13))-origin)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=6.3806144140099965
# Store candidate vertex colors on the same sRGB8 grid used by Blender glTF import.
# This changes source precision, preserving the existing Euclidean validation rule.
for name in ['BatterMesh','Bat']:
 mesh=bpy.data.objects[name].data;old=mesh.color_attributes['Color'];values=[tuple(c.color) for c in old.data];new=mesh.color_attributes.new(name='ColorQuantized',type='BYTE_COLOR',domain='POINT')
 for c,v in zip(new.data,values):c.color=v
 mesh.color_attributes.remove(old);new.name='Color';mesh.color_attributes.active_color=new
s['readability_pass']=True
for layer in rig.animation_data.action.layers:
 for strip in layer.strips:
  for bag in strip.channelbags:
   for fc in bag.fcurves:
    for key in fc.keyframe_points:key.interpolation='LINEAR'
s.frame_set(1);bpy.ops.wm.save_as_mainfile(filepath=str(src))
