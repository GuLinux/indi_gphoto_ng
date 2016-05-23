FROM ubuntu:16.04
COPY root/ /
RUN apt-key add /etc/apt/sources.list.d/indi-ppa.key && rm -f /etc/apt/sources.list.d/indi-ppa.key && apt-get update && apt-get install -y \
    build-essential cmake libraw-dev libjpeg-dev cimg-dev libindi-dev libgphoto2-dev libgphoto2-dev libcfitsio3-dev
VOLUME ["/src", "/dest"]
WORKDIR /build
CMD rm -f /build/indi_gphoto_ng_ccd-*.deb && \
    cmake /src -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DUBUNTU_TARGET_VERSION=xenial && \
    make -j2 && make package && cp -av /build/indi_gphoto_ng_ccd-*.deb /dest/


