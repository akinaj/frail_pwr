# generate simple geometry for maze given through stdin
# output properties:
#  - single obj file for level geometry
#  - each cell represented by (possibly open) box
#  - material definition file
#  - ... ?
#  - quite a lot of redundand geometry (a bit closer to real art)

# +x - west
# +y - up
# +z - south

import sys, math

kScale = 1.0
kCellSizeX = 10.0 * kScale
kCellSizeY = 10.0 * kScale
kCellSizeZ = 6.0 * kScale

wall_cells = ["#"]
empty_cells = [" ", "P", "E"]

obstacles = { "o" : 0.2, "O": 0.5 }

lvl = []
kWidth = 1
for line in sys.stdin:
      if line == None or len(line) == 0:
         break
      else:
           lvl = lvl + list(line.strip())
           kWidth = max(kWidth, len(line.strip()))
kHeight = len(lvl) / kWidth

vertices = []
texcoords = []
normals = []
faces = None

faces_walls = []
faces_floor = []
faces_ceil  = []
faces_obstacles = []

cur_normal_multiplier = 1.0

def vec_scalar_mul(vec, sc):
    return (vec[0] * sc, vec[1] * sc, vec[2] * sc)

def vec_sub(l, r):
    return (l[0] - r[0], l[1] - r[1], l[2] - r[2])

def vec_len(v):
    return math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])

def vec_normalize(v):
    if v == (0.0, 0.0, 0.0): return v
    else:
         length = vec_len(v)
         return (v[0] / length, v[1] / length, v[2] / length)

def vec_cross(l, r):
    return (l[1] * r[2] - l[2] * r[1], l[2] * r[0] - l[0] * r[2], l[0] * r[1] - l[1] * r[0])

def triangle(verts):
    indices = []
    # calc normal
    vec1 = vec_sub(verts[0], verts[1])
    vec2 = vec_sub(verts[2], verts[1])
    tr_norm = vec_cross(vec1, vec2)
    tr_norm = vec_normalize(tr_norm)
    tr_norm = vec_scalar_mul(tr_norm, cur_normal_multiplier)

    for v in verts:
        vertices.append(v)
        # todo texcoords and normals
        texcoords.append((0.0, 0.0))
        normals.append(tr_norm)
        indices.append(len(vertices))
    faces.append(tuple(zip(indices, indices, indices)))

def gen_quad(vs):
    t1 = vs[0], vs[1], vs[2]
    t2 = vs[0], vs[2], vs[3]
    triangle(t1)
    triangle(t2)

#  A---D
#  |   |   +----> +x
#  |   |   |
#  B---C   v +y

def gen_wall(x, y):
    ax, ay = x * kCellSizeX,        y * kCellSizeY
    bx, by = ax,                   (y + 1) * kCellSizeY
    cx, cy = (x + 1) * kCellSizeX, by
    dx, dy = cx,                   ay
    lo_z = -1.0 * (kCellSizeZ * 0.25)
    hi_z = kCellSizeZ * 1.25

    global faces
    faces = faces_walls
    gen_quad(((ax, ay, lo_z), (bx, by, lo_z), (bx, by, hi_z), (ax, ay, hi_z)))
    gen_quad(((dx, dy, lo_z), (ax, ay, lo_z), (ax, ay, hi_z), (dx, dy, hi_z)))
    gen_quad(((bx, by, lo_z), (cx, cy, lo_z), (cx, cy, hi_z), (bx, by, hi_z)))
    gen_quad(((cx, cy, lo_z), (dx, dy, lo_z), (dx, dy, hi_z), (cx, cy, hi_z)))

def gen_obstacle(ob, x, y):
    ax, ay = x * kCellSizeX,        y * kCellSizeY
    bx, by = ax,                   (y + 1) * kCellSizeY
    cx, cy = (x + 1) * kCellSizeX, by
    dx, dy = cx,                   ay
    lo_z = -1.0 * (kCellSizeZ * 0.25)
    hi_z = kCellSizeZ * 1.25 * obstacles[ob]

    global faces
    faces = faces_obstacles
    gen_quad(((ax, ay, lo_z), (bx, by, lo_z), (bx, by, hi_z), (ax, ay, hi_z)))
    gen_quad(((dx, dy, lo_z), (ax, ay, lo_z), (ax, ay, hi_z), (dx, dy, hi_z)))
    gen_quad(((bx, by, lo_z), (cx, cy, lo_z), (cx, cy, hi_z), (bx, by, hi_z)))
    gen_quad(((cx, cy, lo_z), (dx, dy, lo_z), (dx, dy, hi_z), (cx, cy, hi_z)))
    gen_quad(((ax, ay, hi_z), (bx, by, hi_z), (cx, cy, hi_z), (dx, dy, hi_z)))

# todo: more interesting floor (now single quad for whole level as floor and another one as ceiling would do)
def gen_floor(x, y):
    ax, ay = x * kCellSizeX,        y * kCellSizeY
    bx, by = ax,                   (y + 1) * kCellSizeY
    cx, cy = (x + 1) * kCellSizeX, by
    dx, dy = cx,                   ay
    lo_z = 0
    hi_z = kCellSizeZ

    global faces

    faces = faces_floor
    gen_quad(((ax, ay, 0), (bx, by, 0), (cx, cy, 0), (dx, dy, 0)))

for i in range(len(lvl)):
    x, y = i % kWidth, i / kWidth
    c = lvl[i]

    if   c in wall_cells: gen_wall(x, y)
    elif c in empty_cells: gen_floor(x, y)
    elif c in obstacles.keys(): gen_obstacle(c, x, y)
    else: None

# big quads as floor and ceiling base
minx, miny = -10.0, -10.0
maxx, maxy = kCellSizeX * 1.25 * kWidth, kCellSizeY * 1.25 * kHeight
minz, maxz = -1.0, kCellSizeZ * 1.25

#faces = faces_floor
#gen_quad(((minx, miny, minz), (maxx, miny, minz), (maxx, maxy, minz), (minx, maxy, minz)))

#faces = faces_ceil
#gen_quad(((minx, miny, maxz), (maxx, miny, maxz), (maxx, maxy, maxz), (minx, maxy, maxz)))

# output geometry
obj_file = open("level.obj", "wt")

obj_file.write("mtllib level.mtl")

obj_file.write("# vertices\n")
for vert in vertices:
    obj_file.write("v %.5f %.5f %.5f\n" % (vert[0], vert[2], vert[1]))

obj_file.write("# texcoords\n")
for uv in texcoords:
    obj_file.write("vt %.3f %.3f\n" % uv)

obj_file.write("# normals\n")
for normal in normals:
    obj_file.write("vn %.5f %.5f %.5f\n" % (normal[0], normal[2], normal[1]))

def faces_by_mtl(faces, material):
    obj_file.write("o %s\ng %s\nusemtl %s\n" % (material, material, material))
    for (v1, v2, v3) in faces:
        # vertex no/texcoord no/normal no
        obj_file.write("f %d/%d/%d %d/%d/%d %d/%d/%d\n" % (v1 + v2 + v3))

faces_by_mtl(faces_floor, "floor")
faces_by_mtl(faces_walls, "walls")
faces_by_mtl(faces_ceil, "ceil")
faces_by_mtl(faces_obstacles, "obstacles")

obj_file.close()

# output material
mtl_file = open("level.mtl", "wt")
mtl_file.write("""
newmtl walls
Ka 0.3 0.3 0.3
Kd 0.75 0.2 0.2

newmtl floor
Ka 0.3 0.3 0.3
Kd 0.0 0.9 0.3

newmtl ceil
Ka 0.3 0.3 0.3
Kd 0.0 0.3 0.9

newmtl obstacles
Ka 0.3 0.3 0.3
Kd 0.0 0.3 0.9
""")
mtl_file.close()

print "mesh: level.obj"
print "material: level.mtl"
