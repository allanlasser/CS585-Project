#pragma once

#include "stdafx.h"
#include "helpers.h"
#include "car-speed.h"
#include "optical-flow.h"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
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