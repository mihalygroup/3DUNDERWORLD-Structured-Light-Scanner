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
#include "CameraController.h"


CameraController::CameraController(bool webFlag)
{
	web=webFlag;

	if(webFlag == true)
		webCam = new WebCam(webCamID,cam_w,cam_h);
	else
		canonCam = new CanonCamera();
}

CameraController::CameraController(bool webFlag, bool aSync)
{
	web = webFlag;

	if (webFlag == true)
		webCam = new WebCam(webCamID, cam_w, cam_h);
	else
		canonCam = new CanonCamera(aSync);
}

CameraController::~CameraController(void)
{
	if(web == true)
		delete webCam;
	else
		delete canonCam;
}

bool CameraController::isWebCam()
{
	return web;
}

bool CameraController::isCanonCam()
{
	return !web;
}


void CameraController::startLiveview()
{
	if(web == true)
		webCam->startLiveview();
	else
		canonCam->startLiveview();
}

void CameraController::endLiveview()
{
	if(web == true)
		webCam->endLiveview();
	else
		canonCam->endLiveview();
}

void CameraController::UpdateView()
{
	if(web==true)
		webCam->UpdateView();
	else
		canonCam->UpdateView();
}

void CameraController::captureImg()
{
	if(web==true)
		webCam->captureImg();
	else
		canonCam->captureImg();
}

void CameraController::captureImg(char* path)
{
	if(web)
		webCam->captureImg(path);
	else
		canonCam->captureImg();
}

int CameraController::getNumOfCams()
{
	if(web==true)
		return webCam->getNumOfCams();
	else
		return canonCam->getNumOfCams();
}

//only available for webCams
void CameraController::resetSaveCount()
{
	if(web==true)
		webCam->resetSaveCount();
}