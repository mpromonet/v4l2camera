/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2web.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/version.h>
#include <linux/videodev2.h>

#include <stdexcept>
#include <iostream>
	
#include "logger.h"

#include "V4l2Capture.h"
#include "V4l2Output.h"
#include "v4l2web.h"

#include "DeviceSourceFactory.h"

static unsigned int add_ctrl(int fd, unsigned int i, Json::Value & json) 
{
	unsigned int ret=0;
	struct v4l2_queryctrl   qctrl;
	memset(&qctrl,0,sizeof(qctrl));
	qctrl.id = i;
	if (0 == ioctl(fd,VIDIOC_QUERYCTRL,&qctrl))
	{
		if (!(qctrl.flags & V4L2_CTRL_FLAG_DISABLED))
		{
			struct v4l2_control control;  
			memset(&control,0,sizeof(control));
			control.id = qctrl.id;
			if (0 == ioctl(fd,VIDIOC_G_CTRL,&control))
			{
				Json::Value value;
				value["id"           ] = control.id;
				value["name"         ] = (const char*)qctrl.name;
				value["type"         ] = qctrl.type;
				value["minimum"      ] = qctrl.minimum;
				value["maximum"      ] = qctrl.maximum;
				value["step"         ] = qctrl.step;
				value["default_value"] = qctrl.default_value;
				value["value"        ] = control.value;

				Json::Value flags;
				if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED  ) flags.append("V4L2_CTRL_FLAG_DISABLED"  );
				if (qctrl.flags & V4L2_CTRL_FLAG_GRABBED   ) flags.append("V4L2_CTRL_FLAG_GRABBED"   );
				if (qctrl.flags & V4L2_CTRL_FLAG_READ_ONLY ) flags.append("V4L2_CTRL_FLAG_READ_ONLY" );
				if (qctrl.flags & V4L2_CTRL_FLAG_UPDATE    ) flags.append("V4L2_CTRL_FLAG_UPDATE"    );
				if (qctrl.flags & V4L2_CTRL_FLAG_SLIDER    ) flags.append("V4L2_CTRL_FLAG_SLIDER"    );
				if (qctrl.flags & V4L2_CTRL_FLAG_WRITE_ONLY) flags.append("V4L2_CTRL_FLAG_WRITE_ONLY");
				value["flags"]   = flags;
				
				if ( (qctrl.type == V4L2_CTRL_TYPE_MENU) 
#ifdef V4L2_CTRL_TYPE_INTEGER_MENU 
					|| (qctrl.type == V4L2_CTRL_TYPE_INTEGER_MENU) 
#endif
				   )
				{
					Json::Value menu;
					struct v4l2_querymenu querymenu;
					memset(&querymenu,0,sizeof(querymenu));
					querymenu.id = qctrl.id;
					for (querymenu.index = qctrl.minimum; querymenu.index <= qctrl.maximum; querymenu.index++) 
					{
						if (0 == ioctl(fd,VIDIOC_QUERYMENU,&querymenu))
						{
							Json::Value label;
							label["value"] = querymenu.index;
							if (qctrl.type == V4L2_CTRL_TYPE_MENU)
							{
								label["label"] = (const char*)querymenu.name;
							}
#ifdef V4L2_CTRL_TYPE_INTEGER_MENU 
							else if (qctrl.type == V4L2_CTRL_TYPE_INTEGER_MENU)
							{
								label["label"] = (int)querymenu.value;
							}
#endif
							menu.append(label);
						}
					}
					value["menu"] = menu;
				}
				json.append(value);
			}
		}
		ret = qctrl.id;
	}
	return ret;
}

static void add_frameIntervals(int fd, unsigned int pixelformat, unsigned int width, unsigned int height, Json::Value & frameSize) 
{
	Json::Value frameIntervals;
	struct v4l2_frmivalenum frmival;
	frmival.index = 0;
	frmival.pixel_format = pixelformat;
	frmival.width = width;
	frmival.height = height;
	while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) 
	{
		Json::Value frameInter;
		if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) 
		{
			frameInter["fps"] = 1.0*frmival.discrete.denominator/frmival.discrete.numerator;
		}
		else
		{
			Json::Value fps;
			fps["min"]  = frmival.stepwise.max.denominator/frmival.stepwise.max.numerator;
			fps["max"]  = frmival.stepwise.min.denominator/frmival.stepwise.min.numerator;
			fps["step"] = frmival.stepwise.step.denominator/frmival.stepwise.step.numerator;
			frameInter["fps"] = fps;
		}
		frameIntervals.append(frameInter);
		frmival.index++;
	}
	frameSize["intervals"] = frameIntervals;
}

std::map<std::string,HttpServerRequestHandler::httpFunction>& V4l2web::getHttpFunc() {
	if (m_httpfunc.empty()) {
		m_httpfunc["/api/capabilities"]   = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->capabilities();
		};
		m_httpfunc["/api/inputs"]         = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->inputs();
		};
		m_httpfunc["/api/formats"]        = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->formats();
		};
		m_httpfunc["/api/format"]         = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->format(in);
		};
		m_httpfunc["/api/controls"]       = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->controls();
		};
		m_httpfunc["/api/control"]        = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->control(in);
		};	
		m_httpfunc["/api/start"]          = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->start();
		};
		m_httpfunc["/api/stop"]           = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->stop();
		};
		m_httpfunc["/api/isCapturing"]    = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return this->isCapturing();
		};
		m_httpfunc["/api/help"]           = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			Json::Value answer;
			for (auto it : this->m_httpfunc) {
				answer.append(it.first);
			}
			return answer;
		};	
	}
	return m_httpfunc;
}

std::map<std::string,HttpServerRequestHandler::wsFunction>& V4l2web::getWsFunc() {
	if (m_wsfunc.empty()) {
		m_wsfunc["/ws"]  = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value { 
			return in;
		};
	}
	return m_wsfunc;
}
	
V4l2web::V4l2web(V4l2Capture*  videoCapture, V4l2Output*  videoOutput, const std::vector<std::string> & options): 
	m_videoCapture(videoCapture),
	m_videoOutput(videoOutput),
	m_encoder(NULL),
	m_httpServer(this->getHttpFunc(), this->getWsFunc(), options),
	m_isCapturing(false),
	m_stopCapturing(false),
	m_rtspServer(8554),
	m_stopStreaming(0) {	

		
	if (m_videoOutput && m_videoCapture) {
		std::map<std::string,std::string> opt;
		m_encoder = CodecFactory::get().Create(m_videoCapture->getFormat(), m_videoOutput->getFormat(), m_videoCapture->getWidth(), m_videoCapture->getHeight(), opt, 0);
		if (!m_encoder)
		{
			LOG(WARN) << "Cannot create encoder " << V4l2Device::fourcc(m_videoOutput->getFormat()); 
		}
	}
			
	m_capturing = std::thread([this]() {
		this->capturing();
	});

	m_streaming = std::thread([this]() {
		this->streaming();
	});
}

V4l2web::~V4l2web() {
	m_stopCapturing = true;
	m_capturing.join();
	m_stopStreaming = 1;
	m_streaming.join();
}

void V4l2web::streaming()
{
	StreamReplicator* videoReplicator = DeviceSourceFactory::createStreamReplicator(m_rtspServer.env(), m_videoCapture->getFormat(), new VideoCaptureAccess(m_videoCapture));
	if (videoReplicator)
	{
		m_rtspServer.AddUnicastSession("", videoReplicator, NULL);			
		m_rtspServer.eventLoop(&m_stopStreaming); 
	}	
}

void V4l2web::capturing()
{
	while (!m_stopCapturing) {
		if (m_isCapturing && m_videoCapture->isReady())
		{
			const std::lock_guard<std::mutex> lock(m_deviceMutex);
			
			struct timeval tv;
			timerclear(&tv);
			tv.tv_sec = 1;
			if (m_videoCapture->isReadable(&tv))
			{
				// update format informations
				m_videoCapture->queryFormat();
				// read image
				int bufferSize = m_videoCapture->getBufferSize();
				char buf[bufferSize];
				ssize_t size = m_videoCapture->read(buf, bufferSize);
				LOG(DEBUG) << "read size:" << size << " buffersize:" << bufferSize;
				
				// post to subscribers
				if (size>0)
				{
					m_httpServer.publishBin("/ws",buf, size);
				}
				
				// encode
				if (m_encoder && m_videoOutput) {
					m_encoder->convertAndWrite(buf, size, m_videoOutput);
				}
			}
		} else {
			sleep(1); 
		}
	}
}

Json::Value V4l2web::capabilities() 
{
	int fd = m_videoCapture->getFd();		
	Json::Value json;
	v4l2_capability cap;
	memset(&cap,0,sizeof(cap));
	if (-1 != ioctl(fd,VIDIOC_QUERYCAP,&cap))
	{
		json["driver"]     = (const char*)cap.driver;
		json["card"]       = (const char*)cap.card;
		json["bus_info"]   = (const char*)cap.bus_info;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)		
		Json::Value capabilities;
		if (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) capabilities.append("V4L2_CAP_VIDEO_CAPTURE");
		if (cap.device_caps & V4L2_CAP_VIDEO_OUTPUT ) capabilities.append("V4L2_CAP_VIDEO_OUTPUT" );
		if (cap.device_caps & V4L2_CAP_READWRITE    ) capabilities.append("V4L2_CAP_READWRITE"    );
		if (cap.device_caps & V4L2_CAP_ASYNCIO      ) capabilities.append("V4L2_CAP_ASYNCIO"      );
		if (cap.device_caps & V4L2_CAP_STREAMING    ) capabilities.append("V4L2_CAP_STREAMING"    );
		json["capabilities"]   = capabilities;
#endif		
	}
	else
	{
		json["errno"]  = errno;
		json["error"]  = strerror(errno);			
	}
	return json;
}

Json::Value V4l2web::inputs() 
{
	int fd = m_videoCapture->getFd();		
	Json::Value json;
	for (int i = 0;; i++) 
	{
		v4l2_input input;
		memset(&input,0,sizeof(input));
		input.index = i;
		if (-1 == ioctl(fd,VIDIOC_ENUMINPUT,&input))
			break;
		
		Json::Value value;
		value["name"]   = (const char*)input.name;
		switch (input.type)
		{
			case V4L2_INPUT_TYPE_TUNER : value["type"] = "V4L2_INPUT_TYPE_TUNER" ; break;
			case V4L2_INPUT_TYPE_CAMERA: value["type"] = "V4L2_INPUT_TYPE_CAMERA"; break;
			default : break;
		}		
		Json::Value status;
		if (input.status & V4L2_IN_ST_NO_POWER ) status.append("V4L2_IN_ST_NO_POWER" );
		if (input.status & V4L2_IN_ST_NO_SIGNAL) status.append("V4L2_IN_ST_NO_SIGNAL");
		if (input.status & V4L2_IN_ST_NO_COLOR ) status.append("V4L2_IN_ST_NO_COLOR" );
		value["status"] = status;			
		json.append(value);
	}
	return json;
}
		
Json::Value V4l2web::formats() 
{
	int fd = m_videoCapture->getFd();		
	Json::Value json;
	for (int i = 0;; i++) 
	{
		struct v4l2_fmtdesc     fmtdesc;
		memset(&fmtdesc,0,sizeof(fmtdesc));
		fmtdesc.index = i;
		fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc))
			break;
		
		Json::Value value;
		value["description"] = (const char*)fmtdesc.description;
		value["type"]        = fmtdesc.type;
		value["format"]      = V4l2Device::fourcc(fmtdesc.pixelformat);		

		Json::Value frameSizeList;
		struct v4l2_frmsizeenum frmsize;
		memset(&frmsize,0,sizeof(frmsize));
		frmsize.pixel_format = fmtdesc.pixelformat;
		frmsize.index = 0;
		while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) 
		{
			if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) 
			{				
				Json::Value frameSize;
				frameSize["width"] = frmsize.discrete.width;
				frameSize["height"] = frmsize.discrete.height;
				add_frameIntervals(fd, frmsize.pixel_format, frmsize.discrete.width, frmsize.discrete.height, frameSize);
				frameSizeList.append(frameSize);
			}
			else 
			{
				Json::Value frameSize;				
				Json::Value width;
				width["min"] = frmsize.stepwise.min_width;
				width["max"] = frmsize.stepwise.max_width;
				width["step"] = frmsize.stepwise.step_width;				
				frameSize["width"] = width;
				
				Json::Value height;
				height["min"] = frmsize.stepwise.min_height;
				height["max"] = frmsize.stepwise.max_height;
				height["step"] = frmsize.stepwise.step_height;				
				frameSize["height"] = height;
				
				add_frameIntervals(fd, frmsize.pixel_format, frmsize.stepwise.max_width, frmsize.stepwise.max_height, frameSize);
				frameSizeList.append(frameSize);				
			}
			frmsize.index++;
		}
		value["frameSizes"]     = frameSizeList;

		json.append(value);
	}
	return json;
}

Json::Value V4l2web::format(const Json::Value & input) 
{
	int fd = m_videoCapture->getFd();		
	Json::Value output;
	
	// set format POST
	if (input.isNull() == false)
	{		
		const std::lock_guard<std::mutex> lock(m_deviceMutex);
		if (m_isCapturing) {
			m_videoCapture->stop();
		}
		
		struct v4l2_format     format;
		memset(&format,0,sizeof(format));
		format.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (0 == ioctl(fd,VIDIOC_G_FMT,&format))
		{
			Json::Value setfmt;
			try
			{
				format.fmt.pix.width = input.get("width",format.fmt.pix.width).asUInt();
				format.fmt.pix.height = input.get("height",format.fmt.pix.height).asUInt();
				std::string formatstr = input.get("format","").asString();
				if (!formatstr.empty())
				{
					format.fmt.pix.pixelformat = V4l2Device::fourcc(formatstr.c_str());
				}
				errno=0;
				setfmt["ioctl"] = ioctl(fd,VIDIOC_S_FMT,&format);
				setfmt["errno"]  = errno;
				setfmt["error"]  = strerror(errno);	
			}
			catch (const std::runtime_error &e)
			{
				setfmt["exception"]  = e.what();
			}			
			output["setfmt"] = setfmt;
		}		
		struct v4l2_streamparm parm;
		memset(&parm,0,sizeof(parm));
		parm.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (0 == ioctl(fd,VIDIOC_G_PARM,&parm))
		{
			if (parm.parm.capture.timeperframe.numerator != 0)
			{
				Json::Value setparm;
				try
				{
					parm.parm.capture.timeperframe.denominator=input.get("fps",parm.parm.capture.timeperframe.denominator/parm.parm.capture.timeperframe.numerator).asUInt();
					parm.parm.capture.timeperframe.numerator=1;
					errno=0;
					setparm["ioctl"] = ioctl(fd,VIDIOC_S_PARM,&parm);
					setparm["errno"]  = errno;
					setparm["error"]  = strerror(errno);	
				}
				catch (const std::runtime_error &e)
				{
					setparm["exception"]  = e.what();
				}			
				output["setparm"] = setparm;
			}
		}
		
		std::string outformatStr = input.get("outformat","").asString();
		if (!outformatStr.empty() && m_videoOutput) {
			if (m_encoder) {
				delete m_encoder;
			}
			std::map<std::string,std::string> opt;
			int outformat = V4l2Device::fourcc(outformatStr.c_str());
			int width = m_videoCapture->getWidth();
			int height = m_videoCapture->getHeight();
			
			m_videoOutput->stop();
			m_videoOutput->setFormat(outformat, width, height);
			m_videoOutput->start();
			
			m_encoder = CodecFactory::get().Create(outformat, m_videoCapture->getFormat(),  width, height, opt, 0);

			if (!m_encoder)
			{
				LOG(WARN) << "Cannot create encoder " << outformatStr; 
			}
		}
		
		if (m_isCapturing) {
			m_videoCapture->start();
		}
	}

	// query the format
	struct v4l2_format     format;
	memset(&format,0,sizeof(format));
	format.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(fd,VIDIOC_G_FMT,&format))
	{
		output["width"]     = format.fmt.pix.width;
		output["height"]    = format.fmt.pix.height;
		output["sizeimage"] = format.fmt.pix.sizeimage;
		output["format"]    = V4l2Device::fourcc(format.fmt.pix.pixelformat);			
		
	}
	struct v4l2_streamparm parm;
	memset(&parm,0,sizeof(parm));
	parm.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(fd,VIDIOC_G_PARM,&parm))
	{
		Json::Value capabilities;
		if (parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) capabilities.append("V4L2_CAP_TIMEPERFRAME");
		output["capabilities"]  = capabilities;		
		Json::Value capturemode;
		if (parm.parm.capture.capturemode & V4L2_MODE_HIGHQUALITY) capturemode.append("V4L2_MODE_HIGHQUALITY");
		output["capturemode"]   = capturemode;		
		output["readbuffers"]   = parm.parm.capture.readbuffers;		
		double fps = 0.0;
		if (parm.parm.capture.timeperframe.numerator != 0) fps = 1.0*parm.parm.capture.timeperframe.denominator/parm.parm.capture.timeperframe.numerator;
		output["fps"]           = fps; 
	}
	
	if (m_videoOutput) {
		output["outformat"]    = V4l2Device::fourcc(m_videoOutput->getFormat());		
	}
	
	return output;
}

Json::Value V4l2web::controls() 
{
	int fd = m_videoCapture->getFd();		
	Json::Value json;
	for (unsigned int i = V4L2_CID_BASE; i<V4L2_CID_LASTP1; add_ctrl(fd,i,json), i++);
	for (unsigned int i = V4L2_CID_LASTP1+1; i != 0 ; i=add_ctrl(fd,i|V4L2_CTRL_FLAG_NEXT_CTRL,json));
	return json;
}

Json::Value V4l2web::control(const Json::Value & input) 
{
	int fd = m_videoCapture->getFd();		
	Json::Value output;
		
	if (input.isNull() == false)
	{		
		try
		{
			unsigned int key = input["id"].asUInt();
			
			struct v4l2_control control;  
			memset(&control,0,sizeof(control));
			control.id = key;
			if (input.isMember("value"))
			{
				int value = input["value"].asInt();
				control.value = value;
				errno=0;
				output["ioctl"] = ioctl(fd,VIDIOC_S_CTRL,&control);
				output["errno"]  = errno;
				output["error"]  = strerror(errno);
			}
			else
			{
				errno=0;
				output["ioctl"] = ioctl(fd,VIDIOC_G_CTRL,&control);
				output["errno"]  = errno;
				output["error"]  = strerror(errno);
			}
			output["id"] = control.id;
			output["value"] = control.value;
			
		}
		catch (const std::runtime_error &e)
		{
			output["exception"]  = e.what();
		}
		
	}
	return output;
}

Json::Value V4l2web::start() 
{
	const std::lock_guard<std::mutex> lock(m_deviceMutex);
	m_isCapturing = true;
	Json::Value answer(m_videoCapture->start());
	return answer;	
}

Json::Value V4l2web::stop() 
{
	const std::lock_guard<std::mutex> lock(m_deviceMutex);
	m_isCapturing = false;
	Json::Value answer(m_videoCapture->stop());
	return answer;	
}

Json::Value V4l2web::isCapturing() 
{
	Json::Value answer(m_isCapturing && m_videoCapture->isReady());
	return answer;	
}

