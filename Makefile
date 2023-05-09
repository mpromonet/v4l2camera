CFLAGS = -W -Wall -std=c++11 -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
PREFIX=/usr
DESTDIR?=$(PREFIX)
VERSION?=$(shell git describe --tags --always --dirty)

C = $(CROSS)gcc
CXX = $(CROSS)g++

ifneq ($(wildcard $(SYSROOT)$(PREFIX)/include/log4cpp/Category.hh),)
$(info with log4cpp)
CFLAGS += -DHAVE_LOG4CPP -I $(SYSROOT)$(PREFIX)/include
LDFLAGS += -llog4cpp
endif

#
CFLAGS += -g -fpermissive
CFLAGS +=  -I inc -DVERSION=\"$(VERSION)\"
LDFLAGS += -ldl


all: v4l2web webroot

upgrade:
	git submodule foreach git pull origin master

vuejs/node_modules:
	npm i -C vuejs

webroot: vuejs/node_modules $(wildcard vuejs/src/* vuejs/src/*/*)
	npm run build -C vuejs

# v4l2tools
v4l2tools/Makefile:
	git submodule update --recursive --init v4l2tools

v4l2tools/libyuv.a: v4l2tools/Makefile
	make -C v4l2tools libyuv.a

# v4l2rtsp
v4l2rtspserver/CMakeList.txt:
	git submodule update --recursive --init v4l2rtspserver

v4l2rtspserver/liblibv4l2rtspserver.a: v4l2rtspserver/CMakeList.txt
	cd v4l2rtspserver && cmake . && make libv4l2rtspserver		

LIVE = v4l2rtspserver/live
CFLAGS += -I ${LIVE}/groupsock/include -I ${LIVE}/liveMedia/include -I ${LIVE}/UsageEnvironment/include -I ${LIVE}/BasicUsageEnvironment/include
CFLAGS += -I v4l2rtspserver/inc
LIBS+=v4l2rtspserver/liblibv4l2rtspserver.a 

CFLAGS +=  -I v4l2tools/include -I v4l2tools/libyuv/include 
LIBS+=v4l2tools/libyuv.a

# libx264
ifneq ($(wildcard /usr/include/x264.h),)
CFLAGS += -DHAVE_X264
LDFLAGS += -lx264
endif

# libx265
ifneq ($(wildcard /usr/include/x265.h),)
CFLAGS += -DHAVE_X265
LDFLAGS += -lx265
endif

# libvpx
ifneq ($(wildcard /usr/include/vpx),)
CFLAGS += -DHAVE_VPX
LDFLAGS += -lvpx
endif

# libjpeg
ifneq ($(wildcard /usr/include/jpeglib.h),)
CFLAGS += -DHAVE_JPEG
LDFLAGS += -ljpeg
endif

# alsa
ifneq ($(wildcard /usr/include/alsa/asoundlib.h),)
CFLAGS += -DHAVE_ALSA
LDFLAGS += -lasound
endif

# civetweb
libhttpjsonserver/Makefile:
	git submodule update --recursive --init libhttpjsonserver
	
libhttpjsonserver.a: libhttpjsonserver/Makefile
	cd libhttpjsonserver && cmake . && make	httpjsonserver
	mv libhttpjsonserver/$@ .

CFLAGS += -I libhttpjsonserver/inc  -I libhttpjsonserver/civetweb/include -I libhttpjsonserver/jsoncpp/include -DOPENSSL_API_1_1 -DCRYPTO_LIB="libcrypto.so.1.1" -DSSL_LIB="libssl.so.1.1"
LIBS+=libhttpjsonserver.a -lssl -lcrypto

# libv4l2cpp
liblibv4l2cpp.a: 
	git submodule update --init libv4l2cpp
	cd libv4l2cpp && cmake . && make libv4l2cpp
	mv libv4l2cpp/$@ .
	
CFLAGS += -I libv4l2cpp/inc 
LIBS+=liblibv4l2cpp.a

v4l2web: $(wildcard src/*.cpp) $(LIBS)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	-@$(RM) -rf v4l2web webroot .*o *.a

install: all
	mkdir -p $(DESTDIR)/bin
	install -D -m 0755 v4l2web $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/share/v4l2web
	cp -pr webroot $(DESTDIR)/share/v4l2web/

