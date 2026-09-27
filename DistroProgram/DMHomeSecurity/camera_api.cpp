// The web addresses every camera (master and slaves) answers:
//
//   /stream                        live video
//   /capture                       one still picture
//   /api/status                    camera info and current settings
//   /api/control?var=NAME&val=N    change a setting, e.g. ?var=brightness&val=1
//   /api/control?var=reset         go back to factory settings
//   /api/reboot                    restart the camera
//
// Each live video viewer is handled by its own background helper, so watching
// video never slows down the rest of the website.
#include "module.h"
#include "node.h"
#include "net.h"
#include "http.h"
#include "camera.h"
#include "frame_hub.h"
#include <atomic>

namespace {

#define PART_BOUNDARY "dmframe"
const char* STREAM_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
const char* PART_FMT    = "\r\n--" PART_BOUNDARY "\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\nX-Seq: %u\r\n\r\n";

QueueHandle_t    g_queue;
std::atomic<int> g_busy{0};
bool             g_hubOk = false;   // true when the camera is ready for video

void serveStream(httpd_req_t* r) {
  httpd_resp_set_type(r, STREAM_TYPE);
  Http::cors(r);
  httpd_resp_set_hdr(r, "Cache-Control", "no-store");

  FrameHub::addDemand();
  uint32_t lastSeq = 0;
  int misses = 0;
  char hdr[128];

  for (;;) {
    const FrameHub::Frame* f = FrameHub::waitNext(lastSeq, 3000);
    if (!f) {                       // no picture: give up after about 15 seconds
      if (++misses >= 5) break;
      continue;
    }
    misses = 0;
    lastSeq = f->seq;
    int n = snprintf(hdr, sizeof(hdr), PART_FMT, (unsigned)f->len, (unsigned)f->seq);
    esp_err_t e = httpd_resp_send_chunk(r, hdr, n);
    if (e == ESP_OK) e = httpd_resp_send_chunk(r, (const char*)f->buf, f->len);
    FrameHub::release(f);
    if (e != ESP_OK) break;          // viewer closed the page
  }
  FrameHub::dropDemand();
  httpd_resp_send_chunk(r, nullptr, 0);
}

void streamWorker(void*) {
  for (;;) {
    httpd_req_t* r;
    if (xQueueReceive(g_queue, &r, portMAX_DELAY) != pdTRUE) continue;
    serveStream(r);
    httpd_req_async_handler_complete(r);
    g_busy--;
  }
}

esp_err_t hStream(httpd_req_t* r) {
  if (!g_hubOk)                       return Http::sendError(r, "503 Service Unavailable", "camera not available");
  if (g_busy >= MAX_STREAM_CLIENTS)  return Http::sendError(r, "503 Service Unavailable", "too many viewers");
  httpd_req_t* copy = nullptr;
  if (httpd_req_async_handler_begin(r, &copy) != ESP_OK)
    return Http::sendError(r, "500 Internal Server Error", "async begin failed");
  g_busy++;
  if (xQueueSend(g_queue, &copy, 0) != pdTRUE) {
    g_busy--;
    httpd_req_async_handler_complete(copy);
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t hCapture(httpd_req_t* r) {
  if (!g_hubOk) return Http::sendError(r, "503 Service Unavailable", "camera not available");
  FrameHub::addDemand();
  const FrameHub::Frame* f = FrameHub::waitNext(0, 3000);
  FrameHub::dropDemand();
  if (!f) return Http::sendError(r, "504 Gateway Timeout", "no frame");
  Http::cors(r);
  httpd_resp_set_type(r, "image/jpeg");
  httpd_resp_set_hdr(r, "Cache-Control", "no-store");
  httpd_resp_set_hdr(r, "Content-Disposition", "inline; filename=capture.jpg");
  esp_err_t e = httpd_resp_send(r, (const char*)f->buf, f->len);
  FrameHub::release(f);
  return e;
}

esp_err_t hStatus(httpd_req_t* r) {
  String j;
  j.reserve(1400);
  j += "{\"id\":\"";   j += Node::id();
  j += "\",\"role\":\""; j += Node::roleName();
  j += "\",\"hostname\":\""; j += Node::hostname();
  j += "\",\"fw\":\"" FW_VERSION "\",\"ip\":\""; j += Net::ip();
  j += "\",\"rssi\":"; j += Net::rssi();
  j += ",\"uptime_s\":"; j += millis() / 1000;
  j += ",\"heap\":"; j += ESP.getFreeHeap();
  j += ",\"psram\":"; j += ESP.getFreePsram();
  j += ",\"fps\":"; j += String(FrameHub::fps(), 1);
  j += ",\"drops\":"; j += FrameHub::drops();
  j += ",\"viewers\":"; j += g_busy.load();
  j += ",";
  Cam::appendStatusJson(j);
  j += "}";
  return Http::sendJson(r, j);
}

esp_err_t hControl(httpd_req_t* r) {
  char var[32], val[16];
  if (!Http::query(r, "var", var, sizeof(var))) return Http::sendError(r, "400 Bad Request", "missing var");
  if (strcmp(var, "reset") == 0) { Cam::resetSettings(); return Http::sendJson(r, "{\"ok\":true}"); }
  if (!Http::query(r, "val", val, sizeof(val))) return Http::sendError(r, "400 Bad Request", "missing val");
  if (!Cam::set(var, atoi(val)))                return Http::sendError(r, "400 Bad Request", "rejected");
  return Http::sendJson(r, "{\"ok\":true}");
}

esp_err_t hReboot(httpd_req_t* r) {
  Http::sendJson(r, "{\"ok\":true}");
  delay(200);
  ESP.restart();
  return ESP_OK;
}

bool apiBegin() {
  g_queue = xQueueCreate(MAX_STREAM_CLIENTS, sizeof(httpd_req_t*));
  for (int i = 0; i < MAX_STREAM_CLIENTS; i++) {
    char name[12]; snprintf(name, sizeof(name), "strm%d", i);
    xTaskCreatePinnedToCore(streamWorker, name, 4096, nullptr, 4, nullptr, 1);
  }
  g_hubOk = Cam::ok() && FrameHub::begin();
  Http::on("/stream",      HTTP_GET,  hStream);
  Http::on("/capture",     HTTP_GET,  hCapture);
  Http::on("/api/status",  HTTP_GET,  hStatus);
  Http::on("/api/control", HTTP_GET,  hControl);
  Http::on("/api/reboot",  HTTP_POST, hReboot);
  return g_hubOk;                    // even without a camera, the website keeps working
}

} // namespace

const Module kModCameraApi = { "camera_api", apiBegin, nullptr, false };
