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
#include <thread>
#include <mutex>

#include "json/json.h"

#include "HttpServerRequestHandler.h"
#include "encoderfactory.h"
#include "V4l2RTSPServer.h"

class V4l2web {
	public:
		V4l2web(V4l2Capture*  videoCapture, V4l2Output*  videoOutput, const std::vector<std::string> & options);
		virtual ~V4l2web();
		const void* getContext() { return m_httpServer.getContext(); }
	
	private:	
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
		
		void capturing();
		void streaming();
	
		std::map<std::string,HttpServerRequestHandler::httpFunction>& getHttpFunc();
		std::map<std::string,HttpServerRequestHandler::wsFunction>&   getWsFunc();

	private:
		std::mutex                                                    m_deviceMutex; 
		V4l2Capture*                                                  m_videoCapture;
		V4l2Output*                                                   m_videoOutput;
		Encoder*                                                      m_encoder;

		std::map<std::string,HttpServerRequestHandler::httpFunction>  m_httpfunc;
		std::map<std::string,HttpServerRequestHandler::wsFunction>    m_wsfunc;
		HttpServerRequestHandler                                      m_httpServer;

		bool                                                          m_isCapturing; 
		std::thread                                                   m_capturing;
		bool                                                          m_stopCapturing;

		V4l2RTSPServer                                                m_rtspServer;
		std::thread                                                   m_streaming;
		char                                                          m_stopStreaming;
};


