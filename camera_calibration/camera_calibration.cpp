// camera_calibration.cpp : Defines the entry point for the console application.
//

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
//Included libraries
#include "stdafx.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;


//Function Prototypes
void createKnownPoints(Size checkerboard, float square_size, vector<Point3f> &corners);


//Globals
	//Program Settings

//Number of Internal corners in the calibration image. Couldnt be bothered creating a config file.
Size checkerboard = Size(6, 9);				//Creates a variable with the size of our calibration image in terms of internal corners
float square_size = 0.025f;				//Size of the side of one of the checkboxs, in meters
int min_number_frames = 40;					//Must be > 0
int num_frames = 0;
string output_file_name;
int camera_id = 0;							// <---Camera ID goes here
enum {DETECTION = 0, CAPTURING = 1, CALIBRATING = 2};


void createKnownPoints(Size checkerboard, float square_size, vector<Point3f> &corners)
{
	//determines the "known" point locations (the internal corners of our calibration image)
	for (int i = 0; i < checkerboard.height; i++)
	{
		for (int j = 0; j < checkerboard.width; j++)
		{
			corners.push_back(Point3f(j * square_size, i * square_size, 0.0f));
		}
	}
}


void getCheckerboardCorners(vector<Mat> images, vector<vector<Point2f>> & foundCorners)
{
	//tries to determine the "actual" corners for our calibration image
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		vector<Point2f> pointBuffer;
		bool found = findChessboardCorners(*iter, Size(6, 9), pointBuffer, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (found)
		{
			foundCorners.push_back(pointBuffer);
		}
		
	}
}

void calibrate_camera(vector<Mat> calibration_images, Size checkerboard, float square_size, Mat& camera_matrix, Mat disctance_cooef)
{
	//starts calibrating based on "known" and "actual" points
	vector<vector<Point2f>> checkerboard_found_points;
	getCheckerboardCorners(calibration_images, checkerboard_found_points);

	vector<vector<Point3f>> world_corner_points(1);

	createKnownPoints(checkerboard, square_size, world_corner_points[0]);
	vector<Mat> rvec, tvec;
	disctance_cooef = Mat::zeros(8, 1, CV_64F);
	//run openCV calibrateCamera
	calibrateCamera(world_corner_points, checkerboard_found_points, checkerboard, camera_matrix, disctance_cooef, rvec, tvec);

}

bool save_camera_calibration(string name, Mat camera_matrix, Mat distance_coef)
{
	ofstream out_stream(name);
	if (out_stream)
	{
		uint16_t rows = camera_matrix.rows;
		uint16_t columns = camera_matrix.cols;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c <columns; c++)
			{
				double value = camera_matrix.at<double>(r, c);
				out_stream << value << endl;
			}
		}

		rows = distance_coef.rows;
		columns = distance_coef.cols;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c <columns; c++)
			{
				double value = distance_coef.at<double>(r, c);
				out_stream << value << endl;
			}
		}

		out_stream.close();
		return true;

	}
	return false;
}

int main(int argc, char* args[])
{
	Mat frame;
	Mat frame_output;
	Mat camera_matrix = Mat::eye(3, 3, CV_64F);

	Mat distance_coef;
	vector<Mat> stored_images;

	vector<vector<Point2f>> markerCorners, failedCorners;

	VideoCapture vid(camera_id);
	if (!vid.isOpened())
	{
		return 0;
	}

	int fps = 20;

	namedWindow("Camera", CV_WINDOW_AUTOSIZE);

	while (true)
	{
		vector<Vec2f> found_points;
		bool found = false;

		found = findChessboardCorners(frame, checkerboard, found_points, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(frame_output);
		drawChessboardCorners(frame_output, checkerboard, found_points, found);
		if (found)
		{
			imshow("Camera", frame_output);
		}
		else
			imshow("Camera", frame);
		char character = waitKey(1000 / fps);

		switch (character)
		{
			case ' ': 
				//save image on space input
				if (found)
				{
					Mat temp;
					frame.copyTo(temp);
					stored_images.push_back(temp);
				}
				break;

			case 13:
				//start calibration on Enter key
				//check if there are enough images first
				if (stored_images.size() > 15)
				{
					calibrate_camera(stored_images, checkerboard, square_size, camera_matrix, distance_coef);
					save_camera_calibration("Camera_Calibration_01504282", camera_matrix, distance_coef);
				}
				 break;

			case 27:
				//exit program on esc key
				return 0;
				break;

		}
	}

    return 0;
}




