"""Batter-only GLB contract and saved-source/import round-trip checks."""
import argparse,hashlib,json,math,struct,sys,tomllib
from functools import cache
from pathlib import Path
import bpy
from mathutils import Matrix,Vector,Quaternion,kdtree
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True);a=p.parse_args(sys.argv[sys.argv.index('--')+1:]);meta=tomllib.loads((HERE/'batter.toml').read_text());raw=(HERE/'batter.glb').read_bytes();assert meta['source_sha256']==hashlib.sha256((HERE/'batter.blend').read_bytes()).hexdigest();assert meta['glb_sha256']==hashlib.sha256(raw).hexdigest()
n=struct.unpack_from('<I',raw,12)[0];doc=json.loads(raw[20:20+n]);binary=raw[28+n:];assert len(doc['meshes'])==2 and len(doc['animations'])==1;assert not any(doc.get(k) for k in ['materials','cameras','images','textures','extensionsUsed']);nodes=doc['nodes'];names={n['name']:i for i,n in enumerate(nodes)};assert len(names)==len(nodes)
assert names['bat_grip'] in nodes[names['hand_R']]['children'];assert all(names[n] in nodes[names['bat_grip']]['children'] for n in ['bat_barrel','bat_tip']);assert not any('AUTHORING' in n or 'Review_' in n for n in names)
@cache
def acc(i):
 t=doc['accessors'][i];v=doc['bufferViews'][t['bufferView']];fmt,size={5126:('f',4),5123:('H',2),5121:('B',1),5125:('I',4)}[t['componentType']];count={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[t['type']];stride=v.get('byteStride',size*count);offset=v.get('byteOffset',0)+t.get('byteOffset',0);rows=[struct.unpack_from('<'+fmt*count,binary,offset+j*stride) for j in range(t['count'])]
 if t.get('normalized'):rows=[tuple(x/({5121:255,5123:65535}[t['componentType']]) for x in row) for row in rows]
 assert all(math.isfinite(x) for row in rows for x in row);return rows
anim=doc['animations'][0];assert anim['name']==meta['clip']['name']=='swing_L';assert meta['clip']['loop']==False
for sampler in anim['samplers']:
 ts=acc(sampler['input']);assert sampler['interpolation']=='LINEAR' and ts[0][0]==0 and abs(ts[-1][0]-meta['clip']['duration_s'])<1e-6;assert all(x[0]<y[0] for x,y in zip(ts,ts[1:]))
source=json.loads((a.evidence/'source-samples.json').read_text());samples=source['frames'];assert len(samples)==meta['clip']['end_frame']==225
meshes={}
for node in nodes:
 if 'mesh' not in node:continue
 prim=doc['meshes'][node['mesh']]['primitives'];assert len(prim)==1;attrs=prim[0]['attributes'];assert set(attrs)=={'POSITION','COLOR_0','JOINTS_0','WEIGHTS_0'};skin=doc['skins'][node['skin']];weights=acc(attrs['WEIGHTS_0']);assert max(abs(sum(w)-1) for w in weights)<2e-5 and all(min(w)>=0 for w in weights)
 ibm=[Matrix([r[i:i+4] for i in range(0,16,4)]).transposed() for r in acc(skin['inverseBindMatrices'])];meshes[node['name']]=(acc(attrs['POSITION']),acc(attrs['JOINTS_0']),weights,skin['joints'],ibm)
 colors=acc(attrs['COLOR_0']);palette={tuple(c[:3]) for c in source['colors'][node['name']]};tree=kdtree.KDTree(len(palette))
 for i,c in enumerate(palette):tree.insert(Vector(c),i)
 tree.balance();assert max(tree.find(Vector(c[:3]))[2] for c in colors)<3e-5

def pose(time):
 trs=[{k:list(n.get(k,d)) for k,d in [('translation',(0,0,0)),('rotation',(0,0,0,1)),('scale',(1,1,1))]} for n in nodes]
 for c in anim['channels']:
  sm=anim['samplers'][c['sampler']];ts=acc(sm['input']);vs=acc(sm['output']);lo=min(round(time*60),len(ts)-1);assert abs(ts[lo][0]-time)<1e-6;trs[c['target']['node']][c['target']['path']]=vs[lo]
 local=[Matrix.LocRotScale(Vector(t['translation']),Quaternion((t['rotation'][3],*t['rotation'][:3])),Vector(t['scale'])) for t in trs];world=[None]*len(nodes)
 def visit(i,parent):
  world[i]=parent@local[i]
  for ch in nodes[i].get('children',[]):visit(ch,world[i])
 for root in doc['scenes'][doc.get('scene',0)]['nodes']:visit(root,Matrix.Identity(4))
 return world

def distance(a,b):
 tree=kdtree.KDTree(len(b))
 for i,p in enumerate(b):tree.insert(Vector(p),i)
 tree.balance();return max(tree.find(Vector(p))[2] for p in a)
glb_error=0;bounds_error=0;node_error=0
for f,sample in samples.items():
 world=pose((int(f)-1)/60)
 for name,(positions,js,ws,joints,ibm) in meshes.items():
  palette=[world[j]@m for j,m in zip(joints,ibm)];pts=[]
  for p,ji,wi in zip(positions,js,ws):
   out=Vector()
   for j,w in zip(ji,wi):
    if w:out+=(palette[j]@Vector(p))*w
   pts.append((out.x,-out.z,out.y))
  src=sample[name];glb_error=max(glb_error,distance(pts,src),distance(src,pts));bounds_error=max(bounds_error,max(abs(fn(p[k] for p in pts)-fn(p[k] for p in src)) for k in range(3) for fn in [min,max]))
 for name in ['bat_grip','bat_barrel','bat_tip']:
  t=world[names[name]].translation;node_error=max(node_error,(Vector((t.x,-t.z,t.y))-Matrix(sample['nodes'][name]).translation).length)
assert glb_error<.0001 and bounds_error<.0001 and node_error<.0001,(glb_error,bounds_error,node_error)
bpy.ops.wm.read_factory_settings(use_empty=True);s=bpy.context.scene;s.render.fps=60;s.render.fps_base=1;bpy.ops.import_scene.gltf(filepath=str(HERE/'batter.glb'));rig=next(o for o in s.objects if o.type=='ARMATURE');rt_error=0;rt_node=0;rt_color=0
for name in meshes:
 o=bpy.data.objects[name];assert len(o.data.color_attributes)==1;tree=kdtree.KDTree(len(source['colors'][name]))
 for i,c in enumerate(source['colors'][name]):tree.insert(Vector(c[:3]),i)
 tree.balance();rt_color=max(rt_color,max(tree.find(Vector(c.color[:3]))[2] for c in o.data.color_attributes[0].data))
for f,sample in samples.items():
 s.frame_set(int(f)-1);dg=bpy.context.evaluated_depsgraph_get()
 for name in meshes:
  o=bpy.data.objects[name].evaluated_get(dg);m=o.to_mesh();pts=[list(o.matrix_world@v.co) for v in m.vertices];o.to_mesh_clear();rt_error=max(rt_error,distance(pts,sample[name]),distance(sample[name],pts))
 for name in ['bat_grip','bat_barrel','bat_tip']:rt_node=max(rt_node,(rig.matrix_world@rig.pose.bones[name].matrix.translation-Matrix(sample['nodes'][name]).translation).length)
assert rt_error<.0001 and rt_node<.0001 and rt_color<.005,(rt_error,rt_node,rt_color)
report={'status':'PASS','all_frames':225,'meshes':list(meshes),'skin_count':len(doc['skins']),'bones':len(rig.data.bones),'clip':'swing_L','GLB_vs_source_max_m':glb_error,'bounds_max_m':bounds_error,'GLB_semantic_max_m':node_error,'roundtrip_mesh_max_m':rt_error,'roundtrip_semantic_max_m':rt_node,'roundtrip_color_max':rt_color,'runtime_verified':False}
(a.evidence/'roundtrip-validation.json').write_text(json.dumps(report,indent=2));print(report)
