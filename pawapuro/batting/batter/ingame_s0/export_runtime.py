"""Read-only Gate C export. Blender 4.5.13; run --background --factory-startup --python-exit-code 1 --python this_file.
The single clip transports the saved source motion. Native applies Gate B's concrete
phase mapping and entry residual; the baked preview actions are independent fixtures.
"""
import bpy, hashlib, math
from pathlib import Path
from mathutils import Matrix
D=Path(__file__).resolve().parent
source=D/'batter_ingame_s0.blend'
sha=hashlib.sha256(source.read_bytes()).hexdigest()
assert sha=='c54c32f84fb9da3d54800685cd514bd6b1fc7e49abc5c0e156f08c3da068d32e'
bpy.ops.wm.open_mainfile(filepath=str(source))
C=Matrix(((1,0,0,0),(0,0,1,0),(0,-1,0,0),(0,0,0,1)))
# Independent expected matrices come from saved baked actions, never from Native/formulas.
rows=['# Gate B saved-source world matrices, glTF basis; source '+sha]
for case,c in [('Take',0),('Early',432),('Nominal',456),('Late',496)]:
 s=next(s for s in bpy.data.scenes if s.get('case')==case); bpy.context.window.scene=s
 r=next(o for o in s.objects if o.type=='ARMATURE')
 ticks={0,360,384,408,432,456,464,496,672,(s.frame_end-1)*4}
 if c: ticks.update(c+k/2 for k in range(0,89)); ticks.update([c+96,c+200,c+400,c+455.5])
 for tick in sorted(t for t in ticks if t<=(s.frame_end-1)*4):
  f=1+tick/4; s.frame_set(int(f),subframe=f-int(f)); er=r.evaluated_get(bpy.context.evaluated_depsgraph_get())
  for b in er.pose.bones:
   m=C@b.matrix
   rows.append(case+' '+str(c)+' '+str(tick)+' '+b.name+' '+' '.join(format(m[i][j],'.9g') for j in range(4) for i in range(4)))
(D/'motion_expected.txt').write_text('\n'.join(rows)+'\n',encoding='utf-8')
s=bpy.data.scenes['01_Take']; bpy.context.window.scene=s
rig=next(o for o in s.objects if o.type=='ARMATURE')
a=bpy.data.actions['AcceptedReference_swing_L_READ_ONLY_COPY'];rig.animation_data.action=a
if a.slots:rig.animation_data.action_slot=a.slots[0]
s.frame_start=1;s.frame_end=225;s.render.fps=60;s.render.fps_base=1;s.frame_set(1)
bpy.ops.object.select_all(action='DESELECT')
for o in s.objects:
 if o==rig or (o.type=='MESH' and any(m.type=='ARMATURE' and m.object==rig for m in o.modifiers)):o.select_set(True)
bpy.context.view_layer.objects.active=rig
output=D/'motion.glb'
assert bpy.ops.export_scene.gltf(filepath=str(output),export_format='GLB',use_selection=True,use_active_scene=True,export_yup=True,export_materials='NONE',export_vertex_color='NAME',export_vertex_color_name='Color',export_all_vertex_colors=False,export_active_vertex_color_when_no_material=True,export_normals=False,export_texcoords=False,export_tangents=False,export_animations=True,export_animation_mode='ACTIVE_ACTIONS',export_nla_strips_merged_animation_name='ingame_source',export_frame_range=True,export_frame_step=1,export_force_sampling=True,export_sampling_interpolation_fallback='LINEAR',export_anim_slide_to_zero=True,export_optimize_animation_size=False,export_skins=True,export_influence_nb=4,export_all_influences=False,export_def_bones=False,export_leaf_bone=False,export_morph=False,export_extras=False,export_cameras=False,export_lights=False,export_gpu_instances=False)=={'FINISHED'}
(D/'motion.toml').write_text(f'''schema_version = 1
source_sha256 = "{sha}"
glb_sha256 = "{hashlib.sha256(output.read_bytes()).hexdigest()}"
# Exporter identity uses UTF-8 without BOM and normalized LF, independent of checkout EOL.
exporter_sha256 = "{hashlib.sha256(Path(__file__).read_text(encoding='utf-8-sig').encode('utf-8')).hexdigest()}"
blender_version = "{bpy.app.version_string}"
recipe = "gate-b-world-residual-v1"
source_frames = 225
source_fps = 60
entry_frames = 6
velocity_epsilon_frames = 0.01
commit_first_tick = 432
commit_last_tick = 496
# These identify the saved Gate B regression domain, not runtime input eligibility.
# Native early-entry support now bounds remaining plant duration to six frames.
# Remaining descent interpolates Gate B's two unfinished-support entries.
# At/after authoring frame 117 the existing planted foot is retained.
plant_duration_knots = [[109.0, 6.0], [115.0, 3.0], [117.0, 0.0]]
''',encoding='utf-8')
assert hashlib.sha256(source.read_bytes()).hexdigest()==sha
print('EXPORTED_READ_ONLY',output,'fixtures',len(rows)-1)
