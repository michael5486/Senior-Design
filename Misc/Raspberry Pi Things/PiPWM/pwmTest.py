#import GPIO pins, needs sudo (superuser permissions)
import RPi.GPIO as GPIO
import time
#use the numbers on the Pi Cobbler
step = .01
GPIO.setmode(GPIO.BCM)
GPIO.setup(17,GPIO.OUT)
#set frequency to 100 Hz
my_pwm=GPIO.PWM(17,100)

while True:
    for x in range(0,100,5):
        my_pwm.start(x)
        print x
        #sleep for second
        time.sleep(step)
    for x in range(100,0,-5):
        my_pwm.start(x)
        print x
        #sleep for quarter second
        time.sleep(step)
my_pwm.stop()
GPIO.cleanup()
