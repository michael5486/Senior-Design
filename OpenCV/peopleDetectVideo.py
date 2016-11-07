import cv2.cv as cv
import numpy as np
import cv2

print "Press Q to quit."
cap = cv.CaptureFromCAM(0)

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

while True:
    #format = IplImage
    frame = cv.QueryFrame(cap)
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


    #converts numpt array back to IplImage
    cv.ShowImage("PeopleFinder", cv.fromarray(arr))
    
    if cv.WaitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv.DestroyAllWindows()
