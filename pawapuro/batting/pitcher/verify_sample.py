"""S0 authoring/export checks, run with Blender Python. No runtime implementation."""
import argparse
import sys
from functools import cache
import hashlib
import json
import math
import struct
import tomllib
from pathlib import Path

import bpy
from mathutils import Matrix, Quaternion, Vector, kdtree

HERE=Path(__file__).resolve().parent
parser=argparse.ArgumentParser()
parser.add_argument("--evidence",type=Path,default=HERE.parents[2]/"build"/"pitcher-s02a")
parser.add_argument("--asset-dir",type=Path,default=HERE)
args=parser.parse_args(sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else [])
EVIDENCE=args.evidence
ASSET=args.asset_dir


def require(condition,message):
    if not condition: raise RuntimeError(message)


def main():
    raw=(ASSET/"pitcher.glb").read_bytes()
    magic,version,length=struct.unpack_from("<III",raw)
    require(magic==0x46546c67 and version==2 and length==len(raw),"GLB header")
    json_length,json_type=struct.unpack_from("<II",raw,12)
    require(json_type==0x4e4f534a,"JSON chunk")
    doc=json.loads(raw[20:20+json_length])
    bin_length,bin_type=struct.unpack_from("<II",raw,20+json_length)
    require(bin_type==0x004e4942,"BIN chunk")
    binary=raw[28+json_length:28+json_length+bin_length]
    meta=tomllib.loads((ASSET/"pitcher.toml").read_text(encoding="utf-8"))
    staging=tomllib.loads((HERE.parent/"staging.toml").read_text(encoding="utf-8"))
    require(meta["source_sha256"]==hashlib.sha256((ASSET/"pitcher.blend").read_bytes()).hexdigest(),"Stale source hash")
    require(meta["glb_sha256"]==hashlib.sha256(raw).hexdigest(),"Stale GLB hash")
    require(meta["space"]["runtime_scale"]==1 and meta["space"]["placement_game_m"]==staging["pitcher_blockout"]["position_m"],"Placement/scale changed")
    require(not doc.get("extensionsRequired") and not doc.get("extensionsUsed"),"Unexpected extensions")
    require(not any(doc.get(k) for k in ("cameras","images","textures","materials")),"Preview or material resources leaked")
    require(len(doc["meshes"])==1 and len(doc["skins"])==1 and len(doc["animations"])==1,"Single asset contract")
    require(all("targets" not in p for m in doc["meshes"] for p in m["primitives"]),"Morph targets")
    formats={5120:("b",1),5121:("B",1),5122:("h",2),5123:("H",2),5125:("I",4),5126:("f",4)}
    sizes={"SCALAR":1,"VEC2":2,"VEC3":3,"VEC4":4,"MAT4":16}
    @cache
    def accessor(index):
        a=doc["accessors"][index]
        require("sparse" not in a,"Sparse accessor outside sample contract")
        view=doc["bufferViews"][a["bufferView"]]
        require(view.get("buffer",0)==0,"External buffer")
        code,width=formats[a["componentType"]]; size=sizes[a["type"]]
        stride=view.get("byteStride",width*size)
        offset=view.get("byteOffset",0)+a.get("byteOffset",0)
        rows=[struct.unpack_from("<"+code*size,binary,offset+i*stride) for i in range(a["count"])]
        if a.get("normalized"):
            divisor={5121:255,5123:65535}[a["componentType"]]
            rows=[tuple(x/divisor for x in row) for row in rows]
        require(all(math.isfinite(v) for row in rows for v in row),"Nonfinite accessor")
        return rows
    nodes=doc["nodes"]
    names={n["name"]:i for i,n in enumerate(nodes)}
    require(len(names)==len(nodes),"Node names must be unique in this sample")
    require(set(names)=={"root","pelvis","chest","head","arm_R","forearm_R","hand_R","grip","arm_L","forearm_L","hand_L","foot_R","foot_L","PitcherRig","PitcherMesh"},"Unexpected node set")
    require(names["grip"] in nodes[names["hand_R"]].get("children",[]),"Grip parent")
    animation=doc["animations"][0]
    require(animation.get("name")==meta["clip"]["name"]=="pitch_R","Clip name")
    require(not meta["clip"]["loop"] and meta["clip"]["playback_speed"]==1,"Playback contract")
    duration=meta["clip"]["duration_s"]
    for sampler in animation["samplers"]:
        require(sampler["interpolation"]=="LINEAR","Nonlinear output")
        times=[x[0] for x in accessor(sampler["input"])]
        require(times[0]==0 and abs(times[-1]-duration)<1e-6 and all(a<b for a,b in zip(times,times[1:])),"Timeline origin/range/order")
    require(meta["clip"]["fps"]==60 and meta["clip"]["fps_base"]==1,"FPS")
    require(meta["release"]["time_s"]==(meta["release"]["authoring_frame"]-meta["clip"]["start_frame"])/60,"Marker mapping")
    require(meta["release"]["tick_at_240_hz"]==round(meta["release"]["time_s"]*240),"Tick mapping")
    primitive=doc["meshes"][0]["primitives"][0]
    require(len(doc["meshes"][0]["primitives"])==1 and primitive.get("mode",4)==4,"Triangle primitive")
    attrs=primitive["attributes"]
    require(set(attrs)=={"POSITION","COLOR_0","JOINTS_0","WEIGHTS_0"},"Vertex attribute subset")
    positions=accessor(attrs["POSITION"]); colors=accessor(attrs["COLOR_0"])
    joints=accessor(attrs["JOINTS_0"]); weights=accessor(attrs["WEIGHTS_0"])
    indices=accessor(primitive["indices"])
    require(len(indices)%3==0 and all(i[0]<len(positions) for i in indices),"Triangle indices")
    skin=doc["skins"][0]
    max_weight_error=max(abs(sum(w)-1) for w in weights)
    require(max_weight_error<2e-5 and all(min(w)>=0 for w in weights),"Skin weights")
    require(all(len(w)==4 for w in weights) and all(j<len(skin["joints"]) for row in joints for j in row),"Influence indices")
    palette_expected=[(.22,.48,.78),(.97,.74,.51),(.16,.19,.24),(.76,.80,.86),(.55,.30,.13)]
    palette_glb=set(tuple(c[:3]) for c in colors)
    glb_color_error=max(min(max(abs(a-b) for a,b in zip(c,p)) for p in palette_expected) for c in palette_glb)
    require(len(palette_glb)==5 and glb_color_error<1e-5,"Actual COLOR_0 values differ")
    ibms=[Matrix([row[i:i+4] for i in range(0,16,4)]).transposed() for row in accessor(skin["inverseBindMatrices"])]
    require(len(ibms)==len(skin["joints"]),"Inverse bind count")
    channels=[(c["target"]["node"],c["target"]["path"],animation["samplers"][c["sampler"]]) for c in animation["channels"]]
    def pose(time=None):
        trs=[{k:n.get(k,d) for k,d in (("translation",(0,0,0)),("rotation",(0,0,0,1)),("scale",(1,1,1)))} for n in nodes]
        if time is not None:
            for node,path,sampler in channels:
                require(path in ("translation","rotation","scale"),"Unsupported channel")
                ts=[x[0] for x in accessor(sampler["input"])]; vals=accessor(sampler["output"])
                lo=max(i for i,t in enumerate(ts) if t<=time+1e-7); hi=min(lo+1,len(ts)-1)
                a=max(0,min(1,(time-ts[lo])/(ts[hi]-ts[lo]))) if hi!=lo else 0
                if path=="rotation":
                    q0=Quaternion((vals[lo][3],*vals[lo][:3])); q1=Quaternion((vals[hi][3],*vals[hi][:3]))
                    q=q0.slerp(q1,a); trs[node][path]=(q.x,q.y,q.z,q.w)
                else: trs[node][path]=tuple(x+(y-x)*a for x,y in zip(vals[lo],vals[hi]))
        local=[]
        for n,t in zip(nodes,trs):
            require("matrix" not in n,"Unexpected matrix node")
            q=t["rotation"]
            local.append(Matrix.LocRotScale(Vector(t["translation"]),Quaternion((q[3],*q[:3])),Vector(t["scale"])))
        world=[None]*len(nodes)
        def visit(i,parent):
            world[i]=parent@local[i]
            for child in nodes[i].get("children",[]): visit(child,world[i])
        for root in doc["scenes"][doc.get("scene",0)]["nodes"]: visit(root,Matrix.Identity(4))
        require(all(m is not None for m in world),"Unreachable nodes")
        return world
    bind=pose()
    bind_error=max(abs((bind[j]@m)[r][c]-(1 if r==c else 0)) for j,m in zip(skin["joints"],ibms) for r in range(4) for c in range(4))
    require(bind_error<2e-5,"Rest skin does not reproduce mesh positions")
    source=json.loads((EVIDENCE/"source-samples.json").read_text(encoding="utf-8"))
    def distance(a,b):
        tree=kdtree.KDTree(len(b))
        for i,v in enumerate(b): tree.insert(Vector(v),i)
        tree.balance()
        return max(tree.find(Vector(v))[2] for v in a)
    pose_errors={}
    for frame,sample in source.items():
        world=pose((int(frame)-1)/60)
        skinned=[]
        for p,js,ws in zip(positions,joints,weights):
            result=Vector((0,0,0))
            for joint,weight in zip(js,ws):
                if weight: result+=(world[skin["joints"][joint]]@ibms[joint]@Vector(p))*weight
            skinned.append((result.x,-result.z,result.y))
        pose_errors[frame]=max(distance(skinned,sample["vertices_blender_m"]),distance(sample["vertices_blender_m"],skinned))
    require(max(pose_errors.values())<.0001,"GLB deformation differs from source by >=0.1mm")
    world=pose(meta["release"]["time_s"])
    grip=world[names["grip"]].translation
    placement=Vector(meta["space"]["placement_game_m"])
    # Keep authored decimal reference in double precision, not a float32 Vector.
    origin=meta["space"]["placement_game_m"]
    grip_game=[a+b for a,b in zip((-grip.x,grip.y,grip.z),origin)]
    delta=[a-b for a,b in zip(grip_game,staging["release"]["position_m"])]
    release_error=math.sqrt(sum(d*d for d in delta))
    require(release_error<.0001,"Release differs from reference by >=0.1mm")
    require(grip.x>0 and grip_game[0]-origin[0]<0,"Right hand mirrored")
    source_grip=source[str(meta["release"]["authoring_frame"])]["grip_blender_m"]
    source_game=[a+b for a,b in zip((-source_grip[0],source_grip[2],-source_grip[1]),origin)]
    planted={}
    require(set(source)=={str(f) for f in range(meta["clip"]["start_frame"],meta["clip"]["end_frame"]+1)},"Source must sample the entire clip")
    require(len(meta.get("contacts",[]))==4,"Expected initial and final support intervals for both feet")
    for contact in meta["contacts"]:
        name=contact["bone"]
        interval=range(contact["start_frame"],contact["end_frame"]+1)
        transforms=[pose((f-1)/60)[names[name]] for f in interval]
        planted[f"{name}_{interval.start}_{interval.stop-1}"]=max(abs(m[r][c]-transforms[0][r][c]) for m in transforms for r in range(4) for c in range(4))
    require(max(planted.values())<1e-6,"Planted foot transform moved")
    # Independent Blender importer round-trip, into an empty scene, at the same 60fps.
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.render.fps=60; bpy.context.scene.render.fps_base=1
    bpy.ops.import_scene.gltf(filepath=str(ASSET/"pitcher.glb"))
    imported_mesh=bpy.data.objects["PitcherMesh"]
    imported_rig=next(o for o in bpy.context.scene.objects if o.type=="ARMATURE")
    shapes={p.custom_shape for p in imported_rig.pose.bones if p.custom_shape}
    require(len([o for o in bpy.context.scene.objects if o.type=="MESH" and o not in shapes])==1,"Round-trip helper leak")
    require(len(imported_mesh.data.color_attributes)==1,"Round-trip vertex color missing")
    imported_colors=imported_mesh.data.color_attributes[0]
    palette=set(tuple(c.color[:3]) for c in imported_colors.data)
    roundtrip_color_error=max(min(max(abs(a-b) for a,b in zip(c,p)) for p in palette_expected) for c in palette)
    require(len(palette)==5 and roundtrip_color_error<.005,"Round-trip color palette")
    roundtrip={}
    for frame,sample in source.items():
        bpy.context.scene.frame_set(int(frame)-1)
        dg=bpy.context.evaluated_depsgraph_get()
        obj=imported_mesh.evaluated_get(dg); mesh=obj.to_mesh()
        vertices=[tuple(obj.matrix_world@v.co) for v in mesh.vertices]
        error=max(distance(vertices,sample["vertices_blender_m"]),distance(sample["vertices_blender_m"],vertices))
        obj.to_mesh_clear()
        g=imported_rig.matrix_world@imported_rig.pose.bones["grip"].matrix.translation
        roundtrip[frame]={"mesh_max_error_m":error,"grip_error_m":(g-Vector(sample["grip_blender_m"])).length}
    require(max(v["mesh_max_error_m"] for v in roundtrip.values())<.0001,"Round-trip mesh drift")
    require(max(v["grip_error_m"] for v in roundtrip.values())<.0001,"Round-trip grip drift")
    report={"status":"PASS","vertices":len(positions),"triangles":len(indices)//3,"nodes":len(nodes),
            "skin_joints":len(skin["joints"]),"clips":1,"clip_name":animation["name"],"channels":len(channels),
            "duration_s":duration,"release_frame":meta["release"]["authoring_frame"],"release_tick_240":meta["release"]["tick_at_240_hz"],
            "glb_release_game_m":grip_game,"source_release_game_m":source_game,"release_delta_m":delta,
            "release_error_m":release_error,"planted_transform_max_drift":planted,
            "glb_color_max_error":glb_color_error,"roundtrip_color_max_error":roundtrip_color_error,
            "rest_bounds_glb_m":[[min(p[a] for p in positions) for a in range(3)],[max(p[a] for p in positions) for a in range(3)]],"max_weight_sum_error":max_weight_error,"bind_matrix_max_error":bind_error,
            "glb_vs_source_mesh_max_error_m":pose_errors,"blender_roundtrip":roundtrip,
            "roundtrip_palette":sorted(palette),"runtime_verified":False}
    (EVIDENCE/"sample-validation.json").write_text(json.dumps(report,indent=2),encoding="utf-8")
    print(json.dumps(report,indent=2))


if __name__=="__main__": main()
