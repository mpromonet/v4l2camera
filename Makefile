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
CFLAGS += -I mongoose -I v4l2wrapper/inc -I inc
LDFLAGS += -ljpeg

all: $(ALL_PROGS)

upgrade:
	git submodule foreach git pull origin master

mongoose/mongoose.c: 
	git submodule init mongoose
	git submodule update mongoose

mongoose.o : mongoose/mongoose.c
	$(CC) -o $@ -c $^

libv4l2wrapper.a: 
	git submodule init v4l2wrapper
	git submodule update v4l2wrapper
	make -C v4l2wrapper
	mv v4l2wrapper/libv4l2wrapper.a .
	make -C v4l2wrapper clean

v4l2web: src/main.cpp src/v4l2web.cpp mongoose.o libv4l2wrapper.a
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	-@$(RM) $(ALL_PROGS) .*o *.a
