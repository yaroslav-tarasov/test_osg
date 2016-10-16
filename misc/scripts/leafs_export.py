import maya.cmds as cmds
import operator
import xml.etree.cElementTree as ET
import maya.OpenMaya as om 
import os

g_vec_axis = [0.0,1.0,0.0]

def  MVectorFromList(l):
     da = om.MScriptUtil()
     da.createFromList(l,3)
     return da.asDoublePtr()

def  getRotate(v):
     chips=om.MQuaternion() 
     chips=om.MVector(g_vec_axis[0],g_vec_axis[1],g_vec_axis[2]).rotateTo(om.MVector(v[0],v[1],v[2])) 
     rot = chips.asEulerRotation()
     return [rot.x, rot.y ,rot.z]

ax = cmds.upAxis( q=True, axis=True )
if ax == 'z':
   g_vec_axis = [0.0,0.0,1.0] 

w_path = cmds.workspace( q=True, dir=True )

print "---------------------------------------------------"
print "Current directory: " + os.getcwd()
print "Current workspace directory: " + w_path
print "Current axis_up: " + ax

print "---------------------------------------------------"

nodes = cmds.ls( tr=True )
th_node = [ x for x in nodes if 'leaf_TRA0_3_LP' in x or 'leaft2_TRA0_3_LP' in x or 'leaft3_TRA0_3_LP' in x]
th_children = cmds.listRelatives(th_node) 
    
thc_nodes = [ x for x in th_children if 'polySurface' in x ]
[cmds.xform(x, cp=True) for x in thc_nodes]
normals =  [[ float(y) for y in cmds.polyInfo(x + '.f[0]', fn=True )[0].split()[2:5]] for x in thc_nodes]
print [getRotate(v) for v in normals]

bboxes = [ [x, cmds.xform(x, q=True, bb=True, ws=True), getRotate([ float(y) for y in cmds.polyInfo(x + '.f[0]', fn=True )[0].split()[2:5]]) ] for x in thc_nodes ]
bbx =  [ [ round((bb[1][0] + bb[1][3])/2,1), round((bb[1][1] + bb[1][4])/2,1), round((bb[1][2] + bb[1][5])/2,1), round((-bb[1][2] + bb[1][5]), 1 if int((bb[1][2] - bb[1][5]))==0 else 0 )] for bb in bboxes ]
bbx.sort()


#for pbb in bboxes :
#     bb = pbb[1]
#     print  pbb[0], bb, " %0.2f, %0.2f ,%0.2f " % ( (bb[0] + bb[3])/2, (bb[1] + bb[4])/2,   (bb[2] + bb[5])/2) 

   
root = ET.Element("root")

for el in out_list :
    ET.SubElement(root, "value", x='%.5f' % el[0], y='%.5f' %el[1], z='%.5f' %el[2], h='%.5f' %el[3])

tree = ET.ElementTree(root)
tree.write(w_path + "data_leafs.xml")