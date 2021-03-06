/*
						DetectionTemporal_.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
* \file    DetectionTemporal_.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Detection method by temporal movement.
*/

#pragma once


#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <boost/tokenizer.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/core.hpp>
#include "ELogSeverityLevel.h"
#include "TimeDate.h"
#include "Fits2D.h"
#include "Fits.h"
#include "Frame.h"
#include "EStackMeth.h"
#include "ECamBitDepth.h"
#include "GlobalEvent.h"
#include "LocalEvent.h"
#include "Detection.h"
#include "EParser.h"
#include "SaveImg.h"
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace std;
using namespace cv;

class DetectionTemporal_ : public Detection{

	private :

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetectionTemporal_"));
				}
		} _initializer;

        vector<GlobalEvent>	listGlobalEvents;

		vector<Point> subdivisionPos;

		vector<Scalar> listColors; // B, G, R

		Mat localMask;

		bool initStatus;

		Mat prevThreshMap;

		vector<GlobalEvent>::iterator GEToSave;

		bool DET_DOWNSAMPLE_ENABLED;

		CamBitDepth ACQ_BIT_DEPTH;

		bool DET_SAVE_GEMAP;

		bool DET_SAVE_POS;

		bool DET_DEBUG;

		bool DET_SAVE_DIRMAP;

		bool DET_SAVE_GE_INFOS;

		string DET_DEBUG_PATH;

		bool DET_DEBUG_VIDEO;

        bool detStatus ;

        int nbGE;

		int roiSize[2];

		Mat prevthresh;

		vector<GlobalEvent>::iterator geToSave;

		vector<Scalar> availableGeColor;

		VideoWriter videoDebug;

		int imgNum;

		vector<float> stdev;

		vector<int> numFrame;

		float maxstdev;

		bool DET_SAVE_STDEV;

        int maxNumFrame;

        bool ACQ_MASK_ENABLED;

        Mat prevImg;

        int ACQ_FPS;

        bool STATIC_MASK_OPTION;

        int STATIC_MASK_INTERVAL;

        Mat staticMask;

        bool staticMaskInitialized;

        // Map of pixel with high intensity.
        Mat HighIntensityMap;

        Mat mask;

	public:

		DetectionTemporal_();

		~DetectionTemporal_();

        bool initMethod(string cfg_path);

		bool run(Frame &c, Frame &p);

		void saveDetectionInfos(string p);

		void resetDetection();

		int getNumFirstEventFrame() {return (*GEToSave).getNumFirstFrame();};

		string getDateEvent()       {return (*GEToSave).getDate();};

		int getNumLastEventFrame()  {return (*GEToSave).getNumLastFrame();};

    private :

		void initDebug();

		int defineThreshold(Mat i);

		void subdivideFrame(vector<Point> &sub, int n, int imgH, int imgW);

		vector<Scalar> getColorInEventMap(Mat &eventMap, Point roiCenter);

		void colorRoiInBlack(Point p, int h, int w, Mat &region);

		void searchROI(Mat &region, Mat &frame, Mat &eventMap, vector<LocalEvent> &listLE, Point regionPosInFrame, int maxNbLE);

};

