/*
								StackThread.cpp

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
* \file    StackThread.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Stack frames.
*/

#include "StackThread.h"

boost::log::sources::severity_logger< LogSeverityLevel >  StackThread::logger;
StackThread::_Init StackThread::_initializer;

StackThread::StackThread(   boost::mutex							*cfg_m,
							string									cfg_p,
							bool									*sS,
							boost::mutex							*sS_m,
							boost::condition_variable				*sS_c,
							boost::circular_buffer<Frame>		    *fb,
							boost::mutex                            *fb_m,
							boost::condition_variable               *fb_c){

	cfg_mutex				= cfg_m;
	cfgPath					= cfg_p;
	thread      			= NULL;
	mustStop				= false;
    frameBuffer             = fb;
    frameBuffer_mutex       = fb_m;
    frameBuffer_condition   = fb_c;
	stackSignal				= sS;
    stackSignal_mutex		= sS_m;
    stackSignal_condition	= sS_c;
	completeDataPath		= "";
    isRunning               = false;

}

StackThread::~StackThread(void){

    BOOST_LOG_SEV(logger,notification) << "Call StackThread destructor.";
	if (thread!=NULL) delete thread;

}

bool StackThread::startThread(){

	boost::mutex::scoped_lock lock(*cfg_mutex);

	BOOST_LOG_SEV(logger,notification) << "Try to load parameters for StackThread...";

	if(!loadStackParameters()){
		lock.unlock();
		BOOST_LOG_SEV(logger,fail) << "Fail to load parameters for StackThread.";
		return false;
	}

	BOOST_LOG_SEV(logger,notification) << "Success to load parameters for StackThread.";

	lock.unlock();

    BOOST_LOG_SEV(logger,notification) << "Create StackThread.";
    thread = new boost::thread(boost::ref(*this));
	return true;
}

void StackThread::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop =true;
	mustStopMutex.unlock();

	BOOST_LOG_SEV(logger,notification) << "Signal to stop StackThread received. Wait 2 seconds before interruption ...";

    while(thread->timed_join(boost::posix_time::seconds(2)) == false){

        BOOST_LOG_SEV(logger,notification) << "Interruption signal sent.";
        thread->interrupt();

    }
}

bool StackThread::loadStackParameters(){

	try{

		Configuration cfg;
		cfg.Load(cfgPath);

        // Get acquisition frequency.
		int ACQ_FPS;
		cfg.Get("ACQ_FPS", ACQ_FPS);
		BOOST_LOG_SEV(logger,notification) << "Load ACQ_FPS : " << ACQ_FPS;

        // Get camera format.
		string acq_bit_depth; cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
		EParser<CamBitDepth> cam_bit_depth;
		ACQ_BIT_DEPTH = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);
		BOOST_LOG_SEV(logger,notification) << "Load ACQ_BIT_DEPTH : " << acq_bit_depth;

        // Get time of stacking frames.
		cfg.Get("STACK_TIME", STACK_TIME);
		BOOST_LOG_SEV(logger,notification) << "Load STACK_TIME : " << STACK_TIME;
		STACK_TIME = STACK_TIME * ACQ_FPS;

        // Get time to wait before a new stack.
		cfg.Get("STACK_INTERVAL", STACK_INTERVAL);
		BOOST_LOG_SEV(logger,notification) << "Load STACK_INTERVAL : " << STACK_INTERVAL;

        // Get stack method to use.
		string stack_method; cfg.Get("STACK_MTHD", stack_method);
		EParser<StackMeth> stack_mth;
		STACK_MTHD = stack_mth.parseEnum("STACK_MTHD", stack_method);
		BOOST_LOG_SEV(logger,notification) << "Load STACK_MTHD : " << stack_method;

        cfg.Get("STACK_REDUCTION", STACK_REDUCTION);
        BOOST_LOG_SEV(logger,notification) << "Load STACK_REDUCTION : " << STACK_REDUCTION;

		cfg.Get("STATION_NAME", STATION_NAME);
		BOOST_LOG_SEV(logger,notification) << "Load STATION_NAME : " << STATION_NAME;

		cfg.Get("CFG_FILECOPY_ENABLED", CFG_FILECOPY_ENABLED);
		BOOST_LOG_SEV(logger,notification) << "Load CFG_FILECOPY_ENABLED : " << CFG_FILECOPY_ENABLED;

		cfg.Get("DATA_PATH", DATA_PATH);
		BOOST_LOG_SEV(logger,notification) << "Load DATA_PATH : " << DATA_PATH;

		fitsHeader.loadKeywordsFromConfigFile(cfgPath);


	}catch(exception& e){

		cout << e.what() << endl;
		BOOST_LOG_SEV(logger,critical) << e.what();
		return false;

	}

	return true;

}

bool StackThread::buildStackDataDirectory(string date){

	namespace fs = boost::filesystem;
	string	YYYYMMDD	= TimeDate::get_YYYYMMDD_fromDateString(date);
	string	root		= DATA_PATH + STATION_NAME + "_" + YYYYMMDD +"/";
    string	subDir		= "astro/";
    string	finalPath	= root + subDir;

	completeDataPath	= finalPath;
	BOOST_LOG_SEV(logger,notification) << "CompleteDataPath : " << completeDataPath;

    path p(DATA_PATH);
    path p1(root);
    path p2(root + subDir);

    // If DATA_PATH exists
    if(fs::exists(p)){

        // If DATA_PATH/STATION_YYYYMMDD/ exists
        if(fs::exists(p1)){

            // If DATA_PATH/STATION_YYYYMMDD/astro/ doesn't exists
            if(!fs::exists(p2)){

                // If fail to create DATA_PATH/STATION_YYYYMMDD/astro/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create astro directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/astro/
                }else{

                   BOOST_LOG_SEV(logger,notification) << "Success to create astro directory : " << p2.string();
                   return true;

                }
            }

        // If DATA_PATH/STATION_YYYYMMDD/ doesn't exists
        }else{

            // If fail to create DATA_PATH/STATION_YYYYMMDD/
            if(!fs::create_directory(p1)){

                BOOST_LOG_SEV(logger,fail) << "Unable to create STATION_YYYYMMDD directory : " << p1.string();
				return false;

            // If success to create DATA_PATH/STATION_YYYYMMDD/
            }else{

                BOOST_LOG_SEV(logger,notification) << "Success to create STATION_YYYYMMDD directory : " << p1.string();

                // If fail to create DATA_PATH/STATION_YYYYMMDD/astro/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create astro directory : " << p2.string();
					return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/astro/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create astro directory : " << p2.string();
					return true;

                }
            }
        }

    // If DATA_PATH doesn't exists
    }else{

        // If fail to create DATA_PATH
        if(!fs::create_directory(p)){

            BOOST_LOG_SEV(logger,fail) << "Unable to create DATA_PATH directory : " << p.string();
			return false;

        // If success to create DATA_PATH
        }else{

            BOOST_LOG_SEV(logger,notification) << "Success to create DATA_PATH directory : " << p.string();

            // If fail to create DATA_PATH/STATION_YYYYMMDD/
            if(!fs::create_directory(p1)){

                BOOST_LOG_SEV(logger,fail) << "Unable to create STATION_YYYYMMDD directory : " << p1.string();
				return false;

            // If success to create DATA_PATH/STATION_YYYYMMDD/
            }else{

                BOOST_LOG_SEV(logger,notification) << "Success to create STATION_YYYYMMDD directory : " << p1.string();

                // If fail to create DATA_PATH/STATION_YYYYMMDD/astro/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create astro directory : " << p2.string();
					return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/astro/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create astro directory : " << p2.string();
					return true;

                }
            }
        }
    }
}

bool StackThread::getRunStatus(){

    return isRunning;

}

void StackThread::operator()(){

    bool stop = false;
    isRunning = true;

    BOOST_LOG_SCOPED_THREAD_TAG("LogName", "STACK_THREAD");
	BOOST_LOG_SEV(logger,notification) << "\n";
	BOOST_LOG_SEV(logger,notification) << "==============================================";
	BOOST_LOG_SEV(logger,notification) << "============== Start stack thread ============";
	BOOST_LOG_SEV(logger,notification) << "==============================================";

    try{

        do{

            try{

                BOOST_LOG_SEV(logger,notification) << "Stack thread is going to sleep ... ";
                boost::this_thread::sleep(boost::posix_time::millisec(STACK_INTERVAL*1000));
                BOOST_LOG_SEV(logger,notification) << "Stack thread wake up ...";

                // Create a stack to accumulate n frames.
                BOOST_LOG_SEV(logger, notification) << "Create a new STACK.";
                Stack stack(STACK_TIME);

                do{

                    // Communication with AcqThread. Wait for a new frame.
                    BOOST_LOG_SEV(logger,normal) << "Waiting for a new frame ...";
                    boost::mutex::scoped_lock lock(*stackSignal_mutex);
                    while(!(*stackSignal)) stackSignal_condition->wait(lock);
                    BOOST_LOG_SEV(logger,normal) << "End waiting for a new frame.";
                    *stackSignal = false;
                    lock.unlock();

                    double t = (double)getTickCount();

                    // Fetch last frame grabbed.
                    boost::mutex::scoped_lock lock2(*frameBuffer_mutex);
                    Frame newFrame = frameBuffer->back();
                    BOOST_LOG_SEV(logger,normal) << "New frame received by stackThread :  "<< newFrame.getNumFrame();
                    lock2.unlock();

                    // Add the new frame to the stack.
                    stack.addFrame(newFrame);

                    if(stack.getFullStatus()){

                        if(buildStackDataDirectory(stack.getDateFirstFrame())){

                            if(!stack.saveStack(fitsHeader, completeDataPath, STACK_MTHD, STATION_NAME, STACK_REDUCTION)){

                                BOOST_LOG_SEV(logger,critical) << "Error saving stack data.";
                                throw "Error saving stack data.";

                            }

                            BOOST_LOG_SEV(logger,notification) << "Stack saved : " << completeDataPath;

                        }else{

                            BOOST_LOG_SEV(logger,fail) << "Fail to build stack directory. ";

                        }

                    }

                    t = (((double)getTickCount() - t)/getTickFrequency())*1000;
                    std::cout << "[ Stack time ] : " << std::setprecision(5) << std::fixed << t << " ms" << endl;
                    BOOST_LOG_SEV(logger,normal) << "[ Stack time ] : " << std::setprecision(5) << std::fixed << t << " ms" ;

                }while(!stack.getFullStatus());

                // Get the "must stop" state (thread-safe)
                mustStopMutex.lock();
                stop = mustStop;
                mustStopMutex.unlock();

            }catch(const boost::thread_interrupted&){

                BOOST_LOG_SEV(logger,notification) << "Stack thread INTERRUPTED";
                break;

            }

        }while(!stop);

    }catch(const char * msg){

        cout << msg << endl;
        BOOST_LOG_SEV(logger,critical) << msg;

    }catch(exception& e){

        cout << "An exception occured : " << e.what() << endl;
        BOOST_LOG_SEV(logger, critical) << e.what();

    }

    isRunning = false;

	BOOST_LOG_SEV(logger,notification) << "Stack thread TERMINATED";

}

