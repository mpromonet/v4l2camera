CFLAGS = -W -Wall -std=c++11 -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
ALL_PROGS = v4l2web
PREFIX=/usr

CC = g++

ifneq ($(wildcard $(SYSROOT)$(PREFIX)/include/log4cpp/Category.hh),)
$(info with log4cpp)
CFLAGS += -DHAVE_LOG4CPP -I $(SYSROOT)$(PREFIX)/include
LDFLAGS += -llog4cpp
endif

# jsoncpp
CFLAGS += -I $(PREFIX)/include/jsoncpp 
LDFLAGS += -ljsoncpp 

#
CFLAGS += -g -fpermissive
CFLAGS +=  -I v4l2wrapper/inc -I inc
LDFLAGS += -ljpeg -ldl

all: $(ALL_PROGS)

upgrade:
	git submodule foreach git pull origin master


# civetweb
civetweb/Makefile:
	git submodule update --init civetweb
	
libcivetweb.a: civetweb/Makefile
	make lib WITH_CPP=1 COPT="$(CFLAGS)" -C civetweb
	mv civetweb/$@ .
	make -C civetweb clean

CFLAGS += -I civetweb/include -DUSE_WEBSOCKET
LIBS+=libcivetweb.a

# libv4l2cpp
libv4l2wrapper.a: 
	git submodule init v4l2wrapper
	git submodule update v4l2wrapper
	make -C v4l2wrapper
	mv v4l2wrapper/libv4l2wrapper.a .
	make -C v4l2wrapper clean
	
LIBS+=libv4l2wrapper.a

v4l2web: src/main.cpp src/v4l2web.cpp src/HttpServerRequestHandler.cpp $(LIBS)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	-@$(RM) $(ALL_PROGS) .*o *.a
