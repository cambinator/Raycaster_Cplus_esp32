import xml.etree.ElementTree as ET

map_name = input("Enter map filename: ")
index = int(input("Enter map index: "))

map_tree = ET.parse(map_name)
map_root = map_tree.getroot()
layer = map_root.find("layer")
width = layer.attrib["width"]
height = layer.attrib["height"]
data = layer.find("data")

out_file = open("map_" + str(index) + ".c", "w")
out_file.write("const unsigned char map" + str(index) + "_width = " + str(width) + ";\n")
out_file.write("const unsigned char map" + str(index) + "_height = " + str(height) + ";\n")
out_file.write("const unsigned char map" + str(index) + "[] = {")
out_file.write(data.text + "};")
out_file.close()
