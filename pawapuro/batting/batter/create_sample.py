"""One-time concrete batter sample. Export never calls this authoring script."""
import argparse,json,math,sys,tomllib
from pathlib import Path
import bpy
from mathutils import Matrix,Vector
HERE=Path(__file__).resolve().parent
B=Matrix(((-1,0,0),(0,0,-1),(0,1,0)))
def bv(v):return B@Vector(v)
def rot(yaw=0,tilt=0):return Matrix.Rotation(math.radians(yaw),3,'Y')@Matrix.Rotation(math.radians(tilt),3,'Z')
def track(a,d):return Matrix.LocRotScale(bv(a),bv(d).to_track_quat('Y','Z'),Vector((1,1,1)))
def curve(keys,f):
    # Shape-preserving Hermite, per component; no ease-to-zero at each sample.
    if f<=keys[0][0]:return keys[0][1]
    if f>=keys[-1][0]:return keys[-1][1]
    i=next(i for i in range(len(keys)-1) if keys[i][0]<=f<=keys[i+1][0])
    def slope(j):
        if j==0 or j==len(keys)-1:return 0.
        d0=(keys[j][1]-keys[j-1][1])/(keys[j][0]-keys[j-1][0]);d1=(keys[j+1][1]-keys[j][1])/(keys[j+1][0]-keys[j][0])
        return 2*d0*d1/(d0+d1) if d0*d1>0 else 0.
    a,x=keys[i];b,y=keys[i+1];t=(f-a)/(b-a);h=b-a
    return (2*t**3-3*t*t+1)*x+(t**3-2*t*t+t)*h*slope(i)+(-2*t**3+3*t*t)*y+(t**3-t*t)*h*slope(i+1)
def vec(keys,f):return Vector([curve([(k,v[a]) for k,v in keys],f) for a in range(3)])
def main(output):
    assert not output.exists(),'Refusing to overwrite editable source; make an explicit candidate revision instead.'
    data=tomllib.loads((HERE.parent/'staging.toml').read_text());origin=Vector(data['batter_blockout']['position_m']);h=data['batter_blockout']['height_m'];assert h==1.7
    bpy.ops.wm.read_factory_settings(use_empty=True);s=bpy.context.scene;s.unit_settings.system='METRIC';s.render.fps=60;s.render.fps_base=1;s.frame_start=1;s.frame_end=225
    s['placement_game_m']=list(origin);s['source_staging']='../staging.toml';s['authoring_revision']='Batter S0 candidate';s['non_looping']=True
    poses={1:'Ready',72:'Gather',88:'Lead lift',99:'Stride',107:'Plant',113:'Bat lag',118:'Acceleration',121:'Contact-area',134:'Follow-through',165:'Late follow-through',225:'Finish'}
    s['key_poses']=json.dumps(poses);s['contact_intervals']=json.dumps({'foot_R':[[1,71],[107,225]],'foot_L':[[1,114]]});s['rear_toe_support']=[115,225]
    for f,name in poses.items():s.timeline_markers.new('contact_area' if f==121 else 'pose_'+name.replace(' ','_'),frame=f)
    for f,name in [(49,'pitcher_coil_guide'),(72,'pitcher_stride_guide'),(79,'pitcher_front_contact_guide'),(97,'pitcher_release_guide'),(121,'pitch_arrival_APPROX_guide')]:s.timeline_markers.new(name,frame=f)
    asset=bpy.data.collections.new('BatterAsset');s.collection.children.link(asset);helpers=bpy.data.collections.new('AuthoringOnly');s.collection.children.link(helpers)
    def link(o,c=asset):c.objects.link(o);return o
    rig=link(bpy.data.objects.new('BatterRig',bpy.data.armatures.new('BatterSkeleton')));bpy.context.view_layer.objects.active=rig;rig.select_set(True)
    grip0=Vector((-.42,1.04,-.36));axis0=Vector((.15,.94,-.32)).normalized();foot_y=.06885
    shoulder0={'R':Vector((0,.93,.275)),'L':Vector((0,.93,-.275))}
    def arm_points(shoulder,hand,side):
        # Concrete authored rubber-arm arc, not an IK or two-hand solver.
        bend=Vector((-.13,-.07,.06 if side=='R' else -.08));mid=(shoulder+hand)*.5+bend
        return [(1-t)**2*shoulder+2*t*(1-t)*mid+t*t*hand for t in (0,1/3,2/3,1)]
    rest={'root':((0,0,0),(0,.16,0),None),'pelvis':((0,.53,0),(0,.77,0),'root'),'chest':((0,.93,0),(0,1.09,0),'pelvis'),'head':((0,1.309,0),(0,1.50,0),'chest')}
    armrest={}
    for side in ['R','L']:
        hand=grip0+(axis0*.17 if side=='L' else Vector());pts=arm_points(shoulder0[side],hand,side);armrest[side]=pts
        for j in range(3):rest[f'arm_{side}_{j}']=(pts[j],pts[j]+(pts[j+1]-pts[j]).normalized()*.16,'chest' if j==0 else f'arm_{side}_{j-1}')
        rest[f'hand_{side}']=(hand,hand+axis0*.12,f'arm_{side}_2')
    rest['foot_R']=((-.08,foot_y,.34),(-.28,foot_y,.34),'root');rest['foot_L']=((-.08,foot_y,-.34),(-.28,foot_y,-.34),'root')
    rest['bat_grip']=(grip0,grip0+axis0*.12,'hand_R');rest['bat_barrel']=(grip0+axis0*.86,grip0+axis0*.98,'bat_grip');rest['bat_tip']=(grip0+axis0*1.20,grip0+axis0*1.30,'bat_grip')
    bpy.ops.object.mode_set(mode='EDIT')
    for name,(a,b,parent) in rest.items():
        bone=rig.data.edit_bones.new(name);bone.head=bv(a);bone.tail=bv(b)
        if parent:bone.parent=rig.data.edit_bones[parent]
        bone.use_deform=name not in ('root','bat_barrel','bat_tip')
    bpy.ops.object.mode_set(mode='OBJECT');rig.show_in_front=True;rig.data.display_type='STICK'
    skin=(.97,.74,.51,1);purple=(.72,.08,.10,1);navy=(.16,.19,.24,1);pants=(.76,.80,.86,1);sole=(.48,.52,.58,1);wood=(.94,.68,.27,1)
    shoes=(.72,.08,.10,1) # Separate from the dark eye palette.
    V=[];F=[];C=[];W=[]
    def vertex(p,c,w):V.append(tuple(bv(p)));C.append(c);W.append(w);return len(V)-1
    def quad(a,b,c,d):F.extend([(a,c,b),(a,d,c)])
    def ellipsoid(center,radii,color,bone,n=20,r=10):
        start=len(V)
        for j in range(r+1):
            a=math.pi*j/r
            for i in range(n):
                b=2*math.pi*i/n;vertex(Vector(center)+Vector((radii[0]*math.sin(a)*math.cos(b),radii[1]*math.cos(a),radii[2]*math.sin(a)*math.sin(b))),color,{bone:1})
        for j in range(r):
            for i in range(n):
                a=start+j*n+i;b=start+j*n+(i+1)%n
                if j:F.append((a,a+n,b))
                if j<r-1:F.append((b,a+n,b+n))
    # Broad continuous shirt hem and overlapping shorts.
    rings=[(.48,.275,.24),(.57,.275,.24),(.73,.27,.235),(.91,.25,.23),(1.03,.21,.20),(1.07,.15,.15)];start=len(V)
    for y,rx,rz in rings:
        t=max(0,min(1,(y-.58)/.42))
        for i in range(24):
            a=2*math.pi*i/24;vertex((rx*math.cos(a),y,rz*math.sin(a)),purple,{'pelvis':1-t,'chest':t})
    for j in range(len(rings)-1):
        for i in range(24):quad(start+j*24+i,start+j*24+(i+1)%24,start+(j+1)*24+(i+1)%24,start+(j+1)*24+i)
    ellipsoid((0,.51,0),(.27,.14,.24),pants,'pelvis')
    ellipsoid((0,1.309,0),(.422,.404,.386),skin,'head');ellipsoid((0,1.529,0),(.468,.285,.431),purple,'head')
    ellipsoid((-.04,1.447,.404),(.422,.04,.294),purple,'head')
    for x in [-.12,.12]:ellipsoid((x,1.34,.372),(.035,.049,.018),navy,'head',12,6)
    ellipsoid((0,1.29,.399),(.06,.055,.05),skin,'head',12,6)
    footids={}
    for side,z in [('R',.34),('L',-.34)]:
        name='foot_'+side;start=len(V);footids[name]=[]
        for level,rad in [(0,.96),(.023,1),(.044,.98),(.088,.89),(.128,.60),(.1377,.06)]:
            for i in range(24):
                a=i*2*math.pi/24;front=math.cos(a);width=.27*(1-.18*max(-front,0));x=-.08-.40*rad*front;zz=z+width*rad*math.sin(a)
                y=level*(1-.32*max(front,0));footids[name].append(vertex((x,y,zz),shoes,{name:1}))
        for j in range(5):
            for i in range(24):quad(start+j*24+i,start+j*24+(i+1)%24,start+(j+1)*24+(i+1)%24,start+(j+1)*24+i)
        for j in [0,5]:
            center=vertex((-.08,[0,.1377][j==5],z),shoes,{name:1});footids[name].append(center)
            for i in range(24):F.append((center,start+j*24+(i+1)%24,start+j*24+i) if j==0 else (center,start+j*24+i,start+j*24+(i+1)%24))
    for side in ['R','L']:
        pts=armrest[side];start=len(V);names=[f'arm_{side}_{i}' for i in range(3)]+[f'hand_{side}']
        for j in range(25):
            t=j/24;q=min(int(t*3),2);u=t*3-q;point=pts[q].lerp(pts[q+1],u);tangent=(pts[q+1]-pts[q]).normalized();a=tangent.cross(Vector((0,1,0))).normalized();b=tangent.cross(a)
            for i in range(12):vertex(point+(.073*(1-t)+.055*t)*(a*math.cos(i*math.tau/12)+b*math.sin(i*math.tau/12)),skin,{names[q]:1-u,names[q+1]:u})
        for j in range(24):
            for i in range(12):quad(start+j*12+i,start+j*12+(i+1)%12,start+(j+1)*12+(i+1)%12,start+(j+1)*12+i)
        ellipsoid(pts[-1],(.099,)*3,skin,'hand_'+side)
    def mesh_object(name):
        mesh=bpy.data.meshes.new(name);mesh.from_pydata(V,[],F);mesh.update();obj=link(bpy.data.objects.new(name,mesh));attr=mesh.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='POINT')
        for i,c in enumerate(C):attr.data[i].color=c
        mesh.color_attributes.active_color=attr
        for bone in rest:obj.vertex_groups.new(name=bone)
        for i,weights in enumerate(W):
            for bone,w in weights.items():
                if w>0:obj.vertex_groups[bone].add([i],w,'REPLACE')
        mod=obj.modifiers.new('Skin','ARMATURE');mod.object=rig
        mat=bpy.data.materials.get('VertexColorPreview')
        if not mat:
            mat=bpy.data.materials.new('VertexColorPreview');mat.use_nodes=True;n=mat.node_tree.nodes;n.clear();a=n.new('ShaderNodeVertexColor');a.layer_name='Color';e=n.new('ShaderNodeEmission');o=n.new('ShaderNodeOutputMaterial');mat.node_tree.links.new(a.outputs['Color'],e.inputs[0]);mat.node_tree.links.new(e.outputs[0],o.inputs[0])
        mesh.materials.append(mat);return obj
    body=mesh_object('BatterMesh');body['foot_vertex_ids']=json.dumps(footids)
    V.clear();F.clear();C.clear();W.clear()
    a=axis0.cross(Vector((0,1,0))).normalized();b=axis0.cross(a)
    for dist,radius in [(-.05,.043),(0,.027),(.23,.027),(.50,.045),(.76,.068),(1.15,.074),(1.20,.055)]:
        for i in range(20):vertex(grip0+axis0*dist+radius*(a*math.cos(i*math.tau/20)+b*math.sin(i*math.tau/20)),wood,{'bat_grip':1})
    for j in range(6):
        for i in range(20):quad(j*20+i,j*20+(i+1)%20,(j+1)*20+(i+1)%20,(j+1)*20+i)
    for j,dist in [(0,-.05),(6,1.20)]:
        c=vertex(grip0+axis0*dist,wood,{'bat_grip':1})
        for i in range(20):F.append((c,j*20+(i+1)%20,j*20+i) if j==0 else (c,j*20+i,j*20+(i+1)%20))
    mesh_object('Bat')
    # Independent timing tracks: pelvis opens first; chest and hand/barrel follow.
    bodykeys=[(1,(0,.53,0)),(65,(0,.53,0)),(88,(.015,.57,-.035)),(99,(0,.55,.035)),(107,(-.03,.53,.10)),(121,(-.07,.53,.13)),(145,(-.06,.55,.14)),(180,(-.05,.56,.13)),(225,(-.04,.55,.13))]
    pkeys=[(1,0),(65,0),(88,-14),(99,-10),(107,15),(115,42),(121,65),(145,100),(180,109),(225,112)]
    ckeys=[(1,0),(65,0),(90,-22),(103,-19),(110,-5),(117,24),(121,51),(137,95),(165,124),(205,133),(225,132)]
    hkeys=[(1,0),(99,0),(121,4),(140,12),(180,30),(225,34)]
    grips=[(1,tuple(grip0)),(65,tuple(grip0)),(88,(-.38,1.08,-.40)),(107,(-.40,1.02,-.36)),(113,(-.51,.97,-.23)),(116,(-.53,.90,.06)),(119,(-.48,.84,.32)),(121,(-.40,.82,.46)),(124,(-.20,.88,.59)),(128,(.07,1.00,.57)),(134,(.34,1.20,.39)),(145,(.48,1.46,.14)),(165,(.48,1.51,-.13)),(195,(.48,1.43,-.24)),(225,(.48,1.40,-.25))]
    axes=[(1,tuple(axis0)),(107,tuple(axis0)),(113,(.55,.55,-.63)),(116,(.55,.18,-.82)),(119,(-.55,.02,-.83)),(121,(-1,-.04,-.03)),(124,(-.60,.05,.8)),(128,(.2,.25,.95)),(134,(.65,.45,.6)),(145,(.35,.9,-.25)),(165,(-.45,.75,-.48)),(195,(-.7,.52,-.48)),(225,(-.75,.45,-.48))]
    def bodymat(name,pos,R):return Matrix.Translation(bv(pos))@(B@R@B.inverted()@rig.data.bones[name].matrix_local.to_3x3()).to_4x4()
    for f in range(1,226):
        s.frame_set(f);pel=vec(bodykeys,f);pr=rot(curve(pkeys,f));cr=rot(curve(ckeys,f),curve([(1,0),(107,0),(137,-6),(225,-3)],f));headrot=rot(curve(hkeys,f))
        chest=pel+Vector((0,.40,0));head=chest+cr@Vector((0,.379,0));g=vec(grips,f);axis=vec(axes,f).normalized()
        mats={'root':rig.data.bones['root'].matrix_local.copy(),'pelvis':bodymat('pelvis',pel,pr),'chest':bodymat('chest',chest,cr),'head':bodymat('head',head,headrot)}
        for side,sign in [('R',1),('L',-1)]:
            shoulder=chest+cr@Vector((0,0,sign*.275));hand=g+(axis*.17 if side=='L' else Vector());pts=arm_points(shoulder,hand,side)
            for j in range(3):mats[f'arm_{side}_{j}']=track(pts[j],pts[j+1]-pts[j])
            mats['hand_'+side]=track(hand,axis)
        mats['bat_grip']=mats['hand_R']@rig.data.bones['hand_R'].matrix_local.inverted()@rig.data.bones['bat_grip'].matrix_local
        for name in ['bat_barrel','bat_tip']:mats[name]=mats['bat_grip']@rig.data.bones['bat_grip'].matrix_local.inverted()@rig.data.bones[name].matrix_local
        lead=vec([(1,(-.08,foot_y,.34)),(71,(-.08,foot_y,.34)),(88,(-.13,foot_y+.32,.22)),(97,(-.12,foot_y+.23,.31)),(107,(-.08,foot_y,.49)),(225,(-.08,foot_y,.49))],f)
        mats['foot_R']=bodymat('foot_R',lead,Matrix.Identity(3))
        rearrot=rot(curve([(1,0),(114,0),(137,48),(165,55),(225,55)],f),curve([(1,0),(114,0),(137,20),(175,17),(225,16)],f))
        toe=Vector((-.08-.40*.96,0,-.34));offset=Vector((-.40*.96,-foot_y,0));rear=toe-rearrot@offset
        mats['foot_L']=bodymat('foot_L',rear,rearrot)
        for name in rest:
            bone=rig.data.bones[name];pb=rig.pose.bones[name];kw={'parent_matrix':mats[bone.parent.name],'parent_matrix_local':bone.parent.matrix_local} if bone.parent else {}
            local=bone.convert_local_to_pose(mats[name],bone.matrix_local,invert=True,**kw);pb.location,pb.rotation_quaternion,pb.scale=local.decompose();pb.rotation_mode='QUATERNION'
            for channel in ['location','rotation_quaternion','scale']:pb.keyframe_insert(channel,frame=f,group=name)
    rig.animation_data.action.name='swing_L'
    for layer in rig.animation_data.action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for fc in bag.fcurves:
                    for k in fc.keyframe_points:k.interpolation='LINEAR'
    def material(name,color):
        m=bpy.data.materials.new(name);m.use_nodes=True;n=m.node_tree.nodes;n.clear();e=n.new('ShaderNodeEmission');e.inputs[0].default_value=(*color,1);o=n.new('ShaderNodeOutputMaterial');m.node_tree.links.new(e.outputs[0],o.inputs[0]);return m
    def helpermesh(name,points,faces,color):
        m=bpy.data.meshes.new(name);m.from_pydata([bv(Vector(v)-origin) for v in points],[],faces);o=link(bpy.data.objects.new(name,m),helpers);o.data.materials.append(material(name,color));return o
    helpermesh('Ground_AUTHORING_ONLY',[(-5,0,-4),(5,0,-4),(5,0,6),(-5,0,6)],[(0,3,2,1)],(.21,.15,.105))
    w=data['strike_zone']['width_m'];helpermesh('HomePlate_AUTHORING_ONLY',[(-w/2,.003,0),(w/2,.003,0),(w/2,.003,w/2),(0,.003,w),(-w/2,.003,w/2)],[(0,4,3,2,1)],(.8,.79,.73))
    # Zone wires are diagnostic only, excluded from clean previews and GLB.
    o=link(bpy.data.objects.new('ZonePlane_AUTHORING_ONLY',None),helpers);o.location=bv(Vector((0,(data['strike_zone']['bottom_m']+data['strike_zone']['top_m'])/2,w/2))-origin);o.empty_display_type='CUBE';o.empty_display_size=1;o.scale=(w/2,.001,(data['strike_zone']['top_m']-data['strike_zone']['bottom_m'])/2)
    o=link(bpy.data.objects.new('Placement_AUTHORING_ONLY',None),helpers);o.empty_display_type='PLAIN_AXES';o.empty_display_size=.25
    def camera(name,pos,target):
        c=link(bpy.data.objects.new(name,bpy.data.cameras.new(name)),helpers);c.location=bv(Vector(pos)-origin);c.rotation_euler=(bv(Vector(target)-origin)-c.location).to_track_quat('-Z','Y').to_euler();return c
    side=camera('Review_ThreeQuarter',(-2.8,2.7,3.4),(1.05,1.03,.13));side.data.type='ORTHO';side.data.ortho_scale=3.65
    cam=camera('Review_Batting',data['camera']['position_m'],data['camera']['target_m']);cam.data.sensor_fit='VERTICAL';cam.data.sensor_height=24;cam.data.lens=24/(2*math.tan(math.radians(data['camera']['vertical_fov_degrees'])/2))
    s.render.resolution_x=1280;s.render.resolution_y=720;s.render.resolution_percentage=100
    from bpy_extras.object_utils import world_to_camera_view
    focus=bv(Vector((0,(data['strike_zone']['bottom_m']+data['strike_zone']['top_m'])/2,w/2))-origin);bpy.context.view_layer.update();x0=world_to_camera_view(s,cam,focus).x;cam.data.shift_x=.1;x1=world_to_camera_view(s,cam,focus).x;cam.data.shift_x=(.5-x0)/((x1-x0)/.1)
    s.camera=side;s.render.engine='BLENDER_EEVEE_NEXT';s.eevee.taa_render_samples=16;s.world=bpy.data.worlds.new('AuthoringWorld');s.world.use_nodes=True;s.world.node_tree.nodes['Background'].inputs[0].default_value=(.035,.045,.055,1);s.view_settings.view_transform='Standard';s.view_settings.look='None';s.render.image_settings.file_format='PNG';s.frame_set(1)
    bpy.ops.object.select_all(action='DESELECT');body.select_set(True);bpy.context.view_layer.objects.active=body
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type=='VIEW_3D':area.spaces.active.region_3d.view_perspective='CAMERA';area.spaces.active.shading.type='MATERIAL'
    output.parent.mkdir(parents=True,exist_ok=True);bpy.ops.wm.save_as_mainfile(filepath=str(output));print('CREATED',output)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,default=HERE/'batter.blend');a=p.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []);main(a.output.resolve())
