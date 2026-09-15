"""Compare the two supplied rubber-arm assets; only --write-fixture writes a candidate fixture.
Requires source-comparison.json and both evaluated source samples in --evidence.
"""
from pathlib import Path
import json,struct,tomllib,hashlib,argparse
here=Path(__file__).resolve().parent
parser=argparse.ArgumentParser()
parser.add_argument('--baseline',type=Path,default=here)
parser.add_argument('--candidate',type=Path,default=here/'review/style-rubber-arm-v1a')
parser.add_argument('--evidence',type=Path,default=here.parents[2]/'build/style-rubber-arm-v1a')
parser.add_argument('--write-fixture',action='store_true')
args=parser.parse_args();root=args.baseline;candidate=args.candidate;out=args.evidence
label='Rubber Arm v1A'
def glb(p):
 b=p.read_bytes();n=struct.unpack_from('<I',b,12)[0];return json.loads(b[20:20+n]),b[28+n:]
a,ab=glb(root/'pitcher.glb');b,bb=glb(candidate/'pitcher.glb')
attrs=a['meshes'][0]['primitives'][0]['attributes'];pos=attrs['POSITION'];joints=attrs['JOINTS_0']
def data(j,raw,index):
 ac=j['accessors'][index];v=j['bufferViews'][ac['bufferView']];return raw[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']]
for i in range(len(a['accessors'])):
 if i not in (pos,attrs['JOINTS_0'],attrs['WEIGHTS_0']):assert data(a,ab,i)==data(b,bb,i),f'non-position accessor {i} changed'
pa,pb=data(a,ab,pos),data(b,bb,pos);js=data(a,ab,joints)
source_report=json.loads((out/'source-comparison.json').read_text())
# Export preserves source vertex ordering in this concrete sample. Check indices
# and every protected vertex across positions, joints and weights.
assert a['accessors'][pos]['count']==2362
changed=[];armcount=312
for attribute in ('POSITION','JOINTS_0','WEIGHTS_0'):
 index=attrs[attribute];old=data(a,ab,index);new=data(b,bb,index);stride=len(old)//2362
 assert len(old)==len(new)
 assert old[:2050*stride]==new[:2050*stride],attribute+' changed outside arm tubes'
 if attribute=='POSITION':changed=[i for i in range(2362) if old[i*stride:(i+1)*stride]!=new[i*stride:(i+1)*stride]]
for j in (a,b):
 for key in ('min','max'):j['accessors'][pos].pop(key,None)
assert a==b,'GLB structural/animation difference'
ma=tomllib.loads((root/'pitcher.toml').read_text());mb=tomllib.loads((candidate/'pitcher.toml').read_text());diff=[k for k in ma if ma[k]!=mb[k]]
assert set(ma)==set(mb) and set(diff)=={'source_sha256','glb_sha256'},diff
for directory,metadata in ((root,ma),(candidate,mb)):
 for ext,field in (('blend','source_sha256'),('glb','glb_sha256')):
  assert hashlib.sha256((directory/('pitcher.'+ext)).read_bytes()).hexdigest()==metadata[field], 'Stale asset provenance'
assert source_report['official_hashes']['blend']==ma['source_sha256'] and source_report['candidate_sha256']==mb['source_sha256']
src=json.loads((out/'source-samples.json').read_text());indices=[0,200,500,800,1100,1300,1500,1800,1900,2100,2300,2361]
armids=set(range(2050,2362))
oldsrc=json.loads((out/'baseline-source-samples.json').read_text())
assert all(row['grip_blender_m']==oldsrc[f]['grip_blender_m'] for f,row in src.items())
lines=(root/'motion_expected.txt').read_text().splitlines();new=[];edits=[];idx=0
for line in lines:
 if line.startswith('# source_sha256'):new.append('# source_sha256 '+mb['source_sha256'])
 elif line.startswith('# glb_sha256'):new.append('# glb_sha256 '+mb['glb_sha256'])
 elif line.startswith('# ') and 'evaluated Blender samples;' in line:new.append(f'# {label} rubber-arm candidate evaluated Blender samples; human style review pending; game local (-x,z,-y).')
 elif line.startswith('sample'):
  parts=line.split();tick=int(parts[1]);frame=tick//4+1;row=src[str(frame)];pts=[[-p[0],p[2],-p[1]] for p in row['vertices_blender_m']]
  oldpts=[[-p[0],p[2],-p[1]] for p in oldsrc[str(frame)]['vertices_blender_m']]
  bounds=[min(p[k] for p in pts) for k in range(3)]+[max(p[k] for p in pts) for k in range(3)]
  oldbounds=[min(p[k] for p in oldpts) for k in range(3)]+[max(p[k] for p in oldpts) for k in range(3)]
  for k,(oldbound,newbound) in enumerate(zip(oldbounds,bounds)):
   if oldbound!=newbound:
    assert any(p[k%3]==newbound for i,p in enumerate(pts) if i in armids) or any(p[k%3]==oldbound for i,p in enumerate(oldpts) if i in armids), 'Non-arm bound change'
  assert all(pts[i]==oldpts[i] for i in range(len(pts)) if i not in armids)
  text=' '.join(parts[:5]+[format(v,'.9g') for v in bounds]);new.append(text);idx=0
  if text!=line:edits.append(['bounds',frame])
 elif line.startswith('point'):
  if indices[idx] in armids:
   text='point '+' '.join(format(v,'.9g') for v in pts[indices[idx]]);new.append(text)
   if text!=line:edits.append(['arm_point',frame,indices[idx]])
  else:
   assert max(abs(x-y) for x,y in zip(map(float,line.split()[1:]),pts[indices[idx]]))<1e-8
   new.append(line)
  idx+=1
 else:new.append(line)
expected='\n'.join(new)+'\n'
if args.write_fixture:
 (candidate/'motion_expected.txt').write_text(expected)
else:
 assert (candidate/'motion_expected.txt').read_text()==expected,'Candidate fixture mismatch'
report={'all_protected_accessors_byte_identical':True,'glb_structure_exact_except_position_minmax':True,'arm_vertices':armcount,'changed_arm_vertices':len(changed),'unchanged_nonarm_vertices':2362-armcount,'metadata_changed_fields':diff,'fixture_changed_records':edits}
(out/'export-comparison.json').write_text(json.dumps(report,indent=2));print(report)
