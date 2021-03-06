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

//Globals
//Program Settings
Size CHECKERBOARD = Size(6, 9);								//Creates a variable with the size of our calibration image in terms of internal corners
float SQUARE_SIZE = 0.025f;									//Measured size of the side of one of the checkerboxs, in meters
int MIN_NUMBER_FRAMES = 40;									//Must be > 0 (recommend higher than 20)
string OUTPUT_FILE_NAME = "Camera_Calibration_01504282";	//Output File name
int CAMERA_ID = 0;											// <---Camera ID goes here

															//Function Prototpyes
void createKnownPoints(Size CHECKERBOARD, float SQUARE_SIZE, vector<Point3f> &corners);
//Creates a file that contains the locations the corner points for a perfect calibration image

void getCheckerboardCorners(vector<Mat> images, vector<vector<Point2f>> & foundCorners);
//Attempts to identify the locations of the corners for the calibration image being imported by a camera

void calibrate_camera(vector<Mat> calibration_images, Size CHECKERBOARD, float SQUARE_SIZE, Mat& camera_matrix, Mat& distance_cooef);
//Uses the difference between the known and actual corners to derive a calibration matrix, and distance cooeficients

bool save_camera_calibration(string name, Mat camera_matrix, Mat distortion_coef);
//Saves the output into an extensionless file, returns true if saved correctly

bool load_camera_calibration(string name, Mat& camera_matrix, Mat& distortion_coef);
//Loads the output, returns true if loaded correctly

int main(int argc, char* args[])
{
	Mat frame;
	Mat frame_output;
	Mat camera_matrix = Mat::eye(3, 3, CV_64F);

	Mat distortion_coef;
	vector<Mat> stored_images;
	Mat undistorted;
	vector<vector<Point2f>> markerCorners, failedCorners;

	//Open the Camera, sets a desired FPS variable and opens up a window
	VideoCapture vid(CAMERA_ID);
	if (!vid.isOpened())
	{
		cerr << "Video not opened" << endl;
		return 0;
	}

	int fps = 20;
	namedWindow("Camera", CV_WINDOW_AUTOSIZE);

	//checks to see if we already have a calibration file (also loads it if we do)
	//runs through calibration if we dont
	if (!load_camera_calibration(OUTPUT_FILE_NAME, camera_matrix, distortion_coef))
	{
		/*if we cant load a file. at 20fps try each frame and see if we can find the chessboard points. if we can flag it
		as a potential working frame. Allow the user to designate a good frame every 50ms but looking at the output and then
		hitting space*/
		while (true)
		{
			vector<Vec2f> found_points;
			bool found = false;
			vid >> frame;
			//Can i successfully find the chessboard corners (true/false)?
			found = findChessboardCorners(frame, CHECKERBOARD, found_points, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
			//If i can, shift the current frame into frame_output
			frame.copyTo(frame_output);
			//draw the corners on the output frame
			drawChessboardCorners(frame_output, CHECKERBOARD, found_points, found);
			if (found)
			{
				//show the output frame with the chessboard corners
				imshow("Camera", frame_output);
			}
			else
				//show the frame without corners
				imshow("Camera", frame);

			//wait for an input key for 50ms (based on fps of 20) 
			// " " = save image
			// "Enter"  = start calibration
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
				//save the calibration matrix
				if (stored_images.size() > MIN_NUMBER_FRAMES)
				{
					calibrate_camera(stored_images, CHECKERBOARD, SQUARE_SIZE, camera_matrix, distortion_coef);
					save_camera_calibration(OUTPUT_FILE_NAME, camera_matrix, distortion_coef);
				}
				break;

			case 27:
				//exit program on esc key
				return 0;
				break;

			}
		}
	}


	//opens a second window. displays the current frame in the first, undistorts it, and displays in second
	while (true)
	{
		namedWindow("Undistorted", CV_WINDOW_AUTOSIZE);
		vid >> frame;
		undistort(frame, undistorted, camera_matrix, distortion_coef);
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

void createKnownPoints(Size CHECKERBOARD, float SQUARE_SIZE, vector<Point3f> &corners)
{
	//determines the "known" point locations (the internal corners of our calibration image)
	//based on the checkerboard size parameters and the measured SQUARE_SIZE of the printed out cal image
	for (int i = 0; i < CHECKERBOARD.height; i++)
	{
		for (int j = 0; j < CHECKERBOARD.width; j++)
		{
			corners.push_back(Point3f(j * SQUARE_SIZE, i * SQUARE_SIZE, 0.0f));
		}
	}
}

void getCheckerboardCorners(vector<Mat> images, vector<vector<Point2f>> & foundCorners)
{
	//tries to determine the "actual" corners for our calibration image
	//iterates through the images we have stored, and grabs a list of found corners
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

void calibrate_camera(vector<Mat> calibration_images, Size CHECKERBOARD, float SQUARE_SIZE, Mat& camera_matrix, Mat& distortion_coef)
{
	//starts calibrating based on "known" and "actual" points
	vector<vector<Point2f>> checkerboard_found_points;
	getCheckerboardCorners(calibration_images, checkerboard_found_points);

	vector<vector<Point3f>> world_corner_points(1);

	createKnownPoints(CHECKERBOARD, SQUARE_SIZE, world_corner_points[0]);
	vector<Mat> rvec, tvec;
	distortion_coef = Mat::zeros(8, 1, CV_64F);
	//run openCV calibrateCamera
	calibrateCamera(world_corner_points, checkerboard_found_points, CHECKERBOARD, camera_matrix, distortion_coef, rvec, tvec);

}

bool save_camera_calibration(string name, Mat camera_matrix, Mat distortion_coef)
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

		rows = distortion_coef.rows;
		columns = distortion_coef.cols;

		//save number of rows and columns
		out_stream << rows << endl;
		out_stream << columns << endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c <columns; c++)
			{
				double value = distortion_coef.at<double>(r, c);
				out_stream << value << endl;
			}
		}

		out_stream.close();
		return true;

	}
	return false;
}

bool load_camera_calibration(string name, Mat& camera_matrix, Mat& distortion_coef)
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

		distortion_coef = Mat::zeros(rows, columns, CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				in_stream >> read;
				distortion_coef.at<double>(r, c) = read;
				cout << distortion_coef.at<double>(r, c) << "\n";

			}
		}

		in_stream.close();
		return true;
	}
	return false;
}