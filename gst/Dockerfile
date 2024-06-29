FROM fedora:40

COPY webrtc-sendrecv .

RUN dnf update -y

RUN dnf install -y gstreamer1-rtsp-server gstreamer1-plugins-good-extras gstreamer1-plugins-bad-free-extras libnice-gstreamer1

CMD echo "Waiting a few seconds for you to open the browser at localhost:8080" \
    && sleep 10 \
    && ./webrtc-sendrecv \
    --peer-id=1 \
    --server=ws://signalling:8443 \
    --disable-ssl

# docker build . -t fedora/webrtc-sendrecv