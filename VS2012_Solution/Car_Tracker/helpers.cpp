/*
** These functions do not rely on OpenCV libraries.
*/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <vector>

using namespace std;

float length(float x, float y){
	return sqrtf(x*x+y*y);
}

// Read current image as video frame
// Every image files is 10 digits + filetype
// Currently supports up to 999,999 frames
string getFilename (string path, string ext, int counter) {
	string toReturn = path;
	if (counter < 10) {
			toReturn += "000000000";
		} else if (counter < 100) {
			toReturn += "00000000";
		} else if (counter < 1000) {
			toReturn += "0000000";
		} else if (counter < 10000) {
			toReturn += "000000";
		} else {
			toReturn += "00000";
		}
	toReturn += to_string(counter) + ext;
	return toReturn;
}

// TODO: Standard error detection functions