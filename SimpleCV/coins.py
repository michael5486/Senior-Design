from SimpleCV import Image, Blob 
import numpy as np
img = Image("coins.jpg")
coins = img.invert().findBlobs(minsize = 500) 
value = 0.0
# The value of the coins in order of their size
# http://www.usmint.gov/about_the_mint/?action=coin_specifications 
coin_diameter_values = np.array([
	[ 19.05, 0.10],
	[ 21.21, 0.01],
	[ 17.91, 0.05],
	[ 24.26, 0.25]]);

# Use a quarter to calibrate (in this example we must have one)
px2mm = coin_diameter_values[3,0] / max([c.radius()*2 for c in coins])
for c in coins:
	diameter_in_mm = c.radius() * 2 * px2mm
	distance = np.abs(diameter_in_mm - coin_diameter_values[:,0]) 
	index = np.where(distance == np.min(distance))[0][0]
	value += coin_diameter_values[index, 1]
print "The total value of the coins is $", value