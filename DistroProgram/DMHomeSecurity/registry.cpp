// MASTER ONLY - keeps the list of cameras and the website's colours.
//
//   /api/cams      get or save the camera list (names, order, on/off)
//   /api/register  slaves call this every few seconds to say "I'm here"
//   /api/theme     get or save the website's title and colours
//
// Camera types:
//   "local" - the master's own camera
//   "node"  - a DMHomeSecurity slave camera (its settings can be changed)
//   "mjpeg" - any other video stream you added by URL (view only)
//
// The list and colours are saved to the board's storage (cams.json and
// theme.json), only when something actually changes.
#include "node.h"
#if IS_MASTER
#include "module.h"
#include "http.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

namespace {

enum Kind : uint8_t { KIND_LOCAL, KIND_NODE, KIND_MJPEG };
const char* KIND_NAMES[] = { "local", "node", "mjpeg" };

struct Entry {
  char     id[24];
  char     name[48];
  char     url[128];     // slave: "http://ip"; other stream: full URL; master: empty
  Kind     kind;
  bool     enabled;
  uint32_t lastSeen;     // when we last heard from it (0 = never)
};

Entry             g_cams[MAX_CAMERAS];
int               g_count = 0;
SemaphoreHandle_t g_mtx;
volatile bool     g_dirty = false;
bool              g_fsOk = false;

const char* CAMS_FILE  = "/cams.json";
const char* THEME_FILE = "/theme.json";
constexpr size_t THEME_MAX = 8192;

struct Lock {
  Lock()  { xSemaphoreTake(g_mtx, portMAX_DELAY); }
  ~Lock() { xSemaphoreGive(g_mtx); }
};

Kind kindFrom(const char* s) {
  if (s && !strcmp(s, "node"))  return KIND_NODE;
  if (s && !strcmp(s, "local")) return KIND_LOCAL;
  return KIND_MJPEG;
}

void copyStr(char* dst, size_t n, const char* src) {
  strlcpy(dst, src ? src : "", n);
}

int findById(const char* id) {
  for (int i = 0; i < g_count; i++) if (!strcmp(g_cams[i].id, id)) return i;
  return -1;
}

int findByUrl(const char* url) {
  for (int i = 0; i < g_count; i++) if (!strcmp(g_cams[i].url, url)) return i;
  return -1;
}

void defaultName(char* out, size_t n) {
  snprintf(out, n, "Camera %d", g_count + 1);
}

// Make sure the master's own camera is always in the list.
void ensureSelf() {
  if (findById(Node::id()) >= 0) return;
  if (g_count >= MAX_CAMERAS) g_count = MAX_CAMERAS - 1;
  memmove(&g_cams[1], &g_cams[0], sizeof(Entry) * g_count);
  Entry& e = g_cams[0];
  memset(&e, 0, sizeof(e));
  copyStr(e.id, sizeof(e.id), Node::id());
  copyStr(e.name, sizeof(e.name), "Master");
  e.kind = KIND_LOCAL;
  e.enabled = true;
  g_count++;
  g_dirty = true;
}

// ---- load / save the camera list --------------------------------------------
void loadCams() {
  Lock l;
  g_count = 0;
  if (g_fsOk) {
    File f = LittleFS.open(CAMS_FILE, "r");
    if (f) {
      JsonDocument doc;
      if (!deserializeJson(doc, f)) {
        for (JsonObject o : doc.as<JsonArray>()) {
          if (g_count >= MAX_CAMERAS) break;
          Entry& e = g_cams[g_count];
          memset(&e, 0, sizeof(e));
          copyStr(e.id,   sizeof(e.id),   o["id"]   | "");
          copyStr(e.name, sizeof(e.name), o["name"] | "Camera");
          copyStr(e.url,  sizeof(e.url),  o["url"]  | "");
          e.kind    = kindFrom(o["kind"] | "mjpeg");
          e.enabled = o["enabled"] | true;
          if (e.id[0]) g_count++;
        }
      }
      f.close();
    }
  }
  ensureSelf();
  Serial.printf("[reg] %d camera(s) loaded\n", g_count);
}

void saveCams() {
  if (!g_fsOk) { g_dirty = false; return; }
  JsonDocument doc;
  {
    Lock l;
    JsonArray a = doc.to<JsonArray>();
    for (int i = 0; i < g_count; i++) {
      JsonObject o = a.add<JsonObject>();
      o["id"] = g_cams[i].id;  o["name"] = g_cams[i].name;
      o["url"] = g_cams[i].url; o["kind"] = KIND_NAMES[g_cams[i].kind];
      o["enabled"] = g_cams[i].enabled;
    }
    g_dirty = false;
  }
  File f = LittleFS.open(CAMS_FILE, "w");
  if (f) { serializeJson(doc, f); f.close(); Serial.println("[reg] cams saved"); }
}

// ---- web requests ----------------------------------------------------------
esp_err_t hGetCams(httpd_req_t* r) {
  JsonDocument doc;
  doc["self"] = Node::id();
  JsonArray a = doc["cams"].to<JsonArray>();
  uint32_t now = millis();
  {
    Lock l;
    for (int i = 0; i < g_count; i++) {
      const Entry& e = g_cams[i];
      JsonObject o = a.add<JsonObject>();
      o["id"] = e.id; o["name"] = e.name; o["url"] = e.url;
      o["kind"] = KIND_NAMES[e.kind]; o["enabled"] = e.enabled;
      bool online;
      if (e.kind == KIND_LOCAL)      online = true;
      else if (e.kind == KIND_NODE)  online = e.lastSeen && (now - e.lastSeen) < NODE_OFFLINE_AFTER_MS;
      else                           online = true;           // can't tell for other streams
      o["online"] = online;
      o["age"] = e.lastSeen ? (int)((now - e.lastSeen) / 1000) : -1;
    }
  }
  String out;
  serializeJson(doc, out);
  return Http::sendJson(r, out);
}

esp_err_t hPostCams(httpd_req_t* r) {
  String body;
  if (!Http::readBody(r, body, 8192)) return Http::sendError(r, "400 Bad Request", "bad body");
  JsonDocument doc;
  if (deserializeJson(doc, body) || !doc.is<JsonArray>())
    return Http::sendError(r, "400 Bad Request", "expected JSON array");

  Entry next[MAX_CAMERAS];
  int n = 0;
  {
    Lock l;
    for (JsonObject o : doc.as<JsonArray>()) {
      if (n >= MAX_CAMERAS) break;
      Entry& e = next[n];
      memset(&e, 0, sizeof(e));
      const char* id = o["id"] | "";
      if (!id[0]) snprintf(e.id, sizeof(e.id), "ext-%06lx", (unsigned long)(esp_random() & 0xFFFFFF));
      else        copyStr(e.id, sizeof(e.id), id);
      copyStr(e.name, sizeof(e.name), o["name"] | "Camera");
      copyStr(e.url,  sizeof(e.url),  o["url"]  | "");
      e.kind    = kindFrom(o["kind"] | "mjpeg");
      e.enabled = o["enabled"] | true;

      if (!strcmp(e.id, Node::id())) { e.kind = KIND_LOCAL; e.url[0] = 0; }
      else if (e.kind == KIND_LOCAL)  e.kind = KIND_NODE;        // only the master is "local"
      if (e.kind != KIND_LOCAL && strncmp(e.url, "http://", 7) != 0) continue;  // skip bad addresses
      // remove a trailing "/" from slave addresses
      size_t ul = strlen(e.url);
      if (e.kind == KIND_NODE && ul > 7 && e.url[ul - 1] == '/') e.url[ul - 1] = 0;

      int old = findById(e.id);
      if (old >= 0) e.lastSeen = g_cams[old].lastSeen;
      n++;
    }
    memcpy(g_cams, next, sizeof(Entry) * n);
    g_count = n;
    ensureSelf();
    g_dirty = true;
  }
  return hGetCams(r);
}

esp_err_t hRegister(httpd_req_t* r) {
  String body;
  if (!Http::readBody(r, body, 512)) return Http::sendError(r, "400 Bad Request", "bad body");
  JsonDocument doc;
  if (deserializeJson(doc, body)) return Http::sendError(r, "400 Bad Request", "bad json");
  const char* id = doc["id"] | "";
  int port = doc["port"] | 80;
  if (!id[0]) return Http::sendError(r, "400 Bad Request", "missing id");

  String ip = Http::peerIp(r);
  char url[128];
  if (port == 80) snprintf(url, sizeof(url), "http://%s", ip.c_str());
  else            snprintf(url, sizeof(url), "http://%s:%d", ip.c_str(), port);

  const char* name = "";
  {
    Lock l;
    int i = findById(id);
    if (i < 0) {
      // If you added this slave by IP before it checked in, link the two up.
      int j = findByUrl(url);
      if (j >= 0 && g_cams[j].kind == KIND_NODE && !strncmp(g_cams[j].id, "ext-", 4)) {
        copyStr(g_cams[j].id, sizeof(g_cams[j].id), id);
        i = j; g_dirty = true;
      }
    }
    if (i < 0) {
      if (g_count >= MAX_CAMERAS) return Http::sendError(r, "507 Insufficient Storage", "registry full");
      Entry& e = g_cams[g_count];
      memset(&e, 0, sizeof(e));
      copyStr(e.id, sizeof(e.id), id);
      defaultName(e.name, sizeof(e.name));
      e.kind = KIND_NODE; e.enabled = true;
      i = g_count++;
      g_dirty = true;
      Serial.printf("[reg] new camera %s at %s\n", id, url);
    }
    Entry& e = g_cams[i];
    if (strcmp(e.url, url)) { copyStr(e.url, sizeof(e.url), url); g_dirty = true; }
    e.lastSeen = millis() | 1;
    name = e.name;
    JsonDocument resp;
    resp["ok"] = true; resp["name"] = name;
    String out; serializeJson(resp, out);
    return Http::sendJson(r, out);
  }
}

esp_err_t hGetTheme(httpd_req_t* r) {
  String s;
  if (g_fsOk) {
    File f = LittleFS.open(THEME_FILE, "r");
    if (f) { s = f.readString(); f.close(); }
  }
  if (!s.length()) s = "{}";
  return Http::sendJson(r, s);
}

esp_err_t hPostTheme(httpd_req_t* r) {
  String body;
  if (!Http::readBody(r, body, THEME_MAX)) return Http::sendError(r, "400 Bad Request", "bad body");
  JsonDocument doc;
  if (deserializeJson(doc, body) || !doc.is<JsonObject>())
    return Http::sendError(r, "400 Bad Request", "expected JSON object");
  if (!g_fsOk) return Http::sendError(r, "500 Internal Server Error", "no filesystem");
  File f = LittleFS.open(THEME_FILE, "w");
  if (!f) return Http::sendError(r, "500 Internal Server Error", "write failed");
  serializeJson(doc, f);
  f.close();
  return Http::sendJson(r, "{\"ok\":true}");
}

bool regBegin() {
  g_mtx = xSemaphoreCreateMutex();
  g_fsOk = LittleFS.begin(true);      // prepares the storage on first boot
  if (!g_fsOk) Serial.println("[reg] storage failed - in the Arduino IDE pick Partition Scheme: Default with spiffs");
  loadCams();
  Http::on("/api/cams",     HTTP_GET,  hGetCams);
  Http::on("/api/cams",     HTTP_POST, hPostCams);
  Http::on("/api/register", HTTP_POST, hRegister);
  Http::on("/api/theme",    HTTP_GET,  hGetTheme);
  Http::on("/api/theme",    HTTP_POST, hPostTheme);
  return true;
}

void regLoop() {
  if (g_dirty) saveCams();
}

} // namespace

const Module kModRegistry = { "registry", regBegin, regLoop, true };
#endif // IS_MASTER
