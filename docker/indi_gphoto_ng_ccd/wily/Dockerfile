FROM ubuntu:15.10
COPY root/ /
RUN apt-key add /etc/apt/sources.list.d/indi-ppa.key && rm -f /etc/apt/sources.list.d/indi-ppa.key && apt-get update && apt-get install -y build-essential cmake libraw-dev libjpeg-dev cimg-dev \
		libindi-dev libgphoto2-dev libgphoto2-dev
VOLUME ["/src", "/dest"]
WORKDIR /build
CMD cmake /src -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr && make -j2 && make package


