/*
** optical-flow.cpp
** by Ashley Hansberry
*/

#include "stdafx.h"

Rect getRoadRect(Mat image) {
	int width = image.cols;
	int height = image.rows;
	Rect road = Rect(width/6,height-50,width*2/3,50);
	return road;
}

Mat getOptFlowROI(Rect ROI, Mat flow) {
	Mat flowROI;
	flow(ROI).copyTo(flowROI);
	return flowROI;
}

Point2d getRoadAvgVector(Rect road, Mat flow) {
	float xa = 0;
	float ya = 0;
	int count = 0;
	int step = 16;
	int left_max = road.x + (road.width/3);
	int right_min = road.x + (road.width*2/3);
	for (int y = road.y; y < road.y+road.height; y += step) {
        for(int x = road.x; x < road.x+road.width; x += step) {
            const Point2f& fxy = flow.at<Point2f>(y, x);
			if (x < left_max || x > right_min) {
				xa += fxy.x;
				ya += fxy.y;
				count++;
			}
        }
	}
	xa /= count;
	ya /= count;
	return Point2d(xa,ya);
}

vector<Point2d> gatherOptFlowVectors(vector<Rect> ROIs, Mat flow) {
	vector<Point2d> flowROIs;
	for (size_t i = 0; i < ROIs.size(); i++) {
		Point2d avg = getAverageFlow(flow, ROIs[i]);
		flowROIs.push_back(avg);
	}
	return flowROIs;
}

Point2d getAverageFlow(Mat flow, Rect ROI) {
	float xa = 0;
	float ya = 0;
	int count = 0;
	int step = 16;
	int weight = 2;
	int min_weight_x;
	int max_weight_x;
	int min_weight_y;
	int max_weight_y;
	min_weight_x = ROI.width/3;
	max_weight_x = ROI.width*2/3;
	min_weight_y = ROI.height/3;
	max_weight_y = ROI.height*2/3;
	for(int y = ROI.y; y < ROI.y+ROI.height; y += step) {
        for(int x = ROI.x; x < ROI.x+ROI.width; x += step) {
            const Point2f& fxy = flow.at<Point2f>(y, x);
			xa += fxy.x;
			ya += fxy.y;
			count++;
        }
	}
	xa /= count;
	ya /= count;
	return Point2d(xa,ya);
}

void opticalFlowMagnitudeAngle(const Mat& flow, Mat& magnitude, Mat& angle) {
    if (magnitude.rows != flow.rows || magnitude.cols != flow.cols)
        magnitude.create(flow.rows, flow.cols, CV_32FC1);
	if (angle.rows != flow.rows || angle.cols != flow.cols)
        angle.create(flow.rows, flow.cols, CV_32FC1);
	Mat xy[2];
	split(flow, xy);
	try { cartToPolar(xy[0], xy[1], magnitude, angle, true); }
	catch (Exception e) { throwError(e); }
}

void drawAverageFlowVectors(const Mat& flow, Mat& frame, vector<Rect> cars, vector<Point2d> avgs) {
	for (size_t i = 0; i < cars.size(); i++) {
		Point middle = Point(cars[i].width/2, cars[i].height/2);
		Point frame_middle = Point(cars[i].x + middle.x, cars[i].y + middle.y);
		Point line_end = Point2d((double)frame_middle.x + avgs[i].x, (double)frame_middle.y + avgs[i].y);
		line(frame, frame_middle, line_end, Scalar(255, 255, 255), 2);
	}
}

void drawDetectionFlows (Mat& frame, Mat&flow, vector<Rect> cars, const Scalar& color) {
	for (size_t i = 0; i < cars.size(); i++)
		drawOptFlowMapROI(flow, frame, cars[i], 16, 1.5, CV_RGB(0, 255, 0));
}

// drawOptFlowMap and drawOptFlowMapROI are copied from OpenCV sample fback.cpp
// Optical flow is stored as two-channel floating point array in a Mat of type CV_32FC2

void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step, double, const Scalar& color) {
    for (int y = 0; y < cflowmap.rows; y += step) {
        for (int x = 0; x < cflowmap.cols; x += step) {
            const Point2f& fxy = flow.at<Point2f>(y, x);
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), color);
        }
	}
}

void drawOptFlowMapROI(const Mat& flow, Mat& cflowmap, Rect ROI, int step, double, const Scalar& color) {
    for (int y = ROI.y; y < ROI.y+ROI.height; y += step) {
        for (int x = ROI.x; x < ROI.x+ROI.width; x += step) {
            const Point2f& fxy = flow.at<Point2f>(y, x);
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), color);
        }
	}
}