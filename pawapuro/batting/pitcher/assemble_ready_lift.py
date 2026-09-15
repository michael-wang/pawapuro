"""S0.2A review sheets from existing-camera renders; no source edits."""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True);a=p.parse_args()
font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',16)
small=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14)

def sheet(name, entries, columns, width=320, height=320):
    boxes={}
    for view in {e[1] for e in entries}:
        boxes[view]=[]
        for revision,_,f in [e for e in entries if e[1]==view]:
            im=Image.open(a.evidence/f'{revision}-{view}'/f'{f:04}.png').convert('RGB')
            ar=np.asarray(im); mask=(ar[:,:,0]>60)|(ar[:,:,2]>55)
            yy,xx=np.where(mask)
            boxes[view].append((max(0,int(xx.min())-25),max(0,int(yy.min())-25),min(im.width,int(xx.max())+25),min(im.height,int(yy.max())+25)))
        bb=boxes[view];boxes[view]=(min(b[0] for b in bb),min(b[1] for b in bb),max(b[2] for b in bb),max(b[3] for b in bb))
    out=Image.new('RGB',(columns*width,((len(entries)+columns-1)//columns)*(height+48)),(16,23,29));d=ImageDraw.Draw(out)
    for i,(revision,view,f) in enumerate(entries):
        im=Image.open(a.evidence/f'{revision}-{view}'/f'{f:04}.png').convert('RGB').crop(boxes[view]);ratio=min(width/im.width,height/im.height);im=im.resize((round(im.width*ratio),round(im.height*ratio)))
        x=i%columns*width;y=i//columns*(height+48)
        out.paste(im,(x+(width-im.width)//2,y+48+(height-im.height)//2))
        label='S0.1' if revision=='before' else 'S0.2A'
        d.text((x+8,y+4),f'{label} | {view} | f{f} | {(f-1)/60:.3f}s',font=font,fill='white')
        d.text((x+8,y+27),'Crop of unchanged camera; source frame',font=small,fill=(165,180,190))
    out.save(a.evidence/name,quality=92)

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
