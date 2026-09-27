// SLAVE ONLY - tells the master "I'm here" every few seconds, so the master
// always knows this camera's current address.
#include "node.h"
#if !IS_MASTER
#include "module.h"
#include "net.h"
#include <ESPmDNS.h>
#include <HTTPClient.h>

namespace {
  IPAddress g_master;              // master's address (looked up once, then remembered)
  uint32_t  g_last = 0;
  uint32_t  g_gen = 0;
  bool      g_everOk = false;

  bool resolveMaster() {
    if (strlen(MASTER_IP_OVERRIDE)) return g_master.fromString(MASTER_IP_OVERRIDE);
    IPAddress ip = MDNS.queryHost(MASTER_HOSTNAME, 2000);
    if (ip == IPAddress((uint32_t)0)) {
      Serial.printf("[hb] can't find %s.local - is the master on? (or set MASTER_IP_OVERRIDE)\n", MASTER_HOSTNAME);
      return false;
    }
    g_master = ip;
    return true;
  }

  void beat() {
    if (g_master == IPAddress((uint32_t)0) && !resolveMaster()) return;

    HTTPClient http;
    String url = "http://" + g_master.toString() + "/api/register";
    http.setConnectTimeout(2000);
    http.setTimeout(2000);
    if (!http.begin(url)) { g_master = IPAddress((uint32_t)0); return; }
    http.addHeader("Content-Type", "application/json");
    String body = String("{\"id\":\"") + Node::id() + "\",\"port\":80,\"fw\":\"" FW_VERSION "\"}";
    int code = http.POST(body);
    http.end();

    if (code == 200) {
      if (!g_everOk) Serial.printf("[hb] registered with master %s\n", g_master.toString().c_str());
      g_everOk = true;
    } else {
      Serial.printf("[hb] master did not answer (%d) - will look for it again\n", code);
      g_master = IPAddress((uint32_t)0);
    }
  }

  bool hbBegin() { return true; }

  void hbLoop() {
    if (!Net::connected()) return;
    uint32_t gen = Net::linkGeneration();
    // Check in right after WiFi connects, then every HEARTBEAT_INTERVAL_MS.
    if (gen != g_gen || millis() - g_last >= HEARTBEAT_INTERVAL_MS) {
      g_gen = gen;
      g_last = millis();
      beat();
    }
  }
}

const Module kModHeartbeat = { "heartbeat", hbBegin, hbLoop, false };
#endif
