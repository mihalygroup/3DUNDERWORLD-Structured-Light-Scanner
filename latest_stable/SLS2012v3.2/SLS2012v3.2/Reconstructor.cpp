//------------------------------------------------------------------------------------------------------------
//* Copyright � 2010-2013 Immersive and Creative Technologies Lab, Cyprus University of Technology           *
//* Link: http://ict.cut.ac.cy                                                                               *
//* Software developer(s): Kyriakos Herakleous                                                               *
//* Researcher(s): Kyriakos Herakleous, Charalambos Poullis                                                  *
//*                                                                                                          *
//* This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.*
//* Link: http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US                                        *
//------------------------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include <opencv2/imgproc.hpp>
#include "Reconstructor.h"
#include <filesystem>


Reconstructor::Reconstructor(int numOfCams_)
{
	numOfCams = numOfCams_;
	pathSet=false;

	shadowMask = NULL;

	decRows = NULL;
	decCols = NULL;

	points3DProjView = NULL;

	autoContrast_ = false;
	saveAutoContrast_ = false;
	saveShadowMask_ = false;
	

	cameras = new  VirtualCamera[numOfCams];

	camsPixels = new cv::vector<cv::Point>*[numOfCams];
	camFolder = new std::string[numOfCams];
	imgPrefix = new std::string[numOfCams];
	imgSuffix = new std::string[numOfCams];
	pathSet = new bool[numOfCams];

	for(int i=0; i< numOfCams; i++)
	{
		camsPixels[i] = NULL;
		camFolder[i] = "";
		imgPrefix[i] = "";
		imgSuffix[i] = "";
		pathSet[i] = false;
	}
}


Reconstructor::~Reconstructor(void)
{

	unloadCamImgs();
	
	if(points3DProjView)
		delete points3DProjView ;

}

void Reconstructor::enableAutoContrast()
{
	autoContrast_ = true;
}

void Reconstructor::disableAutoContrast()
{
	autoContrast_ = false;
}

void Reconstructor::enableSavingAutoContrast()
{
	saveAutoContrast_ = true;
}

void Reconstructor::disableSavingAutoContrast()
{
	saveAutoContrast_ = false;
}

void Reconstructor::enableSavingShadowMask()
{
	saveShadowMask_ = true;
}

void Reconstructor::disableSavingShadowMask()
{
	saveShadowMask_ = false;
}
void Reconstructor::enableRaySampling()
{
	raySampling_ = true;
}

void Reconstructor::disableRaySampling()
{
	raySampling_ = false;
}

void Reconstructor::setBlackThreshold(int val)
{
	blackThreshold = val;
}

void Reconstructor::setWhiteThreshold(int val)
{
	whiteThreshold = val;
}


void Reconstructor::decodePaterns()
{
	auto start = std::chrono::system_clock::now();
	int w=camera->width;
	int h=camera->height;


	cv::Point projPixel;
	
	for(int i=0; i<w; i++)
	{
		for(int j=0; j<h; j++)
		{
			int current = j + (i * h);
			int total = w * h;
			Utilities::LogProgress("Decoding Patterns", current, total);

			//if the pixel is not shadow reconstruct
			if(shadowMask.at<uchar>(j,i))
			{

				//get the projector pixel for camera (i,j) pixel
				bool error = getProjPixel(i,j,projPixel);

				if(error)
				{
					shadowMask.at<uchar>(j,i)=0;
					continue;
				}

				camPixels[ac(projPixel.x,projPixel.y)].push_back(cv::Point(i,j));

			}
		}
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << " ... done [" << diff.count() << "s]" << std::endl;
}


void Reconstructor::loadCameras()
{
	for(int i=0; i<numOfCams; i++)
	{
		std::string path;
		
		path = camFolder[i];
		path += "input/cam_matrix.txt";
		cameras[i].loadCameraMatrix(path.c_str());

		path = camFolder[i];
		path += "input/cam_distortion.txt";
		cameras[i].loadDistortion(path.c_str());

		path = camFolder[i];
		path += "input/cam_rotation_matrix.txt";
		cameras[i].loadRotationMatrix(path.c_str());

		path = camFolder[i];
		path += "input/cam_trans_vectror.txt";
		cameras[i].loadTranslationVector(path.c_str());

		cameras[i].height=0;
		cameras[i].width =0;
	}

}

//load camera images
void Reconstructor::loadCamImgs( std::string folder,std::string prefix,std::string suffix)
{
	auto start = std::chrono::system_clock::now();
	cv::Mat tmp;
	
	if(!camImgs.empty())
		unloadCamImgs();

	for(int i=0; i<numberOfImgs;i++)
	{
		Utilities::LogProgress("Load Camera Image", i, numberOfImgs);

		std::stringstream path;

		path << _getcwd(NULL, 0) << "/" << folder.c_str();
		if (i < 10)
			path << "0";
		path << prefix.c_str() << i << suffix.c_str();
		
		tmp.release();
		tmp = cv::imread(path.str().c_str());
		
		if(tmp.empty())
		{
			std::cout << "Error loading cam image [" << path.str().c_str() << "]" << std::endl;
			getch();
			exit(-1);
		}

		//auto contrast
		if(autoContrast)
		{
			Utilities::autoContrast((cv::Mat)tmp, (cv::Mat&)tmp);

			if(saveAutoContrast)
			{
				std::stringstream p;
				p<<folder.c_str() << "AutoContrastSave/" << prefix.c_str() << i << suffix.c_str();
				std::cout << "Save Auto Contrast to [" << p.str().c_str() << "]" << std::endl;

				cv::imwrite(p.str().c_str(),tmp);
			}
		}
		
		if(i==0)
		{
			color = tmp;
		}
		cv::cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);

		camImgs.push_back(tmp);
	}

	if(camera->width==0)
	{
		camera->height=camImgs[0].rows;
		camera->width =camImgs[0].cols;
	}

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << " ... done [" << diff.count() << "s]" << std::endl;
}

//unload camera images
void Reconstructor::unloadCamImgs()
{

	if(camImgs.size())
	{
		for(int i=0; i<numberOfImgs; i++)
		{
			camImgs[i].release();
		}
	}
	
	color.release();

	camImgs.clear();	
}



void Reconstructor::computeShadows()
{
	auto start = std::chrono::system_clock::now();

	int w = camera->width;
	int h = camera->height;

	shadowMask.release();

	shadowMask = cv::Mat(h,w,CV_8U,cv::Scalar(0));

	for(int i=0; i<w; i++)
	{
		for(int j=0; j<h; j++)
		{

			int current = j + (i * h);
			int total = w * h;
			Utilities::LogProgress("Estimating Shadows", current, total);

			float blackVal, whiteVal;

			blackVal  = (float) Utilities::matGet2D( camImgs[1], i, j);
			whiteVal  = (float) Utilities::matGet2D( camImgs[0], i, j);

			if(whiteVal - blackVal > blackThreshold)
			{
				Utilities::matSet2D(shadowMask,i,j,1);
			}
			else
			{
				Utilities::matSet2D(shadowMask,i,j,0);
			}
		}
	}

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << " ... done [" << diff.count() << "s]" << std::endl;
}



void Reconstructor::runReconstruction()
{
	for(int i=0; i< numOfCams; i++)
	{
		if(cameras[i].distortion.empty())
		{
			std::cout<<"Camera "<<i<< "is not set." << std::endl;
			exit(-1);
		}

		if(pathSet[i] == false)
		{
			std::cout<<"Image path for camera "<< i <<" is not set." << std::endl;
			exit(-1);
		}
	}

	GrayCodes grays(proj_w,proj_h);

	numOfColBits = grays.getNumOfColBits();
	numOfRowBits = grays.getNumOfRowBits();
	numberOfImgs = grays.getNumOfImgs();
	

	for(int i=0; i < numOfCams; i++)
	{

		//findProjectorCenter();
		cameras[i].position = cv::Point3f(0,0,0);
		cam2WorldSpace(cameras[i],cameras[i].position);
	
		camera = &cameras[i];
		camsPixels[i] = new cv::vector<cv::Point>[proj_h*proj_w];
		camPixels = camsPixels[i];

		loadCamImgs(camFolder[i],imgPrefix[i],imgSuffix[i]);

		colorImgs.push_back(cv::Mat());
		colorImgs[i] = color;
		computeShadows();
		if(saveShadowMask_)
		{
			std::stringstream path;
			path<<"cam"<<i+1<<"Mask.png";
			saveShadowImg(path.str().c_str());
			std::cout << "Save Shadow Image to [" << path.str().c_str() << "]" << std::endl;

		}
		
		decodePaterns();

		unloadCamImgs();
	}
	
	//reconstruct 
	points3DProjView = new PointCloudImage( proj_w, proj_h , true );
	
	for(int i = 0; i < numOfCams; i++)
	{
		for(int j=i+1; j< numOfCams; j++)
			triangulation(camsPixels[i],cameras[i],camsPixels[j],cameras[j],i,j);
	}
}

//convert a point from camera to world space
void Reconstructor::cam2WorldSpace(VirtualCamera cam, cv::Point3f &p)
{
	
	cv::Mat tmp(3,1,CV_32F);
	cv::Mat tmpPoint(3,1,CV_32F);

	tmpPoint.at<float>(0) = p.x;
	tmpPoint.at<float>(1) = p.y;
	tmpPoint.at<float>(2) = p.z;

	tmp = -cam.rotationMatrix.t() * cam.translationVector ;
	tmpPoint = cam.rotationMatrix.t() * tmpPoint;

	p.x = tmp.at<float>(0) + tmpPoint.at<float>(0);
	p.y = tmp.at<float>(1) + tmpPoint.at<float>(1);
	p.z = tmp.at<float>(2) + tmpPoint.at<float>(2);
	
}


//for a (x,y) pixel of the camera returns the corresponding projector pixel
bool Reconstructor::getProjPixel(int x, int y, cv::Point &p_out)
{
	cv::vector<bool> grayCol;
	cv::vector<bool> grayRow;

	bool error = false;
	//int error_code = 0;
	int xDec,yDec;

	//prosses column images
	for(int count=0; count<numOfColBits; count++)
	{
		//get pixel intensity for regular pattern projection and it's inverse 
		double val1, val2;
		val1 = Utilities::matGet2D(camImgs[count * 2 + 2   ],x,y);
		val2 = Utilities::matGet2D(camImgs[count * 2 + 2 +1],x,y);
		
		//check if intensity deference is in a valid rage
		if(abs(val1-val2) < whiteThreshold )
			error=true;

		//determine if projection pixel is on or off
		if(val1>val2)
			grayCol.push_back(1);
		else
			grayCol.push_back(0);

	}

	xDec = GrayCodes::grayToDec(grayCol);

	//prosses row images
	for(int count=0; count < numOfRowBits; count++)
	{

		double val1, val2;

		val1 = Utilities::matGet2D(camImgs[count*2+2+numOfColBits*2],x,y);
		val2 = Utilities::matGet2D(camImgs[count*2+2+numOfColBits*2+1],x,y);

		if(abs(val1-val2) < whiteThreshold )  //check if the difference between the values of the normal and it's inverce projection image is valid
			error = true;

		if(val1 > val2)
			grayRow.push_back(1);
		else
			grayRow.push_back(0);

	}
	
	//decode
	yDec = GrayCodes::grayToDec(grayRow); 

	if((yDec > proj_h || xDec > proj_w))
	{
		error == true;	
	}

	p_out.x = xDec;
	p_out.y = yDec;

	return error;
}

void Reconstructor::setImgPath(const char folder[],const char prefix[],const char suffix[], int cam_no )
{
	
	camFolder[cam_no] = folder;
	imgPrefix[cam_no] = prefix;
	imgSuffix[cam_no] = suffix;

	pathSet[cam_no] = true;
}


void Reconstructor::triangulation(cv::vector<cv::Point> *cam1Pixels, VirtualCamera camera1, cv::vector<cv::Point> *cam2Pixels, VirtualCamera camera2, int cam1index, int cam2index)
{
	auto start = std::chrono::system_clock::now();

	int w = proj_w;
	int h = proj_h;
	//start reconstraction
	int load=0;

	//reconstraction for every projector pixel
	for(int i=0; i<w; i++)
	{
		for(int j=0; j<h; j++)
		{			
			Utilities::LogProgress("Computing 3D Cloud :", j + (i * h), w * h);

			cv::vector<cv::Point> cam1Pixs,cam2Pixs;

			cam1Pixs = cam1Pixels[ac(i,j)];
			cam2Pixs = cam2Pixels[ac(i,j)];

			cv::Point3f reconstructedPoint(0,0,0);

			if( cam1Pixs.size() == 0 || cam2Pixs.size() == 0)
				continue;

			cv::Vec3f color1,color2;

			for(int c1=0; c1 < cam1Pixs.size(); c1++)
			{

				cv::Point2f camPixelUD = Utilities::undistortPoints(cv::Point2f(cam1Pixs[c1].x,cam1Pixs[c1].y),camera1);//camera 3d point p for (i,j) pixel
				cv::Point3f cam1Point = Utilities::pixelToImageSpace(camPixelUD,camera1); //convert camera pixel to image space
				cam2WorldSpace(camera1, cam1Point);

				cv::Vec3f ray1Vector = (cv::Vec3f) (camera1.position - cam1Point); //compute ray vector 
				Utilities::normalize(ray1Vector);

				//get pixel color for the first camera view
				color1 = Utilities::matGet3D( colorImgs[cam1index], cam1Pixs[c1].x, cam1Pixs[c1].y);

				for(int c2=0; c2 < cam2Pixs.size(); c2++)
				{

					camPixelUD = Utilities::undistortPoints(cv::Point2f(cam2Pixs[c2].x,cam2Pixs[c2].y),camera2);//camera 3d point p for (i,j) pixel

					cv::Point3f cam2Point = Utilities::pixelToImageSpace(camPixelUD,camera2); //convert camera pixel to image space
					cam2WorldSpace(camera2, cam2Point);

					cv::Vec3f ray2Vector = (cv::Vec3f) (camera2.position - cam2Point); //compute ray vector 
					Utilities::normalize(ray2Vector);
					
					cv::Point3f interPoint;
					
					bool ok = Utilities::line_lineIntersection(camera1.position,ray1Vector,camera2.position,ray2Vector,interPoint);


					if (!ok) {
						continue;
						std::cout << std::endl << "Unexpected Error At " << __FILE__  << ":" << __LINE__ << " ignored and continue with next Camera Pixel" << std::endl;
					}
					
					//get pixel color for the second camera view
					color2 = Utilities::matGet3D( colorImgs[cam2index], cam2Pixs[c2].x, cam2Pixs[c2].y);

					points3DProjView->addPoint(i,j,interPoint, (color1 + color2)/2);	

				}

			}

		}
	}
	

	// system("cls");
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << " ... done [" << diff.count() << "s]" << std::endl;
}


void Reconstructor::saveShadowImg(const char path[])
{
	if(!shadowMask.empty())
	{
		cv::threshold( shadowMask, shadowMask, 0, 255, cv::THRESH_BINARY );
		cv::imwrite(path,shadowMask);
	}
}