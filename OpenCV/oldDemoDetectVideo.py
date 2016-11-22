import cv2.cv as cv
import numpy as np
import cv2

foundUser = False
numLoops = 100

print "Press Q to quit."
cap = cv.CaptureFromCAM(0)

def draw_rectangles(img, rects, thickness = 1):
    #for x, y, w, h in rects:
    x = rects[0]
    y = rects[1]
    w = rects[2]
    h = rects[3]
    # the HOG detector returns slightly larger rectangles than the real obj$
    # so we slightly shrink the rectangles to get a nicer output.
    pad_w, pad_h = int(0.15*w), int(0.05*h)
    cv2.rectangle(img, (x+pad_w, y+pad_h), (x+w-pad_w, y+h-pad_h), (0, 255, 0), thickness)

def draw_circles():
    print "Drawing circles"

def findUser(frame):

    
    #converts to numpy array
    arr = np.asarray(frame[:,:])

    hog = cv2.HOGDescriptor()
    hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())
    #takes a numpy array as input
    found, w = hog.detectMultiScale(arr, winStride=(16,16), padding=(32,32), scale=1.05)

    print found

    #if a person is found
    if len(found) != 0:
        for f in found:
            draw_rectangles(arr, f)
        returnVal = f    
    #if no person found
    else:
        returnVal = None

    #converts numpy array back to IplImage
    #cv.ShowImage("PeopleFinder", cv.fromarray(arr))
    return returnVal

def camshift(x,y,w,h):
    global numLoops
    global cap
    for x in range (0, numLoops):
        frame = cv.QueryFrame(cap)

        track_window = (x,y,w,h)


        crit = ( cv.CV_TERMCRIT_EPS | cv.CV_TERMCRIT_ITER, 10, 1)
        (iters, (area, value, rect), track_box) = cv.CamShift(backproject, track_window, crit)
        try:
            #prints the center x and y value of the tracked ellipse
            coord = track_box[0]
            print "center = {}".format(coord)
        except UnboundLocalError:
            print "track_box is None"

        cv.EllipseBox( frame, track_box, cv.CV_RGB(255,0,0), 3, cv.CV_AA, 0 )


while True:

    #format = IplImage
    frame = cv.QueryFrame(cap)
    #converts to numpy array
    arr = np.asarray(frame[:,:])
    cv.ShowImage("PeopleFinder", cv.fromarray(arr))


    rects = findUser(frame)

    #couldn't find user, repeat loop
    if rects == None:
        print "No user, continuing loop"
        continue
    #found user, perform camshift on the user    
    else:    
        x = rects[0]
        y = rects[1]
        w = rects[2]
        h = rects[3]
        camshift(x,y,w,h)
       

    
    if cv.WaitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv.DestroyAllWindows()
