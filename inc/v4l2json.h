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

Json::Value getMenuControl(int fd, int id, int type, int min, int max);
unsigned int add_ctrl(int fd, unsigned int i, Json::Value & json);
Json::Value getframeIntervals(int fd, unsigned int pixelformat, unsigned int width, unsigned int height);
Json::Value getFrameSizeList(int fd, int pixelformat);