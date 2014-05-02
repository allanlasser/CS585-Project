#pragma once

#include "stdafx.h"
#include "helpers.h"
#include "car-tracker.h"
#include "car-speed.h"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

void opticalFlowMagnitudeAngle(const Mat& flow, Mat& magnitude, Mat& angle);
void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step, double, const Scalar& color);
void drawOptFlowMapROI(const Mat& flow, Mat& cflowmap, Rect ROI, int step, double, const Scalar& color);
void drawDetectionFlows (Mat& frame, Mat&flow, vector<Rect> cars, const Scalar& color);
vector<Point2d> gatherOptFlowVectors(vector<Rect> ROIs, Mat flow);
Mat getOptFlowROI(Rect ROI, Mat flow);
Rect getRoadRect(Mat image);
Point2d getAverageFlow(Mat flow, Rect ROI);
void drawAverageFlowVectors(const Mat& flow, Mat& frame, vector<Rect> cars, vector<Point2d> avgs);
Point2d getRoadAvgVector(Rect road, Mat flow);