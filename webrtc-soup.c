
#include <glib.h>
#include <locale.h>

#ifdef G_OS_UNIX
#include <glib-unix.h>
#endif

#include <json-glib/json-glib.h>
#include <libsoup/soup.h>

#define SOUP_HTTP_PORT 57778

SoupWebsocketConnection *receiver;
SoupWebsocketConnection *transceiver;

void soup_websocket_closed_cb(SoupWebsocketConnection *connection, gpointer user_data) {
  GHashTable *receiver_entry_table = (GHashTable *)user_data;
  g_hash_table_remove(receiver_entry_table, connection);
  // gst_print("Closed websocket connection %p\n", (gpointer)connection);
}

void soup_websocket_message_cb2(G_GNUC_UNUSED SoupWebsocketConnection *connection, SoupWebsocketDataType data_type, GBytes *message, gpointer user_data) {
  gsize size;
  const gchar *data;
  gchar *data_string;
  const gchar *type_string;
  const gchar *sdp_string;
  JsonNode *root_json;
  JsonObject *root_json_object;
  JsonObject *data_json_object;
  JsonParser *json_parser = NULL;

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

  if (json_object_has_member(root_json_object, "type")) {
    type_string = json_object_get_string_member(root_json_object, "type");
  } else {
    if (!json_object_has_member(root_json_object, "candidate")) {
      goto unknown_message;
    }
    // gst_print("Received ICE:\n%s\n", data_string);
  }

  const char *protocol = soup_websocket_connection_get_protocol(connection);

  if (protocol) {
    if (g_strcmp0(protocol, "receiver") == 0) {
      if (transceiver) {
        soup_websocket_connection_send_text(transceiver, data_string);
      }
    } else if (g_strcmp0(protocol, "transceiver") == 0) {
      if (receiver) {
        soup_websocket_connection_send_text(receiver, data_string);
      }
    }
  }

cleanup:
  if (json_parser != NULL)
    g_object_unref(G_OBJECT(json_parser));
  g_free(data_string);
  return;

unknown_message:
  g_error("Unknown message \"%s\", ignoring", data_string);
  goto cleanup;
}

void soup_websocket_handler(G_GNUC_UNUSED SoupServer *server, SoupWebsocketConnection *connection, G_GNUC_UNUSED const char *path, G_GNUC_UNUSED SoupClientContext *client_context, gpointer user_data) {
  GHashTable *receiver_entry_table = (GHashTable *)user_data;
  const char *protocol = soup_websocket_connection_get_protocol(connection);

  // gst_print("protocol %s\n", protocol);

  g_signal_connect(G_OBJECT(connection), "closed", G_CALLBACK(soup_websocket_closed_cb), (gpointer)receiver_entry_table);

  if (protocol) {
    if (g_strcmp0(protocol, "receiver") == 0) {
      receiver = connection;
      // gst_print("Processing new websocket connection %p\n", (gpointer)receiver);
    } else if (g_strcmp0(protocol, "transceiver") == 0) {
      transceiver = connection;
      // gst_print("Processing new websocket connection %p\n", (gpointer)transceiver);
    }
  }
  g_object_ref(G_OBJECT(connection));

  g_signal_connect(G_OBJECT(connection), "message", G_CALLBACK(soup_websocket_message_cb2), NULL);

  g_hash_table_replace(receiver_entry_table, connection, connection);
  return;
}

gchar *read_file(const gchar *path) {
  GError *error = NULL;
  gsize length = 0;
  gchar *content = NULL;
  if (!g_file_get_contents(path, &content, &length, &error)) {
    g_error_free(error);
    return NULL;
  }
  return content;
}

void soup_http_handler(G_GNUC_UNUSED SoupServer *soup_server, SoupMessage *message, const char *path, G_GNUC_UNUSED GHashTable *query, G_GNUC_UNUSED SoupClientContext *client_context, G_GNUC_UNUSED gpointer user_data) {
  SoupBuffer *soup_buffer = NULL;
  gchar *file_content = NULL;
  if (g_strcmp0(path, "/") == 0) {
    path = "/index.html";
  }
  gchar *file_path = g_strdup_printf(".%s", path);
  GError *error = NULL;
  gsize file_size = 0;
  if (!g_file_get_contents(file_path, &file_content, &file_size, &error)) {
    soup_message_set_status(message, SOUP_STATUS_NOT_FOUND);
    g_free(file_path);
    if (error != NULL) {
      g_error_free(error);
    }
    return;
  }
  soup_buffer = soup_buffer_new(SOUP_MEMORY_TAKE, file_content, file_size);
  soup_message_body_append_buffer(message->response_body, soup_buffer);
  soup_buffer_free(soup_buffer);
  const char *content_type = "text/html";
  if (g_str_has_suffix(file_path, ".css")) {
    content_type = "text/css";
  } else if (g_str_has_suffix(file_path, ".js")) {
    content_type = "application/javascript";
  } else if (g_str_has_suffix(file_path, ".ico")) {
    content_type = "image/x-icon";
  }
  soup_message_headers_set_content_type(message->response_headers, content_type, NULL);
  soup_message_set_status(message, SOUP_STATUS_OK);
  g_free(file_path);
}

void destroy_receiver_entry() {
  if (receiver != NULL)
    g_object_unref(G_OBJECT(receiver));

  if (transceiver != NULL)
    g_object_unref(G_OBJECT(transceiver));
}

#if defined(G_OS_UNIX) || defined(__APPLE__)
gboolean exit_sighandler(gpointer user_data) {
  // gst_print("Caught signal, stopping mainloop\n");
  GMainLoop *mainloop = (GMainLoop *)user_data;
  g_main_loop_quit(mainloop);
  return TRUE;
}
#endif

int main(int argc, char *argv[]) {
  GMainLoop *mainloop;
  SoupServer *soup_server;
  GHashTable *receiver_entry_table;

  setlocale(LC_ALL, "");

  receiver_entry_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, destroy_receiver_entry);

  mainloop = g_main_loop_new(NULL, FALSE);
  g_assert_nonnull(mainloop);

#if defined(G_OS_UNIX) || defined(__APPLE__)
  g_unix_signal_add(SIGINT, exit_sighandler, mainloop);
  g_unix_signal_add(SIGTERM, exit_sighandler, mainloop);
#endif

  soup_server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "webrtc-soup-server", NULL);
  soup_server_add_handler(soup_server, "/", soup_http_handler, NULL, NULL);
  char *protocols[] = {"receiver", "transceiver", NULL};
  soup_server_add_websocket_handler(soup_server, "/ws", NULL, protocols, soup_websocket_handler, (gpointer)receiver_entry_table, NULL);
  soup_server_listen_all(soup_server, SOUP_HTTP_PORT, (SoupServerListenOptions)0, NULL);

  // gst_print("WebRTC page link: http://127.0.0.1:%d/receiver.html\n", (gint)SOUP_HTTP_PORT);
  // gst_print("WebRTC page link: http://127.0.0.1:%d/transceiver.html\n", (gint)SOUP_HTTP_PORT);

  g_main_loop_run(mainloop);

  g_object_unref(G_OBJECT(soup_server));
  g_hash_table_destroy(receiver_entry_table);
  g_main_loop_unref(mainloop);

  return 0;
}
