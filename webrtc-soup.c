
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

void soup_websocket_closed(SoupWebsocketConnection *connection, gpointer user_data) {
  GHashTable *entry_table = (GHashTable *)user_data;
  g_hash_table_remove(entry_table, connection);
}

void soup_websocket_message(G_GNUC_UNUSED SoupWebsocketConnection *connection, SoupWebsocketDataType data_type, GBytes *message, gpointer user_data) {
  if (data_type == SOUP_WEBSOCKET_DATA_TEXT) {
    gsize size;
    const gchar *data = g_bytes_get_data(message, &size);
    gchar *data_string = g_strndup(data, size);
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
      g_free(data_string);
    }
  }
}

void soup_websocket_handler(G_GNUC_UNUSED SoupServer *server, SoupWebsocketConnection *connection, G_GNUC_UNUSED const char *path, G_GNUC_UNUSED SoupClientContext *client_context, gpointer user_data) {
  GHashTable *entry_table = (GHashTable *)user_data;
  g_object_ref(G_OBJECT(connection));
  const char *protocol = soup_websocket_connection_get_protocol(connection);
  if (protocol) {
    if (g_strcmp0(protocol, "receiver") == 0) {
      receiver = connection;
    } else if (g_strcmp0(protocol, "transceiver") == 0) {
      transceiver = connection;
    }
  }
  g_signal_connect(G_OBJECT(connection), "closed", G_CALLBACK(soup_websocket_closed), (gpointer)entry_table);
  g_signal_connect(G_OBJECT(connection), "message", G_CALLBACK(soup_websocket_message), NULL);
  g_hash_table_replace(entry_table, connection, connection);
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
  if (g_strcmp0(path, "/") == 0) {
    path = "/index.html";
  }

  gchar *file_path = g_strdup_printf(".%s", path);
  gchar *file_content = NULL;
  gsize file_size = 0;
  GError *error = NULL;
  if (!g_file_get_contents(file_path, &file_content, &file_size, &error)) {
    soup_message_set_status(message, SOUP_STATUS_NOT_FOUND);
    g_free(file_path);
    if (error != NULL) {
      g_error_free(error);
    }
    return;
  }

  SoupBuffer *soup_buffer = soup_buffer_new(SOUP_MEMORY_TAKE, file_content, file_size);
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

void destroy_entry() {
  if (receiver != NULL)
    g_object_unref(G_OBJECT(receiver));
  if (transceiver != NULL)
    g_object_unref(G_OBJECT(transceiver));
}

#if defined(G_OS_UNIX) || defined(__APPLE__)
gboolean exit_sighandler(gpointer user_data) {
  GMainLoop *mainloop = (GMainLoop *)user_data;
  g_main_loop_quit(mainloop);
  return TRUE;
}
#endif

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  GHashTable *entry_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, destroy_entry);
  GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);
  g_assert_nonnull(mainloop);

#if defined(G_OS_UNIX) || defined(__APPLE__)
  g_unix_signal_add(SIGINT, exit_sighandler, mainloop);
  g_unix_signal_add(SIGTERM, exit_sighandler, mainloop);
#endif

  SoupServer *soup_server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "webrtc-soup-server", NULL);
  soup_server_add_handler(soup_server, "/", soup_http_handler, NULL, NULL);
  char *protocols[] = {"receiver", "transceiver", NULL};
  soup_server_add_websocket_handler(soup_server, "/ws", NULL, protocols, soup_websocket_handler, (gpointer)entry_table, NULL);
  soup_server_listen_all(soup_server, SOUP_HTTP_PORT, (SoupServerListenOptions)0, NULL);

  g_printf("WebRTC receiver page link: http://127.0.0.1:%d/receiver.html\n", (gint)SOUP_HTTP_PORT);
  g_printf("WebRTC transceiver page link: http://127.0.0.1:%d/transceiver.html\n", (gint)SOUP_HTTP_PORT);
  g_main_loop_run(mainloop);

  g_object_unref(G_OBJECT(soup_server));
  g_hash_table_destroy(entry_table);
  g_main_loop_unref(mainloop);

  return 0;
}
