import cv2.cv as cv
import numpy as np
import cv2
import pdb

#needed for camshift
hist = cv.CreateHist([180], cv.CV_HIST_ARRAY, [(0,180)], 1 )
backproject_mode = False

print "Press Q to quit."
cap = cv.CaptureFromCAM(0)

def is_rect_nonzero(r):
    (_,_,w,h) = r
    return (w > 0) and (h > 0)

def draw_detections(img, rects, thickness = 1):
    #for x, y, w, h in rects:
    x = rects[0]
    y = rects[1]
    w = rects[2]
    h = rects[3]
    # the HOG detector returns slightly larger rectangles than the real obj$
    # so we slightly shrink the rectangles to get a nicer output.
    pad_w, pad_h = int(0.15*w), int(0.05*h)
    cv2.rectangle(img, (x+pad_w, y+pad_h), (x+w-pad_w, y+h-pad_h), (0, 255, 0), thickness)

def findUser(frame):
    print "Finding User"
    #converts to numpy array
    arr = np.asarray(frame[:,:])


    hog = cv2.HOGDescriptor()
    hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())
    #takes a numpy array as input
    found, w = hog.detectMultiScale(arr, winStride=(16,16), padding=(32,32), scale=1.05)

    print found

    if len(found) != 0:
        for f in found:
            draw_detections(arr, f)
        returnVal = f    
    else:
        returnVal = None

    #converts numpt array back to IplImage
    cv.ShowImage("HOGPeopleFinder", cv.fromarray(arr))
    if cv.WaitKey(1) & 0xFF == ord('q'):
        quit()
    return returnVal

def camshift(x,y,w,h, selection):
    print "Performing camshift with x:{} y:{} w:{} h:{}".format(x,y,w,h)
    print selection
    hist = cv.CreateHist([180], cv.CV_HIST_ARRAY, [(0,180)], 1 )

    while True:
        print "entered loop"
    #camshift termination criteria (10 iterations without movement of 1 pixel ends camshift)

        frame = cv.QueryFrame(cap)
        cv.Flip(frame, frame, 1)
            
        #print "switching to HSV"    
        hsv = cv.CreateImage(cv.GetSize(frame), 8, 3)
        cv.CvtColor(frame, hsv, cv.CV_BGR2HSV)
        hue = cv.CreateImage(cv.GetSize(frame), 8, 1)
        cv.Split(hsv, hue, None, None, None)

        #compute back projection
       # print "back projection"
        backproject = cv.CreateImage(cv.GetSize(frame), 8, 1)
        cv.CalcArrBackProject([hue], backproject, hist)

        #run the camshift
        #print "camshift"
        print "Selection"
        #pdb.set_trace()
        print selection
        crit = ( cv.CV_TERMCRIT_EPS | cv.CV_TERMCRIT_ITER, 10, 1)
        (iters, (area, value, rect), track_box) = cv.CamShift(backproject, selection, crit)
        print "rect"
        print rect
        if rect[0] > 0 and rect[1] > 0:
            selection = rect
        print "SelectionNew"
        print selection
        print "track_box"
        print track_box

        #draw the surrounding ellipse
       # print "ellipse"
        cv.EllipseBox(frame, track_box, cv.CV_RGB(255,0,0), 3, cv.CV_AA, 0)

        #draw image
        #print "drawing image"
        cv.ShowImage("CamShift", frame)
        if cv.WaitKey(1) & 0xFF == ord('q'):
            break


while True:
    #format = IplImage
    frame = cv.QueryFrame(cap)
    rects = findUser(frame)

    if rects == None:
        print "No user, continuing loop"
        continue
    else:    
        x = rects[0]
        y = rects[1]
        w = rects[2]
        h = rects[3]
        camshift(x,y,w,h,(x,y,w,h))
       # camshift(0,0,0,0,(266,246,88,132))

cap.release()
cv.DestroyAllWindows()
