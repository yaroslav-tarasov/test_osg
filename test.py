from xml.dom import minidom
xmldoc = minidom.parse('env_vars.props')

macros = xmldoc.getElementsByTagName("PropertyGroup")

#print(len(macros))
print(macros[0].attributes['Label'].value)
print(macros[0].nodeName)
print(macros[0].childNodes[1].childNodes[0])

#for node in macros[0].childNodes:
#    if node.nodeType == node.TEXT_NODE:
#	    if node.childNodes.length >0:
#				print(node.childNodes[0])
			
#for s in macros:
#    print(s.childNodes[0].nodeValue)