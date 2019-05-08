/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2web.h
** 
** -------------------------------------------------------------------------*/

#ifndef V4L2_WEB_H
#define V4L2_WEB_H

#include <functional>
#include <list>

#include "json/json.h"
#include "CivetServer.h"


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
		V4l2Capture*                            m_videoCapture;
};

typedef std::function<Json::Value(struct mg_connection *conn, const Json::Value &)> httpFunction;

/* ---------------------------------------------------------------------------
**  http callback
** -------------------------------------------------------------------------*/
class HttpServerRequestHandler : public CivetServer
{
	public:
		HttpServerRequestHandler(V4l2Capture* videoCapture, const std::vector<std::string>& options); 
	
		httpFunction getFunction(const std::string& uri);
		void addWebsocketConnection(const struct mg_connection *conn);
		void delWebsocketConnection(const struct mg_connection *conn);
	
		void notifyWebsocketConnection(const char* buf, unsigned int size);
				
	protected:
		V4l2web                                                 m_v4l2web;
		std::map<std::string,httpFunction>      m_func;
		std::list<const struct mg_connection *> m_ws;
};

#endif

