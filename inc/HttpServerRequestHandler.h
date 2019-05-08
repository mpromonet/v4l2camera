/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HttpServerRequestHandler.h
** 
** -------------------------------------------------------------------------*/

#pragma once

#include <functional>
#include <list>

#include "v4l2web.h"
#include "json/json.h"
#include "CivetServer.h"


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


