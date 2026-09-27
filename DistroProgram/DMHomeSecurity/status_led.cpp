// Optional: the small built-in LED shows what the camera is doing.
//   fast blinking         = connecting to WiFi
//   short blink every 2 s = connected, nobody watching
//   slow blinking         = someone is watching this camera
//   double blink          = camera problem (check the ribbon cable)
#include "node.h"
#include "module.h"
#if FEATURE_STATUS_LED
#include "board_xiao_s3_sense.h"
#include "net.h"
#include "camera.h"
#include "frame_hub.h"

namespace {
  bool ledBegin() {
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LED_OFF);
    return true;
  }

  void ledLoop() {
    uint32_t t = millis();
    bool on;
    if (!Net::connected())            on = (t / 100) % 2;
    else if (!Cam::ok())              { uint32_t p = t % 2000; on = p < 80 || (p > 200 && p < 280); }
    else if (FrameHub::demand() > 0)  on = (t / 500) % 2;
    else                              on = (t % 2000) < 40;
    digitalWrite(PIN_STATUS_LED, on ? LED_ON : LED_OFF);
  }
}

const Module kModStatusLed = { "status_led", ledBegin, ledLoop, false };
#endif
