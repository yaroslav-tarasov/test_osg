import fbx
import sys
import os
from xml.dom import minidom
from xml.dom.minidom import getDOMImplementation
try:
    from sets import Set
except ImportError:
    Set = set
    
osg_dir = os.getenv('OSG_DIR','NOT_SET')
filepath = osg_dir + r'\OpenSceneGraph-3.2.1\build\bin\ka_50_lp\ka_50.fbx'

manager = fbx.FbxManager.Create()
importer = fbx.FbxImporter.Create( manager, 'myImporter' )
status = importer.Initialize( filepath )

if status == False :
    print ('FbxImporter initialization failed.')
    #print ('Error: %s' % importer.GetLastErrorString())
    sys.exit()

scene = fbx.FbxScene.Create( manager, 'myScene' )

importer.Import( scene )
importer.Destroy()

textureArray = fbx.FbxTextureArray()
scene.FillTextureArray( textureArray )

textures = {}

for i in range( 0, textureArray.GetCount() ):
    texture = textureArray.GetAt( i )
    if texture.ClassId == fbx.FbxFileTexture.ClassId:
        textureFilename = texture.GetFileName()
        textures[ textureFilename ] = (1024, 1024)
        # print 'Texture path - %s\n' % ( os.path.basename(textureFilename) )

#----------------------- PART 2 -----------------------# 

def createMaterialTag( dom, top_element, mat_name ):
    mat = dom.createElement("Material")
    top_element.appendChild(mat)
    mat.setAttribute("name", mat_name)
    return mat;

def createTextureTag( dom, top_element, unit, path, wrap_s, wrap_t ):
    tex = dom.createElement("texture")
    top_element.appendChild(tex)
    tex.setAttribute("unit", unit)
    tex.setAttribute("path", path)
    tex.setAttribute("wrap_s", wrap_s)
    tex.setAttribute("wrap_t", wrap_t)
    

def findTexturesOnNodes( node, textureDictionary, currentPath = [] ):

    currentPath.append( node.GetName() )
    #print 'Path: %s' % currentPath
    for materialIndex in range( 0, node.GetMaterialCount() ):
        material = node.GetMaterial( materialIndex )
        #print '\tMaterial: %s' % material.GetName()
        if not material.GetName() in mats_map.keys() :
           mat_tag = createMaterialTag (doc, root, material.GetName() );
           mats_map[ material.GetName() ] = mat_tag
           for propertyIndex in range( 0, fbx.FbxLayerElement.sTypeTextureCount() ):
                property = material.FindProperty( fbx.FbxLayerElement.sTextureChannelNames( propertyIndex ) )
                #if property.GetName() != '':
                    #print '\t\tProperty: %s' % property.GetName()
                for textureIndex in range( 0, property.GetSrcObjectCount( fbx.FbxFileTexture.ClassId ) ):
                    texture = property.GetSrcObject( fbx.FbxFileTexture.ClassId, textureIndex )
                    #print '\t\t\tTexture: %s' % texture.GetFileName()
                    wrap_mode = "repeat", "clamp"
                    textureFilename = texture.GetFileName()
                    tex_type = ["DiffuseColor", "NormalMap", "AmbientColor", "Incandscene","TransparentColor","SpecularColor","ShininessExponent"]
                    createTextureTag(doc, mat_tag, "%s" % tex_type.index(property.GetName())  , os.path.basename(textureFilename), wrap_mode[texture.GetWrapModeU()], wrap_mode[texture.GetWrapModeV()] )
    
    #print ''
    
    for i in range( 0, node.GetChildCount() ):
        findTexturesOnNodes( node.GetChild( i ), textureDictionary, currentPath )
    
    currentPath.pop()

mats_map = {}
impl   = getDOMImplementation()
doc = impl.createDocument(None, "root", None)
root   = doc.documentElement

findTexturesOnNodes( scene.GetRootNode(), textures )

print (doc.toprettyxml())

with open(os.path.dirname(filepath) + "\\" + os.path.splitext(os.path.basename(filepath))[0] + ".dae.mat.xml", "w") as f:
    f.write(doc.toprettyxml(indent="  "))



