//------------------------------------------------------------------------------------------------------------
//* Copyright � 2010-2013 Immersive and Creative Technologies Lab, Cyprus University of Technology           *
//* Link: http://ict.cut.ac.cy                                                                               *
//* Software developer(s): Kyriakos Herakleous                                                               *
//* Researcher(s): Kyriakos Herakleous, Charalambos Poullis                                                  *
//*                                                                                                          *
//* This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.*
//* Link: http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US                                        *
//------------------------------------------------------------------------------------------------------------

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/core/types_c.h>
#include "VirtualCamera.h"
#include "GrayCodes.h"
#include <conio.h>
#include <direct.h>

#define STRICT
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;

namespace cv
{
	using std::vector;
};

class Utilities
{

	public:
		Utilities(void);
		~Utilities(void);
		static bool				XOR(bool val1, bool val2);
		static void				normalize(cv::Vec3f &vec);
		static void				normalize3dtable(double vec[3]);
		static void				pixelToImageSpace(double p[3], cv::Scalar fc, cv::Scalar cc);
		static cv::Point3f		pixelToImageSpace(cv::Point2f p, VirtualCamera cam);
		static cv::Point2f		undistortPoints( cv::Point2f p, VirtualCamera cam);
		static cv::Scalar			planeRayInter(cv::Scalar planeNormal,cv::Scalar planePoint, cv::Scalar rayVector, cv::Scalar rayPoint );
		static double			matGet2D(cv::Mat m, int x, int y);
		static double			matGet3D(cv::Mat m, int x, int y, int i);
		static cv::Vec3d		matGet3D(cv::Mat m, int x, int y);
		static void				matSet3D(cv::Mat m, int x, int y, cv::Vec3d);
		static void				matSet3D(cv::Mat m, int x, int y,int i, double val);
		static void				matSet2D(cv::Mat m, int x, int y, double val);
		static void				autoContrast(cv::Mat img_in, cv::Mat &img_out);
		static void				autoContrast(cv::Mat &img_in, cv::Mat &img_out);
		static void				exportMat(const char *path, cv::Mat m);
		static bool				line_lineIntersection(cv::Point3f p1, cv::Vec3f v1, cv::Point3f p2,cv::Vec3f v2,cv::Point3f &p);
		static int				accessMat(cv::Mat m, int x, int y, int i);
		static int				accessMat(cv::Mat m, int x, int y);
		static void				folderScan(const char *path);

		// Log
		static void LogProgress(std::string msg, int current, int total);
};

