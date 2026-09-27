// Optional: upload new code over WiFi. In the Arduino IDE, choose the camera
// under Tools > Port > Network ports. Turn off with FEATURE_OTA 0 in config.h.
#include "node.h"
#include "module.h"
#if FEATURE_OTA
#include <ArduinoOTA.h>
#include <WiFi.h>

namespace {
  bool started = false;

  bool otaBegin() { return true; }   // starts later, once WiFi is connected

  void otaLoop() {
    if (!WiFi.isConnected()) return;
    if (!started) {
      ArduinoOTA.setHostname(Node::hostname());
      if (strlen(OTA_PASSWORD)) ArduinoOTA.setPassword(OTA_PASSWORD);
      ArduinoOTA.setMdnsEnabled(false);   // net.cpp already announces the camera
      ArduinoOTA.onStart([] { Serial.println("[ota] start"); });
      ArduinoOTA.onError([](ota_error_t e) { Serial.printf("[ota] error %u\n", e); });
      ArduinoOTA.begin();
      started = true;
    }
    ArduinoOTA.handle();
  }
}

const Module kModOta = { "ota", otaBegin, otaLoop, false };
#endif
