import maya.cmds as cmds

def makebb( parent ) :
    children = cmds.listRelatives(parent) 
    if children == None :
        return
    print parent
    for rn in children :
        x1, y1, z1, x2, y2, z2 = cmds.exactWorldBoundingBox(parent + '|'+ rn, calculateExactly=True)
        cube = cmds.polyCube()[0]
        cmds.move(x1, '%s.f[5]' % cube, x=True)
        cmds.move(y1, '%s.f[3]' % cube, y=True)
        cmds.move(z1, '%s.f[2]' % cube, z=True)
        cmds.move(x2, '%s.f[4]' % cube, x=True)
        cmds.move(y2, '%s.f[1]' % cube, y=True)
        cmds.move(z2, '%s.f[0]' % cube, z=True)
        makebb(rn)

nodes = cmds.ls( tr=True )
lod3_node =   [ x for x in nodes if x.lower() == 'body_lod3' ]

makebb(lod3_node[0])