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

#include <stdio.h>
#include <jpeglib.h>

#include "logger.h"

#include "V4l2Capture.h"

#include "v4l2web.h"


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
			LOG(WARN) << "Buffer to small size:" << width*height*2 << " " << destsize; 
		}
		free(dest);
	}
	jpeg_destroy_compress(&cinfo);
	
	return destsize;
}

/* ---------------------------------------------------------------------------
**  V4L2 processing
** -------------------------------------------------------------------------*/
void v4l2processing(HttpServerRequestHandler & server, V4l2Capture* dev, int width, int height)
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
				// update format informations
				dev->queryFormat();
				
				// read image
				char buf[dev->getBufferSize()];
				ssize_t size = dev->read(buf, dev->getBufferSize());
				LOG(DEBUG) << "read size:" << size << " buffersize:" << dev->getBufferSize();
				
				// compress 
				if ( (size>0) && (dev->getFormat() == V4L2_PIX_FMT_YUYV) )
				{
					size = yuyv2jpeg(buf, width, height, 95);							
				}
				// post to subscribers
				if (size>0)
				{
					server.notifyWebsocketConnection(buf, size);
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
	V4l2Access::IoType ioTypeIn = V4l2Access::IOTYPE_MMAP;
	std::string webroot = "webroot";
	
	while ((c = getopt (argc, argv, "hv::" "W:H:F:r" "P:p:")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose = 1; if (optarg && *optarg=='v') verbose++;  break;

			case 'W':	width = atoi(optarg); break;
			case 'H':	height = atoi(optarg); break;
			case 'F':	fps = atoi(optarg); break;
			case 'r':	ioTypeIn = V4l2Access::IOTYPE_READWRITE; break;			

			case 'P':	port = optarg; break;
			case 'p':	webroot = optarg; break;			
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-P port] [-W width] [-H height] [device]" << std::endl;
				std::cout << "\t -v       : verbose " << std::endl;
				std::cout << "\t -v v     : very verbose " << std::endl;
				std::cout << "\t -P port  : server port (default "<< port << ")" << std::endl;
				std::cout << "\t -p path  : server root path (default "<< webroot << ")" << std::endl;

				std::cout << "\t -W width : V4L2 capture width (default "<< width << ")" << std::endl;
				std::cout << "\t -H height: V4L2 capture height (default "<< height << ")" << std::endl;
				std::cout << "\t -F fps   : V4L2 capture framerate (default "<< fps << ")" << std::endl;
				std::cout << "\t -r       : V4L2 capture using memory mapped buffers (default use read interface)" << std::endl;				

				std::cout << "\t device   : V4L2 capture device (default "<< dev_name << ")" << std::endl;
				exit(0);
			}
		}
	}
	if (optind<argc)
	{
		dev_name = argv[optind];
	}	

	// initialize log4cpp
	initLogger(verbose);

	// init V4L2 capture interface
	int format = V4L2_PIX_FMT_JPEG;
	V4L2DeviceParameters param(dev_name,format,width,height,fps,verbose);
	V4l2Capture* videoCapture =  V4l2Capture::create(param, ioTypeIn);
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot create JPEG capture for device:" << dev_name << " => try YUYV capture"; 
		param.m_format = V4L2_PIX_FMT_YUYV;
		videoCapture = V4l2Capture::create(param, ioTypeIn);
	}
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot create YUYV capture for device:" << dev_name << " => keep current format"; 
		param.m_format = 0;
		videoCapture = V4l2Capture::create(param, ioTypeIn);
	}	
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot create V4L2 capture interface for device:" << dev_name; 
	}
	else
	{		
		std::vector<std::string> options;
		options.push_back("document_root");
		options.push_back(webroot);
		options.push_back("listening_ports");
		options.push_back(port);
		
		HttpServerRequestHandler httpServer(videoCapture, options);
		if (httpServer.getContext() == NULL)
		{
			LOG(WARN) << "Cannot listen on port:" << port; 
		}
		else
		{
			LOG(NOTICE) << "Started on port:" << port << " webroot:" << webroot; 
			while (true) {
				v4l2processing(httpServer, videoCapture, width, height);
			}
		}
		delete videoCapture;
	}
	
	return 0;
}
