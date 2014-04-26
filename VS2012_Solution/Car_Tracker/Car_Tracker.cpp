#include "stdafx.h"
#include <iostream>
#include <fstream>
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

/*camera speeed stuff*/
vector<float> getCarSpeeds(vector<Rect> cars, int &counter, int frame);
float getCameraSpeed(int &counter, int frame);
vector<float> getTimeStampsGPS();
vector<float> getTimeStampsVideo();
/*end camera speed stuff*/

Rect getROI (Point& point, Size patchSize, Size imageSize, int searchRadius);
Rect trackTemplate (Point& point, Rect& patch, Mat& image, int searchRadius);
void drawCascade (Mat& frame, vector<Rect> cars);

int main(int argc, char* argv[]) {
	int frame_counter = 0;
	int frame_total = 300;
	String frame_folder = "./data/video/data/";
	String frame_filetype = ".png";

	int gps_counter = 0;

	vector<Rect> cars;

	//vector<float> f = getTimeStampsVideo(); --just used to test


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
		

		if (frame_counter % 3 == 0) {
			cars = detectCars(frame); // this is the core fuction
		} else {
			for (int i = 0; i < cars.size(); i++) {
				// cout << cars[i] << " --> ";
				cars[i] = trackTemplate(cars[i].tl(), cars[i], frame, 100);
				// cout << cars[i] << "\n";
			}
		}
		
		drawCascade(frame, cars); // this is accessory a.k.a. delete whenevs yo

		/*inserting car speed stuff here for now*/
		vector<float> labels = getCarSpeeds(cars,gps_counter);
		//at this point you would need to label your rects. for now i'm just getting them for you.

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

Rect trackTemplate (Point& point, Rect& patch, Mat& image, int searchRadius) {

	Size patchSize = patch.size();
	Size imageSize = image.size();
	Rect searchArea = getROI(point, patchSize, imageSize, searchRadius);

	Mat search = image(searchArea);
	Mat templ = image(patch);
	Mat result = Mat::zeros(Size(search.cols - patch.width + 1, search.rows - patch.height +1), CV_32FC1);

	matchTemplate(search, templ, result, CV_TM_CCORR_NORMED);
	
	// find maximum in result
	double minVal, maxVal;
	Point minLoc, maxLoc; // match = max
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	Point matchLoc = searchArea.tl() + maxLoc;
	
	return getROI(matchLoc, patchSize, imageSize, 0);
}

void drawCascade (Mat& frame, vector<Rect> cars) {
	for( size_t i = 0; i < cars.size(); i++ ) {
		Point topLeft = Point(cars[i].x, cars[i].y);
		Point bottomRight = Point(cars[i].x+cars[i].width, cars[i].y+cars[i].height);
		rectangle(frame, topLeft, bottomRight, Scalar(255, 255, 255), 2, 8, 0);
	}
}



vector<float> getCarSpeeds(vector<Rect> cars, int counter, int frame){
	float ratio = getCameraSpeed(counter,frame);		 

	Mat opticalFlow; //dummy, same as below

	vector<float> speedLabels;
	for( size_t i = 0; i < cars.size(); i++ ){
		speedLabels.at(i) = mean(opticalFlow(cars.at(i)))[0] * ratio;
	}
}


float getGPSVelocity(int counter){
	vector<float> GPS;//read_data_for_frame
	String dataFile = "./data/oxts/data/";
	if (counter < 10) {
			dataFile += "000000000";
		} else if (counter < 100) {
			dataFile += "00000000";
		} else if (counter < 1000) {
			dataFile += "0000000";
		} else if (counter < 10000) {
			dataFile += "000000";
		} else {
			dataFile += "00000";
		}
	dataFile += to_string(counter) + ".txt";
	char temp[256];
	std::ifstream ifs (dataFile,std::ifstream::in);
	int size = 0;
	while(ifs.good() && (size < 9)){
		ifs.get(temp,10,' ');
		GPS.push_back(atof(temp));
		size ++;
	}
	return length(GPS.at(6),GPS.at(7));
}

float getCameraSpeed(int &counter, int frame){
	float cameraVelocity = 0.0;
	
	vector<float> GPSTime = getTimeStampsGPS();
	vector<float> FrameTime = getTimeStampsVideo();

	while(FrameTime.at(frame) > GPSTime.at(counter)){
		cameraVelocity += getGPSVelocity(counter);
		counter++;
	}

	Rect frontOfCar = Rect(); //dummy  //(shoudl be roughly the front of the car, say, middle fifth of the x, lowest quarter of the y

	Mat opFlow;//assumeing you guys are covereing that part, either I'll pass it through as a pointer or we store it globablly
	Mat focRegion = opFlow(frontOfCar);
	
	double pixelVelocity = mean(focRegion)[0]; //this is how you get the average, though th e0 means for only the first channel
	double realToImage = pixelVelocity/cameraVelocity; //this works as we are treated the time between frames as 1

	return realToImage;
}

/*
Current problem is how to best manage the difference in data vs. frames. aka more gps data than frames
math is easy, just take all relevant readings and average them. simple
but.
actually no, it is simple. just keep track of the number in that main loop and pass it. easy.
*/

float length(float x, float y){
	return sqrtf(x*x+y*y);
}

vector<float> getTimeStampsGPS(){
	String timeStampFile = "./data/oxts/timestamps.txt";
	std::ifstream ifs (timeStampFile, std::ifstream::in);
	vector<float> timeStamps;
	char temp[256];
	String tempB;
	while(ifs.good()){
		ifs.getline(temp,256);
		tempB = temp;
		if(!tempB.empty()){
			tempB = tempB.substr(17);
			timeStamps.push_back(stof(tempB));
		}
	}
	
	return timeStamps;
}

vector<float> getTimeStampsVideo(){
	String timeStampFile = "./data/video/timestamps.txt";
	std::ifstream ifs (timeStampFile, std::ifstream::in);
	vector<float> timeStamps;
	char temp[256];
	String tempB;
	while(ifs.good()){
		ifs.getline(temp,256);
		tempB = temp;
		if(!tempB.empty()){
			tempB = tempB.substr(17);
			timeStamps.push_back(stof(tempB));
		}
	}
	
	return timeStamps;
}