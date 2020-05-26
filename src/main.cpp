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
#include <signal.h>

#include <stdio.h>

#include "logger.h"

#include "V4l2Capture.h"
#include "V4l2Output.h"
#include "v4l2web.h"

/* ---------------------------------------------------------------------------
**  end condition
** -------------------------------------------------------------------------*/
int stop=0;

/* ---------------------------------------------------------------------------
**  SIGINT handler
** -------------------------------------------------------------------------*/
void sighandler(int)
{ 
       printf("SIGINT\n");
       stop =1;
}

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char* argv[]) 
{	
	int verbose=0;
	const char *dev_name = "/dev/video0";	
	int width = 640;
	int height = 480;	
	int fps = 10;	
	int c = 0;
	const char * port = "8080";
	V4l2Access::IoType ioTypeIn = V4l2Access::IOTYPE_MMAP;
	V4l2Access::IoType ioTypeOut = V4l2Access::IOTYPE_MMAP;
	std::string webroot = "webroot";
	std::string nbthreads;
	unsigned int format = ~0;
	std::list<unsigned int> videoformatList;	
	std::string out_devname;	
	unsigned int outFormat = V4L2_PIX_FMT_H264;
	
	while ((c = getopt (argc, argv, "hv::" "f::W:H:F:G:" "O:" "rw" "P:p:N:")) != -1)
	{
		switch (c)
		{
			case 'v': verbose = 1; if (optarg && *optarg=='v') verbose++;  break;

			case 'f': format = V4l2Device::fourcc(optarg); if (format) {videoformatList.push_back(format);};  break;
			case 'W': width = atoi(optarg); break;
			case 'H': height = atoi(optarg); break;
			case 'F': fps = atoi(optarg); break;
			case 'G': sscanf(optarg,"%dx%dx%d", &width, &height, &fps); break;
			
			case 'O': outFormat = V4l2Device::fourcc(optarg); break;

			case 'r': ioTypeIn = V4l2Access::IOTYPE_READWRITE; break;
			case 'w': ioTypeOut = V4l2Access::IOTYPE_READWRITE; break;				

			case 'P': port = optarg; break;
			case 'N': nbthreads = optarg; break;
			case 'p': webroot = optarg; break;			
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-P port] [-W width] [-H height] [-F fps] [-G <w>x<h>x<f>] [device]" << std::endl;
				std::cout << "\t -v               : verbose " << std::endl;
				std::cout << "\t -v v             : very verbose " << std::endl;
				std::cout << "\t -P port          : server port (default "<< port << ")" << std::endl;
				std::cout << "\t -p path          : server root path (default "<< webroot << ")" << std::endl;

				std::cout << "\t -f format        : V4L2 capture using format" << std::endl;
				std::cout << "\t -W width         : V4L2 capture width (default "<< width << ")" << std::endl;
				std::cout << "\t -H height        : V4L2 capture height (default "<< height << ")" << std::endl;
				std::cout << "\t -F fps           : V4L2 capture framerate (default "<< fps << ")" << std::endl;
				std::cout << "\t -G <w>x<h>[x<f>] : V4L2 capture format (default "<< width << "x" << height << "x" << fps << ")"  << std::endl;

				std::cout << "\t -r               : V4L2 capture using memory mapped buffers (default use read interface)" << std::endl;
				std::cout << "\t -w               : V4L2 capture using write interface (default use memory mapped buffers)" << std::endl;				

				std::cout << "\t device           : V4L2 capture device (default "<< dev_name << ")" << std::endl;
				exit(0);
			}
		}
	}
	if (optind<argc)
	{
		dev_name = argv[optind];
		optind++;
	}	
	if (optind<argc)
	{
		out_devname = argv[optind];
		optind++;
	}	

	// initialize log4cpp
	initLogger(verbose);

	// init V4L2 capture interface
	if ((videoformatList.empty()) && (format!=0)) {
		videoformatList.push_back(V4L2_PIX_FMT_MJPEG);
		videoformatList.push_back(V4L2_PIX_FMT_JPEG);
		videoformatList.push_back(V4L2_PIX_FMT_YUYV);
		videoformatList.push_back(0);
	}

	V4L2DeviceParameters param(dev_name,videoformatList,width,height,fps,verbose);
	V4l2Capture* videoCapture =  V4l2Capture::create(param, ioTypeIn);
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot create V4L2 capture interface for device:" << dev_name; 
	}
	else
	{		
		V4l2Output* videoOutput = NULL;
		if (!out_devname.empty()) {
			V4L2DeviceParameters outparam(out_devname.c_str(), outFormat, width, height, fps, verbose);
			videoOutput = V4l2Output::create(outparam, ioTypeOut);
		}

		// http options
		std::vector<std::string> options;
		options.push_back("document_root");
		options.push_back(webroot);
		options.push_back("enable_directory_listing");
		options.push_back("no");
		options.push_back("additional_header");
		options.push_back("X-Frame-Options: SAMEORIGIN");
		options.push_back("access_control_allow_origin");
		options.push_back("*");		
		options.push_back("listening_ports");
		options.push_back(port);
		if (!nbthreads.empty()) {
			options.push_back("num_threads");
			options.push_back(nbthreads);
		}		
		
		// api server
		V4l2web v4l2web(videoCapture, videoOutput, options);
		if (v4l2web.getContext() == NULL)
		{
			LOG(WARN) << "Cannot listen on port:" << port; 
		}
		else
		{		
			LOG(NOTICE) << "Started on port:" << port << " webroot:" << webroot;
			signal(SIGINT,sighandler);	 
			while (!stop) {
				sleep(1); 
			}
		}
		delete videoOutput;
		delete videoCapture;
	}
	
	return 0;
}
