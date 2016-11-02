import cv2
import numpy as np

cap = cv2.VideoCapture(0)
print "Press Q to quit"

while(1):

    # Take each frame
    _, frame = cap.read()

    # Convert BGR to HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # define range of blue color in HSV
   # lower_blue = np.array([110,50,50])
   # upper_blue = np.array([130,255,255])
    lower_org = np.array([10, 0, 0])
    upper_org = np.array([20, 255, 255])


    # Threshold the HSV image to get only blue colors
    mask = cv2.inRange(hsv, lower_org, upper_org)

    # Bitwise-AND mask and original image
    res = cv2.bitwise_and(frame,frame, mask= mask)

    cv2.imshow('frame',frame)
    cv2.imshow('mask',mask)
    cv2.imshow('res',res)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
