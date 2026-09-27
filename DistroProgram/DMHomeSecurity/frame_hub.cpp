#include "frame_hub.h"
#include "camera.h"
#include "config.h"
#include <esp_camera.h>
#include <atomic>

// Pictures are stored in a fixed set of memory "slots" that are reused, so
// memory never gets fragmented. Each slot counts how many viewers are using it.

namespace {

// Enough slots for: the newest picture, the one being taken, and one per viewer.
constexpr int N_SLOTS = MAX_STREAM_CLIENTS + 2;

struct Slot {
  FrameHub::Frame f;
  int16_t refs;          // how many are using this slot (0 = free)
};

Slot               g_slots[N_SLOTS];
int                g_latest = -1;
uint32_t           g_seq = 0;
portMUX_TYPE       g_lock = portMUX_INITIALIZER_UNLOCKED;
EventGroupHandle_t g_evt;
TaskHandle_t       g_task;
std::atomic<int>   g_demand{0};
std::atomic<uint32_t> g_drops{0};
volatile float     g_fps = 0;

constexpr EventBits_t BIT_FRAME = BIT0;

int reserveFreeSlot() {
  int idx = -1;
  portENTER_CRITICAL(&g_lock);
  for (int i = 0; i < N_SLOTS; i++) {
    if (g_slots[i].refs == 0 && i != g_latest) { g_slots[i].refs = 1; idx = i; break; }
  }
  portEXIT_CRITICAL(&g_lock);
  return idx;
}

void publish(int idx) {
  portENTER_CRITICAL(&g_lock);
  g_slots[idx].f.seq = ++g_seq;
  int old = g_latest;
  g_latest = idx;
  if (old >= 0) g_slots[old].refs--;
  portEXIT_CRITICAL(&g_lock);
  // Wake up everyone waiting for a new picture.
  xEventGroupSetBits(g_evt, BIT_FRAME);
  xEventGroupClearBits(g_evt, BIT_FRAME);
}

void captureTask(void*) {
  uint32_t windowStart = millis(), windowFrames = 0;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    if (g_demand.load() <= 0) {
      g_fps = 0;
      ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));
      lastWake = xTaskGetTickCount();
      continue;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) { vTaskDelay(pdMS_TO_TICKS(20)); continue; }

    int idx = (fb->format == PIXFORMAT_JPEG && fb->len <= FRAME_SLOT_BYTES) ? reserveFreeSlot() : -1;
    if (idx < 0) {
      esp_camera_fb_return(fb);
      g_drops++;
      vTaskDelay(pdMS_TO_TICKS(5));
      continue;
    }
    memcpy(g_slots[idx].f.buf, fb->buf, fb->len);
    g_slots[idx].f.len = fb->len;
    g_slots[idx].f.ms  = millis();
    esp_camera_fb_return(fb);
    publish(idx);

    // Measure frames per second every 2 seconds
    windowFrames++;
    uint32_t now = millis();
    if (now - windowStart >= 2000) {
      g_fps = windowFrames * 1000.0f / (now - windowStart);
      windowStart = now; windowFrames = 0;
    }

    // Wait so we don't exceed the frame-rate limit set on the website
    uint8_t cap = Cam::maxFps();
    if (cap < 1) cap = 1;
    TickType_t period = pdMS_TO_TICKS(1000 / cap);
    if (period < 1) period = 1;
    if (!xTaskDelayUntil(&lastWake, period)) lastWake = xTaskGetTickCount();
  }
}

} // namespace

bool FrameHub::begin() {
  for (int i = 0; i < N_SLOTS; i++) {
    g_slots[i].f.buf = (uint8_t*)heap_caps_malloc(FRAME_SLOT_BYTES, MALLOC_CAP_SPIRAM);
    if (!g_slots[i].f.buf) {
      Serial.println("[hub] PSRAM slot alloc failed");
      return false;
    }
    g_slots[i].refs = 0;
  }
  g_evt = xEventGroupCreate();
  // Background task that takes the pictures.
  xTaskCreatePinnedToCore(captureTask, "cap", 4096, nullptr, 5, &g_task, 0);
  return true;
}

void FrameHub::addDemand() {
  if (g_demand.fetch_add(1) == 0 && g_task) xTaskNotifyGive(g_task);
}
void FrameHub::dropDemand() { g_demand.fetch_sub(1); }

const FrameHub::Frame* FrameHub::waitNext(uint32_t afterSeq, uint32_t timeoutMs) {
  uint32_t t0 = millis();
  for (;;) {
    portENTER_CRITICAL(&g_lock);
    if (g_latest >= 0 && g_slots[g_latest].f.seq > afterSeq) {
      Slot& s = g_slots[g_latest];
      s.refs++;
      portEXIT_CRITICAL(&g_lock);
      return &s.f;
    }
    portEXIT_CRITICAL(&g_lock);

    uint32_t elapsed = millis() - t0;
    if (elapsed >= timeoutMs) return nullptr;
    // Sleep until the next picture arrives (or we run out of time).
    xEventGroupWaitBits(g_evt, BIT_FRAME, pdFALSE, pdTRUE, pdMS_TO_TICKS(timeoutMs - elapsed));
  }
}

void FrameHub::release(const Frame* f) {
  if (!f) return;
  portENTER_CRITICAL(&g_lock);
  for (int i = 0; i < N_SLOTS; i++)
    if (&g_slots[i].f == f) { g_slots[i].refs--; break; }
  portEXIT_CRITICAL(&g_lock);
}

float    FrameHub::fps()    { return g_fps; }
uint32_t FrameHub::drops()  { return g_drops.load(); }
int      FrameHub::demand() { return g_demand.load(); }
