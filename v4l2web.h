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

typedef int (*callback)(struct mg_connection *conn);
typedef int (*callback_notify)(struct mg_connection *conn, char* buffer, ssize_t size);
struct url_handler
{
	const char* uri;
	callback handle_req;
	callback handle_close;
	callback_notify handle_notify;
};

extern url_handler urls[];

#endif

