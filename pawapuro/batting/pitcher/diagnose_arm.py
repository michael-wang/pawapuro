"""Read-only S0.1 arm diagnosis; temporary wire/camera objects are never saved."""
import argparse,sys
import bpy,json,math
import numpy as np
from pathlib import Path
from mathutils import Vector
from bpy_extras.object_utils import world_to_camera_view
parser=argparse.ArgumentParser();parser.add_argument('--evidence',type=Path,required=True)
args=parser.parse_args(sys.argv[sys.argv.index('--')+1:])
root=args.evidence; out=root/'arm-diagnosis'; out.mkdir(parents=True,exist_ok=True)
scene=bpy.context.scene; rig=bpy.data.objects['PitcherRig']; obj=bpy.data.objects['PitcherMesh']
scene.render.resolution_x=1280; scene.render.resolution_y=720; scene.render.resolution_percentage=100
scene.render.engine='BLENDER_EEVEE_NEXT'; scene.eevee.taa_render_samples=16; scene.render.image_settings.file_format='PNG'
for o in bpy.data.collections['AuthoringOnly'].objects:
 if 'Release' in o.name:o.hide_render=True
cam=bpy.data.objects['Review_ThreeQuarter'].copy();cam.data=cam.data.copy();scene.collection.objects.link(cam);cam.name='DiagnosticOther'
cam.location=Vector((6,-3.5,3)); target=Vector((0,.55,1.35));cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler()
material=bpy.data.materials.new('DiagnosticWire');material.use_nodes=True
n=material.node_tree.nodes;n.clear();e=n.new('ShaderNodeEmission');e.inputs['Color'].default_value=(.2,1,.3,1);o=n.new('ShaderNodeOutputMaterial');material.node_tree.links.new(e.outputs[0],o.inputs[0])
rows=[]
for view,camera in [('side',bpy.data.objects['Review_ThreeQuarter']),('other',cam)]:
 scene.camera=camera
 for f in [77,79,81]:
  scene.frame_set(f);dg=bpy.context.evaluated_depsgraph_get(); ev=obj.evaluated_get(dg);mesh=ev.to_mesh()
  arm=[ev.matrix_world@mesh.vertices[i].co for i in range(2050,2206)]
  rings=[sum(arm[j*12:(j+1)*12],Vector())/12 for j in range(13)]
  pts={name:rig.matrix_world@rig.pose.bones[name].matrix.translation for name in ['arm_R','forearm_R','hand_R']}
  points={name:[p.x*1280,(1-p.y)*720] for name,point in pts.items() for p in [world_to_camera_view(scene,camera,point)]}
  row={'view':view,'frame':f,'projected_bones':points,'ring_centres':[list(p) for p in rings], 'ring_radii':[min((v-rings[j]).length for v in arm[j*12:(j+1)*12]) for j in range(13)],'bone_matrices':{name:[list(r) for r in rig.pose.bones[name].matrix] for name in pts}}
  rows.append(row)
  scene.render.filepath=str(out/f'{view}-{f}-clean.png');bpy.ops.render.render(write_still=True)
  wire_mesh=bpy.data.meshes.new_from_object(ev);wire=bpy.data.objects.new('EvaluatedWire',wire_mesh);scene.collection.objects.link(wire);wire.matrix_world=obj.matrix_world
  wire.data.materials.clear();wire.data.materials.append(material);mod=wire.modifiers.new('Wire','WIREFRAME');mod.thickness=.006;mod.use_replace=True
  scene.render.filepath=str(out/f'{view}-{f}-wire.png');bpy.ops.render.render(write_still=True)
  bpy.data.objects.remove(wire,do_unlink=True);ev.to_mesh_clear()
metrics=[];previous=None
for f in range(75,84):
 scene.frame_set(f);mat=rig.pose.bones['arm_R'].matrix.copy(); q=mat.to_quaternion()
 metrics.append({'frame':f,'rotation_step_deg':math.degrees(previous.rotation_difference(q).angle) if previous else 0,'determinant':mat.to_3x3().determinant(),'scale':list(mat.to_scale())});previous=q
contraction=[]
for f in [1,25,49,77,79,81]:
 scene.frame_set(f)
 deform={n:rig.pose.bones[n].matrix@rig.data.bones[n].matrix_local.inverted() for n in ['arm_R','forearm_R','hand_R']}
 for left,right in [('arm_R','forearm_R'),('forearm_R','hand_R')]:
  x,y=deform[left],deform[right]
  contraction.append({'frame':f,'pair':[left,right],'relative_rotation_deg':math.degrees(x.to_quaternion().rotation_difference(y.to_quaternion()).angle),'half_blend_min_singular':float(np.linalg.svd((np.array(x.to_3x3())+np.array(y.to_3x3()))*.5,compute_uv=False)[-1])})
(out/'diagnosis.json').write_text(json.dumps({'views':rows,'adjacent_upper_arm':metrics,'skin_rotation_contraction':contraction},indent=2))
print(json.dumps({'adjacent_upper_arm':metrics,'radii79':rows[1]['ring_radii']},indent=2))
