// Connects to WiFi, announces the camera's ".local" name on the network, and
// restarts the board if WiFi stays down too long.
#include "net.h"
#include "node.h"
#include "module.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <atomic>

namespace {
  volatile bool         g_gotIp = false;   // set when WiFi connects
  volatile bool         g_up = false;      // true while connected
  std::atomic<uint32_t> g_gen{0};          // counts reconnects
  uint32_t              g_downSince = 0;

  // Called by the WiFi system when the connection changes.
  void onEvent(arduino_event_id_t ev) {
    switch (ev) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        g_up = true; g_gotIp = true; g_gen++;
        break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        g_up = false;
        break;
      default: break;
    }
  }

  // Makes the camera reachable as http://<name>.local
  void startMdns() {
    MDNS.end();
    if (!MDNS.begin(Node::hostname())) { Serial.println("[net] could not start .local name"); return; }
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("dmhomesec", "tcp", 80);
    MDNS.addServiceTxt("dmhomesec", "tcp", "id", Node::id());
    MDNS.addServiceTxt("dmhomesec", "tcp", "role", Node::roleName());
#if FEATURE_OTA
    // Lets the Arduino IDE show this camera under Tools > Port (network ports).
    MDNS.addService("arduino", "tcp", 3232);
    MDNS.addServiceTxt("arduino", "tcp", "tcp_check", "no");
    MDNS.addServiceTxt("arduino", "tcp", "ssh_upload", "no");
    MDNS.addServiceTxt("arduino", "tcp", "board", ARDUINO_BOARD);
    MDNS.addServiceTxt("arduino", "tcp", "auth_upload", strlen(OTA_PASSWORD) ? "yes" : "no");
#endif
    Serial.printf("[net] http://%s.local  (%s)\n", Node::hostname(), WiFi.localIP().toString().c_str());
  }

  bool netBegin() {
    Node::setState(Node::State::WifiConnecting);
    WiFi.onEvent(onEvent);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(Node::hostname());
#if IS_MASTER && defined(MASTER_STATIC_IP)
    WiFi.config(IPAddress(MASTER_STATIC_IP), IPAddress(MASTER_GATEWAY),
                IPAddress(MASTER_SUBNET), IPAddress(MASTER_DNS));
#endif
    WiFi.setSleep(false);            // WiFi power saving makes video choppy, so turn it off
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait up to 20 seconds. If it isn't connected yet, keep trying in the background.
    uint32_t t0 = millis();
    while (!g_up && millis() - t0 < 20000) delay(100);
    g_downSince = millis();
    if (!g_up) Serial.println("[net] no WiFi yet - still trying (check the name and password in config.h)");
    return true;
  }

  void netLoop() {
    if (g_gotIp) {
      g_gotIp = false;
      startMdns();
      if (Node::state() == Node::State::WifiConnecting) Node::setState(Node::State::Online);
    }
    if (g_up) {
      g_downSince = millis();
    } else {
      Node::setState(Node::State::WifiConnecting);
      if (millis() - g_downSince > WIFI_REBOOT_AFTER_MS) {
        Serial.println("[net] WiFi has been down too long - restarting");
        delay(100);
        ESP.restart();
      }
    }
  }
}

bool     Net::connected()      { return g_up; }
String   Net::ip()             { return WiFi.localIP().toString(); }
int      Net::rssi()           { return WiFi.RSSI(); }
uint32_t Net::linkGeneration() { return g_gen.load(); }

const Module kModNet = { "wifi", netBegin, netLoop, true };
