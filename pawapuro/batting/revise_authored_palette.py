"""One-time palette-only saved-source revision; run after create_sample.py exports construction witnesses.
Witnesses supply authored color assignments, not replacement geometry or animation.
Exact construction topology is required; no spatial, skin-weight, or runtime-color inference.
"""
import bpy,json,sys
from pathlib import Path
from mathutils import Vector
D=Path(__file__).resolve().parent
out=Path(sys.argv[sys.argv.index('--')+1]).resolve()
reports=[]
for who,name,relative,target in [('pitcher','PitcherMesh','pitcher/pitcher.blend',(.06,.17,.32,1)),
 ('batter','BatterMesh','batter/batter.blend',(.72,.08,.10,1)),
 ('batter','BatterMesh','batter/ingame_s0/batter_ingame_s0.blend',(.72,.08,.10,1))]:
 bpy.ops.wm.open_mainfile(filepath=str(out/(who+'-construction.blend')))
 mesh=bpy.data.objects[name].data
 topology=[tuple(p.vertices) for p in mesh.polygons]
 # FLOAT_COLOR construction retains the separately authored eye/shoe assignment.
 target32=tuple(Vector(target))
 selected=[i for i,c in enumerate(mesh.color_attributes['Color'].data) if tuple(c.color)==target32]
 assert selected
 bpy.ops.wm.open_mainfile(filepath=str(D/relative))
 mesh=bpy.data.objects[name].data
 assert topology==[tuple(p.vertices) for p in mesh.polygons]
 colors=mesh.color_attributes['Color'];before=[tuple(c.color) for c in colors.data]
 # Preserve the authored jersey placket from refine_sample.py.
 placket={ring*24+12 for ring in range(6)} if who=='batter' else set()
 light=Vector((-.4,-.5,.8)).normalized();mesh.update()
 for i in selected:
  if i in placket:continue
  factor=.68+.32*max(0,mesh.vertices[i].normal.dot(light)) if who=='batter' else 1
  colors.data[i].color=tuple(c*factor for c in target[:3])+(1,)
 for i in set(range(len(before)))-set(selected):assert tuple(colors.data[i].color)==before[i]
 after=[tuple(c.color) for c in colors.data]
 reports.append({'source':relative,'target_base':target,'authored_target_vertices':len(selected),
   'changed_vertices':sum(a!=b for a,b in zip(before,after)),
   'target_shades':len({after[i] for i in selected}), 'non_target_unchanged':True})
 # Full final authoring colors are used to validate decoded GLB COLOR_0 after export.
 (out/(relative.replace('/','_')+'.colors.json')).write_text(json.dumps({o.name:[list(c.color) for c in o.data.color_attributes['Color'].data] for o in bpy.data.objects if o.type=='MESH' and 'Color' in o.data.color_attributes}),encoding='utf-8')
 bpy.context.preferences.filepaths.save_version=0
 bpy.ops.wm.save_as_mainfile(filepath=str(D/relative))
(out/'authoring-palette.json').write_text(json.dumps(reports,indent=2),encoding='utf-8')
print(reports)
