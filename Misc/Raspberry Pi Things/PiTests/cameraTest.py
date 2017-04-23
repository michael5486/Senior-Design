from picamera import PiCamera, Color
from time import sleep

camera = PiCamera()
camera.rotation = 180

#camera.start_preview(alpha=25)

camera.start_preview()
#camera.framerate = 9
#brightness values range from 0-100
#camera.brightness = 70

#resolution of FLIR LEPTON v3
#camera.resolution = (160,120)

#camera.start_recording('/home/pi/Desktop/video.h264')

#annotation sizes range from 6-160, default is 32
camera.annotate_text_size = 50
camera.annotate_background = Color('orange')
camera.annotate_foreground = Color('black')

for i in range (100):
    camera.annotate_text = "Brightness: %s" % i
    camera.brightness = i
    sleep(0.1)


#camera.capture('/home/pi/Desktop/annotatedImage.jpg')


#camera.stop_recording()
camera.stop_preview()


