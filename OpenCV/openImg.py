import cv2
import numpy as np

img = cv2.imread('home.jpg')
cv2.imshow('image',img)
print "Press any key to exit."
cv2.waitKey(0)
cv2.destroyAllWindows()
