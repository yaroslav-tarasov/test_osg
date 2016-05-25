######################################
##  Get pivot of wheel
##  Create new transform in the center of the wheel

import maya.cmds as cmds
bbx = cmds.xform('wheel_f_lod0', q=True, bb=True, ws=True) # world space
centerX = (bbx[0] + bbx[3]) / 2.0
centerY = (bbx[1] + bbx[4]) / 2.0
centerZ = (bbx[2] + bbx[5]) / 2.0

pivot = cmds.xform('wheel_f_lod0', q=True, rp=True, ws=True)
print pivot

cmds.createNode( 'transform', n='transform1' )
cmds.setAttr( 'transform1' + '.translate', centerX , centerY, centerZ, type="double3")

###########################################
###   Central pivot for wheels and rotors
###

import maya.cmds as cmds

nodes = cmds.ls( tr=True )
nodes = [ x for x in nodes if 'wheel_' in x or 'rotor' in x]
cmds.xform(nodes,  cp=True)

nodes = cmds.ls( tr=True )
rotors_nodes = [ x for x in nodes if ('rotor_' in x or 'rotorl' in x or 'rotorr' in x or 'rotorf' in x) and not ('|' in x)  ]

for rn in rotors_nodes :
    bbx = cmds.xform(rn, q=True, bb=True, ws=True) # world space
    piv = [(bbx[0] + bbx[3]) / 2.0, (bbx[1] + bbx[4]) / 2.0, (bbx[2] + bbx[5]) / 2.0]
    cmds.setAttr( rn + '.translate'     , piv[0], piv[1], piv[2], type="double3")
    
    rn_children = cmds.listRelatives(rn) 
    for rn_child in rn_children :
            cmds.setAttr( rn + '|' + rn_child + '.translate', -piv[0], -piv[1], -piv[2], type="double3")

#####################################
#  For strut pivot point onto top
#
import maya.cmds as cmds
nodes = cmds.ls( tr=True )
shassi_nodes = [ x for x in nodes if 'shassi_' in x]

for sn in shassi_nodes :
    shassi_children = cmds.listRelatives(sn) 
    strut_nodes = [ x for x in shassi_children if 'strut_' in x]
    for str_node in strut_nodes :
        bbx = cmds.xform(str_node, q=True, bb=True, ws=True) # world space
        centerX = (bbx[0] + bbx[3]) / 2.0
        centerY = (bbx[1] + bbx[4]) / 2.0
        centerZ = (bbx[2] + bbx[5]) / 2.0
        cmds.xform(sn,  piv=[centerX,centerY,bbx[5]])

#####################################
# Working only for "clear" copy of LOD0 
#
import maya.cmds as cmds
def renameNodes( parent ) :
    lod3_children = cmds.listRelatives(parent) 
    nodes_to_rename = [ x for x in lod3_children if 'lod0' in x.lower()]
    for rn in nodes_to_rename :
        renameNodes( cmds.rename( parent + '|'+ rn, rn.replace('0','3')))

nodes = cmds.ls( tr=True )

lod3_node =   [ x for x in nodes if x.lower() == 'lod3' ]
lod3_children = cmds.listRelatives(lod3_node) 
nodes_to_rename = [ x for x in lod3_children if 'lod0' in x.lower()]
for rn in nodes_to_rename :
    renameNodes(cmds.rename( lod3_node[0] + '|'+ rn, rn.replace('0','3')))



####################################
## List all Relatives for node 

import maya.cmds as cmds
nodes = cmds.ls( tr=True )
print nodes    
nodes = [ x for x in nodes if 'animgroup' in x]
print nodes
for anim_group in nodes :
    print cmds.listRelatives(anim_group) 

###################################
#  Rename child of LOD3
#

###################################
#  Add lights to airplane
#  Model must be normalized Z-up, Y-forward  
#  Port and starboard 
#  Port is the left-hand side of or direction from a craft,
#  facing forward. Starboard is the right-hand side, facing forward.

import maya.cmds as cmds

def createLights( root_node, positions={}, orients = {} ):
    for key, val in positions.iteritems() :
        nn = cmds.createNode( 'transform', n=key, p=root_node )
        cmds.setAttr( nn + '.translate'     , positions[key][0], positions[key][1], positions[key][2], type="double3")
        cmds.setAttr( nn + '.rotate'        , orients[key][0], orients[key][1], orients[key][2], type="double3")
        cmds.setAttr( nn + '.displayLocalAxis', 1)	


orients = { 'port' : [-90,0,0], 'starboard' : [-90,0,0], 'tail' : [-90,0,0], 'steering_lamp' : [-38.761,0,0], 'landing_lamp' : [-38.028,-15.000,0],
         'landing_lamp1' : [-24.866,15.000,0], 'back_tail' : [152.384,0,0], 'strobe_r' : [185.999,0,34.419], 'strobe_l' : [185.999,0,-34.419]  }

print( [x + ''.join(str(y)) for x, y in orients.iteritems()] )

nodes = cmds.ls( tr=True )
root_node =   [ x for x in nodes if x.lower() == 'root' ][0]

bbx = cmds.xform(root_node, q=True, bb=True, ws=True) # world space
plane_centerX = (bbx[0] + bbx[3]) / 2.0
plane_centerY = (bbx[1] + bbx[4]) / 2.0
plane_centerZ = (bbx[2] + bbx[5]) / 2.0

ags_bbx = cmds.xform('shassi_f_lod0', q=True, bb=True, ws=True) # world space
ags_f_centerX = (ags_bbx[0] + ags_bbx[3]) / 2.0
ags_f_centerY = (ags_bbx[1] + ags_bbx[4]) / 2.0
ags_f_centerZ = (ags_bbx[2] + ags_bbx[5]) / 2.0


positions = { 'port' : [bbx[0] , plane_centerY, plane_centerZ], 'starboard' : [bbx[3] , plane_centerY, plane_centerZ],
              'tail' : [plane_centerX , bbx[1], bbx[5]], 'steering_lamp' : [ags_f_centerX,ags_bbx[4],ags_f_centerZ + ags_f_centerZ/4], 'landing_lamp' : [bbx[3]/6.0 , plane_centerY, plane_centerZ],
              'landing_lamp1' : [bbx[0]/6.0 , plane_centerY, plane_centerZ], 'back_tail' : [plane_centerX , bbx[1], bbx[5]], 'strobe_r' : [bbx[3] , plane_centerY, plane_centerZ], 'strobe_l' : [bbx[0] , plane_centerY, plane_centerZ]  }

createLights(root_node, positions,orients)

#####################################
#  For body geomety setup 
#

import maya.cmds as cmds
nodes = cmds.ls( tr=True )
body_nodes = [ x for x in nodes if 'Body_' in x  or 'body_' in x]

for bn in body_nodes :
    bn_trans = cmds.getAttr(bn + '.translate')
    body_children = cmds.listRelatives(bn) 
    for bc in body_children :
        cmds.setAttr( bn + '|' + bc + '.translate', -bn_trans[0][0],-bn_trans[0][1],-bn_trans[0][2], type="double3")