"""Inspect saved source paths/contacts; outputs review data only."""
import argparse,json,math,sys,tomllib,hashlib
from pathlib import Path
import bpy
from mathutils import Matrix,Vector
from bpy_extras.object_utils import world_to_camera_view
p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True);a=p.parse_args(sys.argv[sys.argv.index('--')+1:]);s=bpy.context.scene;rig=bpy.data.objects['BatterRig'];origin=Vector(s['placement_game_m']);B=Matrix(((-1,0,0),(0,0,-1),(0,1,0)));cam=bpy.data.objects['Review_Batting'];rows=[];head_penetration=[]
for f in range(1,s.frame_end+1):
 s.frame_set(f);row={'frame':f,'time_s':(f-1)/60};row['world']={};row['screen']={}
 for name in ['hand_R','hand_L','bat_grip','bat_barrel','bat_tip','foot_R','foot_L']:
  pos=rig.matrix_world@rig.pose.bones[name].matrix.translation;world=B.inverted()@pos+origin;uv=world_to_camera_view(s,cam,pos);row['world'][name]=list(world);row['screen'][name]=[uv.x*1920,(1-uv.y)*1080]
 row['yaw_degrees']={}
 for name in ['pelvis','chest']:
  delta=rig.pose.bones[name].matrix@rig.data.bones[name].matrix_local.inverted();forward=B.inverted()@(delta.to_3x3()@B@Vector((-1,0,0)));row['yaw_degrees'][name]=math.degrees(math.atan2(forward.z,-forward.x))
 # Check bat surface vertices against the two actual rigid head/helmet ellipsoids.
 delta=rig.pose.bones['head'].matrix@rig.data.bones['head'].matrix_local.inverted();inv=delta.inverted();o=bpy.data.objects['Bat'].evaluated_get(bpy.context.evaluated_depsgraph_get());m=o.to_mesh()
 for v in m.vertices:
  local=B.inverted()@(inv@(o.matrix_world@v.co))
  for center,radii in [((0,1.309,0),(.422,.404,.386)),((0,1.529,0),(.468,.285,.431))]:
   q=sum(((local[i]-center[i])/radii[i])**2 for i in range(3))
   if q<.98:head_penetration.append({'frame':f,'ellipsoid_q':q});break
 o.to_mesh_clear();rows.append(row)
for i,row in enumerate(rows):
 row['barrel_spacing_m']=0 if i==0 else (Vector(row['world']['bat_barrel'])-Vector(rows[i-1]['world']['bat_barrel'])).length
 row['barrel_spacing_px']=0 if i==0 else (Vector(row['screen']['bat_barrel'])-Vector(rows[i-1]['screen']['bat_barrel'])).length
marker=s.timeline_markers['contact_area'].frame;staging=tomllib.loads((Path(bpy.data.filepath).parent.parent/'staging.toml').read_text());zone=staging['strike_zone'];barrel=rows[marker-1]['world']['bat_barrel'];peak=max(rows[105:130],key=lambda r:r['barrel_spacing_m']);screen_peak=max(rows[105:130],key=lambda r:r['barrel_spacing_px'])
report={'source_sha256':hashlib.sha256(Path(bpy.data.filepath).read_bytes()).hexdigest(),'contact_area_frame':marker,'contact_area_time_s':(marker-1)/60,'barrel_at_marker_game_m':barrel,'plane_z_m':zone['width_m']/2,'barrel_plane_delta_m':barrel[2]-zone['width_m']/2,'within_zone_xy':abs(barrel[0])<=zone['width_m']/2 and zone['bottom_m']<=barrel[1]<=zone['top_m'],'peak_barrel_spacing_near_marker':{'frame':peak['frame'],'m_per_frame':peak['barrel_spacing_m'],'px_per_frame':peak['barrel_spacing_px']},'peak_barrel_screen_spacing_near_marker':{'frame':screen_peak['frame'],'px_per_frame':screen_peak['barrel_spacing_px']},'bat_head_surface_inside_ellipsoid_samples':len(head_penetration),'head_overlap_frames':sorted(set(r['frame'] for r in head_penetration))}
(a.evidence/'paths.json').write_text(json.dumps(rows));(a.evidence/'motion-review.json').write_text(json.dumps(report,indent=2));print(report)
