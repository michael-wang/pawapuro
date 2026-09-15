"""Feet-v1B source to rubber-arm review candidate: arm rest vertices and weights only."""
import bpy,json,hashlib,sys,argparse
from pathlib import Path
from mathutils import Vector
sys.dont_write_bytecode=True
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE))
from revise_footprint import snapshot
p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True)
args=p.parse_args(sys.argv[sys.argv.index('--')+1:])
source=HERE/'pitcher.blend';target=HERE/'review/style-rubber-arm-v1a/pitcher.blend'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='f54437abc9ddcf75474962c1b34d6c30e689144dd30adb806b7f03df1faa943b'
assert not target.exists(),'Refuse candidate overwrite'
official={e:hashlib.sha256(source.with_suffix('.'+e).read_bytes()).hexdigest() for e in ['blend','glb','toml']}
bpy.ops.wm.open_mainfile(filepath=str(source));bpy.context.scene.frame_set(1)
before,positions,oldrows=snapshot();o=bpy.data.objects['PitcherMesh'];a={'rest':positions}
# Preserve topology and endpoint rings. Round the existing middle rings, then
# spread the hidden two-bone bend; no pose, solver, or hand-node changes.
for side,start in [('R',2050),('L',2206)]:
 pts=[Vector(v) for v in a['rest'][start:start+156]];centres=[sum(pts[j*12:j*12+12],Vector())/12 for j in range(13)]
 for j in range(13):
  u=(j-3)/6
  if 3<=j<=9:
   centre=(1-u)**2*centres[3]+2*(1-u)*u*centres[6]+u*u*centres[9]
   tangent=(2*(1-u)*(centres[6]-centres[3])+2*u*(centres[9]-centres[6])).normalized()
   original=(centres[6]-centres[0] if j<=6 else centres[12]-centres[6]).normalized()
   q=original.rotation_difference(tangent)
   for k in range(12):o.data.vertices[start+j*12+k].co=centre+q@(pts[j*12+k]-centres[j])
  t=max(0,min(1,(j-2)/8));w=t*t*(3-2*t)
  ids=list(range(start+j*12,start+(j+1)*12))
  for name in ['arm_'+side,'forearm_'+side,'hand_'+side]:o.vertex_groups[name].remove(ids)
  if w<1:o.vertex_groups['arm_'+side].add(ids,1-w,'REPLACE')
  if w>0:o.vertex_groups['forearm_'+side].add(ids,w,'REPLACE')

o.data.update();bpy.context.scene.frame_set(1)
after,actual,newrows=snapshot();arm=set(range(2050,2362));other=set(range(len(positions)))-arm
assert {k:v for k,v in before.items() if k!='weights'}=={k:v for k,v in after.items() if k!='weights'}
assert all(before['weights'][i]==after['weights'][i] and positions[i]==actual[i] for i in other)
assert all(old['bones']==new['bones'] and all(old['points'][i]==new['points'][i] for i in other) for old,new in zip(oldrows,newrows))
target.parent.mkdir(parents=True,exist_ok=True);bpy.context.scene.frame_set(1);bpy.ops.wm.save_as_mainfile(filepath=str(target));bpy.ops.wm.open_mainfile(filepath=str(target))
saved,savedpos,savedrows=snapshot();assert saved==after and savedpos==actual and savedrows==newrows
assert all(hashlib.sha256(source.with_suffix('.'+e).read_bytes()).hexdigest()==h for e,h in official.items())
report={'official_hashes':official,'candidate_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'frames_compared':205,'nonarm_vertices':len(other),'changed_positions':sum(positions[i]!=actual[i] for i in arm),'changed_weights':sum(before['weights'][i]!=after['weights'][i] for i in arm),'all_animation_bones_grip_nonarm_exact':True,'topology_colors_cameras_contacts_exact':True}
args.evidence.mkdir(parents=True,exist_ok=True);(args.evidence/'source-comparison.json').write_text(json.dumps(report,indent=2));print('RUBBER_ARM_SOURCE_PASS',report)
