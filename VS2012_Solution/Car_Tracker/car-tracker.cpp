/*
** car-tracker.cpp
** by Allan Lasser
*/

#include "stdafx.h"
#define CASCADE_FILENAME "cascade.xml"

vector<Rect> detectCars (Mat& frame) {
	// load cascade
	CascadeClassifier carCascade;
	if (!carCascade.load(CASCADE_FILENAME)) {
		Exception e = Exception(0, "Failed to load cascade file", "detectCars()", "car-tracker.cpp", 7);
		throwError(e);
	}
	// detect cars
	vector<Rect> cars;
	Mat grayImage;
	try {
		equalizeHist(frame, grayImage);
		carCascade.detectMultiScale(grayImage, cars, 1.1, 6, 0|CV_HAAR_SCALE_IMAGE, Size(30, 20));
	} catch (Exception e) { throwError(e); }
	return cars;
}

Rect getROI (Point& point, Size patchSize, Size imageSize, int searchRadius) {
	// Get top left and bottom right points
	Point TL, BR;
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
	try { matchTemplate(search, patch, result, CV_TM_CCORR_NORMED); }
	catch (Exception e) { throwError(e); }
	// find maximum in result
	double minVal, maxVal;
	Point minLoc, maxLoc; // match = max
	try { minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat()); }
	catch (Exception e) { throwError(e); }
	Point matchLoc = searchArea.tl() + maxLoc;
	return getROI(matchLoc, patchSize, imageSize, 0);
}

vector<Mat> getPatches (Mat& frame, vector<Rect> cars) {
	vector<Mat> ret;
	for (size_t i = 0; i < cars.size(); i++){
		ret.push_back(Mat());
		try { frame(cars[i]).copyTo(ret.back()); }
		catch (Exception e) { throwError(e); }
	}
	return ret;
}

void drawDetections (Mat& frame, vector<Rect> cars) {
	for (size_t i = 0; i < cars.size(); i++ ) {
		Point topLeft = Point(cars[i].x, cars[i].y);
		Point bottomRight = Point(cars[i].x+cars[i].width, cars[i].y+cars[i].height);
		try { rectangle(frame, topLeft, bottomRight, Scalar(255, 255, 255), 2, 8, 0); }
		catch (Exception e) { throwError(e); }
	}
}