
[Chrome]
chrome://webrtc-internals/

[Firefox]
about:webrtc

https://caniuse.com/webp

https://caniuse.com/mpeg4
https://caniuse.com/hevc
https://caniuse.com/av1


https://caniuse.com/webcodecs

https://caniuse.com/opus
https://caniuse.com/ogg-vorbis

gst-launch-1.0 autovideosrc ! videoconvert ! autovideosink
gst-launch-1.0 autoaudiosrc ! audioconvert ! autoaudiosink

GST_DEBUG=3 gst-launch-1.0 -v avfvideosrc ! videoconvert ! vtenc_h264_hw realtime=true ! vtdec_hw ! videoconvert ! osxvideosink sync=false
GST_DEBUG=3 gst-launch-1.0 -v avfvideosrc ! videoconvert ! vtenc_h265_hw realtime=true ! vtdec_hw ! videoconvert ! osxvideosink sync=false

GST_DEBUG=3 gst-launch-1.0 -v avfvideosrc ! videoconvert ! vp8enc ! vp8dec ! videoconvert ! osxvideosink sync=false
GST_DEBUG=3 gst-launch-1.0 -v avfvideosrc ! videoconvert ! vp9enc target-bitrate=300000 deadline=1 keyframe-max-dist=15 cpu-used=16 threads=16 end-usage=cbr ! vp9dec ! videoconvert ! osxvideosink sync=false

GST_DEBUG=3 gst-launch-1.0 -v avfvideosrc ! videoconvert ! svtav1enc ! dav1ddec ! videoconvert ! osxvideosink sync=false
