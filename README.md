[![CircleCI](https://circleci.com/gh/mpromonet/v4l2camera.svg?style=shield)](https://circleci.com/gh/mpromonet/v4l2camera)
[![CirusCI](https://api.cirrus-ci.com/github/mpromonet/v4l2camera.svg?branch=master)](https://cirrus-ci.com/github/mpromonet/v4l2camera)
[![Snap Status](https://build.snapcraft.io/badge/mpromonet/v4l2camera.svg)](https://build.snapcraft.io/user/mpromonet/v4l2camera)

[![Docker Pulls](https://img.shields.io/docker/pulls/mpromonet/v4l2camera.svg)](https://hub.docker.com/r/mpromonet/v4l2camera)

v4l2camera
=======
Web server for V4L2 interface

The civetweb HTTP server give access to the Video4Linux interface and the web interface using VueJS


Dependencies
------------
 - libjpeg-dev
 
Build
------- 
	make

Usage
------- 
	./v4l2web [-v[v]] [-P port] [-W width] [-H height] [device]
         -v               : verbose 
         -v v             : very verbose 
         -P port          : server port (default 8080)
         -p path          : server root path (default webroot)
         -R port          : RTSP server port (default 8554)
         -f format        : V4L2 capture using format
         -W width         : V4L2 capture width (default 640)
         -H height        : V4L2 capture height (default 480)
         -F fps           : V4L2 capture framerate (default 10)
         -G <w>x<h>[x<f>] : V4L2 capture format (default 640x480x10)
         -r               : V4L2 capture using memory mapped buffers (default use read interface)
         -w               : V4L2 capture using write interface (default use memory mapped buffers)
         device           : V4L2 capture device (default /dev/video0)

Using Docker image
===============
You can start the application using the docker image :

        docker run -p 8080:8080 -it mpromonet/v4l2camera

You can expose V4L2 devices from your host using :

        docker run --device=/dev/video0 -p 8080:8080 -it mpromonet/v4l2camera

The container entry point is the v4l2web application, then you can :

* get the help using :

        docker run -it mpromonet/v4l2camera -h

* run the container specifying some paramters :

        docker run --device=/dev/video0 -p 8080:8080 -it mpromonet/v4l2camera -W800 -H600 
