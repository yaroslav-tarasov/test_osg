import maya.cmds as cmds
import ntpath

main_dir = "C:\\Vis\\Eisk_4\\"

xxx_logic = [ ("kaponir", -90), ("eisk", 180), ("tramplin",180) ]

basicFilter = "*.scn"
path = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2)

main_dir = ntpath.dirname(path[0])
inputfile = open(path[0])

lands_counter = 0
light_masts   = 0
sectors = 0
files_loaded = []
idx = 0
land_line = []

for i in range(4): inputfile.next() # skip first four lines
for line in inputfile:
    if len(line)==0 :
           continue
    tokens = line.split(' ')
    tokens = list(filter(lambda x: x!= '', tokens))
    if len(tokens)<2 or line[:1]=="//" :
            continue
    if tokens[0]=="Land:" :	 
            land_line = tokens
    if tokens[0]=="LCustom:" :
            filename = land_line[1].split('.')
            # Грузим только obj
            print main_dir + "\\Land\\" + land_line[1]
            if lands_counter>0:
                 xxx = list(filter(lambda x: x[0] in land_line[1], xxx_logic))
                 course = 180
                 if xxx != [] :
                     course = xxx[0][1]
                     print course
                 idx = files_loaded.count(filename[0]) 
                 obj_name = filename[0] + idx.__str__()
                 cmds.file(main_dir + "\\Land\\" + filename[0] + ".obj" ,i=True,f=True,typ="OBJ", gr=True, gn= obj_name, ra=True)
                 cmds.setAttr( obj_name + '.rotatePivot', 0, 0, 0, type="double3")
                 cmds.setAttr( obj_name + '.scalePivot', 0, 0, 0, type="double3")
                 cmds.setAttr( obj_name + '.rotate', 90, 0, course, type="double3")
                 cmds.makeIdentity( obj_name , apply=True, translate=True, rotate=True, scale=True, n=0)
                 cmds.setAttr( obj_name + '.translate', float(tokens[2]) , float(tokens[3]), 0, type="double3")
                 cmds.setAttr( obj_name + '.rotate', 0, 0, -float(tokens[1]), type="double3")
                 
                 files_loaded.append(filename[0])
            else:
                 files_loaded.append(filename[0])
                 cmds.file(main_dir + "\\Land\\" + filename[0] + ".obj",o=True,f=True,typ="OBJ")  
                 cmds.selectType( q=True, cv=True )
                 nodes = cmds.ls( geometry=True )
                 cmds.group( nodes, n='root')
                 cmds.setAttr('root.rotatePivot', 0, 0, 0, type="double3")
                 cmds.setAttr('root.scalePivot', 0, 0, 0, type="double3")
                 cmds.setAttr('root.rotate', 90, 0, 180 - float(tokens[1]), type="double3")
                 cmds.makeIdentity( 'root' , apply=True, translate=True, rotate=True, scale=True, n=0)
            lands_counter+=1
    if tokens[0]=="SpotLight:" :
            node_name = "lightmast_%d" % light_masts
            cmds.createNode( 'transform', n=node_name  )  # , p='root'
            light_masts +=1
            cmds.setAttr( node_name + '.translateX', float(tokens[3]))
            cmds.setAttr( node_name + '.translateY', float(tokens[4]))
            cmds.setAttr( node_name + '.translateZ', float(tokens[5]))
            cmds.setAttr( node_name + '.rotate', -45, 0, 0, type="double3")
            cmds.setAttr( node_name + '.displayLocalAxis', 1)
    if tokens[0]=="SECTOR:" :
            node_name = "sector_%d" % sectors
            cmds.createNode( 'transform', n=node_name  ) # , p='root'
            sectors +=1
            cmds.setAttr( node_name + '.translateX', float(tokens[2]))
            cmds.setAttr( node_name + '.translateY', float(tokens[3]))
            cmds.setAttr( node_name + '.translateZ', float(tokens[5]))
            cmds.setAttr( node_name + '.rotate', 0, 0, float(tokens[4]), type="double3")
            cmds.setAttr( node_name + '.displayLocalAxis', 1)	
			
cmds.viewClipPlane( 'perspShape', acp=True, fcp=100000.0 )
#cmds.selectType( q=True, cv=True )
#nodes = cmds.ls( geometry=True )
#cmds.group( nodes, n='root')



