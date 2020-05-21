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

#include "HttpServerRequestHandler.h"

class V4l2web {
	public:
		V4l2web(V4l2Capture*  videoCapture);
	
		// http api callback
		Json::Value capabilities(); 
		Json::Value inputs();
		Json::Value formats();
		Json::Value format(const Json::Value & input);
		Json::Value controls() ;
		Json::Value control(const Json::Value & input) ;
		Json::Value start();
		Json::Value stop();
		Json::Value isCapturing();

		const std::map<std::string,HttpServerRequestHandler::httpFunction> getHttpApi() { return m_httpfunc; }; 		
		const std::map<std::string,HttpServerRequestHandler::wsFunction> getWsApi() { return m_wsfunc; };

	private:
		V4l2Capture*                                                  m_videoCapture;
		std::map<std::string,HttpServerRequestHandler::httpFunction>  m_httpfunc;
		std::map<std::string,HttpServerRequestHandler::wsFunction>    m_wsfunc;
};


