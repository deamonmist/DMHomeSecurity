// Works out this camera's unique ID and network name.
#include "node.h"
#include <esp_mac.h>

namespace {
  char g_id[16];
  char g_host[40];
  volatile Node::State g_state = Node::State::Booting;
}

void Node::init() {
  // Use the last 3 bytes of the board's hardware (MAC) address so every
  // camera gets a different ID without you having to set one.
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(g_id, sizeof(g_id), "cam-%02x%02x%02x", mac[3], mac[4], mac[5]);
#if IS_MASTER
  snprintf(g_host, sizeof(g_host), "%s", MASTER_HOSTNAME);
#else
  snprintf(g_host, sizeof(g_host), "%s-%02x%02x%02x", SLAVE_HOSTNAME_PREFIX, mac[3], mac[4], mac[5]);
#endif
}

const char* Node::id()       { return g_id; }
const char* Node::hostname() { return g_host; }
const char* Node::roleName() { return IS_MASTER ? "master" : "slave"; }
void Node::setState(State s) { g_state = s; }
Node::State Node::state()    { return g_state; }
