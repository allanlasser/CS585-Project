#pragma once

#include "stdafx.h"
#include "helpers.h"
#include "car-tracker.h"
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

vector<float> getCarSpeeds(vector<Rect> cars, int counter, int frameNumber, Mat &frame, Mat &flow);
double getCameraSpeed(int &counter, Mat &frame,int frameNumber, Mat &flow);
vector<float> getTimeStampsGPS();
vector<float> getTimeStampsVideo();
float getGPSVelocity(int counter);