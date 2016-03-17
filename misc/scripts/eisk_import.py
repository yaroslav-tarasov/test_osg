import maya.cmds as cmds

inputfile = open('C:\Vis\Eisk_2\eisk.scn')

lands_counter = 0
for i in range(4): inputfile.next() # skip first four lines
for line in inputfile:
    if len(line)==0 :
           continue
    str = line.split(' ')
    if len(str)<2 or line[:1]=="//" :
            continue
    if str[0]=="Land:" :	   
            print "C:\\Vis\\Eisk_2\\Land\\" + str[1]
            if lands_counter>0: 
			     cmds.file("C:\\Vis\\Eisk_2\\Land\\" + str[1],i=True,f=True,typ="OBJ")
            else:
                 cmds.file("C:\\Vis\\Eisk_2\\Land\\" + str[1],o=True,f=True,typ="OBJ")  			 
            lands_counter+=1

cmds.viewClipPlane( 'perspShape', acp=True, fcp=100000.0 )
cmds.selectType( q=True, cv=True )
nodes = cmds.ls( geometry=True )
cmds.group( nodes, n='root' )

cmds.setAttr('root.rotate', 90, 0, 187, type="double3")