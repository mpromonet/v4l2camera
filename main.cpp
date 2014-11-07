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
#include <fstream>
	
#include "mongoose.h"
#include <jpeglib.h>

#include "V4l2MmapCapture.h"
#include "V4l2ReadCapture.h"
#include "v4l2web.h"

#define LOG(verbose,...) do { if (verbose) fprintf(stderr,__VA_ARGS__); } while(0);

/* ---------------------------------------------------------------------------
**  mongoose callback
** -------------------------------------------------------------------------*/
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

/* ---------------------------------------------------------------------------
**  convert yuyv -> jpeg
** -------------------------------------------------------------------------*/
unsigned long yuyv2jpeg(char* image_buffer, unsigned int width, unsigned int height, unsigned int quality)
{
	struct jpeg_error_mgr jerr;
	struct jpeg_compress_struct cinfo;	
	jpeg_create_compress(&cinfo);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;	
	cinfo.err = jpeg_std_error(&jerr);
	
	unsigned char* dest = NULL;
	unsigned long  destsize = 0;
	jpeg_mem_dest(&cinfo, &dest, &destsize);
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	unsigned char bufline[cinfo.image_width * 3]; 
	while (cinfo.next_scanline < cinfo.image_height) 
	{ 
		// convert line from YUYV -> YUV
		for (unsigned int i = 0; i < cinfo.image_width; i += 2) 
		{ 
			unsigned int base = cinfo.next_scanline*cinfo.image_width * 2 ;
			bufline[i*3  ] = image_buffer[base + i*2  ]; 
			bufline[i*3+1] = image_buffer[base + i*2+1]; 
			bufline[i*3+2] = image_buffer[base + i*2+3]; 
			bufline[i*3+3] = image_buffer[base + i*2+2]; 
			bufline[i*3+4] = image_buffer[base + i*2+1]; 
			bufline[i*3+5] = image_buffer[base + i*2+3]; 
		} 
		JSAMPROW row = bufline; 
		jpeg_write_scanlines(&cinfo, &row, 1); 
	}
	jpeg_finish_compress(&cinfo);
	if (dest != NULL)
	{
		if (destsize < width*height*2)
		{
			memcpy(image_buffer, dest, destsize);
		}
		else
		{
			fprintf(stderr, "compressed image bigger than original one size:%d compressedsize:%ld\n", width*height*2, destsize);
		}
		free(dest);
	}
	jpeg_destroy_compress(&cinfo);
	
	return destsize;
}

void v4l2processing(struct mg_server *server, V4l2Capture* dev, int format, int width, int height, int verbose)
{
	if (dev->isReady())
	{
		int fd = dev->getFd();
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
				char buf[dev->getBufferSize()];
				ssize_t size = dev->read(buf, sizeof(buf));
				LOG(verbose, "read size:%d\n", size);
				
				// compress 
				if ( (size>0) && (format == V4L2_PIX_FMT_YUYV) )
				{
					size = yuyv2jpeg(buf, width, height, 95);							
				}
				// post to subscribers
				if (size>0)
				{
					for (struct mg_connection *c = mg_next(server, NULL); c != NULL; c = mg_next(server, c)) 
					{
						const url_handler* url = find_url(c->uri);
						if (url && url->handle_notify)
						{
							LOG(verbose, "notify:%s %d\n", c->uri ,size)
							url->handle_notify(c, buf, size);
						}
					}							
				}
			}
		}
	}
}

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char* argv[]) 
{	
	int verbose=0;
	const char *dev_name = "/dev/video0";	
	int width = 320;
	int height = 240;	
	int fps = 10;	
	int c = 0;
	const char * port = "8080";
	bool useMmap = false;
	
	while ((c = getopt (argc, argv, "hW:H:P:F:v::rM")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose = 1; if (optarg && *optarg=='v') verbose++;  break;
			case 'W':	width = atoi(optarg); break;
			case 'H':	height = atoi(optarg); break;
			case 'F':	fps = atoi(optarg); break;
			case 'P':	port = optarg; break;
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
	
	int format = V4L2_PIX_FMT_JPEG;
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
	
	if (videoCapture == NULL)
	{	
		printf("Cannot create JPEG capture for device:%s => try YUYV capture \n", dev_name);
		param.m_format = V4L2_PIX_FMT_YUYV;
		if (useMmap)
		{
			videoCapture = V4l2MmapCapture::createNew(param);
		}
		else
		{
			videoCapture = V4l2ReadCapture::createNew(param);
		}
	}
	
	if (videoCapture == NULL)
	{	
		printf("Cannot create V4L2 capture interface %s\n", dev_name);
	}
	else
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
			v4l2processing(server, videoCapture, param.m_format, width, height, verbose);
		}
		mg_destroy_server(&server);
		
		delete videoCapture;
	}
	
	return 0;
}
