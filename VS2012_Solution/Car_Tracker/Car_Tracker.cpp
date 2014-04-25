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

vector<Rect> detectCars (Mat& frame);
Rect getROI (Point& point, Size patchSize, Size imageSize, int searchRadius);
Rect trackTemplate (Point& point, Mat& patch, Mat& image, int searchRadius);
vector<Mat> getPatches (Mat& frame, vector<Rect> cars);
void drawDetections (Mat& frame, vector<Rect> cars);

int main(int argc, char* argv[]) {
	int frame_counter = 0;
	int frame_total = 300;
	String frame_folder = "./data/video/data/";
	String frame_filetype = ".png";

	vector<Rect> cars; // stores ROI for detected cars
	vector<Mat> prevFrame; // stores frames from last iteration for template tracking

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
				
		/* Start Car Tracking */
		if (frame_counter % 7 == 0) {
			cars = detectCars(frame); // this is the feature detection tracking
		} else {
			for (size_t i = 0; i < cars.size(); i++) {
				cars[i] = trackTemplate(cars[i].tl(), prevFrame[i], frame, 100); // this is the updating-template tracking
			}
		}
		prevFrame = getPatches(frame, cars);
		/* End Car Tracking */
		
		drawDetections(frame, cars); // this is accessory a.k.a. delete whenevs yo

		imshow("Frame", frame);
		if (waitKey(10) == 'q')
			break;
		frame_counter++;
	}
}

vector<Rect> detectCars (Mat& frame) {

	String filename = "cascade.xml";
	CascadeClassifier carCascade;

	if (!carCascade.load(filename)) {
		// TODO: Proper error handling
		printf("ERROR: Could not open cascade file.\n");
	}

	vector<Rect> cars;
	Mat grayImage;
	frame.copyTo(grayImage);
	equalizeHist (grayImage, grayImage);
	carCascade.detectMultiScale(grayImage, cars, 1.1, 6, 0|CV_HAAR_SCALE_IMAGE, Size(30, 20));
	return cars;
}

Rect getROI (Point& point, Size patchSize, Size imageSize, int searchRadius) {
	
	Point TL, BR; // TL: top left; BR: bottom right
	TL = Point(point.x-searchRadius, point.y-searchRadius);
	BR = Point(point.x+patchSize.width+searchRadius, point.y+patchSize.height+searchRadius);

	// Check if the ROI will go out of bounds on the image
	if (TL.x < 0)
		TL.x = 0;
	if (TL.y < 0)
		TL.y = 0;
	if (BR.x > imageSize.width)
		BR.x = imageSize.width;
	if (BR.y > imageSize.height)
		BR.y = imageSize.height;

	return Rect(TL, BR);
}

Rect trackTemplate (Point& point, Mat& patch, Mat& image, int searchRadius) {

	Size patchSize = patch.size();
	Size imageSize = image.size();
	Rect searchArea = getROI(point, patchSize, imageSize, searchRadius);

	Mat search = image(searchArea);
	Mat result = Mat::zeros(Size(search.cols - patchSize.width + 1, search.rows - patchSize.height +1), CV_32FC1);

	matchTemplate(search, patch, result, CV_TM_CCORR_NORMED);
	
	// find maximum in result
	double minVal, maxVal;
	Point minLoc, maxLoc; // match = max
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	Point matchLoc = searchArea.tl() + maxLoc;
	
	return getROI(matchLoc, patchSize, imageSize, 0);
}

vector<Mat> getPatches (Mat& frame, vector<Rect> cars) {
	vector<Mat> ret;
	for (size_t i = 0; i < cars.size(); i++){
		ret.push_back(Mat());
		frame(cars[i]).copyTo(ret.back());
	}
	return ret;
}

void drawDetections (Mat& frame, vector<Rect> cars) {
	for( size_t i = 0; i < cars.size(); i++ ) {
		Point topLeft = Point(cars[i].x, cars[i].y);
		Point bottomRight = Point(cars[i].x+cars[i].width, cars[i].y+cars[i].height);
		rectangle(frame, topLeft, bottomRight, Scalar(255, 255, 255), 2, 8, 0);
	}
}
