"""C sample contact sheets and top-view evaluated diagnostic; no asset writes."""
import argparse
import json
import math
import tomllib
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

p = argparse.ArgumentParser()
p.add_argument('--evidence', type=Path, required=True)
a = p.parse_args()
e = a.evidence
font = ImageFont.truetype('C:/Windows/Fonts/arial.ttf', 18)
small = ImageFont.truetype('C:/Windows/Fonts/arial.ttf', 14)
manifest = json.loads((e/'after-side/preview.json').read_text(encoding='utf-8'))
comparison = json.loads((e/'local-comparison.json').read_text(encoding='utf-8'))
audit = json.loads((e/'motion-audit.json').read_text(encoding='utf-8'))
assert manifest['source_sha256'] == comparison['after_sha256']
assert json.loads((e/'before-side/preview.json').read_text(encoding='utf-8'))['source_sha256'] == comparison['before_sha256']
release = manifest['release_frame']
landing = comparison['right_final_contact_frame']
poses = {int(k): v for k, v in manifest['poses'].items()}
frames = [f for f in sorted(poses) if f >= release]


def bounds(dirs, sample_frames):
    boxes = []
    for directory in dirs:
        for f in sample_frames:
            path = e/directory/f'{f:04}.png'
            if not path.exists(): continue
            arr = np.array(Image.open(path).convert('RGB'))
            ys, xs = np.where(np.max(arr, axis=2) > 110)
            boxes.append((xs.min(), ys.min(), xs.max()+1, ys.max()+1))
    # One fixed crop for every before/after image, with room for dark shoes.
    w, h = Image.open(path).size
    return (max(0,min(b[0] for b in boxes)-100), max(0,min(b[1] for b in boxes)-40),
            min(w,max(b[2] for b in boxes)+100), min(h,max(b[3] for b in boxes)+180))


side_crop = bounds(['before-side','after-side'], frames)
batting_crop = bounds(['after-batting'], [1,49]+frames)


def sheet(name, dirs, sample_frames, crop, cols=None):
    cols = cols or len(sample_frames)
    per = math.ceil(len(sample_frames)/cols)
    out = Image.new('RGB', (cols*300, len(dirs)*per*310), (17,25,30))
    draw = ImageDraw.Draw(out)
    for row, directory in enumerate(dirs):
        for i, f in enumerate(sample_frames):
            tile = Image.open(e/directory/f'{f:04}.png').convert('RGB').crop(crop)
            tile.thumbnail((296,255))
            x=(i%cols)*300; y=(row*per+i//cols)*310
            out.paste(tile,(x+(300-tile.width)//2,y+50))
            label = 'S0.2B' if directory.startswith('before') else 'S0.2C'
            draw.text((x+6,y+4),f'{label} | f{f} | {(f-1)/60:.3f}s',font=font,fill='white')
            draw.text((x+6,y+28),poses.get(f,'continuous')+' | fixed camera crop',font=small,fill='#a7bac6')
    out.save(e/name,quality=92)


sheet('follow-before-after.jpg',['before-side','after-side'],frames,side_crop)
sheet('follow-batting.jpg',['after-batting'],frames,batting_crop)
sheet('release-continuous.jpg',['before-side','after-side'],list(range(release,release+12)),side_crop,6)
sheet('landing-continuous.jpg',['after-side'],list(range(landing-7,landing+6)),side_crop,5)
sheet('full-overview.jpg',['after-side'],list(range(1,206,8)),bounds(['after-side'],[1,49]+frames),5)

# Orthographic world-space top view from evaluated bone transforms, not a new camera.
diag_frames=[release,137,151,landing,205]
out=Image.new('RGB',(1700,640),(17,25,30));d=ImageDraw.Draw(out)
d.text((14,8),'S0.2C | EVALUATED TOP VIEW | game -Z points up | arrows = torso front',font=font,fill='white')
d.text((14,34),'Cyan: pelvis   Orange: chest   Yellow: shoulder line   Blue: right foot   White: left foot (outline = footprint guide)',font=font,fill='#b8c8d2')
placement=tomllib.loads(Path(manifest['source']).with_suffix('.toml').read_text(encoding='utf-8'))['space']['placement_game_m']
for col,f in enumerate(diag_frames):
    x0=col*340
    def project(x,z): return (x0+170+x*140,100+(z-16.1)*140)
    row=audit['frames'][f-1]
    def point(n):
        pt=row['bones'][n]['p'];return (pt[0],pt[2]+placement[2])
    d.text((x0+10,65),f'f{f} / {(f-1)/60:.3f}s',font=font,fill='white')
    d.line((x0+25,180,x0+25,110),fill='#879ba7',width=2)
    d.polygon([(x0+25,104),(x0+20,116),(x0+30,116)],fill='#879ba7')
    d.text((x0+3,185),'-Z',font=small,fill='white')
    trail=[project(r['bones']['foot_R']['p'][0],r['bones']['foot_R']['p'][2]+placement[2]) for r in audit['frames'][release-1:landing]]
    d.line(trail,fill='#3b566e',width=2)
    for n,color in [('foot_L','#dae3e9'),('foot_R','#68a9fa')]:
        x,z=point(n);yaw=math.radians(row['bones'][n]['yaw_deg'])
        ellipse=[]
        for i in range(32):
            t=i*2*math.pi/32;u=.22*math.cos(t);v=.465*math.sin(t)
            ellipse.append(project(x+u*math.cos(yaw)+v*math.sin(yaw),z-u*math.sin(yaw)+v*math.cos(yaw)))
        d.line(ellipse+[ellipse[0]],fill=color,width=2)
        px,py=project(x,z);d.ellipse((px-4,py-4,px+4,py+4),fill=color)
    for n,color in [('pelvis','#5cdddc'),('chest','#ffaa55')]:
        x,z=point(n);yaw=math.radians(row['bones'][n]['yaw_deg'])
        start=project(x,z);end=project(x-.45*math.sin(yaw),z-.45*math.cos(yaw))
        d.line([start,end],fill=color,width=4);d.ellipse((end[0]-4,end[1]-4,end[0]+4,end[1]+4),fill=color)
    d.line([project(*point('arm_R')),project(*point('arm_L'))],fill='#e8d36d',width=3)
    for k,n in enumerate(('pelvis','chest')):
        d.text((x0+10,540+k*22),f'{n} yaw {row["bones"][n]["yaw_deg"]:.1f} deg',font=small,fill='white')
    rz=point('foot_R')[1];lz=point('foot_L')[1]
    d.text((x0+10,587),f'R Z {rz:.3f} | L Z {lz:.3f} m',font=small,fill='white')
    d.text((x0+10,610),f'R bottom {row["foot_bottom_m"]["foot_R"]:.4f} m',font=small,fill='#b8c8d2')
out.save(e/'top-diagnostic.jpg',quality=92)
(e/'sheet-provenance.json').write_text(json.dumps({'source_sha256':manifest['source_sha256'],
    'side_fixed_crop':list(map(int,side_crop)),'batting_fixed_crop':list(map(int,batting_crop)),'contact_frames':frames,
    'diagnostic':'Evaluated game-space top projection; no saved camera or asset changes'},indent=2),encoding='utf-8')
print('C_SHEETS_READY',e)
