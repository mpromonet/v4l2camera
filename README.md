[![Build status](https://travis-ci.org/mpromonet/v4l2web.png)](https://travis-ci.org/mpromonet/v4l2web)
[![CircleCI](https://circleci.com/gh/mpromonet/v4l2web.svg?style=shield)](https://circleci.com/gh/mpromonet/v4l2web)
[![CirusCI](https://api.cirrus-ci.com/github/mpromonet/v4l2web.svg?branch=master)](https://cirrus-ci.com/github/mpromonet/v4l2web)
[![Snap Status](https://build.snapcraft.io/badge/mpromonet/v4l2web.svg)](https://build.snapcraft.io/user/mpromonet/v4l2web)

[![Docker Pulls](https://img.shields.io/docker/pulls/mpromonet/v4l2web.svg)](https://hub.docker.com/r/mpromonet/v4l2web)

v4l2web
=======
Web server for V4L2 interface

The civetweb HTTP server give access to the Video4Linux interface and the web interface using AngularJS/EmberJS/ReactJS or Angular.


Dependencies
------------
 - libjsoncpp-dev 
 - libjpeg-dev
 - liblog4cpp5-dev
 
Build
------- 
	make

Usage
------- 
	./v4l2web [-v[v]] [-P port] [-W width] [-H height] [device]
		 -v       : verbose 
		 -v v     : very verbose 
		 -P port  : server port (default 8080)
		 -W width : V4L2 capture width (default 320)
		 -H height: V4L2 capture height (default 240)
		 -F fps   : V4L2 capture framerate (default 10)
		 -M       : V4L2 capture using memory mapped buffers (default use read interface)
		 device   : V4L2 capture device (default /dev/video0)


Using Docker image
===============
You can start the application using the docker image :

        docker run -p 8080:8080 -it mpromonet/v4l2web

You can expose V4L2 devices from your host using :

        docker run --device=/dev/video0 -p 8080:8080 -it mpromonet/v4l2web

The container entry point is the v4l2web application, then you can :

* get the help using :

        docker run -it mpromonet/v4l2web -h

* run the container specifying some paramters :

        docker run --device=/dev/video0 -p 8080:8080 -it mpromonet/v4l2web -W800 -H600 
