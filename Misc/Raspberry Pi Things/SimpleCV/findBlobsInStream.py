from SimpleCV import Camera

cam = Camera()

while True:
    
    img = cam.getImage()
    blobs = img.binarize().findBlobs()
    blobs.image = img
    blobs.draw(width=4)
    blobs.show()
