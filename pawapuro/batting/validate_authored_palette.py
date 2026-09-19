"""Palette-only GLB gate: exact non-color bytes plus final authored color inventory."""
import struct,json,hashlib,sys,collections
from pathlib import Path
D=Path(__file__).resolve().parent;E=Path(sys.argv[1])
def load(p):
 b=p.read_bytes();n=struct.unpack_from('<I',b,12)[0];return json.loads(b[20:20+n]),b[28+n:]
for rel in ['pitcher/pitcher.glb','batter/batter.glb','batter/ingame_s0/motion.glb']:
 a,ab=load(E/'before'/rel);b,bb=load(D/rel)
 print(rel,'JSON identical',a==b,'BIN lengths',len(ab),len(bb))
 if a!=b:
  print('JSON changed keys',[k for k in set(a)|set(b) if a.get(k)!=b.get(k)])
 color=set()
 for m in a['meshes']:
  for p in m['primitives']:
   ac=a['accessors'][p['attributes']['COLOR_0']];v=a['bufferViews'][ac['bufferView']];start=v.get('byteOffset',0)+ac.get('byteOffset',0)
   for i in range(ac['count']):color.update(range(start+i*v.get('byteStride',8),start+i*v.get('byteStride',8)+8))
 diff=[i for i,(x,y) in enumerate(zip(ab,bb)) if x!=y]
 print('changed bytes',len(diff),'non-color changed',sum(i not in color for i in diff))
 assert a==b and len(ab)==len(bb) and all(i in color for i in diff),'STOP: non-color asset difference'
 # Compare the final exported palette to the saved author's full color inventory.
 source={'pitcher/pitcher.glb':'pitcher_pitcher.blend','batter/batter.glb':'batter_batter.blend','batter/ingame_s0/motion.glb':'batter_ingame_s0_batter_ingame_s0.blend'}[rel]
 authored=json.loads((E/(source+'.colors.json')).read_text())
 for m in b['meshes']:
  for primitive in m['primitives']:
   ac=b['accessors'][primitive['attributes']['COLOR_0']];v=b['bufferViews'][ac['bufferView']];start=v.get('byteOffset',0)+ac.get('byteOffset',0)
   actual=collections.Counter(struct.unpack_from('<4H',bb,start+i*v.get('byteStride',8)) for i in range(ac['count']))
   expected=collections.Counter(tuple(round(x*65535) for x in c) for c in authored[m['name']])
   assert actual==expected,'exported colors differ from authored palette'
   print(m['name'],'final color groups',len(actual),'vertices',sum(actual.values()))
 print('sha256',hashlib.sha256((D/rel).read_bytes()).hexdigest())
assert (E/'before/batter/ingame_s0/motion_expected.txt').read_text().splitlines()[1:]==(D/'batter/ingame_s0/motion_expected.txt').read_text().splitlines()[1:]
print('PASS exact GLB JSON/non-COLOR_0 bytes and all 5304 motion samples unchanged; final authored palettes match')
