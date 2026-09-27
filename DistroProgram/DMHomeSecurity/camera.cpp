// Starts the camera and manages its picture settings.
//
// Every adjustable setting is one line in the PARAMS list below. Add a line
// and the new setting is automatically saved on the board and shown on the
// website's settings panel (as a slider).
#include "camera.h"
#include "node.h"
#include "module.h"
#include "board_xiao_s3_sense.h"
#include <esp_camera.h>
#include <Preferences.h>

namespace {

struct Param {
  const char* key;
  int16_t     min, max;
  int  (*set)(sensor_t*, int);   // nullptr = not a camera setting (see "fps")
  int  (*get)(sensor_t*);
};

// Shortcut so each setting fits on one line:
//   SP(name, lowest, highest, camera function, status field, value type)
#define SP(k, lo, hi, SETTER, FIELD, CAST) \
  { k, lo, hi, [](sensor_t* s, int v) { return s->SETTER(s, (CAST)v); }, \
               [](sensor_t* s) -> int { return s->status.FIELD; } }

const Param PARAMS[] = {
  SP("framesize",      0, 15, set_framesize,      framesize,      framesize_t),  // highest value depends on the camera model
  SP("quality",        6, 63, set_quality,        quality,        int),
  SP("brightness",    -2,  2, set_brightness,     brightness,     int),
  SP("contrast",      -2,  2, set_contrast,       contrast,       int),
  SP("saturation",    -2,  2, set_saturation,     saturation,     int),
  SP("sharpness",     -2,  2, set_sharpness,      sharpness,      int),
  SP("special_effect", 0,  6, set_special_effect, special_effect, int),
  SP("awb",            0,  1, set_whitebal,       awb,            int),
  SP("awb_gain",       0,  1, set_awb_gain,       awb_gain,       int),
  SP("wb_mode",        0,  4, set_wb_mode,        wb_mode,        int),
  SP("aec",            0,  1, set_exposure_ctrl,  aec,            int),
  SP("aec2",           0,  1, set_aec2,           aec2,           int),
  SP("ae_level",      -2,  2, set_ae_level,       ae_level,       int),
  SP("aec_value",      0, 1200, set_aec_value,    aec_value,      int),
  SP("agc",            0,  1, set_gain_ctrl,      agc,            int),
  SP("agc_gain",       0, 30, set_agc_gain,       agc_gain,       int),
  SP("gainceiling",    0,  6, set_gainceiling,    gainceiling,    gainceiling_t),
  SP("bpc",            0,  1, set_bpc,            bpc,            int),
  SP("wpc",            0,  1, set_wpc,            wpc,            int),
  SP("raw_gma",        0,  1, set_raw_gma,        raw_gma,        int),
  SP("lenc",           0,  1, set_lenc,           lenc,           int),
  SP("hmirror",        0,  1, set_hmirror,        hmirror,        int),
  SP("vflip",          0,  1, set_vflip,          vflip,          int),
  SP("dcw",            0,  1, set_dcw,            dcw,            int),
  SP("colorbar",       0,  1, set_colorbar,       colorbar,       int),
  { "fps", 1, 30, nullptr, nullptr },          // frame-rate limit (handled by frame_hub)
};
constexpr size_t N_PARAMS = sizeof(PARAMS) / sizeof(PARAMS[0]);
constexpr size_t FPS_IDX  = N_PARAMS - 1;

int16_t     g_val[N_PARAMS];
bool        g_ok = false;
const char* g_sensorName = "none";
int         g_maxFramesize = FRAMESIZE_UXGA;
uint32_t    g_dirtyAt = 0;       // time of last change, 0 = nothing to save
Preferences g_prefs;

// A fingerprint of the settings list. If you add or remove settings, old saved
// values no longer match and are ignored instead of being loaded wrongly.
uint32_t tableSig() {
  uint32_t h = 2166136261u;
  for (size_t i = 0; i < N_PARAMS; i++)
    for (const char* p = PARAMS[i].key; *p; p++) { h ^= (uint8_t)*p; h *= 16777619u; }
  return h ^ N_PARAMS;
}

int clampParam(size_t i, int v) {
  int hi = (i == 0) ? g_maxFramesize : PARAMS[i].max;
  return v < PARAMS[i].min ? PARAMS[i].min : (v > hi ? hi : v);
}

bool applyIdx(size_t i, int v) {
  v = clampParam(i, v);
  if (PARAMS[i].set) {
    sensor_t* s = esp_camera_sensor_get();
    if (!s || PARAMS[i].set(s, v) != 0) return false;
  }
  g_val[i] = (int16_t)v;
  return true;
}

void readDefaultsFromSensor() {
  sensor_t* s = esp_camera_sensor_get();
  for (size_t i = 0; i < N_PARAMS; i++)
    g_val[i] = PARAMS[i].get ? (int16_t)PARAMS[i].get(s) : DEFAULT_MAX_FPS;
}

void applySensorDefaults() {
  sensor_t* s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {           // the OV3660 camera needs these to look right
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, FRAMESIZE_VGA);
  s->set_quality(s, 12);
  readDefaultsFromSensor();
}

void save() {
  g_prefs.putUInt("sig", tableSig());
  g_prefs.putBytes("v", g_val, sizeof(g_val));
  g_dirtyAt = 0;
  Serial.println("[cam] settings saved");
}

bool load() {
  if (g_prefs.getUInt("sig", 0) != tableSig()) return false;
  int16_t tmp[N_PARAMS];
  if (g_prefs.getBytes("v", tmp, sizeof(tmp)) != sizeof(tmp)) return false;
  for (size_t i = 0; i < N_PARAMS; i++) applyIdx(i, tmp[i]);   // resolution is first in the list, so it's applied first
  return true;
}

bool camBegin() {
  camera_config_t c = {};
  c.pin_pwdn = CAM_PIN_PWDN;   c.pin_reset = CAM_PIN_RESET;
  c.pin_xclk = CAM_PIN_XCLK;   c.pin_sccb_sda = CAM_PIN_SIOD; c.pin_sccb_scl = CAM_PIN_SIOC;
  c.pin_d7 = CAM_PIN_D7; c.pin_d6 = CAM_PIN_D6; c.pin_d5 = CAM_PIN_D5; c.pin_d4 = CAM_PIN_D4;
  c.pin_d3 = CAM_PIN_D3; c.pin_d2 = CAM_PIN_D2; c.pin_d1 = CAM_PIN_D1; c.pin_d0 = CAM_PIN_D0;
  c.pin_vsync = CAM_PIN_VSYNC; c.pin_href = CAM_PIN_HREF; c.pin_pclk = CAM_PIN_PCLK;
  c.xclk_freq_hz = CAMERA_XCLK_HZ;
  c.ledc_timer = LEDC_TIMER_0;  c.ledc_channel = LEDC_CHANNEL_0;
  c.pixel_format = PIXFORMAT_JPEG;
  c.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    // Start at the largest resolution so the memory set aside is big enough
    // for any resolution chosen later on the website.
    c.frame_size = FRAMESIZE_UXGA; c.jpeg_quality = 12;
    c.fb_count = 2; c.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    Serial.println("[cam] WARNING: PSRAM is off - in the Arduino IDE set Tools > PSRAM > OPI PSRAM");
    c.frame_size = FRAMESIZE_SVGA; c.jpeg_quality = 14;
    c.fb_count = 1; c.fb_location = CAMERA_FB_IN_DRAM;
    g_maxFramesize = FRAMESIZE_SVGA;
  }

  esp_err_t err = esp_camera_init(&c);
  if (err != ESP_OK) {
    Serial.printf("[cam] camera did not start (error 0x%x) - check the camera ribbon cable\n", err);
    Node::setState(Node::State::Error);
    return false;
  }

  sensor_t* s = esp_camera_sensor_get();
  camera_sensor_info_t* info = esp_camera_sensor_get_info(&s->id);
  if (info) {
    g_sensorName = info->name;
    if ((int)info->max_size < g_maxFramesize) g_maxFramesize = info->max_size;
  }

  g_prefs.begin("cam", false);
  applySensorDefaults();
  bool restored = load();
  g_ok = true;
  Serial.printf("[cam] %s ready, %s settings\n", g_sensorName, restored ? "restored" : "default");
  return true;
}

void camLoop() {
  // Save 3 seconds after the last change (avoids wearing out the flash memory).
  if (g_dirtyAt && millis() - g_dirtyAt > 3000) save();
}

} // namespace

bool        Cam::ok()         { return g_ok; }
const char* Cam::sensorName() { return g_sensorName; }
uint8_t     Cam::maxFps()     { return (uint8_t)g_val[FPS_IDX]; }

bool Cam::set(const char* key, int value) {
  if (!g_ok) return false;
  for (size_t i = 0; i < N_PARAMS; i++) {
    if (strcmp(key, PARAMS[i].key) == 0) {
      if (!applyIdx(i, value)) return false;
      g_dirtyAt = millis() | 1;
      return true;
    }
  }
  return false;
}

void Cam::resetSettings() {
  if (!g_ok) return;
  g_prefs.clear();
  applySensorDefaults();
}

void Cam::appendStatusJson(String& out) {
  out += "\"sensor\":\""; out += g_sensorName; out += "\",";
  out += "\"camera_ok\":"; out += g_ok ? "true" : "false"; out += ",";
  out += "\"framesizes\":[";
  for (int f = 0; f <= g_maxFramesize; f++) {
    if (f) out += ',';
    out += "\""; out += resolution[f].width; out += "x"; out += resolution[f].height; out += "\"";
  }
  out += "],\"settings\":{";
  for (size_t i = 0; i < N_PARAMS; i++) {
    if (i) out += ',';
    out += "\""; out += PARAMS[i].key; out += "\":"; out += g_val[i];
  }
  out += "},\"ranges\":{";
  for (size_t i = 0; i < N_PARAMS; i++) {
    if (i) out += ',';
    out += "\""; out += PARAMS[i].key; out += "\":[";
    out += PARAMS[i].min; out += ','; out += (i == 0 ? g_maxFramesize : PARAMS[i].max); out += "]";
  }
  out += "}";
}

const Module kModCamera = { "camera", camBegin, camLoop, false };
