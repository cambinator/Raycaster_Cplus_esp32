from PIL import Image

imageName = input("Enter filename: ")
xS = int(input("Enter x resolution: "))
yS = int(input("Enter y resolution: "))
image = Image.open(imageName)
pixels = image.load()

out_file = open(imageName[0:-4] +"_16b.bin", "wb")
out_file.write(xS.to_bytes(length = 1, byteorder = 'big'))
out_file.write(yS.to_bytes(length = 1, byteorder = 'big'))
for y in range(yS):
    for x in range(xS):
        if len(pixels[0,0]) == 3:
            r, g, b = pixels[x, y]
        else:
            r, g, b, a = pixels[x, y]
        uint16 = int(((r>>3)<<11) + ((g>>3)<<6) + (b>>3))
        out_file.write(uint16.to_bytes(length = 2, byteorder = 'big'))
