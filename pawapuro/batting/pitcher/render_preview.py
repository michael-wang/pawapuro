"""Render asset-derived evidence; no source save, no ball trajectory."""
import argparse
import json
import sys
from pathlib import Path

import bpy

HERE=Path(__file__).resolve().parent
parser=argparse.ArgumentParser()
parser.add_argument("--output",type=Path,required=True)
parser.add_argument("--camera",choices=("side","batting"),default="side")
parser.add_argument("--frames")
parser.add_argument("--diagnostic",action="store_true")
parser.add_argument("--animation",action="store_true")
parser.add_argument("--roundtrip",action="store_true")
parser.add_argument("--width",type=int,default=960)
args=parser.parse_args(sys.argv[sys.argv.index("--")+1:])
scene=bpy.context.scene
if args.roundtrip:
    for obj in list(bpy.data.collections["PitcherAsset"].objects):
        bpy.data.objects.remove(obj,do_unlink=True)
    # Import into a clean asset collection; retain only explicitly excluded preview helpers.
    bpy.ops.import_scene.gltf(filepath=str(HERE/"pitcher.glb"))
    for obj in bpy.context.selected_objects:
        if obj.type=="MESH":
            obj.data.materials.clear()
            material=bpy.data.materials.get("VertexColorPreview")
            for node in material.node_tree.nodes:
                if node.type=="VERTEX_COLOR": node.layer_name=obj.data.color_attributes[0].name
            obj.data.materials.append(material)
    # The auxiliary held ball was constrained to the removed source rig.
    for obj in list(bpy.data.objects):
        if obj.name.startswith("HeldBall_"): bpy.data.objects.remove(obj,do_unlink=True)
    scene["preview_notice"]="S0 GLB ROUND-TRIP | AUTHORING ONLY"
if not args.diagnostic:
    for obj in bpy.data.collections["AuthoringOnly"].objects:
        if "Release" in obj.name: obj.hide_render=True
scene.camera=bpy.data.objects["Review_Batting" if args.camera=="batting" else "Review_ThreeQuarter"]
scene.render.resolution_x=args.width
scene.render.resolution_y=round(args.width*9/16)
scene.render.resolution_percentage=100
scene.render.engine="BLENDER_EEVEE_NEXT"
scene.eevee.taa_render_samples=16
scene.render.image_settings.file_format="PNG"
args.output.mkdir(parents=True,exist_ok=True)
poses={int(part.split(":")[0]):part.split(":")[1] for part in scene["key_poses"].split(";")}
poses[scene.timeline_markers["release"].frame]="release"
frames=list(range(scene.frame_start,scene.frame_end+1)) if args.animation else ([int(v) for v in args.frames.split(",")] if args.frames else sorted(poses))
(args.output/"preview.json").write_text(json.dumps({"source":bpy.data.filepath,"revision":scene.get("motion_revision","S0"),"start_frame":scene.frame_start,"end_frame":scene.frame_end,"fps":scene.render.fps,"fps_base":scene.render.fps_base,"release_frame":scene.timeline_markers["release"].frame,"poses":poses,"rendered_frames":frames,"camera":args.camera,"width":args.width,"height":scene.render.resolution_y,"diagnostic":args.diagnostic},indent=2),encoding="utf-8")
for frame in frames:
    scene.frame_set(frame)
    # Imported glTF begins at Blender frame 0, unlike the authoring frame 1 origin.
    if args.roundtrip: scene.frame_set(frame-1)
    scene.render.filepath=str(args.output/f"{frame:04}.png")
    bpy.ops.render.render(write_still=True)
print("S0_PREVIEW",args.camera,"roundtrip",args.roundtrip,"fps",scene.render.fps,"fps_base",scene.render.fps_base)
