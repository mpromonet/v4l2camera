CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
ALL_PROGS = v4l2web
PREFIX=/usr

CC = g++
# jsoncpp
CFLAGS += -I $(PREFIX)/include/jsoncpp 
LDFLAGS += -ljsoncpp 
# log4cpp
LDFLAGS += -llog4cpp 
#
CFLAGS += -g -fpermissive
CFLAGS += -I mongoose -I h264_v4l2_rtspserver/inc
LDFLAGS += -lv4l2 -ljpeg

all: $(ALL_PROGS)

upgrade:
	git submodule foreach git pull origin master

mongoose/mongoose.c: 
	git submodule init
	git submodule update

mongoose.o : mongoose/mongoose.c
	$(CC) -o $@ -c $^

v4l2web: main.cpp v4l2web.cpp mongoose.o h264_v4l2_rtspserver/src/V4l2Capture.cpp h264_v4l2_rtspserver/src/V4l2MmapCapture.cpp h264_v4l2_rtspserver/src/V4l2ReadCapture.cpp
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	-@$(RM) $(ALL_PROGS) .*o 
