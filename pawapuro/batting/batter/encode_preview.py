"""Encode the clean source renders at their authored rate; no time stretching."""
import argparse,json,sys
from pathlib import Path
import bpy
p=argparse.ArgumentParser();p.add_argument('--frames',type=Path,required=True);a=p.parse_args(sys.argv[sys.argv.index('--')+1:]);meta=json.loads((a.frames/'preview.json').read_text());files=[a.frames/f'{f:04}.png' for f in meta['frames']];assert all(f.is_file() for f in files);assert meta['frames']==list(range(1,meta['clip_end']+1))
bpy.ops.wm.read_factory_settings(use_empty=True);s=bpy.context.scene;s.render.fps=meta['fps'];s.render.fps_base=meta['fps_base'];im=bpy.data.images.load(str(files[0]));s.render.resolution_x,s.render.resolution_y=im.size;s.render.resolution_percentage=100;s.view_settings.view_transform='Standard';s.view_settings.look='None'
e=s.sequence_editor_create();strip=e.strips.new_image('Clean authoring preview',str(files[0]),channel=1,frame_start=1)
for f in files[1:]:strip.elements.append(f.name)
strip.frame_final_duration=len(files);s.frame_start=1;s.frame_end=len(files);s.render.image_settings.file_format='FFMPEG';s.render.ffmpeg.format='MPEG4';s.render.ffmpeg.codec='H264';s.render.ffmpeg.constant_rate_factor='HIGH';s.render.ffmpeg.ffmpeg_preset='GOOD';s.render.ffmpeg.audio_codec='NONE';out=a.frames.parent/f'swing-{meta["camera"]}-1x.mp4';s.render.filepath=str(out);bpy.ops.render.render(animation=True)
d=e.strips.new_movie('Decode verification',str(out),channel=2,frame_start=1);assert abs(d.fps-60)<1e-4 and d.frame_duration==len(files)
report={**meta,'decoded_fps':d.fps,'decoded_frames':d.frame_duration,'playback_multiplier':1,'clip_duration_s':(len(files)-1)/60,'video_duration_s':len(files)/60,'last_frame_display_s':1/60,'audio':False,'normal_speed_self_watch':False}
out.with_suffix('.json').write_text(json.dumps(report,indent=2));print(out)
