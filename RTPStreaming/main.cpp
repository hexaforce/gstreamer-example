#include <gst/gst.h>

int main(int argc, char *argv[])
{
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    // GStreamerの初期化
    gst_init(&argc, &argv);

    // パイプラインの作成
    pipeline = gst_parse_launch("playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);

    // パイプラインの再生
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // メインループ
    // bus = gst_element_get_bus(pipeline);
    // msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    // メインループ
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    // パイプラインの状態を停止
    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}
