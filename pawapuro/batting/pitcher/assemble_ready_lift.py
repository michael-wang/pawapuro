"""Sample A/B review sheets from existing-camera renders; no source edits."""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True)
p.add_argument('--scope',choices=('ready-lift','arm-glove'),default='ready-lift')
p.add_argument('--before-renders',type=Path)
a=p.parse_args()
font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',16)
small=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14)

def source_frame(revision,view,f):
    if revision=='before' and a.before_renders:
        return a.before_renders/f'after-{view}'/f'{f:04}.png'
    return a.evidence/f'{revision}-{view}'/f'{f:04}.png'

def sheet(name, entries, columns, width=320, height=320):
    boxes={}
    for view in {e[1] for e in entries}:
        boxes[view]=[]
        for revision,_,f in [e for e in entries if e[1]==view]:
            im=Image.open(source_frame(revision,view,f)).convert('RGB')
            ar=np.asarray(im); mask=(ar[:,:,0]>60)|(ar[:,:,2]>55)
            yy,xx=np.where(mask)
            boxes[view].append((max(0,int(xx.min())-25),max(0,int(yy.min())-25),min(im.width,int(xx.max())+25),min(im.height,int(yy.max())+25)))
        bb=boxes[view];boxes[view]=(min(b[0] for b in bb),min(b[1] for b in bb),max(b[2] for b in bb),max(b[3] for b in bb))
    out=Image.new('RGB',(columns*width,((len(entries)+columns-1)//columns)*(height+48)),(16,23,29));d=ImageDraw.Draw(out)
    for i,(revision,view,f) in enumerate(entries):
        im=Image.open(source_frame(revision,view,f)).convert('RGB').crop(boxes[view]);ratio=min(width/im.width,height/im.height);im=im.resize((round(im.width*ratio),round(im.height*ratio)))
        x=i%columns*width;y=i//columns*(height+48)
        out.paste(im,(x+(width-im.width)//2,y+48+(height-im.height)//2))
        label=('S0.2A' if revision=='before' else 'S0.2B') if a.scope=='arm-glove' else ('S0.1' if revision=='before' else 'S0.2A')
        d.text((x+8,y+4),f'{label} | {view} | f{f} | {(f-1)/60:.3f}s',font=font,fill='white')
        d.text((x+8,y+27),'Crop of unchanged camera; source frame',font=small,fill=(165,180,190))
    out.save(a.evidence/name,quality=92)

if a.scope=='arm-glove':
    assert a.before_renders, 'Reuse the verified S0.2A renders'
    sheet('ready-coil-regression.jpg',[(r,'side',f) for r in ('before','after') for f in (1,49)],2,550,440)
    sheet('glove-poses.jpg',[(r,v,f) for v in ('side','batting') for r in ('before','after') for f in (61,72,79,90,97)],5,300,300)
    sheet('transition-continuous.jpg',[('after','side',f) for f in list(range(49,59))+list(range(91,101))],5,280,280)
    sheet('deformation-overview.jpg',[('after','side',f) for f in range(1,206,8)],5,280,280)
    base=a.evidence/'probe-0/arm-diagnosis';final=a.evidence/'final/arm-diagnosis'
    rows=json.loads((base/'diagnosis.json').read_text())['views']
    out=Image.new('RGB',(1440,3*340),(16,23,29));draw=ImageDraw.Draw(out)
    for row,f in enumerate((77,79,81)):
        for vi,view in enumerate(('side','other')):
            points=list(next(r['projected_bones'].values() for r in rows if r['view']==view and r['frame']==f))
            box=(int(min(p[0] for p in points)-80),int(min(p[1] for p in points)-80),int(max(p[0] for p in points)+90),int(max(p[1] for p in points)+85))
            for k,(folder,label) in enumerate(((base,'S0.2A'),(final,'S0.2B'))):
                im=Image.open(folder/f'{view}-{f}-clean.png').convert('RGB').crop(box)
                ratio=min(360/im.width,300/im.height);im=im.resize((round(im.width*ratio),round(im.height*ratio)))
                x=(vi*2+k)*360;y=row*340
                out.paste(im,(x,y+40));draw.text((x+8,y+8),f'{label} | {view} | f{f} | {(f-1)/60:.3f}s',font=font,fill='white')
    out.save(a.evidence/'arm-before-after.jpg',quality=92)
    sys.exit(0)

for view in ('side','batting'):
    sheet(f'contact-{view}.jpg',[(r,view,f) for r in ('before','after') for f in (1,32,49,61,72)],5)
    sheet(f'ready-coil-{view}.jpg',[(r,view,f) for r in ('before','after') for f in (1,49)],2,550,470)
sheet('separation-continuous.jpg',[('after','side',f) for f in range(49,64)],5,300,300)
sheet('transition-continuous.jpg',[('after','side',f) for f in range(64,77)],5,300,300)
sheet('after-overview.jpg',[('after','side',f) for f in list(range(1,206,8))+[205]],5,280,280)
diag=a.evidence/'arm-diagnosis'; data=json.loads((diag/'diagnosis.json').read_text())
out=Image.new('RGB',(1280,6*262),(20,27,32));d=ImageDraw.Draw(out)
for i,row in enumerate(data['views']):
    for j,kind in enumerate(('clean','wire')):
        im=Image.open(diag/f"{row['view']}-{row['frame']}-{kind}.png").convert('RGB')
        if kind=='wire':
            draw=ImageDraw.Draw(im);pts=[row['projected_bones'][n] for n in ('arm_R','forearm_R','hand_R')]
            draw.line([tuple(p) for p in pts],fill=(255,130,0),width=5)
            for x,y in pts:draw.ellipse((x-6,y-6,x+6,y+6),fill=(255,130,0))
        # Fixed crop retains the arm in both diagnostic angles.
        im=im.crop((150,100,1150,470));im.thumbnail((640,230))
        out.paste(im,(j*640,i*262+32));d.text((j*640+8,i*262+5),f"S0.1 f{row['frame']} | {row['view']} | {kind}",font=font,fill='white')
out.save(diag/'diagnostic-sheet.jpg',quality=92)
