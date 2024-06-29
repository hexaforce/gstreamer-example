FROM ubuntu:22.04

# Set environment variable to noninteractive
ENV DEBIAN_FRONTEND=noninteractive

# Install packages
RUN apt-get update && apt-get install -y \
    gstreamer1.0-tools gstreamer1.0-nice gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-plugins-good libgstreamer1.0-dev
RUN apt-get install -y git meson cmake

RUN apt-get install -y libglib2.0-dev libgstreamer-plugins-bad1.0-dev libsoup2.4-dev libjson-glib-dev libgtk-3-dev

WORKDIR /opt/
COPY . /opt/

RUN make

CMD echo "Waiting a few seconds for you to open the browser at localhost:8080" \
    && sleep 10 \
    && ./webrtc-sendrecv \
    --peer-id=5744 \
    --server=wss://webrtc.nirbheek.in:8443 \
    --disable-ssl

