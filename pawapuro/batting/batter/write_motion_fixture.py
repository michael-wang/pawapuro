"""Read saved accepted source only; emit small runtime regression samples. Never save/export."""
import argparse,hashlib,json,sys
from pathlib import Path
import bpy
p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args(sys.argv[sys.argv.index('--')+1:]);source=Path(bpy.data.filepath);sha=hashlib.sha256(source.read_bytes()).hexdigest();s=bpy.context.scene;rig=bpy.data.objects['BatterRig'];assert (s.render.fps,s.render.fps_base)==(60,1)
frames=sorted(set(map(int,json.loads(s['key_poses'])))|{116,119,120,122,124,205})
lines=['# Saved Blender evaluated samples; game-local metres, no placement.','# source_sha256 '+sha,'# glb_sha256 '+hashlib.sha256(source.with_suffix('.glb').read_bytes()).hexdigest()]
def game(v):return (-v.x,v.z,-v.y)
def values(v):return ' '.join(format(x,'.9g') for x in v)
for f in frames:
 s.frame_set(f);dg=bpy.context.evaluated_depsgraph_get();lines.append('sample '+str((f-1)*4)+' '+ ' '.join(values(game(rig.matrix_world@rig.pose.bones[n].matrix.translation)) for n in ['bat_grip','bat_barrel','bat_tip']))
 for name in ['BatterMesh','Bat']:
  o=bpy.data.objects[name].evaluated_get(dg);m=o.to_mesh();points=[game(o.matrix_world@v.co) for v in m.vertices]
  lines.append('bounds '+name+' '+values([fn(p[i] for p in points) for fn in [min,max] for i in range(3)]))
  for i in sorted({round(j*(len(points)-1)/23) for j in range(24)}):lines.append('point '+name+' '+values(points[i]))
  o.to_mesh_clear()
a.output.write_text('\n'.join(lines)+'\n',encoding='utf-8');assert hashlib.sha256(source.read_bytes()).hexdigest()==sha;print('Read-only source samples:',len(frames),'poses',a.output)
