CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
ALL_PROGS = v4l2web

CC = g++
CFLAGS += -lssl -g -O0 -I /usr/include/jsoncpp
LDFLAGS += -ljsoncpp -lv4l2

all: $(ALL_PROGS)

v4l2web: v4l2web.c mongoose.c V4l2Device.cpp V4l2MMAPDeviceSource.cpp
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	-@$(RM) $(ALL_PROGS) *.exe *.obj *.exp .*o *.lib 
