from SimpleCV import Color, Image
import time
img = Image("mandms.jpg")

blue_distance = img.colorDistance(Color.BLUE).invert()
blobs = blue_distance.findBlobs()
blobs.draw(color=Color.PUCE, width=3)
blue_distance.show()
img.addDrawingLayer(blue_distance.dl())
img.show()
time.sleep(10)
