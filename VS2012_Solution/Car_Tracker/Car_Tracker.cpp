#include "stdafx.h"
#include <iostream>
#include <ctype.h>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

vector<Rect> detectCars (Mat& image, CascadeClassifier& carCascade);

int main(int argc, char* argv[]) {
	int frame_counter = 0;
	int frame_total = 114;
	String frame_folder = "./data/";
	String frame_filetype = ".png";

	// instructions for building our own Haar cascade
	// http://note.sonots.com/SciSoftware/haartraining.html
	string carCascadeFile = "cars.xml";
	CascadeClassifier carCascade;
	// cascade code from HW7P2
	if (!carCascade.load(carCascadeFile)) {
		// TODO: Proper error handling
		printf("ERROR: Could not open car cascade file.\n");
		return -1; 
	}

	while (frame_counter < frame_total) {
		// Read current image as video frame
		// Every image files is 10 digits + filetype
		// Currently supports up to 999,999 frames
		String filename = frame_folder;
		if (frame_counter < 10) {
			filename += "000000000";
		} else if (frame_counter < 100) {
			filename += "00000000";
		} else if (frame_counter < 1000) {
			filename += "0000000";
		} else if (frame_counter < 10000) {
			filename += "000000";
		} else {
			filename += "00000";
		}
		filename += to_string(frame_counter) + frame_filetype;

		Mat frame;
		frame = imread(filename, 0);
		
		vector<Rect> cars;
		cars = detectCars(frame, carCascade);

		for( size_t i = 0; i < cars.size(); i++ ) {
			Point center( cars[i].x + cars[i].width*0.5, cars[i].y + cars[i].height*0.5 );
			ellipse( frame, center, Size( cars[i].width*0.5, cars[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
			Mat carROI = frame( cars[i] );
			// draw rect on frame
		}

		imshow("Frame", frame);
		if (waitKey(10) == 'q')
			break;
		frame_counter++;
	}
}

vector<Rect> detectCars (Mat& image, CascadeClassifier& carCascade) {
	vector<Rect> cars;
	Mat grayImage;
	image.copyTo(grayImage);
	equalizeHist (grayImage, grayImage);
	carCascade.detectMultiScale(grayImage, cars, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
	return cars;
}
