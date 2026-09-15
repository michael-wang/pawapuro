"""One official-v1B right-arm local time-remap candidate; no source overwrite."""
import argparse,hashlib,json,math,sys
from pathlib import Path
import bpy
sys.dont_write_bytecode=True
HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE))
from revise_footprint import snapshot,flat
p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True)
args=p.parse_args(sys.argv[sys.argv.index('--')+1:])
source=HERE/'pitcher.blend';target=HERE/'review/style-arm-whip-v1a/pitcher.blend'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='f54437abc9ddcf75474962c1b34d6c30e689144dd30adb806b7f03df1faa943b'
assert not target.exists(),'Refuse candidate overwrite'
official={e:hashlib.sha256(source.with_suffix('.'+e).read_bytes()).hexdigest() for e in ['blend','glb','toml']}
bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene;r=bpy.data.objects['PitcherRig']
before,positions,oldrows=snapshot();names=('arm_R','forearm_R','hand_R')
# Target frame, source frame, source frames per target frame. Smooth monotone
# Hermite timing starts at original speed, concentrates sweep, then settles.
knots=[(97,97,1),(103,112,3),(107,126,1.6),(127,140,.6),(165,170,.85),(205,205,1)]
def remap(f):
 if f<=97:return float(f)
 for (x0,y0,m0),(x1,y1,m1) in zip(knots,knots[1:]):
  if f<=x1:
   h=x1-x0;t=(f-x0)/h
   return (2*t**3-3*t*t+1)*y0+(t**3-2*t*t+t)*h*m0+(-2*t**3+3*t*t)*y1+(t**3-t*t)*h*m1
assert all(remap(f+.01)>remap(f) for f in [97+i*.1 for i in range(1080)])
poses={};sample_frames={f:remap(f) for f in range(98,205)}
for f,t in sample_frames.items():
 s.frame_set(math.floor(t),subframe=t-math.floor(t));e=r.evaluated_get(bpy.context.evaluated_depsgraph_get())
 poses[f]={n:(e.pose.bones[n].location.copy(),e.pose.bones[n].rotation_quaternion.copy(),e.pose.bones[n].scale.copy()) for n in names}
for f,pose in poses.items():
 s.frame_set(f)
 for n,(loc,q,scale) in pose.items():
  pb=r.pose.bones[n];pb.location=loc;pb.rotation_quaternion=q;pb.scale=scale
  for prop in ('location','rotation_quaternion','scale'):pb.keyframe_insert(prop,frame=f,group=n)
for layer in r.animation_data.action.layers:
 for strip in layer.strips:
  for bag in strip.channelbags:
   for fc in bag.fcurves:
    if any(fc.data_path.startswith('pose.bones["'+n+'"]') for n in names):
     for k in fc.keyframe_points:
      if 98<=k.co.x<205:k.interpolation='LINEAR'
s.frame_set(1);after,actual,newrows=snapshot()
assert positions==actual
assert {k:v for k,v in before.items() if k!='keys'}=={k:v for k,v in after.items() if k!='keys'}
for old,new in zip(before['keys'],after['keys']):
 if any(old[1].startswith('pose.bones["'+n+'"]') for n in names):
  assert old[:4]==new[:4]
  protected_old=[k for k in old[4] if k[0][0]<=97 or k[0][0]>=205];protected_new=[k for k in new[4] if k[0][0]<=97 or k[0][0]>=205]
  # Blender recalculates unused AUTO handles on neighbouring LINEAR keys.
  # Protect their values/interpolation and verify evaluated poses below.
  assert all(a[1]==b[1]=='LINEAR' and a[0]==b[0] and a[4:]==b[4:] for a,b in zip(protected_old,protected_new))
 else:assert old==new
protected=[i for i,b in enumerate(r.pose.bones) if b.name not in names+('grip',)]
o=bpy.data.objects['PitcherMesh'];affected={v.index for v in o.data.vertices if any(o.vertex_groups[g.group].name in names and g.weight>0 for g in v.groups)}
for f,(old,new) in enumerate(zip(oldrows,newrows),1):
 assert all(old['bones'][i]==new['bones'][i] for i in protected)
 assert all(old['points'][i]==new['points'][i] for i in range(len(positions)) if i not in affected)
 if f<=97 or f==205:assert old==new
s.frame_set(1);target.parent.mkdir(parents=True,exist_ok=True);bpy.ops.wm.save_as_mainfile(filepath=str(target));bpy.ops.wm.open_mainfile(filepath=str(target))
saved,savedpositions,savedrows=snapshot();assert saved==after and savedpositions==actual and savedrows==newrows
assert all(hashlib.sha256(source.with_suffix('.'+e).read_bytes()).hexdigest()==h for e,h in official.items())
report={'official_hashes':official,'candidate_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'knots_target_source_slope':knots,'source_frame_by_target':sample_frames,'frames_compared':205,'rest_geometry_weights_skeleton_exact':True,'protected_body_bones_exact':True,'pre_release_and_release_and_final_exact':True,'affected_vertex_ids':sorted(affected)}
args.evidence.mkdir(parents=True,exist_ok=True);(args.evidence/'source-comparison.json').write_text(json.dumps(report,indent=2));print('WHIP_SOURCE_PASS', {k:v for k,v in report.items() if k not in ['source_frame_by_target','affected_vertex_ids']})
