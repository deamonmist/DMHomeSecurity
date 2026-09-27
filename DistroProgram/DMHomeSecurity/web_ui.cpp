// MASTER ONLY - sends the website (stored in web_assets.h) to your browser.
#include "node.h"
#if IS_MASTER
#include "module.h"
#include "http.h"
#include "web_assets.h"

namespace {
  esp_err_t hIndex(httpd_req_t* r) { return Http::sendText(r, "text/html; charset=utf-8", WEB_INDEX_HTML, strlen(WEB_INDEX_HTML)); }
  esp_err_t hCss(httpd_req_t* r)   { return Http::sendText(r, "text/css", WEB_APP_CSS, strlen(WEB_APP_CSS)); }
  esp_err_t hJs(httpd_req_t* r)    { return Http::sendText(r, "application/javascript", WEB_APP_JS, strlen(WEB_APP_JS)); }
  esp_err_t hIcon(httpd_req_t* r)  { httpd_resp_set_status(r, "204 No Content"); return httpd_resp_send(r, nullptr, 0); }

  bool uiBegin() {
    return Http::on("/",            HTTP_GET, hIndex)
        && Http::on("/app.css",     HTTP_GET, hCss)
        && Http::on("/app.js",      HTTP_GET, hJs)
        && Http::on("/favicon.ico", HTTP_GET, hIcon);
  }
}

const Module kModWebUi = { "web_ui", uiBegin, nullptr, false };
#endif
