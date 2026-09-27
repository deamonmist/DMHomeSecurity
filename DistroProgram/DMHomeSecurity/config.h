// =============================================================================
//  DMHomeSecurity - SETTINGS
//
//  This is the only file most people need to edit.
//  Fill in your WiFi details, choose MASTER or SLAVE, then upload.
// =============================================================================
#pragma once

// -----------------------------------------------------------------------------
// STEP 1: Choose what this camera is
//
//   ROLE_MASTER - runs the website. You need exactly ONE master.
//   ROLE_SLAVE  - every other camera. They find the master on their own.
// -----------------------------------------------------------------------------
#define ROLE_MASTER 1
#define ROLE_SLAVE  2

#define NODE_ROLE   ROLE_MASTER        // change to ROLE_SLAVE for the other cameras

// -----------------------------------------------------------------------------
// STEP 2: Your WiFi network (all cameras must use the same one)
// -----------------------------------------------------------------------------
#define WIFI_SSID       ""             // your WiFi name,     e.g. "MyHomeWiFi"
#define WIFI_PASS       ""             // your WiFi password, e.g. "hunter2"

// -----------------------------------------------------------------------------
// STEP 3 (optional): Website address
//
// The website will be at  http://<MASTER_HOSTNAME>.local
// With the default below that is  http://dmhome.local
// -----------------------------------------------------------------------------
#define MASTER_HOSTNAME "dmhome"

// Slave cameras get names like dmcam-a1b2c3.local (the end is unique per board)
#define SLAVE_HOSTNAME_PREFIX "dmcam"

// Some phones and PCs can't open ".local" addresses. If that happens, give the
// master a fixed IP address: remove the // in front of the four lines below and
// fill in numbers that suit your router. (Or reserve an IP in your router.)
// #define MASTER_STATIC_IP   192,168,1,50
// #define MASTER_GATEWAY     192,168,1,1
// #define MASTER_SUBNET      255,255,255,0
// #define MASTER_DNS         192,168,1,1

// Slaves only: if a slave can't find the master, type the master's IP here,
// e.g. "192.168.1.50". Leave it "" to find the master automatically.
#define MASTER_IP_OVERRIDE ""

// =============================================================================
//  Advanced settings - the defaults work fine for most people
// =============================================================================

// If WiFi is lost for this long, the camera restarts itself (milliseconds).
#define WIFI_REBOOT_AFTER_MS   90000UL

// How often slaves check in with the master, and how long before the
// website shows a silent slave as "Offline" (milliseconds).
#define HEARTBEAT_INTERVAL_MS  10000UL
#define NODE_OFFLINE_AFTER_MS  35000UL

// Most cameras the website will list (including the master).
#define MAX_CAMERAS            12

// How many people can watch the same camera at once.
#define MAX_STREAM_CLIENTS     4

// Largest single picture allowed, in bytes. Only matters at top resolution.
#define FRAME_SLOT_BYTES       (256 * 1024)

// Starting frame-rate limit. You can change it later on the website.
#define DEFAULT_MAX_FPS        15

// Camera clock speed. Leave this alone unless you know you need to change it.
#define CAMERA_XCLK_HZ         20000000

// Optional extras: 1 = on, 0 = off
#define FEATURE_OTA            1       // upload new code over WiFi from the Arduino IDE
#define OTA_PASSWORD           ""      // password for WiFi uploads ("" = none)
#define FEATURE_STATUS_LED     1       // blink the small built-in LED to show status

// --- Stops the upload early if the WiFi details were left empty -------------
static_assert(sizeof(WIFI_SSID) > 1, "Please enter your WiFi name (WIFI_SSID) in config.h");
