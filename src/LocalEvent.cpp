/*
								LocalEvent.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*	License:		GNU General Public License
*
*	FreeTure is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	FreeTure is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    LocalEvent.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Event occured on a single frame.
*/

#include "LocalEvent.h"

LocalEvent::LocalEvent(Scalar color, Point roiPos, int frameHeight, int frameWidth, const int *roiSize){

    // Color id in frame.
    leColor = color;
    // Save position of the first ROI.
    leRoiList.push_back(roiPos);
    // Create LE map.
    leMap = Mat::zeros(frameHeight, frameWidth, CV_8UC1);
    // Add first ROI in the LE map.
    Mat roi(roiSize[1],roiSize[0],CV_8UC1,Scalar(255));
    roi.copyTo(leMap(Rect(roiPos.x-roiSize[0]/2,roiPos.y-roiSize[1]/2,roiSize[0],roiSize[1])));

}

LocalEvent::LocalEvent(Scalar color, Point sPos, Mat s){

    leColor = color;
    subdivision.push_back(s);
    leRoiList.push_back(sPos);

}

LocalEvent::~LocalEvent(){

}

void LocalEvent::computeMassCenter(){

    float x = 0, y = 0;

    vector<Point>::iterator it;

    for (it = leRoiList.begin(); it != leRoiList.end(); ++it){

        x += (*it).x;
        y += (*it).y;

    }

    x = x / leRoiList.size();
    y = y / leRoiList.size();

    leMassCenter = Point(x,y);

}

void LocalEvent::setMap(Point p, int h, int w){

    // Add new ROI to the LE map.
    Mat roi(h,w,CV_8UC1,Scalar(255));
    roi.copyTo(leMap(Rect(p.x, p.y, w, h)));

}
