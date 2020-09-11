//------------------------------------------------------------------------------------------------------------
//* Copyright © 2010-2013 Immersive and Creative Technologies Lab, Cyprus University of Technology           *
//* Link: http://ict.cut.ac.cy                                                                               *
//* Software developer(s): Kyriakos Herakleous                                                               *
//* Researcher(s): Kyriakos Herakleous, Charalambos Poullis                                                  *
//*                                                                                                          *
//* This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.*
//* Link: http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US                                        *
//------------------------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Scanner.h"


Scanner::Scanner(bool webCam)
{
	web=webCam;
	proj=new Projector(proj_w,proj_h);
	whiteImg = cv::Mat::ones(proj_h,proj_w,CV_8U)*255;
}

Scanner::~Scanner(void)
{
	EdsTerminateSDK();
		///Destroy the window
	delete proj;
}

void Scanner::init()
{
	static bool initDone = false;

	if (!initDone)
	{
		std::cout << "CANON SDK init.\n";

		EdsError	 err = EDS_ERR_OK;
		// EVENT HANDLER
		EdsVoid* context = NULL;

		// Initialization of SDK
		err = EdsInitializeSDK();

		if (err == EDS_ERR_OK)
		{
			std::cout << "Canon SDK loaded.\n";
		}

		err = Scanner::registerCallbacks(context);
		if (err == EDS_ERR_OK)
		{
			std::cout << "Canon callbacks registered.\n";
		}
		initDone = true;
	}
}

// Utils
#define waitForInputKey(duration, message) LogAndWaitForInputKey(__FILE__, __LINE__, duration, message)
int Scanner::LogAndWaitForInputKey(char* szFile, int nLine, int duration, const std::string message)
{
	int key = 0;

	std::cout << "At " << szFile << ":" << nLine << " "
		<< "Press [Enter] to capture image, Press [Space] to ?? or [Esc] to quit."
		<< std::endl;

	key = cv::waitKey(duration);
	return key;
}

bool Scanner::capturePhotoSequence(CameraController *camera)
{

	proj->showImg(whiteImg);

	//startCameraLiveView
	camera->startLiveview();

	
	int key;
	int count=0;

	while(true)
	{
		

		camera->UpdateView();

		key = waitForInputKey(0, "Press [Enter] to capture image, Press [Space] to ?? or [Esc] to quit.");

		///If enter is pressed then capture the image
		if (key == 13)
		{
			camera->captureImg();
			count++;
			std::cout<<count<<" image(s) have been captured.\n";
		}
		
		//32-> enter / 27-> esc 
		if(key == 32 || key == 27)
		{
			break;
		}

		
	}

	camera->endLiveview();

	cv::waitKey(100); // FIXME: replace with notif or error code?
	if(key == 27)
		return 0;
	else 
		return 1;

}

//capture photos for calibration, photos will saved on path
//this is curently available only for 
bool Scanner::capturePhotoSequence(CameraController *camera, char* path)
{
	
	//if is a canon cam regular captureCalib is called
	if(camera->isCanonCam())
	{
		return capturePhotoSequence(camera);
	}

	proj->showImg(whiteImg);
	//startCameraLiveView
	camera->startLiveview();

	
	int key;

	int count=0;

	while(true)
	{
	
		camera->UpdateView();

		key = waitForInputKey(0, "Press [Enter] to capture image, Press [Space] to ?? or [Esc] to quit.");

		///If enter is pressed then capture the image
		if (key == 13)
		{
			camera->captureImg(path);
			count++;
			std::cout<<count<<" image(s) have been captured.\n";
		}
		
		//32-> enter / 27-> esc 
		if(key == 32 || key == 27)
		{
			break;
		}

	}

	camera->endLiveview();

	cv::waitKey(100); // FIXME: replace with notif or error code?

	if(key == 27)
		return 0;
	else 
		return 1;

}

//project paterns and capture photos from all cameras
void Scanner::capturePaterns(CameraController *camera[],int camCount)
{

	std::cout << "\t-Generate Gray Codes..."  ;

	grayCodes= new GrayCodes(proj_w,proj_h);
	grayCodes->generateGrays();

	proj->showImg(grayCodes->getNextImg());

	cv::waitKey(100); // FIXME: replace with notif or error code?

	std::cout << "done!\n" << std::endl;
			
	int key=0;
	
	// FIXME: Infinite loop !?!
	/*
	while(key==0)
		key = cv::waitKey(10);
	*/

	int grayCount=0;

	for(int i=0; i<camCount; i++)
	{
		camera[i]->resetSaveCount();
	}

	

	while(true)
	{
		int key;

		for(int i=0; i<camCount; i++)
		{
			if(camera[i]->isWebCam())
				camera[i]->captureImg("scan/dataSet/");
			else
				camera[i]->captureImg();
		}
			
		grayCount++;

		std::cout<<"Capture " << grayCount<<" of "<<grayCodes->getNumOfImgs()<< ".\n";

		if(grayCount==grayCodes->getNumOfImgs())
			break;

		proj->showImg(grayCodes->getNextImg());
		
		// FIXME: error code or notif
		key = waitForInputKey(100, "Press 'Esc' to Stop");
		if(key == 27)
			break;
	}

	cv::waitKey(300); // FIXME: replace with notif or return code

}

bool Scanner::capturePhotoAllCams(CameraController *cameras[],int camCount)
{

	std::cout << "\nPress 'Enter' to capture 1 photo (both Cameras) for camera - camera calibration..\n" << std::endl;

	proj->showImg(whiteImg);

	for(int i =0; i<camCount; i++)
	{
		//startCameraLiveView
		cameras[i]->startLiveview();
	}

	int key;

	while(true)
	{
		
		for(int i =0; i<camCount; i++)
		{
			//startCameraLiveView
			cameras[i]->UpdateView();
		}
		
		key = waitForInputKey(0, "Press [Enter] to capture image, Press [Space] to ?? or [Esc] to quit.");

		///If enter is pressed then capture the image
		if (key == 13)
		{
			for(int i =0; i<camCount; i++)
			{
				cameras[i]->captureImg();
			}
			break;
		}
		
		//32-> enter / 27-> esc 
		if(key == 32 || key == 27)
		{
			break;
		}

		
	}

	for(int i =0; i<camCount; i++)
	{
		cameras[i]->endLiveview();
	}


	cv::waitKey(100); //FIXME: use notif or error code

	if(key == 27)
		return 0;
	else 
		return 1;
}

// BEGIN EDS TRANSFER METHODS - TO BE MOVE TO CANON CAMERA CLASS
EdsError Scanner::downloadImage(EdsDirectoryItemRef directoryItem)
{
	EdsError err = EDS_ERR_OK;
	EdsStreamRef stream = NULL;
	// CALLBACK PARAMETERS
	EdsProgressOption progressOption = kEdsProgressOption_Periodically; // FIXME add a Scanner Option
	EdsVoid* context = NULL; // FIXME use Camera context

	// Get directory item information
	EdsDirectoryItemInfo dirItemInfo;
	err = EdsGetDirectoryItemInfo(directoryItem, &dirItemInfo);
	// Create file stream for transfer destination
	if (err == EDS_ERR_OK)
	{
		err = EdsCreateFileStream(dirItemInfo.szFileName,
			kEdsFileCreateDisposition_CreateAlways,
			kEdsAccess_ReadWrite, &stream);
	}

	// Set Progress Callback
	err = EdsSetProgressCallback(stream,
		&Scanner::handleProgressEvent,
		progressOption,
		context
	);

	// Download image
	if (err == EDS_ERR_OK)
	{
		err = EdsDownload(directoryItem, dirItemInfo.size, stream);
	}
	// Issue notification that download is complete
	if (err == EDS_ERR_OK)
	{
		err = EdsDownloadComplete(directoryItem);
	}
	// Release stream
	if (stream != NULL)
	{
		EdsRelease(stream);
		stream = NULL;
	}
	return err;
}

// BEGIN EDS Callbacks - TO BE MOVE TO CANON CAMERA CLASS
EdsError Scanner::getCurrentCameraRef(EdsVoid* inContext, EdsCameraRef& outCamera)
{
	EdsError err = EDS_ERR_OK;
	EdsCameraListRef cameraList = NULL;
	EdsCameraRef camera = NULL;
	EdsUInt32	 count = 0;

	// FIXME use context info
	//Acquisition of camera list
	if (err == EDS_ERR_OK)
	{
		err = EdsGetCameraList(&cameraList);
	}

	//Acquisition of number of Cameras
	if (err == EDS_ERR_OK)
	{
		err = EdsGetChildCount(cameraList, &count);
		if (count == 0)
		{
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
		else
		{
			//FIXME get last (added?) camera in the list
			for (int i = count - 1; i < count; i++)
			{
				err = EdsGetChildAtIndex(cameraList,
					i,
					&camera);
				printf("Get current camera [0x%p]\n",
					camera);
				outCamera = camera;
			}
		}
	}

	return err;
}

EdsError Scanner::registerCallbacks(EdsVoid* inContext)
{
	EdsError	 err = EDS_ERR_OK;
	EdsCameraRef camera = NULL;
	EdsVoid* context = NULL;

	err = Scanner::getCurrentCameraRef(inContext, camera);

	// FIXME Set context to EdsCameraRef
	context = (EdsVoid*)camera;
	printf("Register camera [0x%p] callbacks with context [0x%p]\n", 
		camera,
		context);

		// Set Camera Added Event Handler
		if (err == EDS_ERR_OK)
		{
			err = EdsSetCameraAddedHandler(
				&Scanner::handleCameraAddedEvent,
				context); // FIXME
		}

		// Set Object event handler
		if (err == EDS_ERR_OK)
		{
			err = EdsSetObjectEventHandler(camera,
				kEdsObjectEvent_All,
				&Scanner::handleObjectEvent,
				context);
		}


		// Set Property event handler
		if (err == EDS_ERR_OK)
		{
			err = EdsSetPropertyEventHandler(camera,
				kEdsPropertyEvent_All,
				&Scanner::handlePropertyEvent,
				context);
		}
		// Set State event handler
		if (err == EDS_ERR_OK)
		{
			err = EdsSetCameraStateEventHandler(camera,
				kEdsStateEvent_All,
				&Scanner::handleStateEvent,
				context);
		}
	return err;
}

EdsError EDSCALLBACK Scanner::handleProgressEvent(EdsUInt32 inPercent,
	EdsVoid* inContext,
	EdsBool* outCancel
)
{
	EdsError err = EDS_ERR_OK;

	printf("Progress Event [%d] received, is cancelled [%d]\n", inPercent, *outCancel);

	return err;
}

EdsError EDSCALLBACK Scanner::handleCameraAddedEvent(
	EdsVoid* inContext
)
{
	EdsError err = EDS_ERR_OK;
	printf("Camera Added Event received\n");
	Scanner::registerCallbacks(inContext);
	CanonCamera::AddCameraASync();

	return err;
}

EdsError EDSCALLBACK Scanner::handleObjectEvent(EdsObjectEvent inEvent,
	EdsBaseRef inRef,
	EdsVoid* inContext
)
{
	EdsError err = EDS_ERR_OK;

	printf("Object Event [0x%08X] received: ", inEvent);
	switch (inEvent)
	{
	case kEdsObjectEvent_DirItemRequestTransfer:
		printf("event ready for download");
		downloadImage(inRef);
		break;
	default:
		printf("event unknown");
		break;
	}
	printf("\n");

	// Object must be released
	if (inEvent)
	{
		EdsRelease(inRef);
	}
	return err;
}

EdsError EDSCALLBACK Scanner::handlePropertyEvent(EdsPropertyEvent inEvent,
	EdsPropertyID           inPropertyID,
	EdsUInt32               inParam,
	EdsVoid* inContext
)
{
	EdsError err = EDS_ERR_OK;
	EdsDataType dataType = kEdsDataType_Unknown;
	EdsUInt32 dataSize = 0;
	EdsUInt32 intDataValue = 0;

	EdsCameraRef camera = (EdsCameraRef)inContext;

	err = EdsGetPropertySize(camera,
		inPropertyID,
		inParam,
		&dataType,
		&dataSize);

	switch (dataSize)
	{
		case sizeof(EdsUInt32):
			if (err == EDS_ERR_OK)
			{
				err = EdsGetPropertyData(camera,
					inPropertyID,
					inParam,
					dataSize,
					&intDataValue
				);
			}
			printf("Property Event [0x%08X] received from camera[0x%p]: property ID [0x%08X|0x%08X], value [%d]",
				inEvent,
				camera,
				inPropertyID,
				inParam,
				intDataValue
			);
			break;
		default:
			printf("Property Event with unsupported dataSize [0x%08X] received from camera[0x%p]:",
				dataSize,
				camera
			);
			break;
	}

	switch (inEvent)
	{
	default:
		break;
	}
	printf("\n");

	return err;
}

EdsError EDSCALLBACK Scanner::handleStateEvent(EdsStateEvent inEvent,
	EdsUInt32               inEventData,
	EdsVoid* inContext
)
{
	EdsError err = EDS_ERR_OK;

	printf("State Event [0x%08X] received", inEvent);
	switch (inEvent)
	{
	default:
		break;
	}
	printf("\n");

	return err;
}

// END EDS CALLBACKS