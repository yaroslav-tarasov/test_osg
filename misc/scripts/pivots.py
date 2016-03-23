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
cmds.xform('wheel_f_lod0',  cp=True)

#####################################
#  For strut pivot point onto top
#
import maya.cmds as cmds

bbx = cmds.xform('strut_f_lod0', q=True, bb=True, ws=True) # world space
centerX = (bbx[0] + bbx[3]) / 2.0
centerY = (bbx[1] + bbx[4]) / 2.0
centerZ = (bbx[2] + bbx[5]) / 2.0

cmds.xform('shassi_f_lod0',  piv=[centerX,centerY,bbx[5]])

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
#

dict = { 'port' : [-90,0,0], 'starboard' : [-90,0,0], 'tail' : [-90,0,0], 'steering_lamp' : [-38.761,0,0], 'landing_lamp' : [-38.028,-15.000,0],
         'landing_lamp1' : [-24.866,15.000,0], 'back_tail' : [152.384,0,0], 'strobe_r' : [185.999,0,34.419], 'strobe_l' : [185.999,0,-34.419]  }

print( [x + ''.join(str(y)) for x, y in dict.iteritems()] )