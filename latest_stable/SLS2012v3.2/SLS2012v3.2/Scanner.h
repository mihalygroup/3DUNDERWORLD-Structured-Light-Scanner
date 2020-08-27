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


#include "stdafx.h"
#include "SLS2012.h"
#include <iostream>
#include <fstream>
using std::ofstream;
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"

#define STRICT
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>
#include "GrayCodes.h"

#include <conio.h>

#include "CameraController.h"
#include "WebCam.h"
#include "CanonCamera.h"
#include <atlimage.h>
#include "Projector.h"

#define SCANNER_USE_WEBCAM true
#define SCANNER_USE_CANON false

#define SCAN_ONLY true
#define SCAN_N_CALIB false

class Scanner
{

public:
	Scanner(bool web);
	
	
	// BEGIN Async Methods
	void init();

	~Scanner(void);

	// void scan(bool scanOnly);

	void capturePaterns(CameraController *cameras[],int camCount);
	
	bool capturePhotoAllCams(CameraController *cameras[],int camCount);

	bool capturePhotoSequence(CameraController *camera);
	//capture images and save them on path folder
	bool capturePhotoSequence(CameraController *camera, char* path);

	

	// EDS Methods

	static EdsError downloadImage(EdsDirectoryItemRef directoryItem);

	static EdsError Scanner::getCurrentCameraRef(EdsVoid* inContext, EdsCameraRef& inoutCamera);
	static EdsError registerCallbacks(EdsVoid* Context);
	
	// EDS Callbacks Methods
	// see EdsProgressCallback
	static EdsError EDSCALLBACK handleProgressEvent(EdsUInt32 inPercent,
		EdsVoid* inContext,
		EdsBool* outCancel
	);

	// see EdsCameraAddedHandler
	static EdsError EDSCALLBACK handleCameraAddedEvent(
		EdsVoid* inContext
	);


	// See EdsObjectEventHandler
	static EdsError EDSCALLBACK handleObjectEvent(EdsObjectEvent inEvent,
		EdsBaseRef inRef,
		EdsVoid* inContext);

	// See EdsPropertyEventHandler
	static EdsError EDSCALLBACK handlePropertyEvent(EdsPropertyEvent inEvent,
		EdsPropertyID           inPropertyID,
		EdsUInt32               inParam, 
		EdsVoid* inContext);

	// See EdsStateEventHandler
	static EdsError EDSCALLBACK handleStateEvent(EdsStateEvent inEvent,
		EdsUInt32               inEventData,
		EdsVoid* inContext);

private:

	bool web;


	cv::Mat whiteImg;

	GrayCodes *grayCodes;

	Projector *proj;
};

