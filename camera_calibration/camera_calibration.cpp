// camera_calibration.cpp : Defines the entry point for the console application.
//


//Included libraries
#include "stdafx.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdio>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/calib3d.hpp>


using namespace cv;
using namespace std;


//Function Prototypes



//Globals
	//Program Settings

//Number of Internal corners in the calibration image. Couldnt be bothered creating a config file.
int ernal_corners_horizontal = 9;			//Must be > 0
int ernal_corners_vertical = 6;				//Must be > 0
int square_size = 50;
int min_number_frames = 15;					//Must be > 0
int num_images = 0;
string output_file_name;
enum {DETECTION = 0, CAPTURING = 1, CALIBRATING = 2};



//Define Class so that we can pass local commands and not have to resort to lots of arguments
// or modification of globals

class base
{
public:


	Mat next_frame(VideoCapture * capture_input)
	{
		Mat frame;
		if (capture_input->isOpened)
		{
			//take the frame being viewed and put it into a temp file. Copy this into Mat frame
			Mat temp;
			*capture_input >> temp;
			temp.copyTo(frame);
		}
		else if (num_images < images)
			return Mat(frame);
	}
	~base();

private:

};

base::base()
{
}

base::~base()
{
}





Mat calib_image(1830, 1330, CV_8UC1);



int main(int argc, char* args[])
{
	//setup for image capture
	vector<vector<Point2f> > image_points;
	Mat calib_matrix;
	VideoCapture capture_input;


    return 0;
}




