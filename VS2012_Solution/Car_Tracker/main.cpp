#include "stdafx.h"
#include "helpers.h"
#include "car-tracker.h"
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

int main(int argc, char* argv[]) {
	int frame_counter = 0;
	int frame_total = 300;
	String frame_folder = "./data/video/data/";
	String frame_filetype = ".png";

	int GPS_counter = 0;

	vector<Rect> cars; // stores ROI for detected cars
	vector<Rect> cars_backup; // stores backup in case no cars detected
	vector<Mat> prevFrame; // stores frames from last iteration for template tracking

	/* optical flow mats */
	Mat flow, cflow, frame_previous;
	vector<Mat> carOptFlow;

	while (frame_counter < frame_total) {

		Mat frame;
		String filename = getFilename(frame_folder, frame_filetype, frame_counter);
		frame = imread(filename, 0);
		
		frame.copyTo(cflow);
				
		if (frame_counter % 3 == 0) {
			cars_backup = cars;
			cars = detectCars(frame); // this is the feature detection tracking
			if (cars.empty()) {
				cars = cars_backup;
			}
		} else {
			for (size_t i = 0; i < cars.size(); i++) {
				cars[i] = trackTemplate(cars[i].tl(), prevFrame[i], frame, 100); // this is the updating-template tracking
			}
		}
		prevFrame = getPatches(frame, cars);
		
		drawDetections(frame, cars);

        if(frame_counter > 1)
        {
            calcOpticalFlowFarneback(frame_previous, frame, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

			// detect for cars
			drawDetectionFlows(frame, flow, cars, CV_RGB(0, 255, 0));
			vector<Point2d> averages = gatherOptFlowVectors(cars, flow);
			drawAverageFlowVectors(flow, frame, cars, averages);

			// detect for road
			Rect road = getRoadRect(frame);
			drawOptFlowMapROI(flow, frame, road, 16, 1.5, CV_RGB(0, 255, 0));
			Point2d roadAverages = getRoadAvgVector(road, flow);

        }

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
				speed_str << car_speed;
				std::string str = speed_str.str() + " m/s";
				putText(frame, str, topLeft, FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
			}
		}

		// Display results
		imshow("Frame", frame);
		if (waitKey(10) == 'q')
			break;
		
		// Prepare for next iteration
		frame_counter++;
		frame.copyTo(frame_previous);

	}
}