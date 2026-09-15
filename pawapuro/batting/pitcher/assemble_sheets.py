"""Assemble rendered evidence using Pillow; does not modify source images or assets."""
import argparse
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

parser=argparse.ArgumentParser()
parser.add_argument("--evidence",type=Path,required=True)
args=parser.parse_args()
root=args.evidence
font=ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf",22)
small=ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf",17)
frames=[1,43,67,78,85,91,121]
names=["Ready","Leg lift","Stride","Torso rotation","Arm acceleration","Release","Follow-through"]
sheet=Image.new("RGB",(1440,990),"#14212d")
d=ImageDraw.Draw(sheet)
d.text((22,14),"S0 | SOURCE .blend | Seven key poses | 60 fps / release frame 91",font=font,fill="white")
for i,(frame,name) in enumerate(zip(frames,names)):
    x=(i%3)*480; y=60+(i//3)*300
    im=Image.open(root/"source-side"/f"{frame:04}.png").convert("RGB").resize((480,270))
    sheet.paste(im,(x,y+26))
    d.text((x+12,y),f"{name} | frame {frame} | {(frame-1)/60:.3f} s",font=small,fill="white")
d.text((505,704),"One non-looping clip; phases have unequal timing.",font=small,fill="#b8cfdf")
d.text((505,734),"Cyan ring: fixed release reference.",font=small,fill="#80ddeb")
d.text((505,764),"Held ball follows grip; hidden after frame 91.",font=small,fill="#b8cfdf")
d.text((505,794),"Authoring only. No Native release or flight.",font=small,fill="#f6d18e")
sheet.save(root/"seven-key-poses.png")

# Full-size batting render plus a labelled crop, never represented as app evidence.
sheet=Image.new("RGB",(1440,700),"#14212d"); d=ImageDraw.Draw(sheet)
bat=Image.open(root/"source-batting"/"0091.png").convert("RGB")
sheet.paste(bat.resize((960,540)),(0,90))
sheet.paste(bat.crop((635,345,880,625)).resize((392,448)),(1005,120))
d.text((20,18),"SOURCE .blend | Staging batting camera / release frame 91",font=font,fill="white")
d.text((20,52),"Same pose and projection; pitcher-only authoring scene. NOT the app or an occlusion acceptance.",font=small,fill="#b8cfdf")
d.text((1030,590),"Labelled crop / 1.6x",font=small,fill="white")
sheet.save(root/"batting-camera-release.png")

# Actual round-trip renders use identical camera and framing.
sheet=Image.new("RGB",(1440,1100),"#14212d"); d=ImageDraw.Draw(sheet)
d.text((20,16),"Export verification | SOURCE .blend (left) vs GLB reimport (right)",font=font,fill="white")
d.text((20,49),"Same frames/camera; auxiliary held ball is source-only. Not a C++ importer test.",font=small,fill="#b8cfdf")
for i,frame in enumerate((43,91,121)):
    for col,folder in enumerate(("source-side","roundtrip-side")):
        im=Image.open(root/folder/f"{frame:04}.png").convert("RGB").resize((640,360))
        # Keep complete image; row spacing includes headers.
        im=im.resize((568,320))
        sheet.paste(im,(col*720+70,85+i*340))
    d.text((15,90+i*340),f"f{frame}",font=small,fill="white")
sheet.save(root/"roundtrip-comparison.png")
print("S0_SHEETS",root)

# Release boundary: same crop around the fixed cyan reference in all three frames.
im=Image.open(root/"source-side"/"0091.png").convert("RGB")
cyan=[(x,y) for y in range(im.height) for x in range(im.width)
      if im.getpixel((x,y))[0]<60 and im.getpixel((x,y))[1]>150 and im.getpixel((x,y))[2]>170]
cx=(min(x for x,y in cyan)+max(x for x,y in cyan))//2
cy=(min(y for x,y in cyan)+max(y for x,y in cyan))//2
box=(cx-120,cy-120,cx+120,cy+120)
sheet=Image.new("RGB",(1440,650),"#14212d"); d=ImageDraw.Draw(sheet)
d.text((20,15),"SOURCE .blend | Release boundary / fixed cyan reference vs held ball",font=font,fill="white")
for i,f in enumerate((90,91,92)):
    source=Image.open(root/"source-side"/f"{f:04}.png").convert("RGB")
    sheet.paste(source.crop(box).resize((440,440)),(i*480+20,110))
    d.text((i*480+20,75),f"frame {f} / {(f-1)/60:.6f} s",font=font,fill="white")
d.text((20,580),"Same 240 x 240 px crop at 1.83x. Ball hidden at f92; no post-release trajectory is simulated.",font=small,fill="#b8cfdf")
d.text((20,615),"Exported grip error against authored reference: see sample-validation.json. Native alignment remains unverified.",font=small,fill="#b8cfdf")
sheet.save(root/"release-grip-reference.png")
