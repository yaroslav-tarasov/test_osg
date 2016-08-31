import maya.cmds as cmds

nodes = cmds.ls( tr=True, sl=True )
wnodes = [ x for x in nodes if 'wheel_' in x ]
cmds.xform(wnodes,  cp=True)

print 1,nodes
print 2,wnodes

shassi_nodes = [ x for x in nodes if 'shassi_' in x and ( (x.find('|') < 0 and x.find('animgroup_shassi') < 0 ) or x.rfind('|') < x.rfind('shassi_') and x.rfind('animgroup_shassi') < x.rfind('|') ) ]

print 3,shassi_nodes

for sn in shassi_nodes :
    sn_piv = []
    shassi_children = cmds.listRelatives(sn, f=True) 
    ws_node = [ x for x in shassi_children if 'wheels_' in x ]
    strut_nodes = [ x for x in shassi_children if 'strut_' in x and ((x.find('|')  < 0 ) or x.rfind('|') < x.rfind('strut_')) ]
    wheel_nodes = []
    if len(ws_node) == 0 : 
        wheel_nodes = [ x for x in shassi_children if 'wheel_' in x and ((x.find('|')  < 0) or x.rfind('|') < x.rfind('wheel_')) ]
    else :
        wheels_children = cmds.listRelatives(ws_node, f=True) 
        wheel_nodes = [ x for x in wheels_children if 'wheel_' in x and ((x.find('|')  < 0) or x.rfind('|') < x.rfind('wheel_')) ]
        shassi_children.remove(ws_node[0])
        shassi_children = shassi_children + wheels_children

    for str_node in strut_nodes :
        bbx = cmds.xform(str_node, q=True, bb=True, ws=True) # world space
        centerX = (bbx[0] + bbx[3]) / 2.0
        centerY = (bbx[1] + bbx[4]) / 2.0
        sn_piv = [centerX,centerY,bbx[5]]
        cmds.setAttr( sn + '.translate', sn_piv[0], sn_piv[1], sn_piv[2], type="double3")
        cmds.xform(sn,  piv=sn_piv, ws=True)
    for sc_node in shassi_children :
        cmds.setAttr( sc_node + '.translate', -sn_piv[0], -sn_piv[1], -sn_piv[2], type="double3")
    
    for w_node in wheel_nodes :
        wheel_children = cmds.listRelatives(w_node, f=True) 
        bbx = cmds.xform( w_node, q=True, bb=True, ws=True) # world space
        piv_w = [(bbx[0] + bbx[3]) / 2.0, (bbx[1] + bbx[4]) / 2.0,  (bbx[2] + bbx[5]) / 2.0]

        cmds.setAttr( w_node + '.translate', 0, -sn_piv[1] + piv_w[1], -sn_piv[2] + piv_w[2], type="double3")

        for wchild_node in wheel_children :
            cmds.setAttr(  wchild_node + '.translate', -sn_piv[0], -piv_w[1], -piv_w[2], type="double3")
        cmds.xform( w_node,  cp=True)