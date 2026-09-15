"""One-time S0 generator. Refuses to overwrite an editable .blend."""
import argparse
import math
import sys
import tomllib
from pathlib import Path

import bpy
from mathutils import Matrix, Quaternion, Vector

HERE = Path(__file__).resolve().parent
STAGING = HERE.parent / "staging.toml"
KEY_POSES = {1: "ready", 43: "leg_lift", 67: "stride", 78: "torso_rotation",
             85: "arm_acceleration", 91: "release", 121: "follow_through"}
# Reflection preserves the app's catcher-view left/right in Blender's RH space.
# Blender (x,y,z) = game-local (-x,-z,y). GLB = (-game.x,game.y,game.z).
BASIS = Matrix(((-1, 0, 0), (0, 0, -1), (0, 1, 0)))


def bvec(v):
    return BASIS @ Vector(v)


def args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=HERE / "pitcher.blend")
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])


def main():
    output = args().output.resolve()
    if output.exists():
        raise RuntimeError(f"Refusing to overwrite editable source: {output}")
    data = tomllib.loads(STAGING.read_text(encoding="utf-8"))
    h = data["pitcher_blockout"]["height_m"]
    origin = Vector(data["pitcher_blockout"]["position_m"])
    release = Vector(data["release"]["position_m"]) - origin
    style = data["character_style"]
    head_scale = style["head_scale"]
    hand_radius = h * .045 * style["hand_scale"]
    foot_rx = h * .085 * style["foot_planar_scale"]
    foot_ry = h * .045 * style["foot_height_scale"]
    foot_rz = h * .10 * style["foot_planar_scale"]
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1
    scene.render.fps = 60
    scene.render.fps_base = 1.0
    scene.frame_start, scene.frame_end = 1, 151
    scene["source_staging"] = "../staging.toml"
    scene["placement_game_m"] = data["pitcher_blockout"]["position_m"]
    scene["blockout_proportion_scale_m"] = h
    scene["key_poses"] = ";".join(f"{k}:{v}" for k, v in KEY_POSES.items())
    scene["preview_notice"] = "S0 AUTHORING ONLY | NO RUNTIME / NO BALL FLIGHT"
    scene.timeline_markers.new("release", frame=91)
    character = bpy.data.collections.new("PitcherAsset")
    scene.collection.children.link(character)
    helpers = bpy.data.collections.new("AuthoringOnly")
    scene.collection.children.link(helpers)

    def link(obj, collection=character):
        collection.objects.link(obj)
        return obj

    rig = link(bpy.data.objects.new("PitcherRig", bpy.data.armatures.new("PitcherSkeleton")))
    bpy.context.view_layer.objects.active = rig
    rig.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    rest = {
        "root": ((0, 0, 0), (0, .25, 0), None),
        "pelvis": ((0, .34*h, 0), (0, .57*h, 0), "root"),
        "chest": ((0, .57*h, 0), (0, .75*h, 0), "pelvis"),
        "head": ((0, .75*h, 0), (0, .95*h, 0), "chest"),
        "arm_R": ((-.18*h, .51*h, 0), (-.78, 1.42, .08), "chest"),
        "forearm_R": ((-.78, 1.42, .08), (-1.12, 1.28, -.22), "arm_R"),
        "hand_R": ((-1.12, 1.28, -.22), (-1.12, 1.28, -.42), "forearm_R"),
        "grip": ((-1.12, 1.315, -.35), (-1.12, 1.315, -.45), "hand_R"),
        "arm_L": ((.18*h, .51*h, 0), (.78, 1.42, .08), "chest"),
        "forearm_L": ((.78, 1.42, .08), (1.12, 1.28, -.22), "arm_L"),
        "hand_L": ((1.12, 1.28, -.22), (1.12, 1.28, -.42), "forearm_L"),
        "foot_R": ((-.07*h, foot_ry, 0), (-.07*h, foot_ry, -.35), "root"),
        "foot_L": ((.15*h, foot_ry, 0), (.15*h, foot_ry, -.35), "root"),
    }
    for name, (head, tail, parent) in rest.items():
        bone = rig.data.edit_bones.new(name)
        bone.head, bone.tail = bvec(head), bvec(tail)
        if parent:
            bone.parent = rig.data.edit_bones[parent]
        bone.use_deform = name not in {"root", "grip"}
    bpy.ops.object.mode_set(mode="OBJECT")
    rig.show_in_front = True
    rig.data.display_type = "STICK"

    vertices, faces, colors, weights = [], [], [], []
    blue = (.22, .48, .78, 1)
    skin = (.97, .74, .51, 1)
    navy = (.16, .19, .24, 1)
    pants = (.76, .80, .86, 1)
    glove = (.55, .30, .13, 1)

    def vertex(position, color, influence):
        vertices.append(tuple(bvec(position)))
        colors.append(color)
        weights.append(influence)
        return len(vertices)-1

    def quad(a, b, c, d):
        # Reflection reverses winding once when constructing Blender geometry.
        faces.extend(((a, c, b), (a, d, c)))

    def ellipsoid(center, radii, color, bone, segments=20, rings=10):
        start = len(vertices)
        for j in range(rings+1):
            latitude = math.pi*j/rings
            for i in range(segments):
                longitude = 2*math.pi*i/segments
                p = (center[0]+radii[0]*math.sin(latitude)*math.cos(longitude),
                     center[1]+radii[1]*math.cos(latitude),
                     center[2]+radii[2]*math.sin(latitude)*math.sin(longitude))
                vertex(p, color, {bone: 1.0})
        for j in range(rings):
            for i in range(segments):
                a = start+j*segments+i
                b = start+j*segments+(i+1)%segments
                # Parametric ellipsoid winding is inward before reflection.
                if j:
                    faces.append((a, a+segments, b))
                if j < rings-1:
                    faces.append((b, a+segments, b+segments))

    # Broad hem overlapping the shorts. Deformation is gradual between pelvis/chest.
    shirt_rings = [(.295, .175, .155), (.35, .174, .153), (.43, .168, .15),
                   (.52, .158, .15), (.59, .15, .15), (.64, .13, .125)]
    start = len(vertices)
    for y, rx, rz in shirt_rings:
        blend = max(0, min(1, (y-.36)/.23))
        for i in range(20):
            a = 2*math.pi*i/20
            vertex((h*rx*math.cos(a), h*y, h*rz*math.sin(a)), blue,
                   {"pelvis": 1-blend, "chest": blend})
    for j in range(len(shirt_rings)-1):
        for i in range(20):
            quad(start+j*20+i, start+j*20+(i+1)%20,
                 start+(j+1)*20+(i+1)%20, start+(j+1)*20+i)
    for j in (0, len(shirt_rings)-1):
        c = vertex((0, shirt_rings[j][0]*h, 0), blue,
                   {"pelvis" if j==0 else "chest": 1})
        for i in range(20):
            a,b = start+j*20+i, start+j*20+(i+1)%20
            faces.append((c,b,a) if j==0 else (c,a,b))
    ellipsoid((0,.30*h,0), (.17*h,.09*h,.15*h), pants,"pelvis")
    ellipsoid((0,.75*h,0), (.23*h*head_scale,.22*h*head_scale,.21*h*head_scale),skin,"head")
    ellipsoid((0,(.75+.12*head_scale)*h,0),
              (.255*h*head_scale,.155*h*head_scale,.235*h*head_scale),blue,"head")
    ellipsoid((0,(.75+.075*head_scale)*h,-.22*h*head_scale),
              (.23*h*head_scale,.022*h*head_scale,.16*h*head_scale),blue,"head")
    for sign in (-1,1):
        ellipsoid((sign*.075*h*head_scale,(.75+.03*head_scale)*h,-.202*h*head_scale),
                  (.025*h,.032*h,.015*h),navy,"head",12,6)
    ellipsoid(rest["foot_R"][0],(foot_rx,foot_ry,foot_rz),navy,"foot_R")
    ellipsoid(rest["foot_L"][0],(foot_rx,foot_ry,foot_rz),navy,"foot_L")
    ellipsoid(rest["hand_R"][0],(hand_radius,)*3,skin,"hand_R")
    ellipsoid(rest["hand_L"][0],(.09*h,.10*h,.07*h),glove,"hand_L")

    # Connected tube topology across a hidden bend; no detached forearm pieces.
    for side in ("R","L"):
        shoulder, elbow, hand = (Vector(rest[n][0]) for n in (f"arm_{side}",f"forearm_{side}",f"hand_{side}"))
        start = len(vertices)
        for ring in range(13):
            t=ring/12
            if t <= .5:
                point=shoulder.lerp(elbow,2*t)
                tangent=(elbow-shoulder).normalized()
            else:
                point=elbow.lerp(hand,2*t-1)
                tangent=(hand-elbow).normalized()
            u=tangent.cross(Vector((0,1,0))).normalized()
            v=tangent.cross(u).normalized()
            radius=h*(.055*(1-t)+.04*t)
            # Joint translations avoid nonuniform parent-scale shear in exported TRS.
            influence=({f"arm_{side}":1-2*t,f"forearm_{side}":2*t} if t<=.5 else
                       {f"forearm_{side}":2-2*t,f"hand_{side}":2*t-1})
            for i in range(12):
                a=2*math.pi*i/12
                vertex(point+radius*(u*math.cos(a)+v*math.sin(a)),skin,influence)
        for j in range(12):
            for i in range(12):
                quad(start+j*12+i,start+j*12+(i+1)%12,start+(j+1)*12+(i+1)%12,start+(j+1)*12+i)
    mesh=bpy.data.meshes.new("PitcherMesh")
    mesh.from_pydata(vertices,[],faces)
    mesh.update()
    obj=link(bpy.data.objects.new("PitcherMesh",mesh))
    color_attr=mesh.color_attributes.new(name="Color",type="FLOAT_COLOR",domain="POINT")
    for i,color in enumerate(colors):
        color_attr.data[i].color=color
    mesh.color_attributes.active_color=color_attr
    for name in rest:
        obj.vertex_groups.new(name=name)
    for i,influence in enumerate(weights):
        for name,weight in influence.items():
            if weight>0:
                obj.vertex_groups[name].add([i],weight,"REPLACE")
    modifier=obj.modifiers.new("PitcherSkin","ARMATURE")
    modifier.object=rig
    obj.parent=rig
    mat=bpy.data.materials.new("VertexColorPreview")
    mat.use_nodes=True
    nodes=mat.node_tree.nodes
    nodes.clear()
    attr=nodes.new("ShaderNodeVertexColor"); attr.layer_name="Color"
    emission=nodes.new("ShaderNodeEmission")
    out=nodes.new("ShaderNodeOutputMaterial")
    mat.node_tree.links.new(attr.outputs["Color"],emission.inputs["Color"])
    mat.node_tree.links.new(emission.outputs[0],out.inputs[0])
    mesh.materials.append(mat)

    # World-space blocking landmarks in game-local metres, then baked to ordinary FK TRS.
    # The planted front foot holds through deceleration. Rear toe turns before lifting.
    # Columns: pelvis xyz; yaw; forward lean; R elbow; R hand; L glove; L foot xyz; rear lift.
    blocking={
      1: ((0,.833,0),-8,0,(-.43,1.11,-.35),(-.04,1.27,-.60),(.19,1.31,-.63),(.37,foot_ry,-.07),0),
      23: ((-.08,.81,.04),-18,-3,(-.46,1.10,-.34),(-.08,1.31,-.58),(.17,1.35,-.60),(.34,foot_ry,-.04),0),
      43: ((-.14,.92,.03),-32,-4,(-.48,1.30,-.22),(-.03,1.51,-.64),(.20,1.53,-.66),(.28,.89,-.33),0),
      55: ((-.10,.87,-.10),-38,2,(-.79,1.36,.12),(-.81,1.51,.45),(.52,1.51,-.61),(.38,.60,-.88),0),
      67: ((-.04,.79,-.33),-33,8,(-.96,1.50,.03),(-.99,1.98,.22),(.57,1.44,-.95),(.37,.21,-1.45),0),
      73: ((0,.79,-.47),-20,11,(-1.00,1.54,-.30),(-1.10,2.00,-.13),(.48,1.34,-.98),(.3675,foot_ry,-1.666),0),
      78: ((.01,.81,-.55),-3,13,(-.96,1.62,-.58),(-1.12,2.06,-.40),(.37,1.24,-1.02),(.3675,foot_ry,-1.666),.015),
      85: ((.01,.83,-.58),14,16,(-.89,1.65,-.83),(-.89,2.01,-1.03),(.29,1.17,-1.02),(.3675,foot_ry,-1.666),.055),
      88: ((0,.833,-.58),20,18,(-.89,1.63,-1.05),(-.72,1.85,-1.37),(.27,1.16,-1.04),(.3675,foot_ry,-1.666),.10),
      91: ((0,.833,-.5635),23,19,(-.88,1.48,-1.18),tuple(release-Vector((0,.035,-.13))),(.29,1.09,-1.05),(.3675,foot_ry,-1.666),.16),
      98: ((.02,.78,-.66),32,27,(-.65,1.25,-1.49),(-.27,1.07,-1.77),(.31,1.01,-1.03),(.3675,foot_ry,-1.666),.33),
      110: ((.07,.74,-.77),37,32,(-.37,1.08,-1.60),(.22,.72,-1.60),(.36,.97,-1.00),(.3675,foot_ry,-1.666),.63),
      121: ((.11,.76,-.87),30,27,(-.25,1.00,-1.56),(.30,.65,-1.33),(.41,1.01,-1.02),(.3675,foot_ry,-1.666),.52),
      138: ((.12,.80,-1.0),20,14,(-.24,1.03,-1.42),(.21,.72,-1.34),(.40,1.07,-1.08),(.3675,foot_ry,-1.666),.21),
      151: ((.12,.82,-1.07),16,10,(-.25,1.02,-1.38),(.13,.75,-1.32),(.38,1.09,-1.10),(.3675,foot_ry,-1.666),0),
    }
    frames=sorted(blocking)
    def interpolated(frame):
        lo=max(k for k in frames if k<=frame); hi=min(k for k in frames if k>=frame)
        t=(frame-lo)/(hi-lo) if hi!=lo else 0
        # Smooth blocking, with continuous acceleration through the short release interval.
        if hi<=78 or lo>=98: t=t*t*(3-2*t)
        return [tuple(a+(b-a)*t for a,b in zip(x,y)) if isinstance(x,tuple) else x+(y-x)*t
                for x,y in zip(blocking[lo],blocking[hi])]

    def body_matrix(name,pos,yaw=0,lean=0):
        rotation=Matrix.Rotation(math.radians(yaw),3,"Y") @ Matrix.Rotation(math.radians(-lean),3,"X")
        rotation_b=BASIS @ rotation @ BASIS.inverted()
        return Matrix.Translation(bvec(pos)) @ (rotation_b @ rig.data.bones[name].matrix_local.to_3x3()).to_4x4()

    def limb_matrix(name,start,end):
        a,b=bvec(start),bvec(end)
        direction=b-a
        return Matrix.LocRotScale(a,direction.to_track_quat("Y","Z"),Vector((1,1,1)))

    for frame in range(1,152):
        scene.frame_set(frame)
        pelvis,yaw,lean,elbow_r,hand_r,hand_l,foot_l,rear_lift=interpolated(frame)
        pelvis=Vector(pelvis)
        torso_rot=Matrix.Rotation(math.radians(yaw),3,"Y") @ Matrix.Rotation(math.radians(-lean),3,"X")
        chest=pelvis+torso_rot@Vector((0,.23*h,0))
        head=chest+torso_rot@Vector((0,.18*h,0))
        shoulders={s:chest+torso_rot@Vector((sign*.18*h,-.06*h,0)) for s,sign in (("R",-1),("L",1))}
        elbow_l=Vector(hand_l).lerp(shoulders["L"],.53)+Vector((.12,-.17,.09))
        matrices={"root":rig.data.bones["root"].matrix_local.copy(),
                  "pelvis":body_matrix("pelvis",pelvis,yaw*.7,lean*.25),
                  "chest":body_matrix("chest",chest,yaw,lean),
                  "head":body_matrix("head",head,yaw*.18,lean*.25)}
        for side,elbow,hand in (("R",elbow_r,hand_r),("L",elbow_l,hand_l)):
            matrices[f"arm_{side}"]=limb_matrix(f"arm_{side}",shoulders[side],elbow)
            matrices[f"forearm_{side}"]=limb_matrix(f"forearm_{side}",elbow,hand)
            # A spherical hand needs no finger controls. Grip stays fixed in its local space.
            matrices[f"hand_{side}"]=body_matrix(f"hand_{side}",hand,0,0)
        matrices["grip"]=matrices["hand_R"] @ rig.data.bones["hand_R"].matrix_local.inverted() @ rig.data.bones["grip"].matrix_local
        matrices["foot_L"]=body_matrix("foot_L",foot_l)
        rear_z=0 if frame<=78 else -max(0,(frame-78)/73)*1.05
        rear_x=-.07*h if frame<=98 else -.07*h-.10*min(1,(frame-98)/23)
        matrices["foot_R"]=body_matrix("foot_R",(rear_x,foot_ry+rear_lift,rear_z),min(28,rear_lift*90),min(42,rear_lift*130))
        for name in rest:
            bone=rig.data.bones[name]; pose=rig.pose.bones[name]
            kw={"parent_matrix":matrices[bone.parent.name],"parent_matrix_local":bone.parent.matrix_local} if bone.parent else {}
            basis=bone.convert_local_to_pose(matrices[name],bone.matrix_local,invert=True,**kw)
            pose.location,pose.rotation_quaternion,pose.scale=basis.decompose()
            pose.rotation_mode="QUATERNION"
            for channel in ("location","rotation_quaternion","scale"):
                pose.keyframe_insert(channel,frame=frame,group=name)
    action=rig.animation_data.action
    action.name="pitch_R"
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for curve in bag.fcurves:
                    for key in curve.keyframe_points: key.interpolation="LINEAR"

    def simple_material(name,color):
        m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
        n=m.node_tree.nodes; n.clear()
        e=n.new("ShaderNodeEmission"); e.inputs["Color"].default_value=(*color,1)
        o=n.new("ShaderNodeOutputMaterial"); m.node_tree.links.new(e.outputs[0],o.inputs[0])
        return m

    def helper_sphere(name,radius,material):
        bpy.ops.mesh.primitive_uv_sphere_add(segments=16,ring_count=8,radius=radius)
        ball=bpy.context.object; ball.name=name
        for c in list(ball.users_collection): c.objects.unlink(ball)
        helpers.objects.link(ball); ball.data.materials.append(material)
        return ball
    ball=helper_sphere("HeldBall_AUTHORING_ONLY",data["release"]["ball_marker_radius_m"],simple_material("BallPreview",(.93,.9,.77)))
    constraint=ball.constraints.new("COPY_LOCATION"); constraint.target=rig; constraint.subtarget="grip"
    for f,hidden in ((1,False),(91,False),(92,True),(151,True)):
        ball.hide_render=hidden; ball.hide_viewport=hidden
        ball.keyframe_insert("hide_render",frame=f); ball.keyframe_insert("hide_viewport",frame=f)
    # Reference cross is fixed in the authoring scene and excluded from every export.
    reference=link(bpy.data.objects.new("ReleaseReference_AUTHORING_ONLY",None),helpers)
    reference.location=bvec(release); reference.empty_display_type="SPHERE"; reference.empty_display_size=.11
    reference.color=(0,1,1,1)
    bpy.ops.mesh.primitive_torus_add(major_radius=.11,minor_radius=.007,major_segments=32,minor_segments=6,location=bvec(release),rotation=(math.pi/2,0,0))
    ring=bpy.context.object; ring.name="ReleaseRing_AUTHORING_ONLY"
    for c in list(ring.users_collection): c.objects.unlink(ring)
    helpers.objects.link(ring); ring.data.materials.append(simple_material("ReferenceCyan",(.03,.72,.85)))
    # A single neutral support disc supplies foot contact context, not a recreated field.
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=2.15,depth=.035,location=(0,.7,-.0225))
    floor=bpy.context.object; floor.name="Support_AUTHORING_ONLY"
    for c in list(floor.users_collection): c.objects.unlink(floor)
    helpers.objects.link(floor); floor.data.materials.append(simple_material("Support",(.10,.14,.16)))
    def camera(name,pos,target):
        cam=link(bpy.data.objects.new(name,bpy.data.cameras.new(name)),helpers)
        cam.location=bvec(Vector(pos)-origin)
        cam.rotation_euler=(bvec(Vector(target)-origin)-cam.location).to_track_quat("-Z","Y").to_euler()
        cam.data.lens=50
        return cam
    side=camera("Review_ThreeQuarter",(-6.0,3.0,16.0),(0,1.5,17.6))
    side.data.type="ORTHO"; side.data.ortho_scale=6.4
    batting=camera("Review_Batting",data["camera"]["position_m"],data["camera"]["target_m"])
    batting.data.sensor_fit="VERTICAL"; batting.data.sensor_height=24
    batting.data.lens=24/(2*math.tan(math.radians(data["camera"]["vertical_fov_degrees"])/2))
    scene.render.resolution_x,scene.render.resolution_y=1280,720
    scene.render.resolution_percentage=100
    from bpy_extras.object_utils import world_to_camera_view
    focus=Vector((0,(data["strike_zone"]["bottom_m"]+data["strike_zone"]["top_m"])/2,data["strike_zone"]["width_m"]/2))
    focus=bvec(focus-origin)
    bpy.context.view_layer.update()
    x0=world_to_camera_view(scene,batting,focus).x
    batting.data.shift_x=.1
    x1=world_to_camera_view(scene,batting,focus).x
    batting.data.shift_x=(.5-x0)/((x1-x0)/.1)
    scene["batting_focus_ndc_x"]=world_to_camera_view(scene,batting,focus).x
    scene.camera=side
    scene.render.engine="BLENDER_EEVEE_NEXT"
    scene.eevee.taa_render_samples=16
    scene.world.color=(.04,.06,.08)
    scene.world.use_nodes=True
    scene.world.node_tree.nodes["Background"].inputs[0].default_value=(.025,.04,.06,1)
    scene.view_settings.view_transform="Raw"
    scene.view_settings.look="None"
    scene.view_settings.exposure=0
    scene.view_settings.gamma=1
    scene.render.image_settings.file_format="PNG"
    scene.frame_set(43)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True); bpy.context.view_layer.objects.active=obj
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type=="VIEW_3D":
                area.spaces.active.region_3d.view_perspective="CAMERA"
                area.spaces.active.shading.type="MATERIAL"
    output.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(output),check_existing=True)
    print(f"S0_CREATED {output} vertices={len(vertices)} triangles={len(faces)} bones={len(rest)}")


if __name__ == "__main__":
    main()
