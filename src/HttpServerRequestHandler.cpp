/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HttpServerRequestHandler.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include <stdexcept>
#include <iostream>
	
#include "V4l2Capture.h"
#include "HttpServerRequestHandler.h"

/* ---------------------------------------------------------------------------
**  Civet HTTP callback 
** -------------------------------------------------------------------------*/
class RequestHandler : public CivetHandler
{
  public:
	bool handle(CivetServer *server, struct mg_connection *conn)
	{
		bool ret = false;
		const struct mg_request_info *req_info = mg_get_request_info(conn);
		
		std::cout << "uri:" << req_info->request_uri << std::endl;
		
		HttpServerRequestHandler* httpServer = (HttpServerRequestHandler*)server;
		
		httpFunction fct = httpServer->getFunction(req_info->request_uri);
		if (fct != NULL)
		{
			Json::Value  jmessage;			
			
			// read input
			long long tlen = req_info->content_length;
			if (tlen > 0)
			{
				std::string body;	
				long long nlen = 0;
				char buf[1024];			
				while (nlen < tlen) {
					long long rlen = tlen - nlen;
					if (rlen > sizeof(buf)) {
						rlen = sizeof(buf);
					}
					rlen = mg_read(conn, buf, (size_t)rlen);
					if (rlen <= 0) {
						break;
					}
					body.append(buf, rlen);
					
					nlen += rlen;
				}
				std::cout << "body:" << body << std::endl;

				// parse in
				Json::Reader reader;
				if (!reader.parse(body, jmessage)) 
				{
					std::cout << "Received unknown message:" << body << std::endl;
				}
			}
			
			// invoke API implementation
			Json::Value out(fct(conn, jmessage));
			
			// fill out
			if (out.isNull() == false)
			{
				std::string answer(Json::StyledWriter().write(out));
				std::cout << "answer:" << answer << std::endl;	

				mg_printf(conn,"HTTP/1.1 200 OK\r\n");
				mg_printf(conn,"Access-Control-Allow-Origin: *\r\n");
				mg_printf(conn,"Content-Type: text/plain\r\n");
				mg_printf(conn,"Content-Length: %zd\r\n", answer.size());
				mg_printf(conn,"Connection: close\r\n");
				mg_printf(conn,"\r\n");
				mg_printf(conn,answer.c_str());	
				
				ret = true;
			}			
		}		
		
		return ret;
	}
	bool handleGet(CivetServer *server, struct mg_connection *conn)
	{
		return handle(server, conn);
	}
	bool handlePost(CivetServer *server, struct mg_connection *conn)
	{
		return handle(server, conn);
	}	
};


class WebsocketHandler: public CivetWebSocketHandler {	
	virtual bool handleConnection(CivetServer *server, const struct mg_connection *conn) {
		printf("WS connected\n");
		
		HttpServerRequestHandler* httpServer = (HttpServerRequestHandler*)server;
		httpServer->addWebsocketConnection(conn);
		
		return true;
	}

	virtual void handleReadyState(CivetServer *server, struct mg_connection *conn) {
		printf("WS ready\n");

		const char *text = "READY";
		mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	}

	virtual bool handleData(CivetServer *server,
				struct mg_connection *conn,
				int bits,
				char *data,
				size_t data_len) {
		printf("WS got %lu bytes: ", (long unsigned)data_len);
		fwrite(data, 1, data_len, stdout);
		printf("\n");

		mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, data, data_len);
		return (data_len<4);
	}

	virtual void handleClose(CivetServer *server, const struct mg_connection *conn) {
		printf("WS closed\n");
		
		HttpServerRequestHandler* httpServer = (HttpServerRequestHandler*)server;
		httpServer->delWebsocketConnection(conn);		
	}	
};

/* ---------------------------------------------------------------------------
**  Constructor
** -------------------------------------------------------------------------*/
HttpServerRequestHandler::HttpServerRequestHandler(V4l2Capture* videoCapture, const std::vector<std::string>& options) 
	: CivetServer(options), m_v4l2web(videoCapture)
{
	// http api callbacks
	m_func["/capabilities"]   = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_capabilities_reply();
	};
	m_func["/inputs"]         = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_inputs_reply();
	};
	m_func["/formats"]        = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_formats_reply();
	};
	m_func["/format"]         = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_format_reply(in);
	};
	m_func["/controls"]       = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_controls_reply();
	};
	m_func["/control"]        = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_control_reply(in);
	};	
	m_func["/start"]          = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_start_reply();
	};
	m_func["/stop"]           = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_start_reply();
	};
	m_func["/isCapturing"]    = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		return m_v4l2web.send_isCapturing_reply();
	};
	
	m_func["/help"]           = [this](struct mg_connection *conn, const Json::Value & in) -> Json::Value { 
		Json::Value answer;
		for (auto it : m_func) {
			answer.append(it.first);
		}
		return answer;
	};
		
	// register handlers
	for (auto it : m_func) {
		this->addHandler(it.first, new RequestHandler());
	} 	
	
	// register WS handlers
	this->addWebSocketHandler("/ws", new WebsocketHandler());
}	
	

httpFunction HttpServerRequestHandler::getFunction(const std::string& uri)
{
	httpFunction fct = NULL;
	std::map<std::string,httpFunction>::iterator it = m_func.find(uri);
	if (it != m_func.end())
	{
		fct = it->second;
	}
	
	return fct;
}

void HttpServerRequestHandler::addWebsocketConnection(const struct mg_connection *conn)
{
	m_ws.push_back(conn);
}

void HttpServerRequestHandler::delWebsocketConnection(const struct mg_connection *conn)
{
	m_ws.remove(conn);
}

void HttpServerRequestHandler::notifyWebsocketConnection(const char* buffer, unsigned int size)
{
	for (auto ws : m_ws) {
		mg_websocket_write((struct mg_connection *)ws, WEBSOCKET_OPCODE_BINARY, buffer, size);
	}
}
