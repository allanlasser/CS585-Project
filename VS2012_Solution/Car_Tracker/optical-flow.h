#pragma once
#include "stdafx.h"

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