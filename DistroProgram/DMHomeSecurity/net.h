// WiFi status, for other parts of the program.
#pragma once
#include <Arduino.h>

namespace Net {
  bool connected();
  String ip();
  int rssi();                  // signal strength (dBm)
  uint32_t linkGeneration();   // goes up by one every time WiFi reconnects
}
