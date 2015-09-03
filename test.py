import os
from xml.dom import minidom
xmldoc = minidom.parse('env_vars.props')

macros = xmldoc.getElementsByTagName("PropertyGroup")

#print(macros[0].attributes['Label'].value)

for node in macros[0].childNodes:
	if node.childNodes.length >0:
		if node.childNodes[0].nodeType == node.TEXT_NODE:
					os.putenv(node.nodeName, node.childNodes[0].nodeValue)
					print(node.nodeName + '  ' + node.childNodes[0].nodeValue)

print(os.getenv('SIMEX_DIR','NOT_SET'))
