from picamera import PiCamera, Color
from time import sleep

camera = PiCamera()

#camera.rotation = 180
camera.resolution = (64, 64)
camera.framerate = 90

camera.start_preview()
sleep(10)
camera.stop_preview()


