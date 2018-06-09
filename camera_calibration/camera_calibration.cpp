// camera_calibration.cpp : Defines the entry point for the console application.
//Finds the camera calibration matrix using a 6x9 calibration image.
//Runs the camera in a while loop once found and displays the undistorted images.

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
Size checkerboard = Size(6, 9);							//Creates a variable with the size of our calibration image in terms of internal corners
float square_size = 0.025f;								//Measured size of the side of one of the checkerboxs, in meters
int min_number_frames = 40;								//Must be > 0 (recommend higher than 20)
string output_file_name = "Camera_Calibration_01504282";//Output File name
int camera_id = 0;										// <---Camera ID goes here

void createKnownPoints(Size checkerboard, float square_size, vector<Point3f> &corners)
{
	//determines the "known" point locations (the internal corners of our calibration image)
	//based on the checkerboard size parameters and the measured square_size of the printed out cal image
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

		//save number of rows and columns
		out_stream << rows << endl;
		out_stream << columns << endl;

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

		//save number of rows and columns
		out_stream << rows << endl;
		out_stream << columns << endl;

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

bool load_camera_calibration(string name, Mat& camera_matrix, Mat& distance_coef)
{
	ifstream in_stream(name);
	if (in_stream)
	{
		uint16_t rows;
		uint16_t columns;

		in_stream >> rows;
		in_stream >> columns;

		camera_matrix = Mat(Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				in_stream >> read; 
				camera_matrix.at<double>(r, c) = read;
				cout << camera_matrix.at<double>(r, c) << "\n";
			}
		}

		in_stream >> rows;
		in_stream >> columns;

		distance_coef = Mat::zeros(rows, columns, CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				in_stream >> read;
				distance_coef.at<double>(r, c) = read;
				cout << distance_coef.at<double>(r, c) << "\n";

			}
		}

		in_stream.close();
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
	Mat undistorted;
	vector<vector<Point2f>> markerCorners, failedCorners;

	VideoCapture vid(camera_id);
	
	if (!vid.isOpened())
	{
		return 0;
	}

	int fps = 20;
	namedWindow("Camera", CV_WINDOW_AUTOSIZE);

	if (!load_camera_calibration(output_file_name, camera_matrix, distance_coef))
	{
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

			//wait for an input key for 50ms (based on fps of 20) 
			// " " = save image
			// "Entre"  = start calibration
			// " Esc" = exit program
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
					save_camera_calibration(output_file_name, camera_matrix, distance_coef);
				}
				break;

			case 27:
				//exit program on esc key
				return 0;
				break;

			}
		}
	}

	while (true)
	{
		vid.read(frame);
		undistort(frame, undistorted, camera_matrix, distance_coef);
		imshow("Camera", frame);
		imshow("Undistored", undistorted);

		//wait for an input key for 50ms (based on fps of 20) 
		// " Esc" = exit program
		char character = waitKey(1000 / fps);
		switch (character)
		{
		case 27:
			//exit on escape key
			return 0;
			break;
		}

	}

    return 0;
}