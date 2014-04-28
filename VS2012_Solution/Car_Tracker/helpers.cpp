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

// TODO: Standard error detection functions