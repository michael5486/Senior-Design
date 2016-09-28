from picamera import PiCamera, Color
from time import sleep

camera = PiCamera()
camera.rotation = 180

camera.start_preview()
sleep(10)
camera.stop_preview()


