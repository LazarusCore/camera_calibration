#pragma once
//Define Class so that we can pass local commands and not have to resort to lots of arguments
// or modification of globals

#include <iostream>
#include <sstream>


class Workhorse
{
public:

	void write(FileStorage& fs) const                        //Write serialization for this class
	{
		fs << "{" 
			<< "Corners_Horizontal" << corners.width
			<< "Corners_Vertical" << corners.height
			<< "Square_Size" << squareSize

			<< "Export_DetectedPoints" << bwritePoints
			<< "Export_extrinsicParameters" << bwriteExtrinsics
			<< "Export_outputFileName" << outputFileName

			<< "Calibrate_NrOfFrameToUse" << nrFrames
			<< "Calibrate_FixAspectRatio" << aspectRatio
			<< "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
			<< "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

			<< "Show_UndistortedImage" << showUndistorsed

			<< "Input_FlipAroundHorizontalAxis" << flipVertical
			<< "Input_Delay" << delay
			<< "Camera_ID" << camera_id
			<< "}";
	}
	void read(const FileNode& node)                          //Read serialization for this class
	{
		node["Corners_Horizontal"] >> corners.width;
		node["Corners_Vertical"] >> corners.height;
		node["Square_Size"] >> squareSize;

		node["Export_DetectedPoints"] >> bwritePoints;
		node["Export_extrinsicParameters"] >> bwriteExtrinsics;
		node["Export_outputFileName"] >> outputFileName;

		node["Calibrate_NrOfFrameToUse"] >> nrFrames;
		node["Calibrate_FixAspectRatio"] >> aspectRatio;
		node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
		node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;

		node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
		node["Show_UndistortedImage"] >> showUndistorsed;
		node["Camera_ID"] >> camera_id;
		node["Input_Delay"] >> delay;
		initialiseCamera();
	}

	void initialiseCamera()
	{
		importCap.open(cameraID);	
	}


	Mat nextFrame()
	{
		Mat result;
		if (importCap.isOpened())
		{
			//create a temp Mat file, stick the latest captured file into it. Export that as the result of this function
			Mat temp;
			importCap >> temp;
			temp.copyTo(result);
		}

		return result;
	}

public:
	Size corners;					// The size of the board -> corners.width by corners.height
	float squareSize;				// The size of a square in your defined unit (point, millimeter,etc).
	int nrFrames;					// The number of frames to use from the input for calibration
	float aspectRatio;				// The aspect ratio
	int delay;						// For Video use
	bool bwritePoints;				// Write detected feature points
	bool bwriteExtrinsics;			// Write extrinsic parameters
	bool calibZeroTangentDist;		// Assume zero tangential distortion
	bool calibFixPrincipalPoint;	// Fix the principal point at the center
	bool flipVertical;				// Flip the captured images around the horizontal axis
	string outputFileName;			// The name of the file where to write
	bool showUndistorsed;			// Show undistorted images after calibration
	

	int cameraID;					//Camera Identifer
	vector<string> imageList;		//Storage location for images
	int image_list_pos;				//Way of tracking at which image you are up to
	VideoCapture importCap;

private:


};