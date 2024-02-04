/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2json.h
** 
** -------------------------------------------------------------------------*/

#pragma once


#include "json/json.h"

unsigned int add_ctrl(int fd, unsigned int i, Json::Value & json);
Json::Value getframeIntervals(int fd, unsigned int pixelformat, unsigned int width, unsigned int height);
Json::Value getFrameSizeList(int fd, int pixelformat);
std::string getColorspace(int colorspace);
std::string getField(int field);
Json::Value getCapabilities(int device_caps);