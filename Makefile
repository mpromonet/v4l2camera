CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
OUT = -o $@
ALL_PROGS = v4l2web

UNAME_S := $(shell uname -s)
CC = g++
CFLAGS += -lssl -g -O0

ifeq ($(UNAME_S),Linux)
  CFLAGS += -ldl
endif

all: $(ALL_PROGS)

v4l2web: websocket_html.c v4l2web.c mongoose.c V4l2Device.cpp V4l2MMAPDeviceSource.cpp
	$(CC) $^  $(OUT) $(CFLAGS) -lv4l2

websocket_html.c: websocket.html
	perl mkdata.pl $< > $@

clean:
	-@$(RM) $(ALL_PROGS) websocket_html.c *.exe *.obj *.exp .*o *.lib 
