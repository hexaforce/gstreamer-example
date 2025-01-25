#include "webrtc-common.h"

gchar *get_string_from_json_object(JsonObject *object) {
  JsonNode *root;
  JsonGenerator *generator;
  gchar *text;

  /* Make it the root node */
  root = json_node_init_object(json_node_alloc(), object);
  generator = json_generator_new();
  json_generator_set_root(generator, root);
  text = json_generator_to_data(generator, NULL);

  /* Release everything */
  g_object_unref(generator);
  json_node_free(root);
  return text;
}

gboolean bus_watch_cb(GstBus *bus, GstMessage *message, gpointer user_data) {
  GstPipeline *pipeline = user_data;

  switch (GST_MESSAGE_TYPE(message)) {
  case GST_MESSAGE_ERROR: {
    GError *error = NULL;
    gchar *debug = NULL;

    gst_message_parse_error(message, &error, &debug);
    g_error("Error on bus: %s (debug: %s)", error->message, debug);
    g_error_free(error);
    g_free(debug);
    break;
  }
  case GST_MESSAGE_WARNING: {
    GError *error = NULL;
    gchar *debug = NULL;

    gst_message_parse_warning(message, &error, &debug);
    g_warning("Warning on bus: %s (debug: %s)", error->message, debug);
    g_error_free(error);
    g_free(debug);
    break;
  }
  case GST_MESSAGE_LATENCY:
    gst_bin_recalculate_latency(GST_BIN(pipeline));
    break;
  default:
    break;
  }

  return G_SOURCE_CONTINUE;
}

void destroy_receiver_entry(gpointer receiver_entry_ptr) {
  ReceiverEntry *receiver_entry = (ReceiverEntry *)receiver_entry_ptr;

  g_assert(receiver_entry != NULL);

  if (receiver_entry->pipeline != NULL) {
    GstBus *bus;

    gst_element_set_state(GST_ELEMENT(receiver_entry->pipeline), GST_STATE_NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(receiver_entry->pipeline));
    gst_bus_remove_watch(bus);
    gst_object_unref(bus);

    gst_object_unref(GST_OBJECT(receiver_entry->webrtcbin));
    gst_object_unref(GST_OBJECT(receiver_entry->pipeline));
  }

  if (receiver_entry->connection != NULL)
    g_object_unref(G_OBJECT(receiver_entry->connection));

  g_free(receiver_entry);
}

void on_offer_created_cb(GstPromise *promise, gpointer user_data) {
  gchar *sdp_string;
  gchar *json_string;
  JsonObject *sdp_json;
  JsonObject *sdp_data_json;
  GstStructure const *reply;
  GstPromise *local_desc_promise;
  GstWebRTCSessionDescription *offer = NULL;
  ReceiverEntry *receiver_entry = (ReceiverEntry *)user_data;

  reply = gst_promise_get_reply(promise);
  gst_structure_get(reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
  gst_promise_unref(promise);

  local_desc_promise = gst_promise_new();
  g_signal_emit_by_name(receiver_entry->webrtcbin, "set-local-description", offer, local_desc_promise);
  gst_promise_interrupt(local_desc_promise);
  gst_promise_unref(local_desc_promise);

  sdp_string = gst_sdp_message_as_text(offer->sdp);
  gst_print("Negotiation offer created:\n%s\n", sdp_string);

  sdp_json = json_object_new();
  json_object_set_string_member(sdp_json, "type", "sdp");

  sdp_data_json = json_object_new();
  json_object_set_string_member(sdp_data_json, "type", "offer");
  json_object_set_string_member(sdp_data_json, "sdp", sdp_string);
  json_object_set_object_member(sdp_json, "data", sdp_data_json);

  json_string = get_string_from_json_object(sdp_json);
  json_object_unref(sdp_json);

  soup_websocket_connection_send_text(receiver_entry->connection, json_string);
  g_free(json_string);
  g_free(sdp_string);

  gst_webrtc_session_description_free(offer);
}

void on_negotiation_needed_cb(GstElement *webrtcbin, gpointer user_data) {
  GstPromise *promise;
  ReceiverEntry *receiver_entry = (ReceiverEntry *)user_data;

  gst_print("Creating negotiation offer\n");

  promise = gst_promise_new_with_change_func(on_offer_created_cb, (gpointer)receiver_entry, NULL);
  g_signal_emit_by_name(G_OBJECT(webrtcbin), "create-offer", NULL, promise);
}

void on_ice_candidate_cb(G_GNUC_UNUSED GstElement *webrtcbin, guint mline_index, gchar *candidate, gpointer user_data) {
  JsonObject *ice_json;
  JsonObject *ice_data_json;
  gchar *json_string;
  ReceiverEntry *receiver_entry = (ReceiverEntry *)user_data;

  ice_json = json_object_new();
  json_object_set_string_member(ice_json, "type", "ice");

  ice_data_json = json_object_new();
  json_object_set_int_member(ice_data_json, "sdpMLineIndex", mline_index);
  json_object_set_string_member(ice_data_json, "candidate", candidate);
  json_object_set_object_member(ice_json, "data", ice_data_json);

  json_string = get_string_from_json_object(ice_json);
  json_object_unref(ice_json);

  soup_websocket_connection_send_text(receiver_entry->connection, json_string);
  g_free(json_string);
}

void soup_websocket_message_cb(G_GNUC_UNUSED SoupWebsocketConnection *connection, SoupWebsocketDataType data_type, GBytes *message, gpointer user_data) {
  gsize size;
  const gchar *data;
  gchar *data_string;
  const gchar *type_string;
  JsonNode *root_json;
  JsonObject *root_json_object;
  JsonObject *data_json_object;
  JsonParser *json_parser = NULL;
  ReceiverEntry *receiver_entry = (ReceiverEntry *)user_data;

  switch (data_type) {
  case SOUP_WEBSOCKET_DATA_BINARY:
    g_error("Received unknown binary message, ignoring\n");
    return;

  case SOUP_WEBSOCKET_DATA_TEXT:
    data = g_bytes_get_data(message, &size);
    /* Convert to NULL-terminated string */
    data_string = g_strndup(data, size);
    break;

  default:
    g_assert_not_reached();
  }

  json_parser = json_parser_new();
  if (!json_parser_load_from_data(json_parser, data_string, -1, NULL))
    goto unknown_message;

  root_json = json_parser_get_root(json_parser);
  if (!JSON_NODE_HOLDS_OBJECT(root_json))
    goto unknown_message;

  root_json_object = json_node_get_object(root_json);

  if (!json_object_has_member(root_json_object, "type")) {
    g_error("Received message without type field\n");
    goto cleanup;
  }
  type_string = json_object_get_string_member(root_json_object, "type");

  if (!json_object_has_member(root_json_object, "data")) {
    g_error("Received message without data field\n");
    goto cleanup;
  }
  data_json_object = json_object_get_object_member(root_json_object, "data");

  if (g_strcmp0(type_string, "sdp") == 0) {
    const gchar *sdp_type_string;
    const gchar *sdp_string;
    GstPromise *promise;
    GstSDPMessage *sdp;
    GstWebRTCSessionDescription *answer;
    int ret;

    if (!json_object_has_member(data_json_object, "type")) {
      g_error("Received SDP message without type field\n");
      goto cleanup;
    }
    sdp_type_string = json_object_get_string_member(data_json_object, "type");

    if (g_strcmp0(sdp_type_string, "answer") != 0) {
      g_error("Expected SDP message type \"answer\", got \"%s\"\n", sdp_type_string);
      goto cleanup;
    }

    if (!json_object_has_member(data_json_object, "sdp")) {
      g_error("Received SDP message without SDP string\n");
      goto cleanup;
    }
    sdp_string = json_object_get_string_member(data_json_object, "sdp");

    gst_print("Received SDP:\n%s\n", sdp_string);

    ret = gst_sdp_message_new(&sdp);
    g_assert_cmphex(ret, ==, GST_SDP_OK);

    ret = gst_sdp_message_parse_buffer((guint8 *)sdp_string, strlen(sdp_string), sdp);
    if (ret != GST_SDP_OK) {
      g_error("Could not parse SDP string\n");
      goto cleanup;
    }

    answer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_ANSWER, sdp);
    g_assert_nonnull(answer);

    promise = gst_promise_new();
    g_signal_emit_by_name(receiver_entry->webrtcbin, "set-remote-description", answer, promise);
    gst_promise_interrupt(promise);
    gst_promise_unref(promise);
    gst_webrtc_session_description_free(answer);
  } else if (g_strcmp0(type_string, "ice") == 0) {
    guint mline_index;
    const gchar *candidate_string;

    if (!json_object_has_member(data_json_object, "sdpMLineIndex")) {
      g_error("Received ICE message without mline index\n");
      goto cleanup;
    }
    mline_index = json_object_get_int_member(data_json_object, "sdpMLineIndex");

    if (!json_object_has_member(data_json_object, "candidate")) {
      g_error("Received ICE message without ICE candidate string\n");
      goto cleanup;
    }
    candidate_string = json_object_get_string_member(data_json_object, "candidate");

    gst_print("Received ICE candidate with mline index %u; candidate: %s\n", mline_index, candidate_string);

    g_signal_emit_by_name(receiver_entry->webrtcbin, "add-ice-candidate", mline_index, candidate_string);
  } else
    goto unknown_message;

cleanup:
  if (json_parser != NULL)
    g_object_unref(G_OBJECT(json_parser));
  g_free(data_string);
  return;

unknown_message:
  g_error("Unknown message \"%s\", ignoring", data_string);
  goto cleanup;
}

void soup_websocket_closed_cb(SoupWebsocketConnection *connection, gpointer user_data) {
  GHashTable *receiver_entry_table = (GHashTable *)user_data;
  g_hash_table_remove(receiver_entry_table, connection);
  gst_print("Closed websocket connection %p\n", (gpointer)connection);
}

void soup_http_handler(G_GNUC_UNUSED SoupServer *soup_server, SoupMessage *message, const char *path, G_GNUC_UNUSED GHashTable *query, G_GNUC_UNUSED SoupClientContext *client_context, G_GNUC_UNUSED gpointer user_data) {
  gchar *html_source = (gchar *)user_data;
  SoupBuffer *soup_buffer;

  if ((g_strcmp0(path, "/") != 0) && (g_strcmp0(path, "/index.html") != 0)) {
    soup_message_set_status(message, SOUP_STATUS_NOT_FOUND);
    return;
  }

  soup_buffer = soup_buffer_new(SOUP_MEMORY_STATIC, html_source, strlen(html_source));

  soup_message_headers_set_content_type(message->response_headers, "text/html", NULL);
  soup_message_body_append_buffer(message->response_body, soup_buffer);
  soup_buffer_free(soup_buffer);

  soup_message_set_status(message, SOUP_STATUS_OK);
}
