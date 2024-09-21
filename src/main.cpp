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
	const char *dev_name = "/dev/video0,/dev/video0";	
	int width = 640;
	int height = 480;	
	int fps = 25;	
	int c = 0;
	const char * port = "8080";
	std::string httpSslCertificate;
	std::string rtspSslKeyCert;	
	int rtspport = 8554;
	V4l2IoType ioTypeIn = IOTYPE_MMAP;
	V4l2IoType ioTypeOut = IOTYPE_MMAP;
	std::string webroot = "vuejs/dist";
	std::string nbthreads;
	unsigned int format = ~0;
	std::list<unsigned int> videoformatList;	
	std::string out_devname;	
	unsigned int outFormat = V4L2_PIX_FMT_H264;

#ifdef HAVE_ALSA	
	int audioFreq = 44100;
	int audioNbChannels = 2;
	std::list<snd_pcm_format_t> audioFmtList;
#endif	
	
	while ((c = getopt (argc, argv, "hv::" "f::W:H:F:G:" "O:" "rw" "P:c:p:N:R:x:" "A:c:a:")) != -1)
	{
		switch (c)
		{
			case 'v': verbose = 1; if (optarg && *optarg=='v') verbose++;  break;

			case 'f': format = V4l2Device::fourcc(optarg); if (format) {videoformatList.push_back(format);};  break;
			case 'G': sscanf(optarg,"%dx%dx%d", &width, &height, &fps); break;
			
			case 'O': outFormat = V4l2Device::fourcc(optarg); break;

			case 'r': ioTypeIn = IOTYPE_READWRITE; break;
			case 'w': ioTypeOut = IOTYPE_READWRITE; break;				

			case 'P': port = optarg; break;
			case 'c': httpSslCertificate = optarg; break;
			case 'R': rtspport = atoi(optarg); break;
			case 'N': nbthreads = optarg; break;
			case 'p': webroot = optarg; break;		

#ifdef HAVE_ALSA	
			case 'A':	audioFreq = atoi(optarg); break;
			case 'C':	audioNbChannels = atoi(optarg); break;
			case 'a':	if (V4l2RTSPServer::decodeAudioFormat(optarg) != SND_PCM_FORMAT_UNKNOWN) {audioFmtList.push_back(V4l2RTSPServer::decodeAudioFormat(optarg));} ; break;
#endif	

#ifndef NO_OPENSSL
			case 'x':	rtspSslKeyCert              = optarg; break;
#endif	

			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-P httpport] [-c sslkeycert] [-R rtspport] [-W width] [-H height] [-F fps] [-G <w>x<h>x<f>] [device]" << std::endl;
				std::cout << "\t -v               : verbose " << std::endl;
				std::cout << "\t -v v             : very verbose " << std::endl;
				std::cout << "\t -P port          : server port (default "<< port << ")" << std::endl;
				std::cout << "\t -p path          : server root path (default "<< webroot << ")" << std::endl;
				std::cout << "\t -c sslkeycert    : path to private key and certificate for HTTPS" << std::endl;
				std::cout << "\t -R port          : RTSP server port (default "<< rtspport << ")" << std::endl;
#ifndef NO_OPENSSL
				std::cout << "\t -x <sslkeycert>  : key & certificate for SRTP/RTSPS"                                                                                      << std::endl;
#endif							

				std::cout << "\t -f format        : V4L2 capture using format" << std::endl;
				std::cout << "\t -G <w>x<h>[x<f>] : V4L2 capture format (default "<< width << "x" << height << "x" << fps << ")"  << std::endl;

				std::cout << "\t -r               : V4L2 capture using memory mapped buffers (default use read interface)" << std::endl;
				std::cout << "\t -w               : V4L2 capture using write interface (default use memory mapped buffers)" << std::endl;				

#ifdef HAVE_ALSA	
				std::cout << "\t ALSA options"                                                                                               << std::endl;
				std::cout << "\t -A freq          : ALSA capture frequency and channel (default " << audioFreq << ")"                                << std::endl;
				std::cout << "\t -C channels      : ALSA capture channels (default " << audioNbChannels << ")"                                       << std::endl;
				std::cout << "\t -a fmt           : ALSA capture audio format (default S16_BE)"                                                      << std::endl;
#endif
		
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
		videoformatList.push_back(V4L2_PIX_FMT_H264);
		videoformatList.push_back(V4L2_PIX_FMT_MJPEG);
		videoformatList.push_back(V4L2_PIX_FMT_JPEG);
		videoformatList.push_back(V4L2_PIX_FMT_NV12);
		videoformatList.push_back(0);
	}
#ifdef HAVE_ALSA	
	// default audio format tries
	if (audioFmtList.empty()) {
		audioFmtList.push_back(SND_PCM_FORMAT_S16_LE);
		audioFmtList.push_back(SND_PCM_FORMAT_S16_BE);
	}
#endif	
	std::unique_ptr<DeviceInterface> audioCapture;
	std::istringstream is(dev_name);
	std::string videoDev, audioDev;
	getline(is, videoDev, ',');						
	getline(is, audioDev);	
	V4L2DeviceParameters param(videoDev.c_str(), videoformatList, width, height, fps, ioTypeIn);
	std::unique_ptr<V4l2Capture> videoCapture(V4l2Capture::create(param));
	if (!videoCapture)
	{	
		LOG(WARN) << "Cannot create V4L2 capture interface for device:" << audioDev; 
	}
	else
	{	
#ifdef HAVE_ALSA	
		std::string audioDevice = V4l2RTSPServer::getV4l2Alsa(audioDev);
		ALSACaptureParameters param(audioDevice.c_str(), audioFmtList, audioFreq, audioNbChannels);
		audioCapture.reset(ALSACapture::createNew(param));
#endif	

		// output	
		std::unique_ptr<V4l2Output> videoOutput;
		if (!out_devname.empty()) {
			V4L2DeviceParameters outparam(out_devname.c_str(), outFormat, width, height, fps, ioTypeOut, verbose);
			videoOutput.reset(V4l2Output::create(outparam));
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
		if (!httpSslCertificate.empty()) {
			options.push_back("ssl_certificate");
			options.push_back(httpSslCertificate);
		}		
		if (!nbthreads.empty()) {
			options.push_back("num_threads");
			options.push_back(nbthreads);
		}		
		
		// api server
		V4l2web v4l2web(videoCapture.release(), audioCapture.get(), videoOutput.get(), options, rtspport, rtspSslKeyCert, verbose);
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
	}
	
	return 0;
}
