#include <gst/gst.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // GStreamer pipeline string for capturing video from camera and streaming over RTP
    const gchar *pipeline_desc = "v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480 ! videoconvert ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=RECEIVER_IP port=5004";

    // Create GStreamer pipeline
    GError *error = NULL;
    GstElement *pipeline = gst_parse_launch(pipeline_desc, &error);
    if (error != NULL) {
        g_printerr("Error creating pipeline: %s\n", error->message);
        return -1;
    }

    // Start pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait until error or EOS (End Of Stream)
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // Free resources
    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
