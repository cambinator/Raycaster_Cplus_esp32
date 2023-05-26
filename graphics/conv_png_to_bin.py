from PIL import Image

imageName = input("Enter filename: ")
xS = int(input("Enter x resolution: "))
yS = int(input("Enter y resolution: "))
outputImageName = input("Enter output file name: ")
image = Image.open(imageName)
pixels = image.load()

out_file = open(outputImageName +".bin", "wb")
for y in range(yS):
  for x in range(xS):
    if len(pixels[0,0]) == 3:
  	  r, g, b = pixels[x, y]
    else:
      r, g, b, a = pixels[x, y]
    byte = int(((r>>5)<<5) + ((g>>5)<<2) + (b>>6))
    out_file.write(byte.to_bytes(length=1, byteorder = 'little'))
