// camera_calibration.cpp : Defines the entry point for the console application.
//


//Included libraries
#include "stdafx.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdio>
#include "workhorse.h"						//Class definition
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/calib3d.hpp>


using namespace cv;
using namespace std;


//Function Prototypes



//Globals
	//Program Settings

//Number of Internal corners in the calibration image. Couldnt be bothered creating a config file.
int horizontal = 9;							//Must be > 0, Number of internal corners wide on checkerboard image
int vertical = 6;							//Must be > 0, Number of internal corners high on the checkerboard image
int square_size = 25;						//Size of the side of one of the checkboxs, in mm
int min_number_frames = 15;					//Must be > 0
int num_images = 0;
string output_file_name;
int camera_id = 0;							// <---Camera ID goes here
enum {DETECTION = 0, CAPTURING = 1, CALIBRATING = 2};








int main(int argc, char* args[])
{
	//setup for image capture
	vector<vector<Point2f> > image_points;
	Mat calib_matrix;
	VideoCapture capture_input;


    return 0;
}




