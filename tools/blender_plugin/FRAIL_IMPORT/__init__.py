bl_info = {
    "name": "Import FRAiL map file",
    'author': 'Miłosz Mazur',
    "category": "Import-Export",
    'location': "File > Import",
}

import bpy
import json
import os
import re
import collections
from bpy.props import StringProperty
from bpy_extras.io_utils import ImportHelper, ExportHelper

#globals listing all types of game objects. Not a wise idea.
exp_obj = ['barrel_e_r4', 'barrel_e_r2', 'barrel_e_r8', 'barrel_s_r4', 'barrel_s_r2',
         'barrel_s_r8', 'barrel_b_r4', 'barrel_b_r2', 'barrel_b_r8']
cyl_trig = ['lava_lake_r4','power_lake_r8','power_lake_r2', 'invisiblity_bush_r2', 'invisiblity_bush_r1',
            'healing_bandage', 'lava_lake_1', 'teleport', 'double_damage', 'land_mine_r2', 'knockback_trap_r2',
            'snares_of_hatred_r2_b', 'snares_of_hatred_r2_a', 'snares_of_hatred_r2','claymore_trap_r2', 'key'] 
mod_obj = ['snares_r2', 'teleport_1', 'pushtrap_r1', 'foottrap_r1']


game_objects = [] 
cvt = [] # cylinder volume triggers
eo = [] # exploding objects
mo = [] # model objects



class Preset(object):
    def __init__(self, params):
        self.params = params


    def __repr__(self):
        return str(self.params)


    def __str__(self):
        return str(self.params)

class Game_Object(object):
    def __init__(self, properties,name = ''):
        self.properties = collections.OrderedDict(properties.items())
        if 'Preset' in self.properties.keys():
            self.properties.move_to_end('Preset',last=False)
        if 'm_meshName' in self.properties.keys():
            self.name = "StaticCollidableMesh"
        elif 'm_worldSpawnPos' in self.properties.keys():
            self.name = 'PlayerSpawner'
        elif 'm_ambientLight' in self.properties.keys():
            self.name = 'RenderSettingsSetter'
        elif 'm_spawnOrigin' in self.properties.keys():
            self.name = 'AISpawner'
        elif 'm_worldPos' in self.properties.keys():
            self.name = 'DynamicLight'
        elif 'm_worldTransform.position' in self.properties.keys():
            if self.properties['Preset'] in exp_obj:
                self.name = 'ExplodingObject'
            elif self.properties['Preset'] in cyl_trig:
                self.name = 'CylinderVolumeTrigger'
            elif self.properties['Preset'] in mod_obj:
                self.name = 'ModelObject'
        elif 'm_radius' in self.properties.keys():    
            self.name = 'GameObject'
        elif 'ScriptName' in self.properties.keys():    
            self.name = 'GameObject'
        elif 'CylinderVolumeTrigger' in self.properties.keys():
            self.name = 'descriptor' #what the hell is that thing?
        else:
            raise AttributeError('Unexpected data loaded.' + str(self.properties.keys()))
        print(str(self.name))

    def __repr__(self):
        return 'Name: ' + self.name + str(self.properties)

    def __str__(self):
        return 'Name: ' + self.name + str(self.properties)


def cvt_hook(parsed_dict):
    cvt.append(Preset(parsed_dict))


def eo_hook(parsed_dict):
    eo.append(Preset(parsed_dict))


def mo_hook(parsed_dict):
    mo.append(Preset(parsed_dict))


def go_hook(parsed_dict):
    game_objects.append(Game_Object(properties=collections.OrderedDict(parsed_dict)))


def read_map_file(mapfile):
    json.load(mapfile,object_hook=go_hook)
    game_objects.pop() #sloppy handling of the last read object (a matter of parsing .ksons)



def clear_scene():
    # wtf is going on with this shit.
    del_list = [item.name for item in bpy.data.objects if item.type == "MESH" or item.type == "CAMERA"]
    for object_name in del_list:
        bpy.data.objects[object_name].select = True
    bpy.ops.object.delete()
# remove the meshes
    for item in bpy.data.meshes:
        bpy.data.meshes.remove(item)



def load_presets():
    print('current: '+os.getcwd())
    os.chdir('../mod_presets/')
    # yup, loading up all preset files.
    json.load(open('CylinderVolumeTrigger.json'), object_hook=cvt_hook)
    cvt.pop()
    json.load(open('ExplodingObject.json'), object_hook=eo_hook)
    eo.pop()
    json.load(open('ModelObject.json'), object_hook=mo_hook)
    mo.pop()


def load_jsons(filepath):
    mapfile = open(filepath)
    if mapfile:
        (s_name, ext) = (os.path.basename(filepath), mapfile.name[-5:])
        (path,filename) = os.path.split(filepath)
        print("filepath: ", filepath)
        print("mapfile: ", s_name)
        if ext != ".json":
            print("ExtensionError: Not a .json file: " + mapfile)
        else:
            print("Scene cleared successfully")
            read_map_file(mapfile)
            print('reading .json map file succeeded')
            os.chdir(path)
            load_presets()  
            mapfile.close()
    else:
        print("IOError: Error Loading file: " + filepath)


def build_graphics():
    print('Building graphics...')
    os.chdir('../mod_levels/game_objects')
    for ob in game_objects:
        print('name: ' + str(ob.name))
        if ob.name == 'StaticCollidableMesh':
            fname = ob.properties['m_meshName'][:-5]
            bpy.ops.import_scene.obj(filepath=(fname + '.obj'),use_smooth_groups=False)
            curr_obj = bpy.context.selected_objects[0]
            curr_obj.lock_location = [True,True,True]
        elif ob.name == 'CylinderVolumeTrigger':
            fname = ob.properties['Preset']
            bpy.ops.import_scene.obj(filepath=(fname + '.obj'),use_smooth_groups=False)
            curr_obj = bpy.context.selected_objects[0]
            position = ob.properties['m_worldTransform.position']
            #hurr durr, why bother using built-in parameters to do the conversion handling, amirite?
            curr_obj.location = (position[0], position[2] * -1 ,position[1])
            curr_obj.name = ob.properties['Name']
        elif ob.name == 'ExplodingObject':
            fname = ob.properties['Preset']
            bpy.ops.import_scene.obj(filepath=(fname + '.obj'),use_smooth_groups=False)
            curr_obj = bpy.context.selected_objects[0]
            position = ob.properties['m_worldTransform.position']
            #hurr durr, why bother using built-in parameters to do the conversion handling, amirite? Substract 0.625, because these are barrels
            curr_obj.location = (position[0], (position[2] * -1), position[1] - 0.625) # logic.
            curr_obj.name = ob.properties['Name']
            pass
        elif ob.name == 'ModelObject':
            pass
        elif ob.name =='PlayerSpawner':
            fname = "PlayerSpawner"
            bpy.ops.import_scene.obj(filepath=('spawner.obj'),use_smooth_groups=False)
            curr_obj = bpy.context.selected_objects[0]
            position = ob.properties['m_worldSpawnPos']
            curr_obj.location = (position[0], (position[2] * -1), position[1])
            curr_obj.name = fname
        elif ob.name =='AISpawner':
            fname = "AISpawner"
            bpy.ops.import_scene.obj(filepath=('spawner_ai.obj'),use_smooth_groups=False)
            curr_obj = bpy.context.selected_objects[0]
            position = ob.properties['m_spawnOrigin']
            curr_obj.location = (position[0], (position[2] * -1), position[1])
            curr_obj.name = fname
        elif ob.name == 'DynamicLight':
            lamp = bpy.data.objects["Lamp"]
            lamp.location = ob.properties['m_worldPos']
        else:
            pass
        print("Importing game objects finished!")


def export_json(filepath, objects):
    jout = open(filepath,'w')
    jout.write('{')
    for ob in objects:
        # dirty
        if ob.name == 'DynamicLight':
            l = bpy.data.objects["Lamp"].location
            ob.properties['m_worldPos'] = [l.x, l.y, l.z]
        elif ob.name == 'PlayerSpawner':
            l = [x for x in bpy.context.scene.objects if x.name == ob.name][0].location
            ob.properties['m_worldSpawnPos'] = [l.x, l.z, (-1) * l.y] # odwrotne wspolzedne
        elif ob.name == 'AISpawner':
            l = [x for x in bpy.context.scene.objects if x.name == ob.name][0].location
            ob.properties['m_spawnOrigin'] = [l.x, l.z, (-1) * l.y] # odwrotne wspolzedne
        elif ob.name == 'CylinderVolumeTrigger':
            l = [x for x in bpy.context.scene.objects if x.name == ob.properties['Name']][0].location
            ob.properties['m_worldTransform.position'] = [l.x, l.z, (-1) * l.y] # odwrotne wspolzedne
        elif ob.name == 'ExplodingObject':
            l = [x for x in bpy.context.scene.objects if x.name == ob.properties['Name']][0].location
            print(l.x, l.z, l.y)
            ob.properties['m_worldTransform.position'] = [l.x, l.z, l.y] # odwrotne wspolzedne
        d = { ob.name : ob.properties}
        rawr = json.dumps(d)
        rawr = rawr[1:-1]
        #yup, that's all what it takes.
        rawr = re.sub('\[','{',rawr)
        rawr = re.sub('\]','}',rawr)
        print(rawr)
        jout.write(rawr + "\n")
    jout.write('}')
    jout.close()
    print('Exporting map file succeeded!')


class ImportMapFile(bpy.types.Operator, ImportHelper):
    
    """
    Import and enable for editing a map frm FRAIL-supported .json file
    """
    bl_idname = "frail.importer"       
    bl_label = "Import FRAIL .json map file"        
    bl_options = {'REGISTER',  'UNDO'} 

    filepath = StringProperty(subtype='FILE_PATH', default="")


    def execute(self, context): 
        clear_scene() 
        load_jsons(self.filepath)
        build_graphics()
        return {'FINISHED'}


    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}   


class ExportMapFile(bpy.types.Operator, ExportHelper):
    """
    Export a modified version of FRAIL .json map file
    """
    bl_idname = "frail.exporter"       
    bl_label = "Export FRAIL .json map file"        
    bl_options = {'REGISTER',  'UNDO'} 

    filename_ext = ".json"
    filepath = StringProperty(subtype='FILE_PATH')
    

    def execute(self, context):
        global game_objects
        obj_without_names = ['DynamicLight', 'PlayerSpawner','StaticCollidableMesh', 'GameObject','RenderSettingsSetter','AISpawner',]

        print('Exporting map!')
        scene_objects = bpy.context.scene.objects
        print('game_objects in scene: ' + str(len(scene_objects)))
        #loaded game objects still present on scene.
        present_objects = [ob for ob in game_objects if ob.name in obj_without_names or ob.properties['Name'] in scene_objects]
        print('original game_objects in scene: ' + str(len(present_objects)))
        print('Exporting to new .json file ' + self.filepath)
        export_json(self.filepath, present_objects)
        scene_objects = []
        present_objects = []
        return {'FINISHED'}


    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


def menu_import(self, context):  
        self.layout.operator(  
        ImportMapFile.bl_idname,  
        text=("FRAIL map file (.json)"))


def menu_export(self, context):
        self.layout.operator(  
        ExportMapFile.bl_idname,  
        text=("FRAIL map file (.json)")) 


def register():
    print('FRAIL: plugin registered')
    bpy.utils.register_class(ImportMapFile)  
    bpy.types.INFO_MT_file_import.append(menu_import);
    bpy.utils.register_class(ExportMapFile)  
    bpy.types.INFO_MT_file_export.append(menu_export);


def unregister():
    print('FRAIL: plugin unregistered')
    bpy.utils.unregister_class(ImportMapFile)
    bpy.types.INFO_MT_file_import.remove(menu_import)
    bpy.utils.unregister_class(ExportMapFile)
    bpy.types.INFO_MT_file_export.remove(menu_export)
    

if __name__ == "__main__":
    register()
