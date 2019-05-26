FROM ubuntu:18.04 as builder
LABEL maintainer michel.promonet@free.fr
WORKDIR /v4l2web	
COPY . /v4l2web

RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates g++ autoconf automake libtool xz-utils cmake make pkg-config git liblog4cpp5-dev libjsoncpp-dev libjpeg-dev \
    && make install && apt-get clean && rm -rf /var/lib/apt/lists/

FROM ubuntu:18.04
COPY --from=builder /usr/bin/ /usr/bin/
COPY --from=builder /usr/share/v4l2web/ /usr/share/v4l2web/

RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates liblog4cpp5v5 libjsoncpp1 libjpeg8

EXPOSE 8080
ENTRYPOINT [ "/usr/bin/v4l2web" ]
CMD [ "-p", "/usr/share/v4l2web/" ]
