// The built-in web server: starts it and provides small helpers for replies.
#include "http.h"
#include "module.h"
#include "config.h"
#include <lwip/sockets.h>

namespace {
  httpd_handle_t g_srv = nullptr;

  esp_err_t optionsHandler(httpd_req_t* r) {
    Http::cors(r);
    httpd_resp_set_hdr(r, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(r, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_set_status(r, "204 No Content");
    return httpd_resp_send(r, nullptr, 0);
  }

  esp_err_t notFound(httpd_req_t* r, httpd_err_code_t) {
    return Http::sendError(r, "404 Not Found", "not found");
  }

  bool httpBegin() {
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.server_port      = 80;
    cfg.max_uri_handlers = 32;
    // Up to 10 connections at once: 4 video viewers plus room for the website.
    cfg.max_open_sockets = 10;
    // When full, close the oldest idle connection (live video is never closed).
    cfg.lru_purge_enable = true;
    cfg.stack_size       = 8192;
    cfg.recv_wait_timeout = 5;
    cfg.send_wait_timeout = 5;
    cfg.uri_match_fn     = httpd_uri_match_wildcard;

    if (httpd_start(&g_srv, &cfg) != ESP_OK) {
      Serial.println("[http] start failed");
      return false;
    }
    httpd_register_err_handler(g_srv, HTTPD_404_NOT_FOUND, notFound);
    Http::on("/*", HTTP_OPTIONS, optionsHandler);
    Serial.println("[http] listening on :80");
    return true;
  }
}

httpd_handle_t Http::server() { return g_srv; }

bool Http::on(const char* uri, httpd_method_t method, Handler fn) {
  if (!g_srv) return false;
  httpd_uri_t u = {};
  u.uri = uri; u.method = method; u.handler = fn; u.user_ctx = nullptr;
  esp_err_t e = httpd_register_uri_handler(g_srv, &u);
  if (e != ESP_OK) Serial.printf("[http] register %s failed 0x%x\n", uri, e);
  return e == ESP_OK;
}

void Http::cors(httpd_req_t* r) {
  httpd_resp_set_hdr(r, "Access-Control-Allow-Origin", "*");
}

esp_err_t Http::sendText(httpd_req_t* r, const char* type, const char* body, size_t len,
                         const char* status) {
  cors(r);
  httpd_resp_set_status(r, status);
  httpd_resp_set_type(r, type);
  httpd_resp_set_hdr(r, "Cache-Control", "no-cache");
  return httpd_resp_send(r, body, len);
}

esp_err_t Http::sendJson(httpd_req_t* r, const String& json, const char* status) {
  return sendText(r, "application/json", json.c_str(), json.length(), status);
}

esp_err_t Http::sendError(httpd_req_t* r, const char* status, const char* msg) {
  String j = "{\"ok\":false,\"error\":\"";
  j += msg; j += "\"}";
  return sendJson(r, j, status);
}

bool Http::readBody(httpd_req_t* r, String& out, size_t maxLen) {
  size_t len = r->content_len;
  if (len == 0 || len > maxLen) return false;
  out = String();
  if (!out.reserve(len)) return false;
  char buf[512];
  size_t got = 0;
  while (got < len) {
    int n = httpd_req_recv(r, buf, min(sizeof(buf), len - got));
    if (n == HTTPD_SOCK_ERR_TIMEOUT) continue;
    if (n <= 0) return false;
    out.concat(buf, n);
    got += n;
  }
  return true;
}

bool Http::query(httpd_req_t* r, const char* key, char* out, size_t outLen) {
  size_t qlen = httpd_req_get_url_query_len(r);
  if (qlen == 0 || qlen > 255) return false;
  char q[256];
  if (httpd_req_get_url_query_str(r, q, sizeof(q)) != ESP_OK) return false;
  return httpd_query_key_value(q, key, out, outLen) == ESP_OK;
}

String Http::peerIp(httpd_req_t* r) {
  int fd = httpd_req_to_sockfd(r);
  struct sockaddr_storage addr;
  socklen_t alen = sizeof(addr);
  if (getpeername(fd, (struct sockaddr*)&addr, &alen) != 0) return String();
  char s[48] = {0};
  if (addr.ss_family == AF_INET) {
    inet_ntop(AF_INET, &((struct sockaddr_in*)&addr)->sin_addr, s, sizeof(s));
  } else if (addr.ss_family == AF_INET6) {
    struct sockaddr_in6* a6 = (struct sockaddr_in6*)&addr;
    // Convert the address to the usual 192.168.x.x form
    const uint8_t* b = (const uint8_t*)&a6->sin6_addr;
    snprintf(s, sizeof(s), "%u.%u.%u.%u", b[12], b[13], b[14], b[15]);
  }
  return String(s);
}

const Module kModHttp = { "web server", httpBegin, nullptr, true };
