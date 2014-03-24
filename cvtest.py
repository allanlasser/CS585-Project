# instructions for installing python on OSX 10.8/.9 available at
# http://www.jeffreythompson.org/blog/2013/08/22/update-installing-opencv-on-mac-mountain-lion/

import numpy as np
import cv2 as cv

def webcam():
    cv.namedWindow("webcam")
    video = cv.VideoCapture(0)

    while(1):
        _,f = video.read()
        f = cv.GaussianBlur(f,(9,9),100)
        cv.imshow("webcam",f)
        if cv.waitKey(10)==27:
            break

    cv.destroyWindow("webcam")
    
def main():
    webcam()

main()