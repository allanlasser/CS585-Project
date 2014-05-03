#pragma once
#include "stdafx.h"

vector<float> getCarSpeeds(vector<Rect> cars, int counter, int frameNumber, Mat &frame, Mat &flow);
double getCameraSpeed(int &counter, Mat &frame,int frameNumber, Mat &flow);
vector<float> getTimeStamps(string timeStampFile);
float getGPSVelocity(int counter);