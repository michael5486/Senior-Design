from SimpleCV import Camera, time, Color
from time import sleep

cam = Camera()

while True: 
	img = cam.getImage()
	#orange_dist = img.colorDistance(color = Color.ORANGE).dilate(2).invert()
	#orange_dist = img.hueDistance(Color.ORANGE).dilate(2).invert()
	#lobs = img.findBlobs()

	#lobs.draw(width=4)
	#mg_with_blobs = img.applyLayers()
	#bin = orange_dist

	#circles = img.findCircle(canny = 80, thresh = 350)
	circles = img.findCircle()
	print circles
	#if circles[0] == None:
	#do nothing, Circle is empty
	#print "No circles found"	
	#else:
	#	circles.draw(width=4)
	#	img_with_circles = img.applyLayers()


	#final = img.sideBySide(img_with_circles)
	#final.show()