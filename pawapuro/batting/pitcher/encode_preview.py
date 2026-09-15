"""Use Blender's bundled encoder; every source frame is rendered offline at 60 fps."""
import argparse
import json
import sys
from pathlib import Path

import bpy

parser=argparse.ArgumentParser()
parser.add_argument("--frames",type=Path,required=True)
parser.add_argument("--output",type=Path,required=True)
parser.add_argument("--slow",action="store_true")
parser.add_argument("--batting",action="store_true")
args=parser.parse_args(sys.argv[sys.argv.index("--")+1:])
files=sorted(args.frames.glob("[0-9][0-9][0-9][0-9].png"))
if len(files)!=151: raise RuntimeError(f"Expected 151 offline frames; found {len(files)}")
bpy.ops.wm.read_factory_settings(use_empty=True)
scene=bpy.context.scene
scene.render.fps=60; scene.render.fps_base=1
scene.render.resolution_x=1920 if args.batting else 1280
scene.render.resolution_y=1080 if args.batting else 720
scene.render.resolution_percentage=100
scene.view_settings.view_transform="Standard"
scene.view_settings.look="None"
editor=scene.sequence_editor_create()
repeat=4 if args.slow else 1
sequence=[p for p in files for _ in range(repeat)]
strip=editor.strips.new_image("Source asset frames",str(sequence[0]),channel=1,frame_start=1)
for path in sequence[1:]: strip.elements.append(path.name)
strip.frame_final_duration=len(sequence)
scene.frame_start=1; scene.frame_end=len(sequence)
title=editor.strips.new_effect("S0 title",type="TEXT",channel=2,frame_start=1,frame_end=len(sequence)+1)
title.text=("S0 / SOURCE .blend / "+("BATTING CAMERA" if args.batting else "THREE-QUARTER")+
            (" / 0.25x slow motion" if args.slow else " / 1x / 60 fps"))
title.font_size=24 if not args.batting else 34
title.location=(.5,.95); title.alignment_x="CENTER"; title.color=(.9,.95,1,1)
title.use_shadow=True
note=editor.strips.new_effect("Scope",type="TEXT",channel=3,frame_start=1,frame_end=len(sequence)+1)
note.text="Authoring only | Ball hidden after release | No runtime / ball flight / dynamic occlusion approval"
note.font_size=18 if not args.batting else 26
note.location=(.5,.035); note.alignment_x="CENTER"; note.color=(.8,.88,.95,1); note.use_shadow=True
if args.batting:
    note.text="Staging camera + derived horizontal shift | Pitcher only; no batter/field | NOT an app capture"
scene.render.image_settings.file_format="FFMPEG"
scene.render.ffmpeg.format="MPEG4"
scene.render.ffmpeg.codec="H264"
scene.render.ffmpeg.constant_rate_factor="HIGH"
scene.render.ffmpeg.ffmpeg_preset="GOOD"
scene.render.ffmpeg.audio_codec="NONE"
scene.render.filepath=str(args.output)
bpy.ops.render.render(animation=True)
# Decode the produced container with Blender/FFmpeg as a separate movie source.
decoded=editor.strips.new_movie("Encoded verification",str(args.output),channel=4,frame_start=1)
if abs(decoded.fps-60)>1e-4: raise RuntimeError("Encoded FPS mismatch")
if decoded.frame_duration!=len(sequence): raise RuntimeError("Encoded frame count mismatch")
report={"file":str(args.output),"fps":decoded.fps,
        "decoded_frames":decoded.frame_duration,"duration_s":len(sequence)/60,
        "source_frames":len(files),"repeat_per_source_frame":repeat,
        "playback_speed":1/repeat,"offline_render":True,"runtime_capture":False}
args.output.with_suffix(".json").write_text(json.dumps(report,indent=2),encoding="utf-8")
print("S0_ENCODED",json.dumps(report))
