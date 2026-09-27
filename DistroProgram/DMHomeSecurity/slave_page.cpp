// SLAVE ONLY - a simple page shown if you open a slave camera's own address.
#include "node.h"
#if !IS_MASTER
#include "module.h"
#include "http.h"

namespace {
  esp_err_t hRoot(httpd_req_t* r) {
    String h;
    h.reserve(700);
    h += "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
         "<title>";
    h += Node::id();
    h += "</title><style>body{background:#111;color:#ddd;font:15px system-ui;margin:16px}"
         "img{max-width:100%;border-radius:8px}a{color:#8cf}</style><h3>";
    h += Node::id();
    h += " (slave)</h3><img src='/stream'><p>DMHomeSecurity: <a href='http://" MASTER_HOSTNAME ".local/'>http://"
         MASTER_HOSTNAME ".local/</a> · <a href='/api/status'>status</a></p>";
    return Http::sendText(r, "text/html", h.c_str(), h.length());
  }
  bool pageBegin() { return Http::on("/", HTTP_GET, hRoot); }
}

const Module kModSlavePage = { "slave_page", pageBegin, nullptr, false };
#endif
