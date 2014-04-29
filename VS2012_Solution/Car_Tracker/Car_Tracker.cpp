#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
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

/*camera speeed stuff*/
float length(float x, float y);
vector<float> getCarSpeeds(vector<Rect> cars, int counter, int frameNumber, Mat &frame, Mat &flow);
	double getCameraSpeed(int &counter, Mat &frame,int frameNumber, Mat &flow);
vector<float> getTimeStampsGPS();
vector<float> getTimeStampsVideo();
float getGPSVelocity(int counter);
/*end camera speed stuff*/

/* optical flow stuff */
void opticalFlowMagnitudeAngle(const Mat& flow, Mat& magnitude, Mat& angle);
static void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step,
                    double, const Scalar& color);
static void drawOptFlowMapROI(const Mat& flow, Mat& cflowmap, Rect ROI, int step,
                    double, const Scalar& color);
void drawDetectionFlows (Mat& frame, Mat&flow, vector<Rect> cars, const Scalar& color);
vector<Point2d> gatherOptFlowVectors(vector<Rect> ROIs, Mat flow);
Mat getOptFlowROI(Rect ROI, Mat flow);
Rect getRoadRect(Mat image);
Point2d getAverageFlow(Mat flow, Rect ROI);
static void drawAverageFlowVectors(const Mat& flow, Mat& frame, vector<Rect> cars, vector<Point2d> avgs);
Point2d getRoadAvgVector(Rect road, Mat flow);
/* end optical flow stuff */

int main(int argc, char* argv[]) {
	int frame_counter = 0;
	int frame_total = 300;
	String frame_folder = "./data/video/data/";
	String frame_filetype = ".png";

	int GPS_counter = 0;

	vector<Rect> cars; // stores ROI for detected cars
	vector<Mat> prevFrame; // stores frames from last iteration for template tracking

	/* optical flow mats */
	Mat flow, cflow, magnitude, angle;
	Mat gray_old, gray_new;
	vector<Mat> carOptFlow;

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
		frame.copyTo(cflow);
				
		/* Start Car Tracking */
		if (frame_counter % 5 == 0) {
			cars = detectCars(frame); // this is the feature detection tracking
		} else {
			for (size_t i = 0; i < cars.size(); i++) {
				cars[i] = trackTemplate(cars[i].tl(), prevFrame[i], frame, 100); // this is the updating-template tracking
			}
		}
		prevFrame = getPatches(frame, cars);
		/* End Car Tracking */
		
		drawDetections(frame, cars); // this is accessory 


		/* Calculate optical flow each frame */
		frame.copyTo(gray_new);
        if(frame_counter > 1)
        {
            calcOpticalFlowFarneback(gray_old, gray_new, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
            opticalFlowMagnitudeAngle(flow, magnitude, angle);

			//drawOptFlowMap(flow, cflow, 16, 1.5, CV_RGB(0, 255, 0));
	        //imshow("flow", cflow);

			/* cars */
			drawDetectionFlows(frame, flow, cars, CV_RGB(0, 255, 0));
			vector<Point2d> averages = gatherOptFlowVectors(cars, flow);
			drawAverageFlowVectors(flow, frame, cars, averages);

			/* road */
			Rect road = getRoadRect(frame);
			drawOptFlowMapROI(flow, frame, road, 16, 1.5, CV_RGB(0, 255, 0));
			Point2d roadAverages = getRoadAvgVector(road, flow);

        }
		/* end optical flow calculation */



		/* start math code */
		float GPS_speed = getGPSVelocity(GPS_counter);
		std::ostringstream speed_str;
		speed_str << GPS_speed;
		std::string str = speed_str.str() + " m/s";
		putText(frame, str, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
		
		if (frame_counter > 1)
		{
			vector<float> carSpeeds = getCarSpeeds(cars, GPS_counter, frame_counter, frame, flow);
			for( size_t i = 0; i < cars.size(); i++ ) 
			{
				Point topLeft = Point(cars[i].x, cars[i].y-10);
				float car_speed = carSpeeds[i]+GPS_speed;
				std::ostringstream speed_str;
				speed_str << GPS_speed;
				std::string str = speed_str.str() + " m/s";
				putText(frame, str, topLeft, FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
			}
		}

		imshow("Frame", frame);
		if (waitKey(10) == 'q')
			break;
		
		gray_new.copyTo(gray_old); // for optical flow
		frame_counter++;
	}
}

/* 
 * START CAR DETECTION CODE 
 */
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

Rect trackTemplate (Point& point, Mat& patch, Mat& image, int searchRadius) {

	Size patchSize = patch.size();
	Size imageSize = image.size();
	Rect searchArea = getROI(point, patchSize, imageSize, searchRadius);

	Mat search = image(searchArea);
	Mat result = Mat::zeros(Size(search.cols - patchSize.width + 1, search.rows - patchSize.height +1), CV_32FC1);

	matchTemplate(search, patch, result, CV_TM_CCORR_NORMED);
	
	// find maximum in result
	double minVal, maxVal;
	Point minLoc, maxLoc; // match = max
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	Point matchLoc = searchArea.tl() + maxLoc;
	
	return getROI(matchLoc, patchSize, imageSize, 0);
}

vector<Mat> getPatches (Mat& frame, vector<Rect> cars) {
	vector<Mat> ret;
	for (size_t i = 0; i < cars.size(); i++){
		ret.push_back(Mat());
		frame(cars[i]).copyTo(ret.back());
	}
	return ret;
}

void drawDetections (Mat& frame, vector<Rect> cars) {
	for( size_t i = 0; i < cars.size(); i++ ) {
		Point topLeft = Point(cars[i].x, cars[i].y);
		Point bottomRight = Point(cars[i].x+cars[i].width, cars[i].y+cars[i].height);
		rectangle(frame, topLeft, bottomRight, Scalar(255, 255, 255), 2, 8, 0);
	}
}
/* END CAR DETECTION CODE */





/*
 * START OPTICAL FLOW VECTORS CODE
 */

Rect getRoadRect(Mat image) 
{
	int width = image.cols;
	int height = image.rows;

	Rect road = Rect(width/6,height-50,width*2/3,50);
	//cout << road;
	return road;
}

Mat getOptFlowROI(Rect ROI, Mat flow)
{
	Mat flowROI;
	flow(ROI).copyTo(flowROI);
	return flowROI;
}

Point2d getRoadAvgVector(Rect road, Mat flow)
{
	float xa = 0;
	float ya = 0;
	int count = 0;
	int step = 16;


	int left_max = road.x + (road.width/3);
	int right_min = road.x + (road.width*2/3);

	for(int y = road.y; y < road.y+road.height; y += step)
	{
        for(int x = road.x; x < road.x+road.width; x += step)
        {
            //Get the flow vector as a Point2f object
            const Point2f& fxy = flow.at<Point2f>(y, x);
			
			if (x < left_max || x > right_min)
			{
				xa += fxy.x;
				ya += fxy.y;
				count++;
			}
        }
	}

	xa /= count;
	ya /= count;
	return Point(xa,ya);

}

vector<Point2d> gatherOptFlowVectors(vector<Rect> ROIs, Mat flow)
{
	vector<Point2d> flowROIs;
	for (size_t i = 0; i < ROIs.size(); i++) {
		Point2d avg = getAverageFlow(flow, ROIs[i]);
		flowROIs.push_back(avg);
		cout<<avg;
	}
	return flowROIs;
}

Point2d getAverageFlow(Mat flow, Rect ROI)
{
	float xa = 0;
	float ya = 0;
	int count = 0;
	int step = 16;
	int weight = 2;

	int min_weight_x;
	int max_weight_x;
	int min_weight_y;
	int max_weight_y;

	min_weight_x = ROI.width/3;
	max_weight_x = ROI.width*2/3;
	min_weight_y = ROI.height/3;
	max_weight_y = ROI.height*2/3;

	for(int y = ROI.y; y < ROI.y+ROI.height; y += step)
	{
        for(int x = ROI.x; x < ROI.x+ROI.width; x += step)
        {
            //Get the flow vector as a Point2f object
            const Point2f& fxy = flow.at<Point2f>(y, x);
			/*
			if (x >= min_weight_x && x <= max_weight_x && y >= min_weight_y && y <= max_weight_y) 
			{
				xa += weight*fxy.x;
				ya += weight*fxy.y;
				count += weight;
			}else
			{
				xa += fxy.x;
				ya += fxy.y;
				count++;
			}
			*/
			xa += fxy.x;
			ya += fxy.y;
			count++;
        }
	}

	xa /= count;
	ya /= count;

	return Point(xa,ya);
}

static void drawAverageFlowVectors(const Mat& flow, Mat& frame, vector<Rect> cars, vector<Point2d> avgs)
{
	for( size_t i = 0; i < cars.size(); i++ ) {
		Point middle = Point(cars[i].width/2, cars[i].height/2);
		Point frame_middle = Point(cars[i].x + middle.x, cars[i].y + middle.y);
		Point line_end = Point(frame_middle.x + avgs[i].x, frame_middle.y + avgs[i].y);
		line(frame, frame_middle, line_end, Scalar(255, 255, 255), 2);
	}
}

// copied from OpenCV sample fback.cpp
static void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step,
                    double, const Scalar& color)
{
    // Optical flow is stored as two-channel floating point array in a Mat
    // of type CV_32FC2
    for(int y = 0; y < cflowmap.rows; y += step)
	{
        for(int x = 0; x < cflowmap.cols; x += step)
        {
            //Get the flow vector as a Point2f object
            const Point2f& fxy = flow.at<Point2f>(y, x);

            // Draw a line from the image point using the flow vector
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)),
                 color);
        }
	}
}

// copied from OpenCV sample fback.cpp
static void drawOptFlowMapROI(const Mat& flow, Mat& cflowmap, Rect ROI, int step,
                    double, const Scalar& color)
{
    // Optical flow is stored as two-channel floating point array in a Mat
    // of type CV_32FC2
    for(int y = ROI.y; y < ROI.y+ROI.height; y += step)
	{
        for(int x = ROI.x; x < ROI.x+ROI.width; x += step)
        {
            //Get the flow vector as a Point2f object
            const Point2f& fxy = flow.at<Point2f>(y, x);

            // Draw a line from the image point using the flow vector
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)),
                 color);
        }
	}
}

void drawDetectionFlows (Mat& frame, Mat&flow, vector<Rect> cars, const Scalar& color) 
{
	for( size_t i = 0; i < cars.size(); i++ ) 
	{
		//circle(frame, Point(cars[i].x, cars[i].y), 40, color, -1);
		drawOptFlowMapROI(flow, frame, cars[i], 16, 1.5, CV_RGB(0, 255, 0));
	}
}

void opticalFlowMagnitudeAngle(const Mat& flow, Mat& magnitude, Mat& angle)
{
    if(magnitude.rows != flow.rows || magnitude.cols != flow.cols)
    {
        magnitude.create(flow.rows, flow.cols, CV_32FC1);
    }
    if(angle.rows != flow.rows || angle.cols != flow.cols)
    {
        angle.create(flow.rows, flow.cols, CV_32FC1);
    }

	Mat xy[2];
	split(flow, xy);
	cartToPolar(xy[0], xy[1], magnitude, angle, true);

}
/* END OPTICAL FLOW CODE*/






/*
 * START SPEED MATH CODE
 */
vector<float> getCarSpeeds(vector<Rect> cars, int counter, int frameNumber, Mat &frame, Mat &flow){

	Mat opticalFlow, magnitude,angle;
	opticalFlow = flow;
	//opticalFlowMagnitudeAngle(opticalFlow,magnitude,angle);

	double ratio = getCameraSpeed(counter,frame,frameNumber, opticalFlow);		 


	vector<float> speedLabels;
	for( size_t i = 0; i < cars.size(); i++ ){
		Point avgFlow = getAverageFlow(opticalFlow,cars[i]); // BREAKS HERE????
		speedLabels.push_back(abs(length(avgFlow.x, avgFlow.y))*ratio);
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
	while (size < 9) {
		ifs.get(temp,32,' ');
		GPS.push_back(atof(temp));
		//cout << GPS[size] << " + size: " << size << endl;
		ifs.get(temp, 2);
		size ++;
	}
	return length(GPS.at(6),GPS.at(7));
}

double getCameraSpeed(int &counter, Mat &frame,int frameNumber, Mat &flow){
	float cameraVelocity = 0.0;

	vector<float> GPSTime = getTimeStampsGPS();
	vector<float> FrameTime = getTimeStampsVideo();

	while(FrameTime.at(frameNumber) > GPSTime.at(counter)){
		cameraVelocity += getGPSVelocity(counter);
		counter++;
	}

	Rect frontOfCar = getRoadRect(frame);  //(shoudl be roughly the front of the car, say, middle fifth of the x, lowest quarter of the y

	Mat focRegion = flow(frontOfCar);

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