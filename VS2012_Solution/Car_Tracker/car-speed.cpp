/*
** car-speed.h
** by James Racine
*/

#include "stdafx.h"

#define GPS_TIMESTAMP_FILE "./data/oxts/timestamps.txt"
#define VIDEO_TIMESTAMP_FILE "./data/video/timestamps.txt"

vector<float> getCarSpeeds(vector<Rect> cars, int counter, int frameNumber, Mat &frame, Mat &flow) {
	Mat opticalFlow = flow;
	double ratio = getCameraSpeed(counter,frame,frameNumber, opticalFlow);		 
	vector<float> speedLabels;
	for (size_t i = 0; i < cars.size(); i++) {
		Point avgFlow;
		try { avgFlow = getAverageFlow(opticalFlow,cars[i]); }
		catch (Exception e) { throwError(e); }
		speedLabels.push_back(abs(length((float)avgFlow.x, (float)avgFlow.y)) * (float)ratio);
	}
	return speedLabels;
}

float getGPSVelocity(int counter) {
	vector<float> GPS; //read_data_for_frame
	String dataFile = getFilename("./data/oxts/data/", ".txt", counter);
	char temp[256];
	std::ifstream ifs (dataFile,std::ifstream::in);
	int size = 0;
	while (size < 9) {
		ifs.get(temp, 32, ' ');
		GPS.push_back((float)atof(temp));
		ifs.get(temp, 2);
		size++;
	}
	return length(GPS.at(6), GPS.at(7));
}

double getCameraSpeed(int &counter, Mat &frame,int frameNumber, Mat &flow) {
	vector<float> GPSTime = getTimeStamps(GPS_TIMESTAMP_FILE);
	vector<float> FrameTime = getTimeStamps(VIDEO_TIMESTAMP_FILE);
	float cameraVelocity = 0.0;
	while(FrameTime.at(frameNumber) > GPSTime.at(counter))
		cameraVelocity += getGPSVelocity(counter++);
	Rect frontOfCar = getRoadRect(frame);
	Point2d avg = getRoadAvgVector(frontOfCar, flow);
	double pixelVelocity = abs(length((float)avg.x, (float)avg.y));
	double realToImage = pixelVelocity/cameraVelocity;
	return realToImage;
}

vector<float> getTimeStamps(string timeStampFile) {
	std::ifstream ifs (timeStampFile, std::ifstream::in);
	vector<float> timeStamps;
	char temp[256];
	String tempB;
	while(ifs.good()){
		ifs.getline(temp,256);
		tempB = temp;
		if(!tempB.empty()) {
			tempB = tempB.substr(17);
			timeStamps.push_back(stof(tempB));
		}
	}
	return timeStamps;
}