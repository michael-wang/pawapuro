"""Export the currently opened editable batter source; never regenerate or save it."""
import argparse,hashlib,json,sys
from pathlib import Path
import bpy
from mathutils import Vector
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True);a=p.parse_args(sys.argv[sys.argv.index('--')+1:]);a.evidence.mkdir(parents=True,exist_ok=True)
source=Path(bpy.data.filepath);assert source.is_file();sha=hashlib.sha256(source.read_bytes()).hexdigest();s=bpy.context.scene;assert (s.render.fps,s.render.fps_base)==(60,1);rig=bpy.data.objects['BatterRig'];assert rig.animation_data.action.name=='swing_L';markers=[m for m in s.timeline_markers if m.name=='contact_area'];assert len(markers)==1;marker=markers[0];assert s.frame_start<marker.frame<s.frame_end
source_samples={};colors={};diagnostics=[];body=bpy.data.objects['BatterMesh'];feet=json.loads(body['foot_vertex_ids']);saved=s.frame_current
for name in ['BatterMesh','Bat']:colors[name]=[list(c.color) for c in bpy.data.objects[name].data.color_attributes['Color'].data]
restlen={b.name:b.length for b in rig.data.bones};maxbone=0;maxweight=0;minbottom=1e6;footposes={n:[] for n in feet};griplocal=[]
for obj in [body,bpy.data.objects['Bat']]:
 for v in obj.data.vertices:
  ws=[g.weight for g in v.groups];assert ws and min(ws)>=0 and len(ws)<=4;maxweight=max(maxweight,abs(sum(ws)-1))
assert maxweight<1e-6
from bpy_extras.object_utils import world_to_camera_view
for f in range(s.frame_start,s.frame_end+1):
 s.frame_set(f);dg=bpy.context.evaluated_depsgraph_get();entry={};bones={n:rig.matrix_world@b.matrix for n,b in rig.pose.bones.items()}
 for name in ['BatterMesh','Bat']:
  o=bpy.data.objects[name].evaluated_get(dg);mesh=o.to_mesh();entry[name]=[list(o.matrix_world@v.co) for v in mesh.vertices];o.to_mesh_clear()
 entry['nodes']={n:[list(row) for row in bones[n]] for n in ['hand_R','hand_L','bat_grip','bat_barrel','bat_tip','pelvis','chest','foot_R','foot_L']};source_samples[str(f)]=entry
 for b in rig.pose.bones:maxbone=max(maxbone,abs((b.tail-b.head).length-restlen[b.name]));assert max(abs(v-1) for v in b.scale)<2e-5
 row={'frame':f,'time_s':(f-1)/60,'nodes_blender_m':{n:list(m.translation) for n,m in bones.items()},'feet_bottom_m':{n:min(entry['BatterMesh'][i][2] for i in ids) for n,ids in feet.items()},'body_matrices':entry['nodes']}
 minbottom=min(minbottom,*row['feet_bottom_m'].values());barrel=bones['bat_barrel'].translation;screen=world_to_camera_view(s,bpy.data.objects['Review_Batting'],barrel);row['barrel_screen_1920x1080']=[screen.x*1920,(1-screen.y)*1080];diagnostics.append(row)
 for n in feet:footposes[n].append(bones[n].copy())
 griplocal.append(bones['hand_R'].inverted()@bones['bat_grip'])
assert maxbone<2e-5;assert minbottom>=-1e-5,(minbottom,'foot penetration')
contacts=json.loads(s['contact_intervals']);drifts={}
for n,intervals in contacts.items():
 for lo,hi in intervals:
  ref=footposes[n][lo-1];d=max(abs(m[r][c]-ref[r][c]) for m in footposes[n][lo-1:hi] for r in range(4) for c in range(4));drifts[f'{n}:{lo}-{hi}']=d;assert d<2e-6,(n,d)
gdrift=max(abs(m[r][c]-griplocal[0][r][c]) for m in griplocal for r in range(4) for c in range(4));assert gdrift<2e-6
# Rear toe pivot is a concrete mesh vertex; check it stays on the same point.
toe_id=feet['foot_L'][0];toe0=Vector(source_samples['115']['BatterMesh'][toe_id]);toe_drift=max((Vector(source_samples[str(f)]['BatterMesh'][toe_id])-toe0).length for f in range(115,s.frame_end+1));assert toe_drift<2e-6,toe_drift
s.frame_set(1);bpy.ops.object.select_all(action='DESELECT')
for o in bpy.data.collections['BatterAsset'].objects:o.select_set(True)
bpy.context.view_layer.objects.active=rig;glb=source.with_suffix('.glb')
result=bpy.ops.export_scene.gltf(filepath=str(glb),export_format='GLB',use_selection=True,export_yup=True,export_materials='NONE',export_vertex_color='NAME',export_vertex_color_name='Color',export_all_vertex_colors=False,export_active_vertex_color_when_no_material=True,export_normals=False,export_texcoords=False,export_tangents=False,export_animations=True,export_animation_mode='ACTIVE_ACTIONS',export_nla_strips_merged_animation_name='swing_L',export_frame_range=True,export_frame_step=1,export_force_sampling=True,export_sampling_interpolation_fallback='LINEAR',export_anim_slide_to_zero=True,export_optimize_animation_size=False,export_skins=True,export_influence_nb=4,export_all_influences=False,export_def_bones=False,export_leaf_bone=False,export_morph=False,export_extras=False,export_cameras=False,export_lights=False,export_gpu_instances=False)
assert result=={'FINISHED'};s.frame_set(saved);assert hashlib.sha256(source.read_bytes()).hexdigest()==sha
meta=f'''# Derived from saved Blender source. Export never saves or regenerates the source.
schema_version = 1
source = "{source.name}"
source_sha256 = "{sha}"
glb = "{glb.name}"
glb_sha256 = "{hashlib.sha256(glb.read_bytes()).hexdigest()}"
blender_version = "{bpy.app.version_string}"
status = "authoring candidate; runtime and collision unimplemented"
[space]
units = "metres"
placement_game_m = {list(s['placement_game_m'])}
runtime_scale = 1.0
glb_to_game = "(-x,y,z) + staging batter placement; metadata placement is provenance only"
[clip]
name = "swing_L"
fps = 60
fps_base = 1.0
start_frame = {s.frame_start}
end_frame = {s.frame_end}
time_origin_s = 0.0
duration_s = {(s.frame_end-s.frame_start)/60}
loop = false
[contact_area]
marker = "contact_area"
authoring_frame = {marker.frame}
time_s = {(marker.frame-s.frame_start)/60}
meaning = "representative barrel pass through authoring zone, not collision truth"
[bat]
mesh = "Bat"
grip = "bat_grip"
parent = "hand_R"
barrel = "bat_barrel"
tip = "bat_tip"
ownership = "hands throughout; rigid weights to bat_grip; no release"
[review]
key_poses = {json.dumps(s['key_poses'])}
rear_toe_support_frames = {list(s['rear_toe_support'])}
'''
for n,ints in contacts.items():
 for lo,hi in ints:meta+=f'\n[[contacts]]\nbone = "{n}"\nstart_frame = {lo}\nend_frame = {hi}\n'
source.with_suffix('.toml').write_text(meta,encoding='utf-8');(a.evidence/'source-samples.json').write_text(json.dumps({'frames':source_samples,'colors':colors}));(a.evidence/'motion-diagnostics.json').write_text(json.dumps(diagnostics))
report={'source_sha256':sha,'source_reopened':True,'frames':s.frame_end,'bones':len(rig.data.bones),'fixed_bone_max_error_m':maxbone,'weights_max_sum_error':maxweight,'foot_bottom_min_m':minbottom,'planted_transform_max_drift':drifts,'rear_toe_max_drift_m':toe_drift,'bat_grip_local_max_drift':gdrift,'source_unchanged_by_export':True}
(a.evidence/'source-validation.json').write_text(json.dumps(report,indent=2));print(report)
