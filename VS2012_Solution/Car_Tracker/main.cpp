#include "stdafx.h"
#include "helpers.h"
#include "car-tracker.h"
#include "car-speed.h"
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

	vector<Rect> cars; // stores ROI for detected cars
	vector<Mat> prevFrame; // stores frames from last iteration for template tracking

	while (frame_counter < frame_total) {
		// Read current image as video frame
		// Every image files is 10 digits + filetype
		// Currently supports up to 999,999 frames
		String filename = frame_folder;		// TODO: take the file loader and make it a helper function
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
		
		drawDetections(frame, cars); // this is accessory a.k.a. delete whenevs yo

		imshow("Frame", frame);
		if (waitKey(10) == 'q')
			break;
		frame_counter++;
	}
}