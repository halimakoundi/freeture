/*
							CameraGigeSdkAravis.cpp

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
*	Last modified:		21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraGigeSdkAravis.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Use Aravis library to pilot GigE Cameras.
*          https://wiki.gnome.org/action/show/Projects/Aravis?action=show&redirect=Aravis
*/

#include "CameraGigeSdkAravis.h"

#ifdef LINUX

	boost::log::sources::severity_logger< LogSeverityLevel >  CameraGigeSdkAravis::logger;
	CameraGigeSdkAravis::_Init CameraGigeSdkAravis::_initializer;

	CameraGigeSdkAravis::CameraGigeSdkAravis(bool shift){
	    shiftImage = shift;
    }

	CameraGigeSdkAravis::CameraGigeSdkAravis(){
	    shiftImage = false;
    }

	CameraGigeSdkAravis::~CameraGigeSdkAravis(){}

	void CameraGigeSdkAravis::listGigeCameras(){

		arv_update_device_list();

		int n_devices = arv_get_n_devices();

        BOOST_LOG_SEV(logger, notification) << "Cameras detected with Aravis : ";

		cout << "******** DETECTED CAMERAS WITH ARAVIS ********** " << endl;
		cout << "*" << endl;

		for(int i = 0; i < n_devices; i++){

			cout << "* -> [" << i << "] " << arv_get_device_id(i)<< endl;
			BOOST_LOG_SEV(logger, notification) << " -> [" << i << "] " << arv_get_device_id(i);

		}

		cout << "*" << endl;
		cout << "************************************************ " << endl;

	}

	bool CameraGigeSdkAravis::createDevice(int id){

	    cout << "Creating device " << id << endl;
	    BOOST_LOG_SEV(logger, notification) << "Creating device " << id;

	    string deviceName;

	    if(!getDeviceById(id, deviceName))
            return false;

        cout << "Device : " << deviceName << endl;
        BOOST_LOG_SEV(logger, notification) << "Device selected: " << deviceName;

		camera = arv_camera_new(deviceName.c_str());

		if(camera != NULL){

			cout << "Connection success to the camera." << endl;
			BOOST_LOG_SEV(logger, notification) << "Connection success to the camera.";
			return true;

		}else{

			cout << "Connection fail to the camera." << endl;
			BOOST_LOG_SEV(logger, fail) << "Connection fail to the camera.";
			return false;

		}
	}

	bool CameraGigeSdkAravis::getDeviceById(int id, string &device){

		arv_update_device_list();

		int n_devices = arv_get_n_devices();

		for(int i = 0; i< n_devices; i++){

            cout << "ID : " << i << " -> " << arv_get_device_id(i) << endl;

			if(id == i){

				device = arv_get_device_id(i);
				return true;

			}
		}

		return false;

	}

	bool CameraGigeSdkAravis::grabStart(){

		int sensor_width, sensor_height;

		arv_camera_get_sensor_size(camera, &sensor_width, &sensor_height);
		BOOST_LOG_SEV(logger, notification) << "Camera sensor size : " << sensor_width << "x" << sensor_height;

		arv_camera_set_region(camera, 0, 0,sensor_width,sensor_height);

		arv_camera_get_region (camera, NULL, NULL, &width, &height);
		BOOST_LOG_SEV(logger, notification) << "Camera region size : " << width << "x" << height;

		payload = arv_camera_get_payload (camera);
		BOOST_LOG_SEV(logger, notification) << "Camera payload : " << payload;

		pixFormat = arv_camera_get_pixel_format(camera);

		arv_camera_get_exposure_time_bounds (camera, &exposureMin, &exposureMax);
		BOOST_LOG_SEV(logger, notification) << "Camera exposure bound min : " << exposureMin;
		BOOST_LOG_SEV(logger, notification) << "Camera exposure bound max : " << exposureMax;

		arv_camera_get_gain_bounds (camera, &gainMin, &gainMax);
		BOOST_LOG_SEV(logger, notification) << "Camera gain bound min : " << gainMin;
		BOOST_LOG_SEV(logger, notification) << "Camera gain bound max : " << gainMax;

		arv_camera_set_frame_rate(camera, 30);

		fps = arv_camera_get_frame_rate(camera);
		BOOST_LOG_SEV(logger, notification) << "Camera frame rate : " << fps;

		caps_string = arv_pixel_format_to_gst_caps_string(pixFormat);
		BOOST_LOG_SEV(logger, notification) << "Camera format : " << caps_string;

		gain = arv_camera_get_gain(camera);
		BOOST_LOG_SEV(logger, notification) << "Camera gain : " << gain;

		exp = arv_camera_get_exposure_time(camera);
		BOOST_LOG_SEV(logger, notification) << "Camera exposure : " << exp;

		cout << endl;

		cout << "DEVICE SELECTED : " << arv_camera_get_device_id(camera)    << endl;
		cout << "DEVICE NAME     : " << arv_camera_get_model_name(camera)   << endl;
		cout << "DEVICE VENDOR   : " << arv_camera_get_vendor_name(camera)  << endl;
		cout << "PAYLOAD         : " << payload                             << endl;
		cout << "Width           : " << width                               << endl
			 << "Height          : " << height                              << endl;
		cout << "Exp Range       : [" << exposureMin    << " - " << exposureMax   << "]"  << endl;
		cout << "Exp             : " << exp                                 << endl;
		cout << "Gain Range      : [" << gainMin        << " - " << gainMax       << "]"  << endl;
		cout << "Gain            : " << gain                                << endl;
		cout << "Fps             : " << fps                                 << endl;
		cout << "Type            : " << caps_string                         << endl;

		cout << endl;

		// Create a new stream object. Open stream on Camera.
		stream = arv_camera_create_stream(camera, NULL, NULL);

		if(stream == NULL){

			BOOST_LOG_SEV(logger, critical) << "Fail to create stream with arv_camera_create_stream()";
			return false;

        }

		if (ARV_IS_GV_STREAM(stream)){

			bool            arv_option_auto_socket_buffer   = true;
			bool            arv_option_no_packet_resend     = true;
			unsigned int    arv_option_packet_timeout       = 20;
			unsigned int    arv_option_frame_retention      = 100;

			if(arv_option_auto_socket_buffer){

				g_object_set(stream,
							 // ARV_GV_STREAM_SOCKET_BUFFER_FIXED : socket buffer is set to a given fixed value.
							 // ARV_GV_STREAM_SOCKET_BUFFER_AUTO: socket buffer is set with respect to the payload size.
							 "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
							 // Socket buffer size, in bytes.
							 // Allowed values: >= G_MAXULONG
							 // Default value: 0
							 "socket-buffer-size", 0, NULL);

			}

			if(arv_option_no_packet_resend){

				// # packet-resend : Enables or disables the packet resend mechanism

				// If packet resend is disabled and a packet has been lost during transmission,
				// the grab result for the returned buffer holding the image will indicate that
				// the grab failed and the image will be incomplete.
				//
				// If packet resend is enabled and a packet has been lost during transmission,
				// a request is sent to the camera. If the camera still has the packet in its
				// buffer, it will resend the packet. If there are several lost packets in a
				// row, the resend requests will be combined.

				g_object_set(stream,
							 // ARV_GV_STREAM_PACKET_RESEND_NEVER: never request a packet resend
							 // ARV_GV_STREAM_PACKET_RESEND_ALWAYS: request a packet resend if a packet was missing
							 // Default value: ARV_GV_STREAM_PACKET_RESEND_ALWAYS
							 "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER, NULL);

			}

			g_object_set(stream,
						 // # packet-timeout

						 // The Packet Timeout parameter defines how long (in milliseconds) we will wait for
						 // the next expected packet before it initiates a resend request.

						 // Packet timeout, in µs.
						 // Allowed values: [1000,10000000]
						 // Default value: 40000
						 "packet-timeout",/* (unsigned) arv_option_packet_timeout * 1000*/(unsigned)40000,
						 // # frame-retention

						 // The Frame Retention parameter sets the timeout (in milliseconds) for the
						 // frame retention timer. Whenever detection of the leader is made for a frame,
						 // the frame retention timer starts. The timer resets after each packet in the
						 // frame is received and will timeout after the last packet is received. If the
						 // timer times out at any time before the last packet is received, the buffer for
						 // the frame will be released and will be indicated as an unsuccessful grab.

						 // Packet retention, in µs.
						 // Allowed values: [1000,10000000]
						 // Default value: 200000
						 "frame-retention", /*(unsigned) arv_option_frame_retention * 1000*/(unsigned) 200000,NULL);

		}else
            return false;

		// Push 50 buffer in the stream input buffer queue.
		for (int i = 0; i < 50; i++)
			arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));

		return true;

	}

	void CameraGigeSdkAravis::grabStop(){}

	void CameraGigeSdkAravis::acqStart(){

        BOOST_LOG_SEV(logger, notification) << "Set camera to CONTINUOUS MODE";
        arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS);

        BOOST_LOG_SEV(logger, notification) << "Set camera TriggerMode to Off";
        arv_device_set_string_feature_value(arv_camera_get_device (camera), "TriggerMode" , "Off");

        BOOST_LOG_SEV(logger, notification) << "Start acquisition on camera";
		arv_camera_start_acquisition(camera);

	}

	void CameraGigeSdkAravis::acqStop(){

		arv_stream_get_statistics(stream, &nbCompletedBuffers, &nbFailures, &nbUnderruns);

		cout << "Completed buffers = " << (unsigned long long) nbCompletedBuffers	<< endl;
		cout << "Failures          = " << (unsigned long long) nbFailures           << endl;
		cout << "Underruns         = " << (unsigned long long) nbUnderruns          << endl;

		BOOST_LOG_SEV(logger, notification) << "Completed buffers = " << (unsigned long long) nbCompletedBuffers;
		BOOST_LOG_SEV(logger, notification) << "Failures          = " << (unsigned long long) nbFailures;
		BOOST_LOG_SEV(logger, notification) << "Underruns         = " << (unsigned long long) nbUnderruns;

        BOOST_LOG_SEV(logger, notification) << "Stop acquisition on camera";
		arv_camera_stop_acquisition(camera);

		g_object_unref(stream);
		g_object_unref(camera);

	}

	bool CameraGigeSdkAravis::grabImage(Frame &newFrame){

		ArvBuffer *arv_buffer;

		arv_buffer = arv_stream_timeout_pop_buffer(stream,2000000); //us
        char *buffer_data;
        size_t buffer_size;

		if(arv_buffer == NULL){

			 throw runtime_error("arv_buffer is NULL");
			 return false;

		}else{

		    try{

                if(arv_buffer_get_status(arv_buffer) == ARV_BUFFER_STATUS_SUCCESS){

                    //BOOST_LOG_SEV(logger, normal) << "Success to grab a frame.";

                    buffer_data = (char *) arv_buffer_get_data (arv_buffer, &buffer_size);

                    //Timestamping.
                    string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
                    //BOOST_LOG_SEV(logger, normal) << "Date : " << acquisitionDate;
                    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
                    string acqDateInMicrosec = to_iso_extended_string(time);
                    //BOOST_LOG_SEV(logger, normal) << "Date : " << acqDateInMicrosec;

                    Mat image;
                    CamBitDepth imgDepth;
                    int saturateVal = 0;

                    if(pixFormat == ARV_PIXEL_FORMAT_MONO_8){

                        //BOOST_LOG_SEV(logger, normal) << "Creating Mat 8 bits ...";
                        image = Mat(height, width, CV_8UC1, buffer_data);
                        imgDepth = MONO_8;
                        saturateVal = 255;

                    }else if(pixFormat == ARV_PIXEL_FORMAT_MONO_12){

                        //BOOST_LOG_SEV(logger, normal) << "Creating Mat 16 bits ...";
                        image = Mat(height, width, CV_16UC1, buffer_data);
                        imgDepth = MONO_12;
                        saturateVal = 4095;

                        //double t3 = (double)getTickCount();

                        if(shiftImage){

                            //BOOST_LOG_SEV(logger, normal) << "Shifting bits ...";


                                unsigned short * p;

                                for(int i = 0; i < image.rows; i++){
                                    p = image.ptr<unsigned short>(i);
                                    for(int j = 0; j < image.cols; j++)
                                        p[j] = p[j] >> 4;
                                }

                            //BOOST_LOG_SEV(logger, normal) << "Bits shifted.";

                        }

                        //t3 = (((double)getTickCount() - t3)/getTickFrequency())*1000;
                        //cout << "> Time shift : " << t3 << endl;
                    }

                    //BOOST_LOG_SEV(logger, normal) << "Creating frame object ...";
                    newFrame = Frame(image, gain, exp, acquisitionDate);
                    //BOOST_LOG_SEV(logger, normal) << "Setting date of frame ...";
                    newFrame.setAcqDateMicro(acqDateInMicrosec);
                    //BOOST_LOG_SEV(logger, normal) << "Setting fps of frame ...";
                    newFrame.setFPS(fps);
                    newFrame.setBitDepth(imgDepth);
                    //BOOST_LOG_SEV(logger, normal) << "Setting saturated value of frame ...";
                    newFrame.setSaturatedValue(saturateVal);

                    //BOOST_LOG_SEV(logger, normal) << "Re-pushing arv buffer in stream ...";
                    arv_stream_push_buffer(stream, arv_buffer);

                    return true;

                }else{

                    switch(arv_buffer_get_status(arv_buffer)){

                        case 0 :
                            cout << "ARV_BUFFER_STATUS_SUCCESS : the buffer contains a valid image"<<endl;
                            break;
                        case 1 :
                            cout << "ARV_BUFFER_STATUS_CLEARED: the buffer is cleared"<<endl;
                            break;
                        case 2 :
                            cout << "ARV_BUFFER_STATUS_TIMEOUT: timeout was reached before all packets are received"<<endl;
                            break;
                        case 3 :
                            cout << "ARV_BUFFER_STATUS_MISSING_PACKETS: stream has missing packets"<<endl;
                            break;
                        case 4 :
                            cout << "ARV_BUFFER_STATUS_WRONG_PACKET_ID: stream has packet with wrong id"<<endl;
                            break;
                        case 5 :
                            cout << "ARV_BUFFER_STATUS_SIZE_MISMATCH: the received image didn't fit in the buffer data space"<<endl;
                            break;
                        case 6 :
                            cout << "ARV_BUFFER_STATUS_FILLING: the image is currently being filled"<<endl;
                            break;
                        case 7 :
                            cout << "ARV_BUFFER_STATUS_ABORTED: the filling was aborted before completion"<<endl;
                            break;

                    }

                    arv_stream_push_buffer(stream, arv_buffer);

                    return false;
                }

            }catch(exception& e){

                cout << e.what() << endl;
                BOOST_LOG_SEV(logger, critical) << e.what() ;
                return false;

            }
        }
	}


	bool CameraGigeSdkAravis::grabTest(){

	    ArvCamera       *cam;
	    ArvPixelFormat  format;
        ArvStream       *stream_;

        int             width_;
        int             height_;
        double          fps_;
        double          gainMin_,
                        gainMax_;
        unsigned int    payload_;
        const char      *pixel_format_string_;
        double          exposureMin_,
                        exposureMax_;
        const char      *caps_string_;
        int             gain_;
        double          exp_;

        bool			shiftImage;
        guint64         nbCompletedBuffers_;
        guint64         nbFailures_;
        guint64         nbUnderruns_;

        namedWindow( "Display window", WINDOW_AUTOSIZE );

	    /// List connected cameras
	    arv_update_device_list();

		int n_devices = arv_get_n_devices();

		cout << "************** DETECTED CAMERAS *************** " << endl;
		cout << "*" << endl;

		for(int i = 0; i < n_devices; i++){

			cout << "* -> [" << i << "] " << arv_get_device_id(i) << endl;

		}

		cout << "*" << endl;
		cout << "************************************************ " << endl;


        /// Camera to use
        int cameraID = 2;

        /// Parameters
        double e = 4000000; // exposition time to use
        double g = 15; // gain to use
        bool shiftImageBits = true; // 1 for DMK, 0 for BASLER

        /// Get camera name
        string deviceName = "";

		for(int i = 0; i< n_devices; i++){

			if(cameraID == i){

				deviceName = arv_get_device_id(i);
				break;

			}
		}

		if(deviceName != ""){

            cout << "Device : " << deviceName << endl;

            cam = arv_camera_new(deviceName.c_str());

            if(cam != NULL){

                cout << "Connection success to the camera." << endl;

                // Set format MONO_12
                arv_camera_set_pixel_format(cam, ARV_PIXEL_FORMAT_MONO_12);

                // Set exposure
                arv_camera_set_exposure_time(cam, e); // in us

                // Set gain
                arv_camera_set_gain(cam, g);


                int sensor_width, sensor_height;

                arv_camera_get_sensor_size(cam, &sensor_width, &sensor_height);

                // Use maximum sensor size.
                arv_camera_set_region(cam, 0, 0,sensor_width,sensor_height);

                arv_camera_get_region(cam, NULL, NULL, &width_, &height_);

                payload_ = arv_camera_get_payload(cam);

                format = arv_camera_get_pixel_format(cam);

                arv_camera_get_exposure_time_bounds (cam, &exposureMin_, &exposureMax_);

                arv_camera_get_gain_bounds (cam, &gainMin_, &gainMax_);

                arv_camera_set_frame_rate(cam, 3.75);

                fps_= arv_camera_get_frame_rate(cam);

                caps_string_= arv_pixel_format_to_gst_caps_string(format);

                gain_    = arv_camera_get_gain(cam);
                exp_     = arv_camera_get_exposure_time(cam);

                cout << endl;

                cout << "DEVICE SELECTED : " << arv_camera_get_device_id(cam)    << endl;
                cout << "DEVICE NAME     : " << arv_camera_get_model_name(cam)   << endl;
                cout << "DEVICE VENDOR   : " << arv_camera_get_vendor_name(cam)  << endl;
                cout << "PAYLOAD         : " << payload_                             << endl;
                cout << "Width           : " << width_                               << endl
                     << "Height          : " << height_                              << endl;
                cout << "Exp Range       : [" << exposureMin_    << " - " << exposureMax_   << "]"  << endl;
                cout << "Exp             : " << exp_                                 << endl;
                cout << "Gain Range      : [" << gainMin_        << " - " << gainMax_       << "]"  << endl;
                cout << "Gain            : " << gain_                                << endl;
                cout << "Fps             : " << fps_                                 << endl;
                cout << "Type            : " << caps_string_                         << endl;

                cout << endl;

                // Create a new stream object. Open stream on Camera.
                stream_ = arv_camera_create_stream(cam, NULL, NULL);

                if(stream_ != NULL){

                    if(ARV_IS_GV_STREAM(stream_)){

                        g_object_set(stream_, "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO, "socket-buffer-size", 0, NULL);

                        g_object_set(stream_, "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER, NULL);

                        g_object_set(stream_, "packet-timeout", (unsigned)40000, "frame-retention", (unsigned) 200000,NULL);

                    }

                    // Add a buffer in the stream
                    arv_stream_push_buffer(stream_, arv_buffer_new(payload_, NULL));

                    // Set acquisition mode to continuous.
                    //arv_camera_set_acquisition_mode(cam, ARV_ACQUISITION_MODE_CONTINUOUS);

                    arv_camera_set_acquisition_mode(cam, ARV_ACQUISITION_MODE_SINGLE_FRAME);

                    // Configure camera to use software source as a trigger :
                    // - AcquisitionStart in TriggerSelector is set to off.
                    // - FrameStart in TriggerSelector is set to on.
                    // - TriggerActivation is set to RisingEdge.
                    // - TriggerSource is set to Software.
                    //arv_camera_set_trigger(cam, "Software");

                    // Start acquisition.
                    arv_camera_start_acquisition(cam);

                    //sleep(1); // with basler

                    // Send Trigger Command to capture image.
                    //arv_camera_software_trigger(cam);

                    // Get image buffer.
                    ArvBuffer *arv_buffer = /*arv_stream_pop_buffer(stream);*/ arv_stream_timeout_pop_buffer(stream_, e + 1000000); //us

                    char *buffer_data;
                    size_t buffer_size;

                    if (arv_buffer != NULL){

                        if(arv_buffer_get_status(arv_buffer) == ARV_BUFFER_STATUS_SUCCESS){

                            buffer_data = (char *) arv_buffer_get_data (arv_buffer, &buffer_size);

                            Mat image;

                            if(format == ARV_PIXEL_FORMAT_MONO_8){

                                image = Mat(height_, width_, CV_8UC1, buffer_data);

                            }else if(format == ARV_PIXEL_FORMAT_MONO_12){

                                image = Mat(height_, width_, CV_16UC1, buffer_data);

                                if(shiftImageBits){
                                    unsigned short * p;
                                    for(int i = 0; i < image.rows; i++){
                                        p = image.ptr<unsigned short>(i);
                                        for(int j = 0; j < image.cols; j++) p[j] = p[j] >> 4;
                                    }
                                }

                            }

                            imshow( "Display window", image );                   // Show our image inside it.

                            waitKey(0);


                        }else{

                            switch(arv_buffer_get_status(arv_buffer)){

                                case 0 :

                                    cout << "ARV_BUFFER_STATUS_SUCCESS : the buffer contains a valid image"<<endl;

                                    break;

                                case 1 :

                                    cout << "ARV_BUFFER_STATUS_CLEARED: the buffer is cleared"<<endl;

                                    break;

                                case 2 :

                                    cout << "ARV_BUFFER_STATUS_TIMEOUT: timeout was reached before all packets are received"<<endl;

                                    break;

                                case 3 :

                                    cout << "ARV_BUFFER_STATUS_MISSING_PACKETS: stream has missing packets"<<endl;

                                    break;

                                case 4 :

                                    cout << "ARV_BUFFER_STATUS_WRONG_PACKET_ID: stream has packet with wrong id"<<endl;

                                    break;

                                case 5 :

                                    cout << "ARV_BUFFER_STATUS_SIZE_MISMATCH: the received image didn't fit in the buffer data space"<<endl;

                                    break;

                                case 6 :

                                    cout << "ARV_BUFFER_STATUS_FILLING: the image is currently being filled"<<endl;

                                    break;

                                case 7 :

                                    cout << "ARV_BUFFER_STATUS_ABORTED: the filling was aborted before completion"<<endl;

                                    break;


                            }

                        }

                        arv_stream_push_buffer(stream_, arv_buffer);

                   }else{

                        cout << "Fail to pop buffer from stream." << endl;

                   }

                    arv_stream_get_statistics(stream_, &nbCompletedBuffers_, &nbFailures_, &nbUnderruns_);

                    cout << "Completed buffers = " << (unsigned long long) nbCompletedBuffers_	<< endl;
                    cout << "Failures          = " << (unsigned long long) nbFailures_           << endl;
                    cout << "Underruns         = " << (unsigned long long) nbUnderruns_          << endl;


                    // Stop acquisition.
                    arv_camera_stop_acquisition(cam);

                    // TriggerMode off.
                    arv_device_set_string_feature_value(arv_camera_get_device(cam), "TriggerMode" , "Off");

                    g_object_unref(stream_);
                    g_object_unref(cam);

                }

            }

        }

	}

	bool CameraGigeSdkAravis::grabSingleImage(Frame &frame, int camID){

        if(!createDevice(camID))
            return false;

        if(!setPixelFormat(frame.getBitDepth()))
            return false;

        if(!setExposureTime(frame.getExposure()))
            return false;

        if(!setGain(frame.getGain()))
            return false;

		int sensor_width, sensor_height;

		arv_camera_get_sensor_size(camera, &sensor_width, &sensor_height);

        // Use maximum sensor size.
		arv_camera_set_region(camera, 0, 0,sensor_width,sensor_height);

		arv_camera_get_region (camera, NULL, NULL, &width, &height);

		payload = arv_camera_get_payload (camera);

		pixFormat = arv_camera_get_pixel_format (camera);

		arv_camera_get_exposure_time_bounds (camera, &exposureMin, &exposureMax);

		arv_camera_get_gain_bounds (camera, &gainMin, &gainMax);

		arv_camera_set_frame_rate(camera, 3.75);

		fps = arv_camera_get_frame_rate(camera);

		caps_string = arv_pixel_format_to_gst_caps_string(pixFormat);

		gain    = arv_camera_get_gain(camera);
		exp     = arv_camera_get_exposure_time(camera);

		cout << endl;

		cout << "DEVICE SELECTED : " << arv_camera_get_device_id(camera)    << endl;
		cout << "DEVICE NAME     : " << arv_camera_get_model_name(camera)   << endl;
		cout << "DEVICE VENDOR   : " << arv_camera_get_vendor_name(camera)  << endl;
		cout << "PAYLOAD         : " << payload                             << endl;
		cout << "Width           : " << width                               << endl
			 << "Height          : " << height                              << endl;
		cout << "Exp Range       : [" << exposureMin    << " - " << exposureMax   << "]"  << endl;
		cout << "Exp             : " << exp                                 << endl;
		cout << "Gain Range      : [" << gainMin        << " - " << gainMax       << "]"  << endl;
		cout << "Gain            : " << gain                                << endl;
		cout << "Fps             : " << fps                                 << endl;
		cout << "Type            : " << caps_string                         << endl;

		cout << endl;

		// Create a new stream object. Open stream on Camera.
		stream = arv_camera_create_stream(camera, NULL, NULL);

        cout << "shiftBits status : " << shiftImage << endl;

		if(stream != NULL){

            if(ARV_IS_GV_STREAM(stream)){

                bool            arv_option_auto_socket_buffer   = true;
                bool            arv_option_no_packet_resend     = true;
                unsigned int    arv_option_packet_timeout       = 20;
                unsigned int    arv_option_frame_retention      = 100;

                if(arv_option_auto_socket_buffer){

                    g_object_set(stream, "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO, "socket-buffer-size", 0, NULL);

                }

                if(arv_option_no_packet_resend){

                    g_object_set(stream, "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER, NULL);

                }

                g_object_set(stream, "packet-timeout", (unsigned)40000, "frame-retention", (unsigned) 200000,NULL);

            }

            //for(int i = 0; i < 50; i++)
            arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));

            // Set acquisition mode to continuous.
            //arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS);
            arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_SINGLE_FRAME);

            // Configure camera to use software source as a trigger :
            // - AcquisitionStart in TriggerSelector is set to off.
            // - FrameStart in TriggerSelector is set to on.
            // - TriggerActivation is set to RisingEdge.
            // - TriggerSource is set to Software.
            //arv_camera_set_trigger(camera, "Software");

            //arv_device_set_integer_feature_value(arv_camera_get_device (camera), "TriggerDelay" , 0);

            // Start acquisition.
            arv_camera_start_acquisition(camera);


            //sleep(1);


            // Send Trigger Command to capture image.
            //arv_camera_software_trigger(camera);


            //sleep(6);


            // Get image buffer.
            ArvBuffer *arv_buffer = /*arv_stream_pop_buffer(stream);*/ arv_stream_timeout_pop_buffer(stream, frame.getExposure() + 1000000); //us

            char *buffer_data;
            size_t buffer_size;

            if (arv_buffer != NULL){

                if(arv_buffer_get_status(arv_buffer) == ARV_BUFFER_STATUS_SUCCESS){


                    buffer_data = (char *) arv_buffer_get_data (arv_buffer, &buffer_size);

                    //Timestamping.
                    string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
                    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
                    string acqDateInMicrosec = to_iso_extended_string(time);

                    Mat image;

                    if(pixFormat == ARV_PIXEL_FORMAT_MONO_8){

                        image = Mat(height, width, CV_8UC1, buffer_data);


                    }else if(pixFormat == ARV_PIXEL_FORMAT_MONO_12){

                        image = Mat(height, width, CV_16UC1, buffer_data);

                        if(shiftImage){
                            unsigned short * p;
                            for(int i = 0; i < image.rows; i++){
                                p = image.ptr<unsigned short>(i);
                                for(int j = 0; j < image.cols; j++) p[j] = p[j] >> 4;
                            }
                        }

                    }

                    frame = Frame(image, arv_camera_get_gain(camera), arv_camera_get_exposure_time(camera), acquisitionDate);
                    frame.setAcqDateMicro(acqDateInMicrosec);
                    frame.setFPS(arv_camera_get_frame_rate(camera));

                }else{

                    switch(arv_buffer_get_status(arv_buffer)){

                        case 0 :

                            cout << "ARV_BUFFER_STATUS_SUCCESS : the buffer contains a valid image"<<endl;

                            break;

                        case 1 :

                            cout << "ARV_BUFFER_STATUS_CLEARED: the buffer is cleared"<<endl;

                            break;

                        case 2 :

                            cout << "ARV_BUFFER_STATUS_TIMEOUT: timeout was reached before all packets are received"<<endl;

                            break;

                        case 3 :

                            cout << "ARV_BUFFER_STATUS_MISSING_PACKETS: stream has missing packets"<<endl;

                            break;

                        case 4 :

                            cout << "ARV_BUFFER_STATUS_WRONG_PACKET_ID: stream has packet with wrong id"<<endl;

                            break;

                        case 5 :

                            cout << "ARV_BUFFER_STATUS_SIZE_MISMATCH: the received image didn't fit in the buffer data space"<<endl;

                            break;

                        case 6 :

                            cout << "ARV_BUFFER_STATUS_FILLING: the image is currently being filled"<<endl;

                            break;

                        case 7 :

                            cout << "ARV_BUFFER_STATUS_ABORTED: the filling was aborted before completion"<<endl;

                            break;


                    }

                }

                arv_stream_push_buffer(stream, arv_buffer);

           }else{

                cout << "Fail to pop buffer from stream." << endl;
                BOOST_LOG_SEV(logger, fail) << "Fail to pop buffer from stream.";

           }

            arv_stream_get_statistics(stream, &nbCompletedBuffers, &nbFailures, &nbUnderruns);

            cout << "Completed buffers = " << (unsigned long long) nbCompletedBuffers	<< endl;
            cout << "Failures          = " << (unsigned long long) nbFailures           << endl;
            cout << "Underruns         = " << (unsigned long long) nbUnderruns          << endl;


            // Stop acquisition.
            arv_camera_stop_acquisition(camera);

            // TriggerMode off.
            arv_device_set_string_feature_value(arv_camera_get_device (camera), "TriggerMode" , "Off");

            g_object_unref(stream);
            g_object_unref(camera);


            return true;

		}

		return false;

	}

	void CameraGigeSdkAravis::saveGenicamXml(string p){

        const char *xml;

        size_t size;

        xml = arv_device_get_genicam_xml (arv_camera_get_device(camera), &size);

        if (xml != NULL){

            ofstream infFile;
            string infFilePath = p + "genicam.xml";
            infFile.open(infFilePath.c_str());
            infFile << string ( xml, size );
            infFile.close();

        }

	}

	void CameraGigeSdkAravis::getExposureBounds(int &eMin, int &eMax){

		double exposureMin = 0.0;
		double exposureMax = 0.0;

		arv_camera_get_exposure_time_bounds(camera, &exposureMin, &exposureMax);

		eMin = exposureMin;
		eMax = exposureMax;

	}

	void CameraGigeSdkAravis::getGainBounds(int &gMin, int &gMax){

		double gainMin = 0.0;
		double gainMax = 0.0;

		arv_camera_get_gain_bounds(camera, &gainMin, &gainMax);

		gMin = gainMin;
		gMax = gainMax;

	}

	bool CameraGigeSdkAravis::getPixelFormat(CamBitDepth &format){

		ArvPixelFormat pixFormat = arv_camera_get_pixel_format(camera);

		switch(pixFormat){

			case ARV_PIXEL_FORMAT_MONO_8 :

					format = MONO_8;

				break;

			case ARV_PIXEL_FORMAT_MONO_12 :

					format = MONO_12;

				break;

			default :

					return false;

				break;

		}

		return true;
	}

	int CameraGigeSdkAravis::getWidth(){

		int w = 0, h = 0;

		arv_camera_get_region(camera, NULL, NULL, &w, &h);

		return w;

	}

	int CameraGigeSdkAravis::getHeight(){

		int w = 0, h = 0;

		arv_camera_get_region(camera, NULL, NULL, &w, &h);

		return h;

	}

	int CameraGigeSdkAravis::getFPS(void){

		return (int)arv_camera_get_frame_rate(camera);

	}

	string CameraGigeSdkAravis::getModelName(){

		return arv_camera_get_model_name(camera);

	}

	bool CameraGigeSdkAravis::setExposureTime(int val){

		double expMin, expMax;

		arv_camera_get_exposure_time_bounds(camera, &expMin, &expMax);

		if (camera != NULL){

			if((double)val >= expMin && (double)val <= expMax)

				arv_camera_set_exposure_time(camera, (double)val);

			else{

				cout << "> Exposure value (" << val << ") is not in range [ " << expMin << " - " << expMax << " ]" << endl;
				return false;

			}

			return true;

		}

		return false;
	}

	bool CameraGigeSdkAravis::setGain(int val){

		double gMin, gMax;

		arv_camera_get_gain_bounds (camera, &gMin, &gMax);

		if (camera != NULL){

			if((double)val >= gMin && (double)val <= gMax)

				arv_camera_set_gain (camera, (double)val);

			else{

				cout << "> Gain value (" << val << ") is not in range [ " << gMin << " - " << gMax << " ]" << endl;
				return false;

			}

			return true;

		}

		return false;

	}

	bool CameraGigeSdkAravis::setFPS(int fps){

		if (camera != NULL){

			arv_camera_set_frame_rate(camera, fps);

			return true;

		}

		return false;

	}

	bool CameraGigeSdkAravis::setPixelFormat(CamBitDepth depth){

		if (camera != NULL){

			switch(depth){

				case MONO_8 :

					arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_MONO_8);

					break;

				case MONO_12 :

					arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_MONO_12);

					break;

			}

			return true;
		}

		return false;

	}

#endif
