// Basic facts about this camera: its ID, its network name and its role.
#pragma once
#include <Arduino.h>
#include "config.h"

#define FW_VERSION "1.0.0"

#if NODE_ROLE == ROLE_MASTER
  #define IS_MASTER 1
#elif NODE_ROLE == ROLE_SLAVE
  #define IS_MASTER 0
#else
  #error "In config.h, NODE_ROLE must be ROLE_MASTER or ROLE_SLAVE"
#endif

namespace Node {
  const char* id();         // unique ID made from the board's hardware address, e.g. "cam-a1b2c3"
  const char* hostname();   // network name, without ".local"
  const char* roleName();   // "master" or "slave"
  enum class State : uint8_t { Booting, WifiConnecting, Online, Streaming, Error };
  void setState(State s);
  State state();
  void init();
}
