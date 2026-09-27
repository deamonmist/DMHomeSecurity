// Pin numbers for the camera on the Seeed Studio XIAO ESP32-S3 Sense.
// Don't change these unless you are using a different board.
#pragma once

#define CAM_PIN_PWDN   -1
#define CAM_PIN_RESET  -1
#define CAM_PIN_XCLK   10
#define CAM_PIN_SIOD   40
#define CAM_PIN_SIOC   39
#define CAM_PIN_D7     48
#define CAM_PIN_D6     11
#define CAM_PIN_D5     12
#define CAM_PIN_D4     14
#define CAM_PIN_D3     16
#define CAM_PIN_D2     18
#define CAM_PIN_D1     17
#define CAM_PIN_D0     15
#define CAM_PIN_VSYNC  38
#define CAM_PIN_HREF   47
#define CAM_PIN_PCLK   13

// The small built-in LED. It shares a pin with the SD card slot, so turn
// FEATURE_STATUS_LED off in config.h if you ever use the SD card.
#define PIN_STATUS_LED 21
#define LED_ON  LOW
#define LED_OFF HIGH
