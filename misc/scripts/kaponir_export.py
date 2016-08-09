import maya.cmds as cmds
import ntpath
import os 
import xml.etree.cElementTree as ET


basicFilter = "*.scn"
path = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, fileMode=1)

main_dir = ntpath.dirname(path[0])
inputfile = open(path[0])

lands_counter = 0
files_loaded = []

root = ET.Element("root")
ET.SubElement(root, "object_data", name='kaponir' )

xxx_logic = [ ("kaponir", -90), ("eisk", 180), ("tramplin",180) ]

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
                 
                 if 'kaponir' in land_line[1]:
                     ET.SubElement(root, "value", x='%.5f' % float(tokens[2]), y='%.5f' % float(tokens[3]), z='%.5f' % 0, h='%.5f' % 0 , rx='%.5f' %float(0), ry='%.5f' %float(0), rz='%.5f'%-float(tokens[1]) )
                 
                 files_loaded.append(filename[0])
            else:
                 files_loaded.append(filename[0])
            lands_counter+=1
            
            
tree = ET.ElementTree(root)
tree.write("data1.xml")