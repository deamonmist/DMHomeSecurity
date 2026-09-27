// Small helpers for the built-in web server. Every camera runs one on port 80.
#pragma once
#include <Arduino.h>
#include <esp_http_server.h>

namespace Http {
  using Handler = esp_err_t (*)(httpd_req_t*);

  httpd_handle_t server();
  bool on(const char* uri, httpd_method_t method, Handler fn);

  // Send a reply
  esp_err_t sendJson(httpd_req_t* r, const String& json, const char* status = "200 OK");
  esp_err_t sendText(httpd_req_t* r, const char* type, const char* body, size_t len,
                     const char* status = "200 OK");
  esp_err_t sendError(httpd_req_t* r, const char* status, const char* msg);
  void      cors(httpd_req_t* r);

  // Read a request
  bool      readBody(httpd_req_t* r, String& out, size_t maxLen);
  bool      query(httpd_req_t* r, const char* key, char* out, size_t outLen);
  String    peerIp(httpd_req_t* r);
}
