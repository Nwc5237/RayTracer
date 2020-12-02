#################################################
#                                               #
# A little .obj file type converter I wrote     #
# so I could render models I've made and        #
# models I've ripped from games. Converts       #
# to the .ray file type the ray tracer          #
# can parse.                                    #
#                                               #
#################################################


"""
use: set the .obj file path in the obj_path variable
and set the path for the texture in the tex_path
variable below. Make sure to use the texture
that was already mapped to the model so that the
uv mapping looks appropriate on the model.

Important note: the models and texures are flipped
vertically. In the generated ray file I flip the
up vector to fix this, but the texure you load
needs a horizontal reflection. Be careful not to
just rotate by 180, as this produces incorrect
results 
"""

#obj_path = "C:/Users/ncala/OneDrive/Documents/Cmpsc458/PA3_Starter/Project_3/Media/ObjModels/big_daddy_mirror2.obj"
#tex_path = "NULL"#"C:/Users/ncala/OneDrive/Documents/Cmpsc458/PA3_Starter/Project_3/Media/ObjModels/BOUNCER_DIFF_reflected.tga" #"NULL"

obj_path = "C:/Users/ncala/OneDrive/Documents/Cmpsc458/PA3_Starter/Project_3/Media/ObjModels/toasty.obj"
tex_path = "C:/Users/ncala/OneDrive/Documents/Cmpsc458/PA3_Starter/Project_3/Media/ObjModels/Toasty_DIFF_reflected.tga"

#obj_path = "C:/Users/ncala/OneDrive/Documents/Cmpsc458/PA3_Starter/Project_3/Media/ObjModels/BeerCan.obj"
#tex_path = "C:/Users/ncala/OneDrive/Documents/Cmpsc458/PA3_Starter/Project_3/Media/ObjModels/budlight_label_image_reflected.jpg"

outfile_path = "toasty.ray"

class Face:

    #passes in the INDICES to the coordinate arrays not the actual coords
    def __init__(self, a, b, c, texA, texB, texC):
        self.a = a
        self.b = b
        self.c = c
        self.texA = texA
        self.texB = texB
        self.texC = texC

    def print(self):
        print("vert indices: {} {} {}, tex indices: {} {} {}".format(self.a, self.b, self.c, self.texA, self.texB, self.texC))

class Vertex:
    def __init__(self, x, y, z=None):
        self.x = x
        self.y = y
        self.z = z

    def __str__(self):
        if x != None:
            return "vertex coords: {} {} {}".format(self.x, self.y, self.z)
        else:
            return "texture coords: {} {}".format(self.x, self.y)

#strip the vertices from a line
def get_vertex(line):
    vertex = []
    line = line[2:]

    if line[0] == '-':
        vertex.append(float(line[:9]))
        line = line[10:]
    else:
        vertex.append(float(line[:8]))
        line = line[9:]

    #print("(%.6f, " % vertex[0], end='')

    if line[0] == '-':
        vertex.append(float(line[:9]))
        line = line[10:]
    else:
        vertex.append(float(line[:8]))
        line = line[9:]

    #print("%.6f, " % vertex[1], end='')

    if line[0] == '-':
        vertex.append(float(line[:9]))
        #line = line[9:]
    else:
        vertex.append(float(line[:8]))
        #line = line[8:]

    #print("%.6f)" % vertex[2])
    return vertex


def get_int(line):
    digits = "0123456789"
    i = 0
    ans = ""
    while i < len(line) and line[i] in ' /':
        i += 1

    while i < len(line) and line[i] in digits:
        ans += line[i]
        i += 1
        
    return [int(ans), i]




#########################################
#                                       #
# Change this function to parse out     #
# the info for uvs and normals since    #
# we're already passing over them       #
#########################################


#returns [[vertex coords for triangle], [texture coords]]
def get_face_data(line):
    verts = []
    tex_coords = []
    
    #noj files index from 1 so we subtract 1 to get correct index into
    #vertex array, and same for uv index. We only use the first two
    #numbers (looks like "xxx/yyy/zzz") sincee the third is normals
    #which are not being used right now
    v = get_int(line)
    verts.append(v[0] - 1)
    line = line[v[1]:]
    
    #two next nums are irrelevant right now
    t = get_int(line)
    tex_coords.append(t[0] - 1)
    line = line[t[1]:]

    #skip next num
    line = line[get_int(line)[1]:]

    #do the same thing two more times
    v = get_int(line)
    verts.append(v[0] - 1)
    line = line[v[1]:]
    t = get_int(line)
    tex_coords.append(t[0] - 1)
    line = line[t[1]:]
    line = line[get_int(line)[1]:]

    v = get_int(line)
    verts.append(v[0] - 1)
    line = line[v[1]:]
    t = get_int(line)
    tex_coords.append(t[0] - 1)

    return [verts, tex_coords]
    
 
def get_tex_coords(line):
    uv = []
    line = line[2:]

    uv.append(float(line[:8]))
    line = line[9:]

    uv.append(float(line[:8]))
    line = line[9:]

    return(uv)

    
    



f = open(obj_path, 'r')
lines = f.readlines()
vertices = []
faces = []
tex_indices = []
tex_coords = []

print("reading obj file")
for line in lines:
    if line[:2] == 'v ':
        vert = get_vertex(line)
        vertices.append(Vertex(vert[0], vert[1], vert[2]))
    if line[:2] == 'f ':
        face_data = get_face_data(line[2:])
        faces.append(Face(face_data[0][0], face_data[0][1], face_data[0][2],
                     face_data[1][0], face_data[1][1], face_data[1][2])) #these are all indices
    if line[:2] == 'vt':
        tex = get_tex_coords(line[2:])
        tex_coords.append(Vertex(tex[0], tex[1]))
print("done reading obj file")


ray_file = """# author: Nick

Background {{
	# Color of the background 
	color 0.15 0.15 0.25 # blue-gray
	# low intensity white light
	ambientLight 0.3 0.3 0.3 
}}

Camera {{
	eye 4 0.000001 0.000001
	lookAt 0 0 0
	up 0 0 1
	fovy 45
}}

Lights {{
	Light {{
		position 10 0 12
		color 1 1 1
	}}
}}

Materials {{
	Material {{
		textureFilename {}
		diffuseColor 1 1 1
		specularColor 0.3 0.3 0.3
		reflectiveColor 0.3 0.3 0.3
		shininess 50
		transparentColor 0 0 0 
		indexOfRefraction 0.6
	}}
}}

Group {{

""".format(tex_path)

for face in faces:
    ray_file += """	Triangle {{
		vertex0 {:.6f} {:.6f} {:.6f}
		vertex1 {:.6f} {:.6f} {:.6f}
		vertex2 {:.6f} {:.6f} {:.6f}
		tex_xy_0 {} {}
		tex_xy_1 {} {}
		tex_xy_2 {} {}
		materialIndex 0
	}}
""".format(vertices[face.a].x, vertices[face.a].y, vertices[face.a].z,
           vertices[face.b].x, vertices[face.b].y, vertices[face.b].z,
           vertices[face.c].x, vertices[face.c].y, vertices[face.c].z,
           tex_coords[face.texA].x, tex_coords[face.texA].y,
           tex_coords[face.texB].x, tex_coords[face.texB].y,
           tex_coords[face.texC].x, tex_coords[face.texC].y)

ray_file += "\n}"

f = open(outfile_path, 'w')

print("Writing file...")
f.write(ray_file)
f.close()
print("File written.")




















