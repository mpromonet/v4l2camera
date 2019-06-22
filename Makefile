CFLAGS = -W -Wall -std=c++11 -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
ALL_PROGS = v4l2web
PREFIX=/usr
DESTDIR?=$(PREFIX)

C = $(CROSS)gcc
CXX = $(CROSS)g++

ifneq ($(wildcard $(SYSROOT)$(PREFIX)/include/log4cpp/Category.hh),)
$(info with log4cpp)
CFLAGS += -DHAVE_LOG4CPP -I $(SYSROOT)$(PREFIX)/include
LDFLAGS += -llog4cpp
endif

#
CFLAGS += -g -fpermissive
CFLAGS +=  -I inc
LDFLAGS += -ljpeg -ldl

all: $(ALL_PROGS)

upgrade:
	git submodule foreach git pull origin master


# civetweb
libhttpjsonserver/Makefile:
	git submodule update --recursive --init libhttpjsonserver
	
libhttpjsonserver.a: libhttpjsonserver/Makefile
	cd libhttpjsonserver && cmake . && make	httpjsonserver
	mv libhttpjsonserver/$@ .

CFLAGS += -I libhttpjsonserver/inc  -I libhttpjsonserver/civetweb/include -I libhttpjsonserver/jsoncpp/include
LIBS+=libhttpjsonserver.a -lssl -lcrypto

# libv4l2cpp
libv4l2wrapper.a: 
	git submodule init v4l2wrapper
	git submodule update v4l2wrapper
	make -C v4l2wrapper
	mv v4l2wrapper/libv4l2wrapper.a .
	make -C v4l2wrapper clean
	
CFLAGS += -I v4l2wrapper/inc 
LIBS+=libv4l2wrapper.a

v4l2web: src/main.cpp src/v4l2web.cpp $(LIBS)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	-@$(RM) $(ALL_PROGS) .*o *.a

install: all
	mkdir -p $(DESTDIR)/bin
	install -D -m 0755 $(ALL_PROGS) $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/share/v4l2web
	install webroot/* -t $(DESTDIR)/share/v4l2web/

