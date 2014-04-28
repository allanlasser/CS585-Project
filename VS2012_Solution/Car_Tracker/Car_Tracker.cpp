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
vector<float> getCarSpeeds(vector<Rect> cars, int &counter, int frame);
float getCameraSpeed(int &counter, int frame);
vector<float> getTimeStampsGPS();
vector<float> getTimeStampsVideo();
/*end camera speed stuff*/

/* optical flow stuff */
void opticalFlowMagnitudeAngle(const Mat& flow, Mat& magnitude, Mat& angle);
static void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step,
                    double, const Scalar& color);
vector<Mat> gatherOptFlowVectors(vector<Rect> ROIs, Mat flow);
Mat getOptFlowROI(Rect ROI, Mat flow);
/* end optical flow stuff */

int main(int argc, char* argv[]) {
	int frame_counter = 0;
	int frame_total = 300;
	String frame_folder = "./data/video/data/";
	String frame_filetype = ".png";

	vector<Rect> cars; // stores ROI for detected cars
	vector<Mat> prevFrame; // stores frames from last iteration for template tracking

	/* optical flow mats */
	Mat flow, cflow, magnitude, angle;
	Mat gray_old, gray_new;
	vector<Mat> carOptFlow;
	namedWindow( "flow", 1 );

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
		if (frame_counter % 7 == 0) {
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

			drawOptFlowMap(flow, cflow, 16, 1.5, CV_RGB(0, 255, 0));
	        imshow("flow", cflow);

        }
		/* end optical flow calculation */



		imshow("Frame", frame);
		if (waitKey(10) == 'q')
			break;
		
		gray_new.copyTo(gray_old); // for optical flow
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

/*
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

/*
 * Calculating Optical Flow Vectors
 */

/*
calcOpticalFlowVector(ROI, frame, frame) {
}

getAllOpticalFlow(Vector<ROIs>, frame, frame) {
}
*/

Mat getOptFlowROI(Rect ROI, Mat flow)
{
	Mat flowROI;
	flow(ROI).copyTo(flowROI);
	return flowROI;
}

vector<Mat> gatherOptFlowVectors(vector<Rect> ROIs, Mat flow)
{
	vector<Mat> flowROIs;
	for (size_t i = 0; i < ROIs.size(); i++) {
		flowROIs.push_back(getOptFlowROI(ROIs[i], flow));
	}
	return flowROIs;
}

Point2d getAverageFlow(Mat flowROI)
{
	float x = 0;
	float y = 0;
	int count = 0;

	int channels = flowROI.channels();
    for(int row=0; row<flowROI.rows; row++)
    {
        //the index for the start of the row is only calculated once per row
        int rowIndex = row*flowROI.step[0];
        for(int col=0; col<flowROI.cols; col++)
        {
            int index = rowIndex + col*channels + 0; // channel 0 = x, channel 1 = y
			x += flowROI.data[index];
			y += flowROI.data[index+1];
			count++;
        }
    }

	x /= count;
	y /= count;

	return Point(x,y);

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
            //circle(cflowmap, Point(x,y), 2, color, -1);
        }
	}
}


//Required: Using the drawOptFlowMap function as inspiration, compute the magnitude and angle
//of every point in the flow field so it can be visualized 
//Your angles should be in degrees (atan2 returns radians) and they should go from -180 to 180.
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

//Given: Function to visualize the magnitude and orientation of the flow field
//The magnitude needs to be normalized for visualization. On my test video, the flow was not more than 10.
//You may want to adjust the maximum flow or computer it automatically
void visualizeFlow(const Mat& magnitude, const Mat& angle, Mat& magnitude8UC1, Mat& angle8UC3)
{
    if(magnitude8UC1.rows != magnitude.rows || magnitude8UC1.cols != magnitude.cols || magnitude8UC1.type() != CV_8UC3)
    {
        magnitude8UC1.create(magnitude.rows, magnitude.cols, CV_8UC1);
    }
    if(angle8UC3.rows != angle.rows || angle8UC3.cols != angle.cols || angle8UC3.type() != CV_8UC3)
    {
        angle8UC3.create(angle.rows, angle.cols, CV_8UC3);
    }

    double minMag, maxMag;
    //minMaxLoc(magnitude, &minMag, &maxMag); //to automatically compute the maximum
    minMag = 0;
    maxMag = 10; // your mileage my vary

    Mat tempMagnitude;
    tempMagnitude = (magnitude - minMag)/(maxMag-minMag)*255;
    tempMagnitude.convertTo(magnitude8UC1, CV_8UC1);

    //Going to create a rainbow image where the hue correspondes to the angle
    for(int y=0; y<angle.rows; y++)
    {
        unsigned char* rowPtr = angle8UC3.ptr<unsigned char>(y);
        for(int x=0; x<angle.cols; x++)
        {
            int index = x*3;
            rowPtr[index] = (angle.at<float>(y,x)+180)/2.0;
            rowPtr[index+1] = 128; //don't use full saturation or you'll go blind.
            rowPtr[index+2] = 196;
        }
    }
    cvtColor(angle8UC3, angle8UC3, CV_HSV2BGR);
}