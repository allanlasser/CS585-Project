#include "stdafx.h"
#include "helpers.h"
#include "car-tracker.h"
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

vector<float> getCarSpeeds(vector<Rect> cars, int counter, int frame){
	double ratio = getCameraSpeed(counter,frame);		 

	Mat opticalFlow; //dummy, same as below

	vector<float> speedLabels;
	for( size_t i = 0; i < cars.size(); i++ ){
		speedLabels.at(i) = mean(opticalFlow(cars.at(i)))[0] * (float)ratio;
	}
	return speedLabels;
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

double getCameraSpeed(int &counter, int frame){
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