
#include "stdafx.h"

vector<Rect> detectCars (Mat& frame);
Rect getROI (Point& point, Size patchSize, Size imageSize, int searchRadius);
Rect trackTemplate (Point& point, Mat& patch, Mat& image, int searchRadius);
vector<Mat> getPatches (Mat& frame, vector<Rect> cars);
void drawDetections (Mat& frame, vector<Rect> cars);