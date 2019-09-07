/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2web.h
** 
** -------------------------------------------------------------------------*/

#pragma once

#include <functional>
#include <list>

#include "json/json.h"


class V4l2web {
	public:
		V4l2web(V4l2Capture*  videoCapture): m_videoCapture(videoCapture) {}
	
		Json::Value send_capabilities_reply(); 
		Json::Value send_inputs_reply();
		Json::Value send_formats_reply();
		Json::Value send_format_reply(const Json::Value & input);
		Json::Value send_controls_reply() ;
		Json::Value send_control_reply(const Json::Value & input) ;
		Json::Value send_start_reply();
		Json::Value send_stop_reply();
		Json::Value send_isCapturing_reply();
	
	private:
		V4l2Capture*              m_videoCapture;
};


