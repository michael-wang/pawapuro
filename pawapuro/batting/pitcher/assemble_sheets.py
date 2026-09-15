"""Make review sheets from complete source renders and evaluated motion evidence."""
import argparse
import json
import math
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

parser=argparse.ArgumentParser()
parser.add_argument('--evidence',type=Path,required=True)
args=parser.parse_args(); root=args.evidence
font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',19)
small=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',16)
manifest=json.loads((root/'source-side/preview.json').read_text())
release=manifest['release_frame']; end=manifest['end_frame']
phase={label:int(frame) for frame,label in manifest['poses'].items()}

def sheet(folder, frames, name, title, labels=None, cols=4):
    w,h=400,250
    out=Image.new('RGB',(cols*w,55+math.ceil(len(frames)/cols)*h),(18,29,40)); draw=ImageDraw.Draw(out)
    draw.text((12,12),title,font=font,fill='white')
    for i,f in enumerate(frames):
        x,y=(i%cols)*w,55+(i//cols)*h
        im=Image.open(root/folder/f'{f:04}.png').convert('RGB'); im.thumbnail((w,225))
        out.paste(im,(x,y+25))
        label=(labels or {}).get(str(f),'')
        draw.text((x+8,y+2),f'{label} | f{f} | {(f-1)/60:.3f}s',font=small,fill=(224,234,245))
    out.save(root/name)

poses=manifest['poses']
sheet('source-side',sorted(map(int,poses)),'contact-sheet.png','S0.1 | source .blend | unchanged oblique camera | 60 fps',poses)
sheet('source-side',list(range(release-10,release+13)),'release-continuous.png','Every frame around release | no omitted frames | release marker from .blend')
rear_contact=json.loads((root/'motion-audit.json').read_text())['summary']['contacts']['foot_R'][-1][0]
sheet('source-side',list(range(rear_contact-24,rear_contact+13)),'recovery-continuous.png','Every frame: rear foot descent, landing and renewed support')
sheet('source-side',list(range(1,end+1,6))+([end] if (end-1)%6 else []),'whole-motion.png','Whole clip overview | every 6th frame, plus endpoint; NOT a playback review')
sheet('source-batting',sorted(map(int,poses)),'batting-contact-sheet.png','S0.1 | unchanged staging camera | no batter/field; NOT app capture',poses)
sheet('before-side',[1,43,67,78,85,91,110,121,138,151],'before-contact-sheet.png','S0 before | same camera | original 2.5s clip; source retained')
sheet('roundtrip-side',sorted(map(int,poses)),'roundtrip-contact-sheet.png','Exported GLB reimport | unchanged oblique camera',poses)

# One authoring-only diagnostic: evaluated world yaw, grip speed, top-down traces.
audit=json.loads((root/'motion-audit.json').read_text()); rows=audit['frames']
out=Image.new('RGB',(1600,1020),(18,29,40)); d=ImageDraw.Draw(out)
d.text((20,12),'S0.1 evaluated diagnostics | game-local metres | arrows never appear in clean previews',font=font,fill='white')
colors={'pelvis':(250,180,70),'chest':(75,200,245),'head':(170,240,155)}
for name,color in colors.items():
    pts=[(50+(r['frame']-1)/(end-1)*1460,220-r['bones'][name]['yaw_deg']*1.8) for r in rows]
    d.line(pts,fill=color,width=3)
    d.text((50+list(colors).index(name)*240,48),name+' world yaw',font=font,fill=color)
d.line((50,220,1510,220),fill=(85,100,115)); d.text((50,370),'Separate openings: pelvis then chest; positive yaw places throwing shoulder back.',font=font,fill='white')
for f,label in [(phase['front_contact'],'front contact'),(release,'release'),(rear_contact,'rear contact')]:
    x=50+(f-1)/(end-1)*1460; d.line((x,80,x,350),fill=(130,130,140)); d.text((x+4,330),f'{label} f{f}',font=small,fill='white')
pts=[(50+(r['frame']-1)/(end-1)*1460,570-r['grip_speed_mps']*6) for r in rows]
d.line(pts,fill=(250,155,160),width=3); d.text((50,420),'Grip speed | 0..25 m/s | diagnostic only, NOT Native ball speed',font=font,fill=(250,155,160))
for i,f in enumerate([phase['coil'],phase['front_contact'],phase['torso_opening'],release,phase['rear_follow'],phase['rear_contact']]):
    r=rows[f-1]; cx=135+i*260; cy=760
    def xy(p): return (cx+p[0]*70,cy+p[2]*70)
    for name,color in colors.items():
        p=r['bones'][name]['p']; x,y=xy(p); a=math.radians(r['bones'][name]['yaw_deg'])
        tip=(x-math.sin(a)*38,y-math.cos(a)*38)
        d.line((x,y,*tip),fill=color,width=4); d.ellipse((tip[0]-3,tip[1]-3,tip[0]+3,tip[1]+3),fill=color)
    sr=xy(r['bones']['arm_R']['p']); sl=xy(r['bones']['arm_L']['p']); d.line((*sr,*sl),fill='white',width=2)
    for foot in ['foot_R','foot_L']:
        x,y=xy(r['bones'][foot]['p']); grounded=abs(r['foot_bottom_m'][foot])<1e-6
        d.ellipse((x-16,y-10,x+16,y+10),outline=(100,250,150) if grounded else (200,110,110),width=3)
    trace=[xy(t['bones']['grip']['p']) for t in rows[max(0,f-10):min(end,f+10)]]
    d.line(trace,fill=(250,155,160),width=2)
    d.text((cx-70,820),f'f{f} / {(f-1)/60:.3f}s',font=font,fill='white')
d.text((35,890),'Top-down: white shoulder line; pink grip arc (+/-10f); green foot=ground contact; red=airborne.',font=font,fill='white')
d.text((35,930),'Full evaluated geometry checked every frame. Head ellipsoid proxy is not a complete collision certificate.',font=font,fill='white')
out.save(root/'motion-diagnostic.png')
print('REVIEW_SHEETS',root)
