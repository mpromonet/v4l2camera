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

#include "v4l2json.h"

std::map<std::string,HttpServerRequestHandler::httpFunction>& V4l2web::getHttpFunc() {
	if (m_httpfunc.empty()) {
		m_httpfunc["/api/capabilities"]   = [this](const struct mg_request_info *, const Json::Value & ) -> Json::Value { 
			return this->capabilities();
		};
		m_httpfunc["/api/formats"]        = [this](const struct mg_request_info *, const Json::Value & ) -> Json::Value { 
			return this->formats();
		};
		m_httpfunc["/api/format"]         = [this](const struct mg_request_info *, const Json::Value & in) -> Json::Value { 
			return this->format(in);
		};
		m_httpfunc["/api/controls"]       = [this](const struct mg_request_info *, const Json::Value & ) -> Json::Value { 
			return this->controls();
		};
		m_httpfunc["/api/control"]        = [this](const struct mg_request_info *, const Json::Value & in) -> Json::Value { 
			return this->control(in);
		};	
		m_httpfunc["/api/start"]          = [this](const struct mg_request_info *, const Json::Value & ) -> Json::Value { 
			return this->start();
		};
		m_httpfunc["/api/stop"]           = [this](const struct mg_request_info *, const Json::Value & ) -> Json::Value { 
			return this->stop();
		};
		m_httpfunc["/api/isCapturing"]    = [this](const struct mg_request_info *, const Json::Value & ) -> Json::Value { 
			return this->isCapturing();
		};
		m_httpfunc["/api/rtspinfo"]       = [this](const struct mg_request_info *, const Json::Value & in) -> Json::Value { 
			return this->rtspInfo(in);
		};
		m_httpfunc["/api/version"] = [this](const struct mg_request_info *, const Json::Value &) -> Json::Value {
			return Json::Value(VERSION);
		};		
		m_httpfunc["/api/help"]           = [this](const struct mg_request_info *, const Json::Value & ) -> Json::Value { 
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

int NullLogger(const struct mg_connection *, const char *) {
	return 1;
}

V4l2web::V4l2web(V4l2Capture*  videoCapture, DeviceInterface* audioCapture, V4l2Output*  videoOutput, const std::vector<std::string> & options, int rtspport, int verbose): 
	m_askToInterupt(false),
	m_videoCapture(videoCapture),
	m_videoInterface(new VideoCaptureAccess(m_videoCapture)),
	m_audioInterface(audioCapture),
	m_videoOutput(videoOutput),
#ifdef WITH_COMPRESS
	m_encoder(NULL),
#endif
	m_httpServer(this->getHttpFunc(), this->getWsFunc(), options, verbose ? NULL : NullLogger),
	m_isCapturing(true),
	m_stopCapturing(false),
	m_rtspServer(rtspport),
	m_videoReplicator(NULL),
	m_audioReplicator(NULL),
	m_sms(NULL),
	m_stopStreaming(0),
	m_rtspuri("") {

		m_jsonWriterBuilder.settings_["emitUTF8"] = true;
		
#ifdef WITH_COMPRESS
	if (m_videoOutput && m_videoCapture) {
		std::map<std::string,std::string> opt;
		m_encoder = CodecFactory::get().Create(m_videoCapture->getFormat(), m_videoOutput->getFormat(), m_videoCapture->getWidth(), m_videoCapture->getHeight(), opt, 0);
		if (!m_encoder)
		{
			LOG(WARN) << "Cannot create encoder " << V4l2Device::fourcc(m_videoOutput->getFormat()); 
		}
	}
#endif
			
	m_capturing = std::thread([this]() {
		this->capturing();
	});

	m_streaming = std::thread([this]() {
		m_rtspServer.eventLoop(&m_stopStreaming); 
	});
}

V4l2web::~V4l2web() {
	m_stopCapturing = true;
	m_capturing.join();
	m_stopStreaming = 1;
	m_streaming.join();
}

void V4l2web::createRtspSession(const std::string & rtspuri, const std::string & multicasturi)
{
	if (m_sms) {
		m_rtspServer.RemoveSession(m_sms);
	}
	if (m_audioInterface) {
		m_audioReplicator = DeviceSourceFactory::createStreamReplicator(m_rtspServer.env(), 0, m_audioInterface, 5);
	}
	m_videoReplicator = DeviceSourceFactory::createStreamReplicator(m_rtspServer.env(), m_videoCapture->getFormat(), m_videoInterface, 10, V4L2DeviceSource::NOCAPTURE);
	if (m_videoReplicator)
	{
		if (multicasturi.empty())
		{
			m_sms = m_rtspServer.AddUnicastSession(rtspuri, m_videoReplicator, m_audioReplicator);
			m_multicasturi.clear();
		}
		else
		{
			m_sms = m_rtspServer.AddMulticastSession(rtspuri, multicasturi, m_multicasturi, m_videoReplicator, m_audioReplicator);			
		}
		m_rtspuri = rtspuri;
	}
}

void V4l2web::capturing()
{	
	this->createRtspSession(m_rtspuri);

	while (!m_stopCapturing) {
		if (m_isCapturing && m_videoCapture->isReady())
		{
			std::unique_lock<std::mutex> lock(m_deviceMutex);
			m_actionPending.wait(lock, [this] {return !bool(m_askToInterupt);});

			struct timeval tv;
			timerclear(&tv);
			tv.tv_usec = 100*1000;
			if (m_videoCapture->isReadable(&tv))
			{
				// update format informations
				m_videoCapture->queryFormat();
				// read image
				timeval ref;
				gettimeofday(&ref, NULL);	
				int bufferSize = m_videoCapture->getBufferSize();
				char* buf = new char[bufferSize];
				ssize_t size = m_videoCapture->read(buf, bufferSize);
				LOG(DEBUG) << "read size:" << size << " buffersize:" << bufferSize;
				
				// post to subscribers
				if ( (size>0) && m_videoReplicator)
				{
					std::string frame;
					V4L2DeviceSource* source = (V4L2DeviceSource*)m_videoReplicator->inputSource();
					if (source) {
						if (source->isKeyFrame(buf, size)) {
							std::list<std::string> initFrames = source->getInitFrames();
							for (auto f : initFrames) {
								frame.append(f);
							}
						}

						// publish frame to websocket
						frame.append(buf, size);
						m_httpServer.publishBin("/ws", frame.c_str(), frame.size());

						// publish foramt to websocket
						Json::Value format;
						format["width"] = m_videoCapture->getWidth();
						format["height"] = m_videoCapture->getHeight();	
						format["format"] = V4l2Device::fourcc(m_videoCapture->getFormat());
						std::string answer(Json::writeString(m_jsonWriterBuilder,format));						
						m_httpServer.publishTxt("/ws", answer.c_str(), answer.size());


						// publish to RTSP 
						char* buffer = new char[size];
						memcpy(buffer, buf, size);	
						source->postFrame(buffer, size, ref);
						m_snapshot.assign(buffer, size);
					}
				}

#ifdef WITH_COMPRESS
				// encode
				if (m_encoder && m_videoOutput) {
					m_encoder->convertAndWrite(buf, size, m_videoOutput);
				}
#endif
				delete [] buf;
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
		json["version"]    = cap.version;
		json["card"]       = (const char*)cap.card;
		json["bus_info"]   = (const char*)cap.bus_info;			
		json["capabilities"]   = getCapabilities(cap.capabilities);	
	}
	else
	{
		json["errno"]  = errno;
		json["error"]  = strerror(errno);			
	}
	return json;
}


Json::Value V4l2web::formats() 
{
	int fd = m_videoCapture->getFd();		
	Json::Value videoformatList(Json::ValueType::arrayValue);
	for (int i = 0;; i++) 
	{
		struct v4l2_fmtdesc     fmtdesc;
		memset(&fmtdesc,0,sizeof(fmtdesc));
		fmtdesc.index = i;
		fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc))
			break;
		
		Json::Value format;
		format["description"] = (const char*)fmtdesc.description;
		format["type"]        = fmtdesc.type;
		format["format"]      = V4l2Device::fourcc(fmtdesc.pixelformat);		
		format["frameSizes"]  = getFrameSizeList(fd, fmtdesc.pixelformat);

		videoformatList.append(format);
	}

	Json::Value audioformatList(Json::ValueType::arrayValue);;
#ifdef HAVE_ALSA	
	if (m_audioInterface) {
		for (int audiofmt : m_audioInterface->getAudioFormatList()) {
			audioformatList.append(V4l2RTSPServer::getAudioFormatName(audiofmt));
		}
	}
#endif	

	Json::Value formatList;
	formatList["video"] = videoformatList;
	formatList["audio"] = audioformatList;

	return formatList;
}

Json::Value V4l2web::format(const Json::Value & input) 
{
	int fd = m_videoCapture->getFd();		
	Json::Value output;
	
	// set format POST
	if (input.isNull() == false)
	{		
		// ask to interrupt capture loop & wait 
		m_askToInterupt = true;
		const std::lock_guard<std::mutex> lock(m_deviceMutex);
		m_askToInterupt = false;
		m_actionPending.notify_all();

		int width = input.get("width",m_videoCapture->getWidth()).asUInt();
		int height = input.get("height",m_videoCapture->getHeight()).asUInt();
		std::string informatstr = input.get("format","").asString();
		unsigned int informat = V4l2Device::fourcc(informatstr.c_str());
		int fps = input.get("fps",0).asUInt();

		m_videoCapture->stop();
		unsigned int oldformat = m_videoCapture->getFormat();
		m_videoCapture->setFormat(informat, width, height);
		m_videoCapture->setFps(fps);

		// update RTSP session
		if (oldformat != informat) {
			this->createRtspSession(m_rtspuri);
		}

		m_videoCapture->start();
		
#ifdef WITH_COMPRESS
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
			
			m_encoder = CodecFactory::get().Create(outformat, m_videoCapture->getFormat(), width, height, opt, 0);

			if (!m_encoder)
			{
				LOG(WARN) << "Cannot create encoder " << outformatStr; 
			}
		}
#endif
	}

	// query the format
	struct v4l2_format     format;
	memset(&format,0,sizeof(format));
	format.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(fd,VIDIOC_G_FMT,&format))
	{
		output["width"]        = format.fmt.pix.width;
		output["height"]       = format.fmt.pix.height;
		output["sizeimage"]    = format.fmt.pix.sizeimage;
		output["bytesperline"] = format.fmt.pix.bytesperline;
		output["colorspace"]   = getColorspace(format.fmt.pix.colorspace);
		output["field"]        = getField(format.fmt.pix.field);
		output["format"]       = V4l2Device::fourcc(format.fmt.pix.pixelformat);			
		
	}
	struct v4l2_streamparm parm;
	memset(&parm,0,sizeof(parm));
	parm.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(fd,VIDIOC_G_PARM,&parm))
	{
		Json::Value capabilities(Json::ValueType::arrayValue);
		if (parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) capabilities.append("V4L2_CAP_TIMEPERFRAME");
		output["capabilities"]  = capabilities;		
		Json::Value capturemode(Json::ValueType::arrayValue);
		if (parm.parm.capture.capturemode & V4L2_MODE_HIGHQUALITY) capturemode.append("V4L2_MODE_HIGHQUALITY");
		output["capturemode"]   = capturemode;		
		output["readbuffers"]   = parm.parm.capture.readbuffers;		
		double fps = 0.0;
		if (parm.parm.capture.timeperframe.numerator != 0) fps = 1.0*parm.parm.capture.timeperframe.denominator/parm.parm.capture.timeperframe.numerator;
		output["fps"]           = fps; 
	}

	//audio
#ifdef HAVE_ALSA	
	if (m_audioInterface) {
		Json::Value audio;
		audio["format"]        = V4l2RTSPServer::getAudioFormatName(m_audioInterface->getAudioFormat());
		audio["samplerate"]    = m_audioInterface->getSampleRate();
		audio["channels"]      = m_audioInterface->getChannels();
		output["audio"]        = audio; 
	}
#endif	

	if (m_videoOutput) {
		Json::Value out;
		out["format"]    = V4l2Device::fourcc(m_videoOutput->getFormat());
		out["width"]     = m_videoOutput->getWidth();		
		out["height"]    = m_videoOutput->getHeight();		
		output["out"]    = out; 
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
	m_isCapturing = true;
	return Json::Value(m_isCapturing);	
}

Json::Value V4l2web::stop() 
{
	m_isCapturing = false;
	return Json::Value(m_isCapturing);	
}

Json::Value V4l2web::isCapturing() 
{
	Json::Value answer(m_isCapturing && m_videoCapture->isReady());
	return answer;	
}

Json::Value V4l2web::rtspInfo(const Json::Value & input) 
{
	Json::Value answer;
	// set format POST
	if (input.isNull() == false)
	{		
		// ask to interrupt capture loop & wait 
		m_askToInterupt = true;
		const std::lock_guard<std::mutex> lock(m_deviceMutex);
		m_askToInterupt = false;
		m_actionPending.notify_all();

		std::string rtspuri = input.get("rtspuri","").asString();
		std::string multicasturi = input.get("multicasturi","").asString();

		// update RTSP session
		if (rtspuri != m_rtspuri || m_multicasturi != multicasturi) {
			m_videoCapture->stop();
			this->createRtspSession(rtspuri, multicasturi);
			m_videoCapture->start();
		}
	}

	answer["rtspuri"] = m_rtspuri;
	answer["multicasturi"] = m_multicasturi;
	answer["url"] = m_rtspServer.getRtspUrl(m_sms);
	answer["numClients"] = m_rtspServer.numClientSessions();

	if (m_sms) {
		char * sdp = m_sms->generateSDPDescription(AF_INET);
		answer["media"] = sdp;
		delete [] sdp;
	}
	return answer;	
}

