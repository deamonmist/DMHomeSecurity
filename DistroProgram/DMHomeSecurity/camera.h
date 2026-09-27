// Starts the camera and handles its picture settings (brightness, resolution, etc.).
// Settings are saved on the board so they survive a restart.
#pragma once
#include <Arduino.h>

namespace Cam {
  bool ok();
  const char* sensorName();
  // Change one setting by name, e.g. set("brightness", 1). Saved automatically.
  // Returns false if the name is unknown or the camera refused the value.
  bool set(const char* key, int value);
  // Adds the camera's current settings to a JSON reply for the website.
  void appendStatusJson(String& out);
  uint8_t maxFps();
  // Go back to the factory settings.
  void resetSettings();
}
