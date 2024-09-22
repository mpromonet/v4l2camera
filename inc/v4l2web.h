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
#include <condition_variable>
#include <atomic>

#include "json/json.h"

#include "HttpServerRequestHandler.h"
#include "V4l2RTSPServer.h"
#include "VideoCaptureAccess.h"

#ifdef WITH_COMPRESS
#include "codecfactory.h"
#endif

class V4l2web {
	public:
		V4l2web(V4l2Capture*  videoCapture, DeviceInterface* audioCapture, V4l2Output*  videoOutput, const std::vector<std::string> & options, int rtspport, const std::string & rtspSslKeyCert);
		virtual ~V4l2web();
		const void* getContext() { return m_httpServer.getContext(); }
	
	private:	
		// http api callback
		Json::Value capabilities(); 
		Json::Value formats();
		Json::Value format(const Json::Value & input);
		Json::Value controls() ;
		Json::Value control(const Json::Value & input) ;
		Json::Value start();
		Json::Value stop();
		Json::Value isCapturing();
		Json::Value rtspInfo(const Json::Value & input);
		
		void capturing();
		void createRtspSession(const std::string & rtspuri, const std::string & multicasturi = "");
	
		std::map<std::string,HttpServerRequestHandler::httpFunction>& getHttpFunc();
		std::map<std::string,HttpServerRequestHandler::wsFunction>&   getWsFunc();

	private:
		std::mutex                                                    m_deviceMutex; 
		std::condition_variable                                       m_actionPending;
		std::atomic<bool>                                             m_askToInterupt;

		V4l2Capture*                                                  m_videoCapture;
		DeviceInterface*                                              m_videoInterface;
		DeviceInterface*                                              m_audioInterface;
		V4l2Output*                                                   m_videoOutput;
#ifdef WITH_COMPRESS
		Codec*                                                        m_encoder;
#endif

		std::map<std::string,HttpServerRequestHandler::httpFunction>  m_httpfunc;
		std::map<std::string,HttpServerRequestHandler::wsFunction>    m_wsfunc;
		HttpServerRequestHandler                                      m_httpServer;

		bool                                                          m_isCapturing; 
		std::thread                                                   m_capturing;
		bool                                                          m_stopCapturing;

		V4l2RTSPServer                                                m_rtspServer;
		StreamReplicator*                                             m_videoReplicator;
		StreamReplicator*                                             m_audioReplicator;
		ServerMediaSession*                                           m_sms;
		std::thread                                                   m_streaming;
		char                                                          m_stopStreaming;
		std::string                                                   m_snapshot;
		std::string                                                   m_rtspuri;
		std::string                                                   m_multicasturi;
		Json::StreamWriterBuilder                                     m_jsonWriterBuilder;
		std::string                                                   m_rtspSslKeyCert;
};


