/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/videodev2.h>
	
#include "mongoose.h"

#include "V4l2MmapCapture.h"
#include "V4l2ReadCapture.h"
#include "v4l2web.h"

const url_handler* find_url(const char* uri)
{
	const url_handler* url = NULL;
	if (uri != NULL)
	{
		for (int i=0; urls[i].uri ; ++i)
		{
			if (strcmp(urls[i].uri, uri) == 0)
			{
				url = &urls[i];
				break;
			}
		}
	}
	return url;
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) 
{
	int ret = MG_FALSE;
	switch (ev) 
	{
		case MG_AUTH: ret = MG_TRUE; break;
		case MG_REQUEST: 
		{
			const url_handler* url = find_url(conn->uri);
			if (url && url->handle_req)
			{
				ret = url->handle_req(conn);
			}
		}
		break;
		case MG_CLOSE:
		{
			const url_handler* url = find_url(conn->uri);
			if (url && url->handle_close)
			{
				ret = url->handle_close(conn);
			}
		}
		break;
		default: break;
	} 
	return ret;
}

int main(int argc, char* argv[]) 
{	
	int verbose=0;
	const char *dev_name = "/dev/video0";	
	int width = 320;
	int height = 240;	
	int fps = 10;	
	int c = 0;
	const char * port = "8080";
	int format = V4L2_PIX_FMT_JPEG;
	bool useMmap = false;
	
	while ((c = getopt (argc, argv, "hW:H:P:F:v::f:M")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose = 1; if (optarg && *optarg=='v') verbose++;  break;
			case 'W':	width = atoi(optarg); break;
			case 'H':	height = atoi(optarg); break;
			case 'F':	fps = atoi(optarg); break;
			case 'P':	port = optarg; break;
			case 'f':	format = V4L2_PIX_FMT_MJPEG; break;
			case 'M':	useMmap = true; break;			
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-P port] [-W width] [-H height] [device]" << std::endl;
				std::cout << "\t -v       : verbose " << std::endl;
				std::cout << "\t -v v     : very verbose " << std::endl;
				std::cout << "\t -P port  : server port (default "<< port << ")" << std::endl;
				std::cout << "\t -W width : V4L2 capture width (default "<< width << ")" << std::endl;
				std::cout << "\t -H height: V4L2 capture height (default "<< height << ")" << std::endl;
				std::cout << "\t -F fps   : V4L2 capture framerate (default "<< fps << ")" << std::endl;
				std::cout << "\t -f MJPEG : V4L2 capture MJPEG format (default JPEG)" << std::endl;	
				std::cout << "\t -M       : V4L2 capture using memory mapped buffers (default use read interface)" << std::endl;				
				std::cout << "\t device   : V4L2 capture device (default "<< dev_name << ")" << std::endl;
				exit(0);
			}
		}
	}
	if (optind<argc)
	{
		dev_name = argv[optind];
	}	
	
	V4L2DeviceParameters param(dev_name,format,width,height,fps,verbose);
	V4l2Capture* videoCapture = NULL;
	if (useMmap)
	{
		videoCapture = V4l2MmapCapture::createNew(param);
	}
	else
	{
		videoCapture = V4l2ReadCapture::createNew(param);
	}
	if (videoCapture)
	{	
		struct mg_server *server = mg_create_server(videoCapture, ev_handler);
		mg_set_option(server, "listening_port", port);
		std::string currentPath(get_current_dir_name());
		currentPath += "/webroot";
		mg_set_option(server, "document_root", currentPath.c_str());
		
		chdir(mg_get_option(server, "document_root"));
		printf("Started on port %s root:%s\n", mg_get_option(server, "listening_port"), mg_get_option(server, "document_root"));	
		for (;;) 
		{
			mg_poll_server(server, 10);
			if (videoCapture->isReady())
			{
				int fd = videoCapture->getFd();
				struct timeval tv;
				timerclear(&tv);
				fd_set read_set;
				FD_ZERO(&read_set);
				FD_SET(fd,&read_set);
				if (select(fd+1, &read_set, NULL, NULL, &tv) >0)
				{
					if (FD_ISSET(fd,&read_set))
					{
						// read image
						char buf[videoCapture->getBufferSize()];
						ssize_t size = videoCapture->read(buf, sizeof(buf));
						if (verbose)
						{
							fprintf(stderr, "read size:%d\n", size);
						}
						// post to subscribers
						if (size>0)
						{
							for (struct mg_connection *c = mg_next(server, NULL); c != NULL; c = mg_next(server, c)) 
							{
								const url_handler* url = find_url(c->uri);
								if (url && url->handle_notify)
								{
									fprintf(stderr, "notify:%s %d\n", c->uri ,size);
									url->handle_notify(c, buf, size);
								}
							}
						}
					}
				}
			}
		}
		mg_destroy_server(&server);
		
		delete videoCapture;
	}
	
	return 0;
}
