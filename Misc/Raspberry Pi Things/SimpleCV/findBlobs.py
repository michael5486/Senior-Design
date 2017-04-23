from SimpleCV import Image
import time
pennies = Image("pennies.jpg") 
binPen = pennies.binarize() 
blobs = binPen.findBlobs() 
blobs.show(width=5)
print "Areas: ", blobs.area()
print "Angles: ", blobs.angle()
print "Centers: ", blobs.coordinates()
time.sleep(5)