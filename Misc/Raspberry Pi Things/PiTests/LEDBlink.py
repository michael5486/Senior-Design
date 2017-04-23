import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BOARD)
GPIO.setup(8,GPIO.OUT)
GPIO.output(8,GPIO.LOW)

print "HelloWorld"

for x in xrange (0,10):
	print x
	GPIO.output(8,GPIO.HIGH)
	time.sleep(0.5)
	GPIO.output(8,GPIO.LOW)
	time.sleep(0.5)

# rpi GPIO.setmode(GPIO.BOARD)
# GPIO.output(8, GPIO.HIGH)

