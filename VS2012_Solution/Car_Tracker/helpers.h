#pragma once

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

float length(float x, float y);
string getFilename(string path, string ext, int counter);
void throwError(Exception e);