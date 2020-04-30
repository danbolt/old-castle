import bpy


def write_some_data(context, filepath):
    f = open(filepath, 'w', encoding='utf-8')
    
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            verts = obj.data.vertices
            colors = obj.data.vertex_colors[0].data
            polygons = obj.data.polygons
            
            f.write(obj.name)
            f.write("\n")
            
            vertData = list(map(lambda v: { "x": int(v.co[0] * 100), "y": int(v.co[1] * 100), "z": int(v.co[2] * 100), "flag": 0, "u": 0, "v": 0, "r": -1, "g": -1, "b": -1, "a": -1 }, verts))
            
            triangleData = []
            for poly in polygons:
                triangleData.append({ "a": poly.vertices[0], "b": poly.vertices[1], "c": poly.vertices[2] })
                for v_ix, l_ix in zip(poly.vertices, poly.loop_indices):
                    vertData[v_ix]["r"] = colors[l_ix].color[0]
                    vertData[v_ix]["g"] = colors[l_ix].color[1]
                    vertData[v_ix]["b"] = colors[l_ix].color[2]
                    vertData[v_ix]["a"] = colors[l_ix].color[3]
            
            f.write("{count} verts\n".format(count = len(vertData)))
            for d in vertData:
                f.write("{{ {x}, {y}, {z}, {flag}, {u}, {v}, {r}, {g}, {b}, {a} }},\n".format( x = d["x"], y = d["y"], z = d["z"], flag = d["flag"], u = d["u"], v = d["v"], r = int(255 * d["r"]), g = int(255 * d["g"]), b = int(255 * d["b"]), a = int(255 * d["a"])))
            f.write("\n")
            
            f.write("{count} triangles\n".format(count = len(triangleData)))
            i = 0
            while i < len(triangleData):
                if i == (len(triangleData) - 1):
                    continue
                
                t1 = triangleData[i]
                t2 = triangleData[i + 1]
                f.write("gSP2Triangles(glistp++, {a1}, {b1}, {c1}, 0, {a2}, {b2}, {c2}, 0);\n".format( a1 = t1["a"], b1 = t1["b"], c1 = t1["c"], a2 = t2["a"], b2 = t2["b"], c2 = t2["c"]  ))
                i += 2
            
            f.write("\n\n")  
            
            
    f.close()

    return {'FINISHED'}


# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class ExportSomeData(Operator, ExportHelper):
    """Exports a mesh into some N64 Display list information."""
    bl_idname = "export_test.some_data"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Export N"

    # ExportHelper mixin class uses this
    filename_ext = ".dl64"

    filter_glob: StringProperty(
        default="*.h",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    type: EnumProperty(
        name="Example Enum",
        description="Choose between two items",
        items=(
            ('OPT_A', "First Option", "Description one"),
            ('OPT_B', "Second Option", "Description two"),
        ),
        default='OPT_A',
    )

    def execute(self, context):
        return write_some_data(context, self.filepath)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportSomeData.bl_idname, text="N64 Vertex Color Display List")


def register():
    bpy.utils.register_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # test call
    bpy.ops.export_test.some_data('INVOKE_DEFAULT')
