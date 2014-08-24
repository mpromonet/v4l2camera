#include <string.h>
#include <time.h>
#include "mongoose.h"

#include <sys/stat.h>
#include <errno.h>
#include "V4l2MMAPDeviceSource.h"
#include <linux/videodev2.h>

struct image 
{
	image(char* buffer, ssize_t length) : m_buffer(buffer), m_length(length) {}
	char* m_buffer;
	ssize_t m_length;
};

V4L2DeviceParameters param("/dev/video0",V4L2_PIX_FMT_JPEG,640,480,25,1);
extern const char *find_embedded_file(const char *, size_t *);

static int iterate_callback(struct mg_connection *c, enum mg_event ev) 
{
	if (ev == MG_POLL && c->is_websocket) 
	{
		image* img = (image*)c->callback_param;
		mg_websocket_write(c, WEBSOCKET_OPCODE_BINARY, img->m_buffer, img->m_length);
	}
	return MG_TRUE;
}

static int send_reply(struct mg_connection *conn) 
{
	if (conn->is_websocket) 
	{	
		mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, conn->content, conn->content_len);
		return conn->content_len == 4 && !memcmp(conn->content, "exit", 4) ? MG_FALSE : MG_TRUE;
	} 
	else 
	{
		size_t index_size;
		const char *index_html = find_embedded_file("websocket.html", &index_size);
		mg_send_header(conn, "Content-Type", "text/html");
		mg_send_data(conn, index_html, index_size);
		return MG_TRUE;
	}
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) 
{
	if (ev == MG_REQUEST) 
	{
		return send_reply(conn);
	} 
	else if (ev == MG_AUTH) 
	{
		return MG_TRUE;
	} 
	else 
	{
		return MG_FALSE;
	}
}

int main(void) 
{
	struct mg_server *server = mg_create_server(NULL, ev_handler);
	mg_set_option(server, "listening_port", "8080");
	printf("Started on port %s\n", mg_get_option(server, "listening_port"));
	
	V4L2Device* videoCapture = V4L2MMAPDeviceSource::createNew(param);
	for (;;) 
	{
		mg_poll_server(server, 100);
		int fd = videoCapture->getFd();
		struct timeval tv;
		timerclear(&tv);
		fd_set read_set;
		FD_ZERO(&read_set);
		FD_SET(fd,&read_set);
		if (select(fd+1, &read_set, NULL, NULL, &tv) >0)
		{
			char buf[videoCapture->getBufferSize()];
			ssize_t size = videoCapture->read(buf, sizeof(buf));
			fprintf(stderr, "read size:%d\n", size);
			if (size>0)
			{
				image img(buf, size);
				mg_iterate_over_connections(server, iterate_callback, &img);
			}
		}
	}

	videoCapture->captureStop();
	delete videoCapture;
	
	mg_destroy_server(&server);
	
	return 0;
}
