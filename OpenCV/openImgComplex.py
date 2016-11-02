#! /usr/bin/env python

print "OpenCV Python version of edge"

import sys
import urllib2
import cv2.cv as cv

# some definitions
win_name = "Edge"

if __name__ == '__main__':
    if len(sys.argv) > 1:
        im = cv.LoadImage( sys.argv[1], cv.CV_LOAD_IMAGE_COLOR)
    else:
        print "fetching url"
        url = 'https://i.vimeocdn.com/portrait/4080732_300x300'
        print "received file"
        filedata = urllib2.urlopen(url).read()
        imagefiledata = cv.CreateMatHeader(1, len(filedata), cv.CV_8UC1)
        cv.SetData(imagefiledata, filedata, len(filedata))
        im = cv.DecodeImage(imagefiledata, cv.CV_LOAD_IMAGE_COLOR)
    
    # create the output im
    col_edge = cv.CreateImage((im.width, im.height), 8, 3)

    #cv.CvtColor(im, gray, cv.CV_BGR2GRAY)


    # create the window
    cv.NamedWindow(win_name, cv.CV_WINDOW_AUTOSIZE)

     # show the im
    cv.ShowImage(win_name, col_edge)   

    # wait a key pressed to end
    cv.WaitKey(0)
    print "waiting for a key"
    cv.DestroyAllWindows()
