/*
				EParser.h

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
*	Last modified:		04/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    EParser.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    04/12/2014
* \brief   Parse some parameters in the configuration file with FreeTure's enumerations.
*/

#pragma once

#include <string>
#include <iostream>
#include <map>
#include <stdexcept>

#include "ECamType.h"
#include "ECamBitDepth.h"
#include "EDetMeth.h"
#include "EStackMeth.h"
#include "ELogSeverityLevel.h"

using namespace std;

//http://stackoverflow.com/questions/726664/string-to-enum-in-c

template<typename T> class EParser{

	public:

	    map<string, T> enumMap;

		EParser();

		T parseEnum(string paramName, const string & value){

            typename map<string, T>::const_iterator iValue = enumMap.find(value);

            if(iValue == enumMap.end()){

                typename map<string, T>::const_iterator it;

                string res = "\nThe parameter " + paramName + " can't has the value : "
                                + value + "\nPlease see the configuration file.\n"
                                + "Possible value for this parameter are only : \n\n";

                for(it = enumMap.begin(); it != enumMap.end(); ++it){
                    res = res + it->first + "\n";
                }

                throw runtime_error(res);

            }

            return iValue->second;

		}

		string getStringEnum(T type){

		    typename map<string, T>::const_iterator it;

            for(it = enumMap.begin(); it != enumMap.end(); ++it){
                if(type == it->second)
                    return it->first;
            }
		}

};
