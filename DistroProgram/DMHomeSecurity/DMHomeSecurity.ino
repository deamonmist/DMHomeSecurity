// =============================================================================
//  DMHomeSecurity by Deamonmist
//  A multi-camera home viewer for the Seeed Studio XIAO ESP32-S3 Sense.
//  Open source under the MIT License - see the LICENSE file.
//
//  Before uploading:
//    1. Open config.h and fill in your WiFi name and password.
//    2. In config.h, set the camera to ROLE_MASTER or ROLE_SLAVE.
//    3. In the Arduino IDE, pick:
//         Board:            XIAO_ESP32S3
//         PSRAM:            OPI PSRAM
//         Partition Scheme: Default with spiffs (3MB APP/1.5MB SPIFFS)
//    4. Install the "ArduinoJson" library (version 7) from the Library Manager.
//
//  You normally don't need to change anything in this file.
// =============================================================================
#include "config.h"
#include "node.h"
#include "module.h"

// The program is made of small parts ("modules"). They start in this order.
// To turn a part off, delete its line. To add a new part, add a line.
static const Module* const MODULES[] = {
#if FEATURE_STATUS_LED
  &kModStatusLed,    // status LED
#endif
  &kModNet,          // connect to WiFi
  &kModCamera,       // start the camera
  &kModHttp,         // start the web server
  &kModCameraApi,    // video stream and camera settings
#if IS_MASTER
  &kModRegistry,     // list of cameras and website colours
  &kModWebUi,        // the website itself
#else
  &kModSlavePage,    // simple page at this camera's own address
  &kModHeartbeat,    // tell the master "I'm here"
#endif
#if FEATURE_OTA
  &kModOta,          // WiFi uploads
#endif
};
static constexpr size_t N_MODULES = sizeof(MODULES) / sizeof(MODULES[0]);

void setup() {
  Serial.begin(115200);
  delay(300);
  Node::init();
  Serial.printf("\n[boot] DMHomeSecurity %s  id=%s  role=%s\n", FW_VERSION, Node::id(), Node::roleName());

  for (size_t i = 0; i < N_MODULES; i++) {
    const Module* m = MODULES[i];
    bool ok = m->begin ? m->begin() : true;
    Serial.printf("[boot] %-12s %s\n", m->name, ok ? "ok" : "FAILED");
    if (!ok && m->required) {
      Serial.println("[boot] an essential part failed - restarting in 5 seconds");
      delay(5000);
      ESP.restart();
    }
  }
}

void loop() {
  for (size_t i = 0; i < N_MODULES; i++)
    if (MODULES[i]->loop) MODULES[i]->loop();
  delay(5);
}
