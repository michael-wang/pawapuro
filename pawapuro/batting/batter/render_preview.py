"""Read-only source preview. Clean views retain only floor/home plate helpers."""
import argparse,json,sys,hashlib
from pathlib import Path
import bpy
p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);p.add_argument('--camera',choices=['side','batting'],default='side');p.add_argument('--frames');p.add_argument('--animation',action='store_true');p.add_argument('--width',type=int,default=960);a=p.parse_args(sys.argv[sys.argv.index('--')+1:])
s=bpy.context.scene;source=Path(bpy.data.filepath);sha=hashlib.sha256(source.read_bytes()).hexdigest();s.camera=bpy.data.objects['Review_Batting' if a.camera=='batting' else 'Review_ThreeQuarter'];s.render.resolution_x=a.width;s.render.resolution_y=round(a.width*9/16);s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG';s.eevee.taa_render_samples=8
a.output.mkdir(parents=True,exist_ok=True);frames=list(range(s.frame_start,s.frame_end+1)) if a.animation else ([int(x) for x in a.frames.split(',')] if a.frames else sorted(map(int,json.loads(s['key_poses']))))
for f in frames:
 s.frame_set(f);s.render.filepath=str(a.output/f'{f:04}.png');bpy.ops.render.render(write_still=True)
(a.output/'preview.json').write_text(json.dumps({'source_sha256':sha,'camera':a.camera,'fps':s.render.fps,'fps_base':s.render.fps_base,'frames':frames,'first_frame':frames[0],'clip_end':s.frame_end,'clean':True},indent=2));assert hashlib.sha256(source.read_bytes()).hexdigest()==sha
