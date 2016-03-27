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

######################################
###   Central pivot
###

import maya.cmds as cmds

nodes = cmds.ls( tr=True )
nodes = [ x for x in nodes if 'wheel_' in x]
cmds.xform(nodes,  cp=True)

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

def createLights( positions={}, orients = {} ):
    for key, val in positions.iteritems() :
        nn = cmds.createNode( 'transform', n=key, p='root' )
        cmds.setAttr( nn + '.translate'     , positions[key][0], positions[key][1], positions[key][2], type="double3")
        cmds.setAttr( nn + '.rotate'        , orients[key][0], orients[key][1], orients[key][2], type="double3")
        cmds.setAttr( nn + '.displayLocalAxis', 1)	


orients = { 'port' : [-90,0,0], 'starboard' : [-90,0,0], 'tail' : [-90,0,0], 'steering_lamp' : [-38.761,0,0], 'landing_lamp' : [-38.028,-15.000,0],
         'landing_lamp1' : [-24.866,15.000,0], 'back_tail' : [152.384,0,0], 'strobe_r' : [185.999,0,34.419], 'strobe_l' : [185.999,0,-34.419]  }

print( [x + ''.join(str(y)) for x, y in orients.iteritems()] )

bbx = cmds.xform('root', q=True, bb=True, ws=True) # world space
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

createLights(positions,orients)

