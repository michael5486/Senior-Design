import cv2
import cv2.cv as cv
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

    mask = cv2.medianBlur(mask,5)
    cimg = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
    circles = cv2.HoughCircles(mask,cv.CV_HOUGH_GRADIENT,1,300,param1=50,param2=30,minRadius=0,maxRadius=0)

    circles = np.uint16(np.around(circles))
    for i in circles[0,:]:
        # draw the outer circle
        cv2.circle(cimg,(i[0],i[1]),i[2],(0,255,0),2)
        # draw the center of the circle
        cv2.circle(cimg,(i[0],i[1]),2,(0,0,255),3)
    cv2.imshow('detected circles',cimg)


    # Bitwise-AND mask and original image
    res = cv2.bitwise_and(frame,frame, mask= mask)

    cv2.imshow('frame',frame)
    cv2.imshow('mask',mask)
    cv2.imshow('res',res)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
