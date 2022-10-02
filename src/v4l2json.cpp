/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2json.cpp
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
	
#include "logger.h"

#include "json/json.h"

Json::Value getMenuControl(int fd, int id, int type, int min, int max) {
	Json::Value menu(Json::ValueType::arrayValue);
	struct v4l2_querymenu querymenu;
	memset(&querymenu,0,sizeof(querymenu));
	querymenu.id = id;
	for (querymenu.index = min; querymenu.index <= max; querymenu.index++) 
	{
		if (0 == ioctl(fd,VIDIOC_QUERYMENU,&querymenu))
		{
			Json::Value label;
			label["value"] = querymenu.index;
			if (type == V4L2_CTRL_TYPE_MENU)
			{
				label["label"] = (const char*)querymenu.name;
			}
#ifdef V4L2_CTRL_TYPE_INTEGER_MENU 
			else if (type == V4L2_CTRL_TYPE_INTEGER_MENU)
			{
				label["label"] = (int)querymenu.value;
			}
#endif
			menu.append(label);
		}
	}
	return menu;
}

unsigned int add_ctrl(int fd, unsigned int i, Json::Value & json) 
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
					value["menu"] = getMenuControl(fd, qctrl.id, qctrl.type, qctrl.minimum, qctrl.maximum);
				}
				json.append(value);
			}
		}
		ret = qctrl.id;
	}
	return ret;
}

Json::Value getframeIntervals(int fd, unsigned int pixelformat, unsigned int width, unsigned int height) 
{
	Json::Value frameIntervals(Json::ValueType::arrayValue);
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
	return frameIntervals;
}

Json::Value getFrameSizeList(int fd, int pixelformat) {
	Json::Value frameSizeList(Json::ValueType::arrayValue);
	struct v4l2_frmsizeenum frmsize;
	memset(&frmsize,0,sizeof(frmsize));
	frmsize.pixel_format = pixelformat;
	frmsize.index = 0;
	while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) 
	{
		Json::Value frameSize;
		if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) 
		{				
			frameSize["width"] = frmsize.discrete.width;
			frameSize["height"] = frmsize.discrete.height;
			frameSize["intervals"] = getframeIntervals(fd, frmsize.pixel_format, frmsize.discrete.width, frmsize.discrete.height);
		}
		else 
		{
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
			
			frameSize["intervals"] = getframeIntervals(fd, frmsize.pixel_format, frmsize.stepwise.max_width, frmsize.stepwise.max_height);
		}
		
		frameSizeList.append(frameSize);
		frmsize.index++;
	}
	return frameSizeList;
}
