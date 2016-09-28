from SimpleCV import Camera, time
from time import sleep

cam = Camera()

while True:
    
    img = cam.getImage()

    img = img.binarize()

    img.drawText("Hello World!")

    img.show()
