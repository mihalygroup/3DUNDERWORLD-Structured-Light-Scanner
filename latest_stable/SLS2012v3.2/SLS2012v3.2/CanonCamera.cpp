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
#include <opencv2/core/core_c.h>
#include "CanonCamera.h"


int CanonCamera::numOfCameras = 0;

CanonCamera::CanonCamera(void)
{
	liveView = false;
	camID = numOfCameras;
	numOfCameras++;

	windowName = "Camera ";
	windowName += '1' + camID;
	windowName += " Window";

	EdsCameraListRef cameraList = NULL;
	
	EdsGetCameraList(&cameraList);

	EdsUInt32	 camCount = 0;
	EdsGetChildCount(cameraList, &camCount);


	if(camCount==0)
	{
		std::cout<<"No Camera Found"<<". Press Any Key to Exit.";
		getch();
		exit(-1);
	}
	else
	{
		std::cout<<camCount<<" Camera(s) found!\n";
	} 

	EdsError err = EDS_ERR_OK;

	err = EdsGetChildAtIndex(cameraList , camID , &camera);

	//Release camera list
	if(cameraList != NULL)
	{
		EdsRelease(cameraList);
	}

	detectedCams=camCount;

	//open sesion
	err=EdsOpenSession(camera);

	if(err!=EDS_ERR_OK)
	{
		std::cout<<"Problem with Camera connection"<<". Press Any Key to Exit";
		getch();
		exit(-1);
	}
}

CanonCamera::~CanonCamera(void)
{
	if(liveView)
		endLiveview();
	EdsCloseSession(camera);
}

EdsError CanonCamera::startLiveview()
{
	cv::namedWindow(windowName.c_str(),cv::WINDOW_AUTOSIZE);
	cv::resizeWindow(windowName.c_str(),640,480);

	EdsError err = EDS_ERR_OK;

	EdsUInt32 device;
	err = EdsGetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device );
	
	if(err == EDS_ERR_OK)
	{
		device |= kEdsEvfOutputDevice_PC;
		err = EdsSetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device);
	}

	liveView = true;
	return err;
}


EdsError CanonCamera::endLiveview()
{
	EdsError err = EDS_ERR_OK;

	// Get the output device for the live view image
	EdsUInt32 device;
	err = EdsGetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device );
	
	if(err == EDS_ERR_OK)
	{
		device &= ~kEdsEvfOutputDevice_PC;
		err = EdsSetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device);
	}

	cv::destroyWindow(windowName.c_str());

	liveView=false;

	return err;
}



void CanonCamera::UpdateView()
{

	if(!liveView)
		return;

    EdsError err = EDS_ERR_OK;
    EdsStreamRef stream = NULL;
    EdsEvfImageRef evfImage = NULL;

    // Create memory stream.
    err = EdsCreateMemoryStream(0, &stream);
  
	if(err != EDS_ERR_OK)
    {
        printf("Error in EdsCreateMemoryStream: 0x%X\n", err);
    }    

    err = EdsCreateEvfImageRef(stream, &evfImage);
    if(err != EDS_ERR_OK)
    {
        printf("Error in EdsCreateEvfImageRef: 0x%X\n", err);  
    }    

	
	bool wait = true;

    int numTries = 0;
    
	while(wait && numTries++ < 20)
    {
        err = EdsDownloadEvfImage(camera, evfImage);
        if(err != EDS_ERR_OK && err != EDS_ERR_OBJECT_NOTREADY)
        {
            printf("Error in EdsDownloadEvfImage: 0x%X\n", err);

        }
        if(err == EDS_ERR_OBJECT_NOTREADY)
        {
            Sleep(250);
        }
        else
        {
            wait = false;
        }
    }
    if(numTries > 20)
    {
        printf("ERROR: camera is taking too long for EdsDownloadEvfImage\n");
    }

    unsigned char* pByteImage = NULL;

    // Get image (JPEG) pointer.
    err = EdsGetPointer(stream, (EdsVoid**)&pByteImage );
    if(err != EDS_ERR_OK)
    {
        printf("Error in EdsGetPointer Histogram: 0x%X\n", err);
    }

    EdsUInt64 size;
    err = EdsGetLength(stream, &size);
    if(err != EDS_ERR_OK)
    {
        printf("Error in EdsGetLength Histogram: 0x%X\n", err);
 
    }

    EdsImageRef image = NULL;
    EdsImageInfo imageInfo;

    err = EdsCreateImageRef(stream, &image);
    if(err != EDS_ERR_OK)
    {
        printf("Error in EdsCreateImageRef: 0x%X\n", err);
 
    }

    err = EdsGetImageInfo(image, kEdsImageSrc_FullView, &imageInfo);
    if(err != EDS_ERR_OK)
    {
        printf("Error in EdsGetImageInfo: 0x%X\n", err);

    }

    if(imageInfo.componentDepth != 8)
    {
        printf("Error imageInfo.componentDepth != 8\n");

    }

    // OpenCV_4
	// liveImage = (cv::Mat*)cvCreateImage(cv::Size(imageInfo.width, imageInfo.height), IPL_DEPTH_8U, imageInfo.numOfComponents);
    liveImage = new cv::Mat(cv::Size(imageInfo.width, imageInfo.height), CV_8U, imageInfo.numOfComponents);
    
    EdsUInt32 DataSize = 0;

    CImage cImage;
    HRESULT hr;

    CComPtr<IStream> iStream = NULL;
    HGLOBAL hMem = GlobalAlloc(GHND, size);
    LPVOID pBuff = GlobalLock(hMem);
    memcpy(pBuff, pByteImage, size);
    GlobalUnlock(hMem);
    hr = CreateStreamOnHGlobal(hMem, TRUE, &iStream);

    // Get the bitmap image from the stream
    if ((hr = cImage.Load(iStream)) == S_OK)
    {
        
        int pitch = cImage.GetPitch();
        int height = cImage.GetHeight();
        BYTE* pBits = (BYTE*)cImage.GetBits();
        if (pitch < 0)
            pBits += (pitch *(height -1));
        memcpy(liveImage->data, pBits, abs(pitch) * height);
		
    }

	cImage.~CImage();
    
    GlobalFree(hMem);
	

    cvFlip(liveImage, NULL, 0);

    // Release stream
    if(stream != NULL)
    {
        err = EdsRelease(stream);
        if(err != EDS_ERR_OK)
        {
            printf("Error in EdsRelease: 0x%X\n", err);

        }
        stream = NULL;
    }

   
   if(evfImage != NULL)
    {
        err = EdsRelease(evfImage);
        if(err != EDS_ERR_OK)
        {
            printf("Error in EdsRelease: 0x%X\n", err);

        }
        evfImage = NULL;
    }

	EdsRelease(image);
	
	cv::imshow(windowName.c_str(), *liveImage);

    //OpenCV_4
	// cvReleaseImage(&liveImage);
    liveImage->release();
    //delete(liveImage);
    //liveImage = NULL;
}

int CanonCamera::getNumOfCams()
{
	return detectedCams;
}

void CanonCamera::captureImg()
{
	EdsError	 err = EDS_ERR_OK;
	do 
	{
		err = EdsSendCommand(camera, kEdsCameraCommand_TakePicture, 0);
		if (err != EDS_ERR_OK)	
			Sleep(500);
	} 
	while (err != EDS_ERR_OK);
}
