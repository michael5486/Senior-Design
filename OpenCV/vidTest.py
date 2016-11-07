import cv2.cv as cv

print "Press Q to quit."
cap = cv.CaptureFromCAM(0)

while True:
	frame = cv.QueryFrame(cap)

	cv.ShowImage("CamShiftDemo", frame)
	if cv.WaitKey(1) & 0xFF == ord('q'):
		break

cap.release()
cv.DestroyAllWindows()
