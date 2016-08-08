import maya.cmds as cmds
import json
import itertools
import operator
import xml.etree.cElementTree as ET

nodes = cmds.ls( tr=True )
th_node = [ x for x in nodes if 'Trees_High' in x or 'trees00' in x ]
th_children = cmds.listRelatives(th_node) 
    
thc_nodes = [ x for x in th_children if 'polySurface' in x ]
bboxes = [ [x, cmds.xform(x, q=True, bb=True, ws=True)] for x in thc_nodes ]
bbx =  [ [ round((bb[1][0] + bb[1][3])/2,1), round((bb[1][1] + bb[1][4])/2,1), bb[1][2], round((-bb[1][2] + bb[1][5]), 1 if int((bb[1][2] - bb[1][5]))==0 else 0 )] for bb in bboxes ]
bbx.sort()

# out_list = list(bbx for bbx,_ in itertools.groupby(bbx, key=operator.itemgetter(0,1,2)))
# print out_list

#for pbb in bboxes :
#     bb = pbb[1]
#     print  pbb[0], bb, " %0.2f, %0.2f ,%0.2f " % ( (bb[0] + bb[3])/2, (bb[1] + bb[4])/2,   (bb[2] + bb[5])/2) 

out_list = []     
for key, group in itertools.groupby(bbx, lambda x: [x[0], x[1]]):
     h = 0 
     for thing in group:
        h = max(h,thing[3])
#        # print("A %s is a %s." % (thing, key))
#     # print("A %s is a %s." % (key, h))
     out_list.append([ thing[0], thing[1], thing[2], h ])   
#     # print
     
print out_list   


with open('data_json.txt', 'w') as outfile:
    json.dump(out_list, outfile)
    
root = ET.Element("root")

for el in out_list :
    ET.SubElement(root, "value", x='%.5f' % el[0], y='%.5f' %el[1], z='%.5f' %el[2], h='%.5f' %el[3])

tree = ET.ElementTree(root)
tree.write("data0.xml")