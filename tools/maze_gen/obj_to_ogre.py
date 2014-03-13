# convert mesh from .obj file (get name through stdin) to OGRE .xml intermediate file
# used for fast but dirty 'pipeline' for testing auto level generation for mkdemo
# caution! generated mesh has a lot of duplicated vertices!
import sys

print "<!--"

for line in sys.stdin:
    ln = line.strip()
    if ln[:6] == "mesh: ":
       mesh_file = ln[6:]
       print "Mesh file:", mesh_file
    elif ln[:10] == "material: ":
         material_file = ln[10:]
         print "Material file:", material_file
    elif len(ln) > 0:
         print "Ignoring input line '" + ln + "'"

mesh_f = open(mesh_file, "rt")
mesh_lines = [ln.strip() for ln in mesh_f.read().split("\n") if len(ln.strip()) > 0 and ln[0] != '#']
mesh_f.close()

vertices = []
normals = []
coords = []
faces_per_mat = { }

cur_mat = "INVALID"

def starts_with(ln, string):
    return ln[:len(string)] == string

def line_content(ln, header):
    return ln[len(header):]

def split_mesh_line(ln, header):
    return tuple(map(float, line_content(ln, header).split(" ")))

for mesh_line in mesh_lines:
    if starts_with(mesh_line, "vn "):
       normals.append(split_mesh_line(mesh_line, "vn "))
    elif starts_with(mesh_line, "vt "):
         coords.append(split_mesh_line(mesh_line, "vt "))
    elif starts_with(mesh_line, "v "):
         vertices.append(split_mesh_line(mesh_line, "v "))
    elif starts_with(mesh_line, "usemtl "):
         cur_mat = line_content(mesh_line, "usemtl ")
         if not faces_per_mat.has_key(cur_mat):
            faces_per_mat[cur_mat] = []
    elif starts_with(mesh_line, "f "):
         cnt = line_content(mesh_line, "f ")
         verts = cnt.split(" ")
         faces_per_mat[cur_mat].append(tuple(map(lambda v: tuple(map(int, v.split("/"))), verts)))

print "Vertices:", len(vertices)
print "Normals:", len(normals)
print "Coords:", len(coords)
print "Materials:", len(faces_per_mat)

total_faces = 0
for mat in faces_per_mat:
    print "Faces with mtl", mat, ":", len(faces_per_mat[mat])
    total_faces += len(faces_per_mat[mat])

print "Total faces:", total_faces
print "-->"

print "<mesh>"
print "  <submeshes>"

def create_ogre_vertex(vert):
    vpos_no, vt_no, vn_no = vert
    vpos = vertices[vpos_no - 1]
    vtex = coords[vt_no - 1]
    vnrm = normals[vn_no - 1]

    return (vpos, vtex, vnrm)

for mtl in faces_per_mat:
    if len(faces_per_mat[mtl]) == 0: continue

    print "    <submesh material=\"" + mtl + "\" usesharedvertices=\"false\">"
    print "      <faces count=\"" + str(len(faces_per_mat[mtl])) + "\">"
    verts = []
    for face in faces_per_mat[mtl]:
        verts = verts + map(create_ogre_vertex, face)
        vlen = len(verts)
        v_indices = (vlen - 3, vlen - 2, vlen - 1)
        print ("      <face v1=\"%d\" v2=\"%d\" v3=\"%d\" />" % v_indices)
    print "      </faces>"
    print "      <geometry vertexcount=\"" + str(len(verts)) + "\">"
    print "        <vertexbuffer positions=\"true\" normals=\"true\" texture_coords=\"1\">"
    for (pos, tex, nrm) in verts:
        #print nrm
        print "          <vertex>"
        print "            <position x=\"%.5f\" y=\"%.5f\" z=\"%.5f\" />" % pos
        print ("            <normal x=\"%.5f\" y=\"%.5f\" z=\"%.5f\" />" % nrm)
        print "            <texcoord u=\"%.5f\" v=\"%.5f\" />" % tex
        print "          </vertex>"
    print "        </vertexbuffer>"
    print "      </geometry>"
    print "    </submesh>"

print "  </submeshes>"
print "</mesh>"
