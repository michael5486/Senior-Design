from SimpleCV import Camera, Color
from time import sleep

cam = Camera()
    

while True:
    img = cam.getImage()

    rot = img.rotate(180)

    rot.show()
#sleep(10)
