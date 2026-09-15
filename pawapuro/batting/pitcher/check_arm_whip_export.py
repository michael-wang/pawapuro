"""Check the concrete right-arm time-remap export; patch only affected candidate fixture records."""
from pathlib import Path
import argparse,hashlib,json,struct,tomllib
here=Path(__file__).resolve().parent;p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,default=here.parents[2]/'build/style-arm-whip-v1a');p.add_argument('--write-fixture',action='store_true');args=p.parse_args();out=args.evidence;candidate=here/'review/style-arm-whip-v1a'
def glb(path):
 raw=path.read_bytes();n=struct.unpack_from('<I',raw,12)[0];return json.loads(raw[20:20+n]),raw[28+n:]
def data(j,raw,i):
 a=j['accessors'][i];v=j['bufferViews'][a['bufferView']];return raw[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']]
a,ab=glb(here/'pitcher.glb');b,bb=glb(candidate/'pitcher.glb');assert a==b,'GLB structure changed'
allowed=set();changed=[];grip_outputs=set()
anim=a['animations'][0]
for channel in anim['channels']:
 name=a['nodes'][channel['target']['node']]['name'];i=anim['samplers'][channel['sampler']]['output']
 if name in ('arm_R','forearm_R','hand_R'):allowed.add(i)
 if name=='grip':grip_outputs.add(i)
for i,accessor in enumerate(a['accessors']):
 x,y=data(a,ab,i),data(b,bb,i)
 if i in grip_outputs:
  stride=len(x)//205;assert x[:97*stride]==y[:97*stride] and x[204*stride:]==y[204*stride:]
  xv=struct.unpack('<'+'f'*(len(x)//4),x);yv=struct.unpack('<'+'f'*(len(y)//4),y)
  width=stride//4
  assert max(sum((xv[j+k]-yv[j+k])**2 for k in range(width))**.5 for j in range(0,len(xv),width))<1e-6,'Exporter grip local drift'
 elif i not in allowed:assert x==y,'Protected accessor changed '+str(i)
 else:
  assert accessor['count']==205 and len(x)==len(y);stride=len(x)//205
  assert x[:97*stride]==y[:97*stride] and x[204*stride:]==y[204*stride:],'Release/pre-release/final channel changed'
  if x!=y:changed.append(i)
ma=tomllib.loads((here/'pitcher.toml').read_text());mb=tomllib.loads((candidate/'pitcher.toml').read_text())
assert set(ma)==set(mb) and {k for k in ma if ma[k]!=mb[k]}=={'source_sha256','glb_sha256'}
for folder,m in [(here,ma),(candidate,mb)]:
 for ext,field in [('blend','source_sha256'),('glb','glb_sha256')]:assert hashlib.sha256((folder/('pitcher.'+ext)).read_bytes()).hexdigest()==m[field]
report=json.loads((out/'source-comparison.json').read_text());affected=set(report['affected_vertex_ids']);old=json.loads((out/'baseline-source-samples.json').read_text());new=json.loads((out/'source-samples.json').read_text())
assert report['candidate_sha256']==mb['source_sha256'] and report['official_hashes']['blend']==ma['source_sha256']
indices=[0,200,500,800,1100,1300,1500,1800,1900,2100,2300,2361];lines=[];edits=[];index=0
basis=lambda p:[-p[0],p[2],-p[1]]
for line in (here/'motion_expected.txt').read_text().splitlines():
 text=line
 if line.startswith('# source_sha256'):text='# source_sha256 '+mb['source_sha256']
 elif line.startswith('# glb_sha256'):text='# glb_sha256 '+mb['glb_sha256']
 elif line.startswith('# Accepted'):text='# Throwing Arm Whip v1A candidate; human timing review pending; game local (-x,z,-y).'
 elif line.startswith('sample'):
  parts=line.split();frame=int(parts[1])//4+1;key=str(frame);index=0
  pts=list(map(basis,new[key]['vertices_blender_m']));oldpts=list(map(basis,old[key]['vertices_blender_m']))
  assert all(pts[i]==oldpts[i] for i in range(len(pts)) if i not in affected)
  if frame<=97 or frame==205:assert new[key]==old[key]
  else:
   grip=basis(new[key]['grip_blender_m']);bounds=[min(p[k] for p in pts) for k in range(3)]+[max(p[k] for p in pts) for k in range(3)]
   oldbounds=[min(p[k] for p in oldpts) for k in range(3)]+[max(p[k] for p in oldpts) for k in range(3)]
   for k,(x,y) in enumerate(zip(oldbounds,bounds)):
    if x!=y:assert any(pts[i][k%3]==y or oldpts[i][k%3]==x for i in affected),'Non-arm bound change'
   text=' '.join(parts[:2]+[format(v,'.9g') for v in grip+bounds])
   if text!=line:edits.append(['right_hand_grip_bounds',frame])
 elif line.startswith('point'):
  if frame>97 and frame<205 and indices[index] in affected:
   text='point '+' '.join(format(v,'.9g') for v in pts[indices[index]])
   if text!=line:edits.append(['right_arm_point',frame,indices[index]])
  else:assert max(abs(x-y) for x,y in zip(map(float,line.split()[1:]),pts[indices[index]]))<1e-8
  index+=1
 lines.append(text)
expected='\n'.join(lines)+'\n'
if args.write_fixture:(candidate/'motion_expected.txt').write_text(expected)
else:assert (candidate/'motion_expected.txt').read_text()==expected
result={'changed_output_accessors':changed,'all_other_accessors_except_grip_bake_exact':True,'grip_local_bake_delta_below_1e_6':True,'all_geometry_weights_skeleton_exact':True,'pre_release_release_final_outputs_exact':True,'fixture_edits':edits};(out/'export-comparison.json').write_text(json.dumps(result,indent=2));print(result)
