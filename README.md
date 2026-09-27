# DMHomeSecurity

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)

**by Deamonmist**

DMHomeSecurity turns Seeed Studio XIAO ESP32-S3 Sense boards into a home
camera system you can watch from any phone, tablet or computer on your own
WiFi. One camera, the **master**, runs a website. The other cameras, the
**slaves**, find the master on their own and show up on that website.

- See every camera on one page: one large view on top, the rest below
- Click any camera to watch it full screen and adjust its picture
- Rename, reorder, hide, add or remove cameras from the website
- Pick your own title, colours and borders with a colour wheel
- Settings are saved, so everything survives a power cut
- Everything stays on your home network. No cloud, no accounts.

---

## What you need

- 2 or more **Seeed Studio XIAO ESP32-S3 Sense** boards, each with its camera
  attached
- A USB-C cable
- A computer with the **Arduino IDE** (version 2 or newer)
- A 2.4 GHz WiFi network. The boards can't use 5 GHz.
- Optional: a 3D printer and a few M3 bolts to build a housing (see below).

---

## Camera housings (3D printable)

The `3dModels` folder contains three versions of the camera housing. Each one
includes a wall clip and an arm, so the camera can be mounted and aimed.
Choose one version per camera; you can mix them across cameras.

> Illustrated assembly instructions are coming soon.

### Option 1: CameraStandardCap (simplest)

The basic housing.

**Print these files** (folder `3dModels/CameraStandardCap`):

| File | Qty |
|---|---|
| `CameraBody.stl` | 1 |
| `CameraCap01.stl` | 1 |
| `VerticalArm.stl` | 1 |
| `BottomBracket.stl` | 1 |
| `WallClip.stl` | 1 |

**Hardware**

| Part | Qty |
|---|---|
| M3 × 6 mm bolt | 2 |
| M3 × 16 mm bolt | 2 |
| M3 nut | 4 |
| M3 washer | 3 |

### Option 2: CameraStickerCap

The Standard housing with a different cap (`CameraCap02`) and one extra printed
piece, the antenna extender. The hardware is the same as the Standard version.

**Print these files** (folder `3dModels/CameraStickerCap`):

| File | Qty |
|---|---|
| `CameraBody.stl` | 1 |
| `CameraCap02.stl` | 1 |
| `AntennaExtendor.stl` | 1 |
| `VerticalArm.stl` | 1 |
| `BottomBracket.stl` | 1 |
| `WallClip.stl` | 1 |

**Hardware**

| Part | Qty |
|---|---|
| M3 × 6 mm bolt | 2 |
| M3 × 16 mm bolt | 2 |
| M3 nut | 4 |
| M3 washer | 3 |

### Option 3: CameraSMACap (external antenna)

Lets you fit an SMA pigtail connector, so you can use an external screw-on
WiFi antenna. That's useful for a camera that's far from your router.

**Print these files** (folder `3dModels/CameraSMACap`):

| File | Qty |
|---|---|
| `CameraBodySMAAttach.stl` | 1 |
| `CameraCapSMAAttach.stl` | 1 |
| `SMABracket.stl` | 1 |
| `VerticalArm.stl` | 1 |
| `BottomBracket.stl` | 1 |
| `WallClip.stl` | 1 |

Bambu Studio users can open `CameraSMACap_AllParts_BambuStudio.3mf` instead.
It has all six parts laid out on one plate, set up for 0.20 mm layers and a
0.4 mm nozzle.

**Hardware**

| Part | Qty |
|---|---|
| M3 × 6 mm bolt | 4 |
| M3 × 16 mm bolt | 2 |
| M3 nut | 6 |
| M3 washer | 3 |
| SMA pigtail (U.FL / IPEX to SMA) | 1 |
| SMA WiFi antenna (2.4 GHz) | 1 |

### Hardware totals at a glance

| | Standard | Sticker | SMA |
|---|---|---|---|
| Printed parts | 5 | 6 | 6 |
| M3 × 6 mm bolts | 2 | 2 | 4 |
| M3 × 16 mm bolts | 2 | 2 | 2 |
| M3 nuts | 4 | 4 | 6 |
| M3 washers | 3 | 3 | 3 |
| SMA pigtail + antenna | – | – | 1 each |

All quantities are per camera.

---

## Step 1: Set up the Arduino IDE (one time only)

1. Open **File → Preferences** and paste this into
   *Additional boards manager URLs*:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. Open **Tools → Board → Boards Manager**, search **esp32**, and install
   **esp32 by Espressif Systems** (version 3.3 or newer).
3. Open **Tools → Manage Libraries**, search **ArduinoJson**, and install it
   (version 7 or newer).

## Step 2: Open the project

Open the `DMHomeSecurity` folder and double-click **DMHomeSecurity.ino**. All
the other files open as tabs automatically.

> The folder must stay named `DMHomeSecurity`. The Arduino IDE won't open the
> project otherwise.

## Step 3: Enter your WiFi details

Click the **config.h** tab and fill in your WiFi name and password:

```cpp
#define WIFI_SSID       "YourWiFiName"
#define WIFI_PASS       "YourWiFiPassword"
```

If you leave these empty, the upload stops with the message
*"Please enter your WiFi name"*.

## Step 4: Upload to the master camera

1. In **config.h**, make sure this line says `ROLE_MASTER`:
   ```cpp
   #define NODE_ROLE   ROLE_MASTER
   ```
2. In the **Tools** menu, choose:
   - **Board:** XIAO_ESP32S3
   - **PSRAM:** OPI PSRAM (**important**: the camera won't work without it)
   - **Partition Scheme:** Default with spiffs (3MB APP/1.5MB SPIFFS)
   - **Port:** the port your board is plugged into
3. Click **Upload**.

## Step 5: Upload to each slave camera

1. In **config.h**, change the line to `ROLE_SLAVE`:
   ```cpp
   #define NODE_ROLE   ROLE_SLAVE
   ```
2. Plug in the next board and click **Upload**. Repeat for every extra camera.
   You don't need to change anything else. Each board gives itself a unique ID.

## Step 6: Open the website

On any device connected to the same WiFi, go to:

**http://dmhome.local**

The slave cameras appear within about 10 seconds of powering on.

> **Can't open dmhome.local?** Some Android phones and older Windows PCs don't
> support `.local` addresses. Open **Tools → Serial Monitor** (115200 baud)
> with the master plugged in and press its reset button. It prints its IP
> address, for example `http://dmhome.local (192.168.1.50)`. Type that number
> into your browser instead. To make the number permanent, see
> *Troubleshooting*.

---

## Using the website

**Main page.** The large tile at the top is the master camera; the others sit
below it. Click any camera to open it.

**Single-camera view**
- The video fills the screen, with the camera's settings on the left. On a
  phone, tap **Adjust**.
- Changes apply straight away and are saved on that camera.
- **Snapshot** saves a still picture. **Reset** restores the factory picture
  settings. **Reboot** restarts the camera.
- The other cameras appear as small squares at the bottom. Click one to
  switch to it.
- **← Back** returns to the main page. The ⛶ button goes true full screen.

**Gear button (bottom right).** This opens the settings panel:
- **Page:** the title, and the background, panel, text and accent colours.
- **Viewport borders:** border on/off, width, corner rounding and colour.
- **Manage cameras**
  - Rename a camera by typing in its box.
  - The small colour square gives that camera its own border colour.
  - The switch hides or shows a camera.
  - **↑ ↓** changes the order.
  - **Large** picks which camera gets the big tile on the main page.
  - **✕** removes a camera.
  - To add a camera by address, type its IP (for a DMHomeSecurity camera) or
    the full URL of any other MJPEG video stream, then press **Add**.
- Press **Save theme** to keep your changes, or **Cancel** to undo them.

> A slave that is removed while it's still powered on adds itself back within a
> few seconds. To keep it off the page, use its switch to hide it instead.

**The small built-in LED on each board**

| LED | Meaning |
|---|---|
| Fast blinking | Connecting to WiFi |
| Short blink every 2 seconds | Connected, nobody watching |
| Slow blinking | Someone is watching this camera |
| Double blink | Camera problem: check the ribbon cable |

---

## ⚠️ Security: please read

DMHomeSecurity has **no password on the website**. Anyone connected to your
WiFi can watch the cameras and change the settings.

- Only use it on a WiFi network you trust, with a strong WiFi password.
- **Never** open it to the internet. Don't set up port forwarding on your
  router to reach these cameras. To watch from away from home, use a VPN into
  your home network.
- The video isn't encrypted, so treat it like any other device on your home
  network.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| Upload stops with *"Please enter your WiFi name"* | Fill in `WIFI_SSID` and `WIFI_PASS` in **config.h**. |
| Serial Monitor says *"PSRAM is off"* | **Tools → PSRAM → OPI PSRAM**, then upload again. |
| Serial Monitor says *"camera did not start"* | Reseat the camera board and its ribbon cable. |
| Serial Monitor says *"storage failed"* | **Tools → Partition Scheme → Default with spiffs**, then upload again. |
| Board never connects to WiFi | Check the name and password, and use a 2.4 GHz network. |
| A slave never shows up | Check the Serial Monitor on the slave. If it says it *can't find dmhome.local*, put the master's IP in `MASTER_IP_OVERRIDE` in **config.h**, e.g. `"192.168.1.50"`, and upload to the slave again. |
| The master's IP address keeps changing | Reserve an IP for it in your router's settings, or fill in the `MASTER_STATIC_IP` lines in **config.h**. |
| Picture is upside down | Open the camera and switch on **Flip vertical**. It's remembered. |
| *"Too many viewers"* | Each camera allows 4 viewers at once, and every open browser tab counts as one. Close some tabs, or raise `MAX_STREAM_CLIENTS` in **config.h**. |
| Board feels warm | Normal while streaming. A camera rests its sensor when nobody is watching. |

If WiFi drops for more than 90 seconds, a camera restarts itself and
reconnects on its own.

---

## Updating over WiFi

After the first USB upload, you can upload new code without a cable. In the
Arduino IDE, go to **Tools → Port → Network ports** and pick the camera, then
click **Upload**. To require a password for this, set `OTA_PASSWORD` in
**config.h**. To turn the feature off, set `FEATURE_OTA` to `0`.

---

## For tinkerers

The program is split into small parts called **modules**, one per file. They
are listed at the top of **DMHomeSecurity.ino**.

| File | What it does |
|---|---|
| `config.h` | All your settings |
| `DMHomeSecurity.ino` | Starts each module in order |
| `net.cpp` | Connects to WiFi and sets up the `.local` name |
| `camera.cpp` | Starts the camera and keeps its picture settings |
| `frame_hub.cpp` | Takes pictures and shares them with all viewers |
| `camera_api.cpp` | Video and settings web addresses (on every camera) |
| `http.cpp` | The built-in web server |
| `registry.cpp` | Master only: camera list and website colours |
| `web_ui.cpp` | Master only: sends the website to your browser |
| `heartbeat.cpp` | Slave only: tells the master "I'm here" |
| `slave_page.cpp` | Slave only: simple page at the slave's own address |
| `ota.cpp` | WiFi uploads |
| `status_led.cpp` | The status LED |
| `web/` | The website's source files |
| `tools/pack_web.py` | Rebuilds `web_assets.h` after you edit `web/` |
| `3dModels/` | Printable camera housings (see *Camera housings*) |

**To add a picture setting,** add one line to the `PARAMS` list in
`camera.cpp`. It's saved automatically and appears on the website as a slider.

**To add a feature,** create a new `.cpp` file with a module, add it to the list
in `module.h`, then add one line to the `MODULES` list in `DMHomeSecurity.ino`.

**To change the website,** edit the files in `web/`, then run
`python3 tools/pack_web.py` from the `DMHomeSecurity` folder and upload to the
master again.

**Web addresses every camera answers**

| Address | What it does |
|---|---|
| `/stream` | Live video (MJPEG) |
| `/capture` | One still picture (JPEG) |
| `/api/status` | Camera info and settings (JSON) |
| `/api/control?var=NAME&val=N` | Change a setting, e.g. `?var=brightness&val=1` |
| `/api/reboot` | Restart (POST) |

The master also answers `/api/cams` (the camera list) and `/api/theme` (the
colours).

---

## Limits

- Top resolution is 1600×1200.
- There's no recording or motion detection. It's live view only.
- The website works in any modern browser. Chrome gives the smoothest video.

---

## License

DMHomeSecurity is open source under the **MIT License**. You're free to use,
copy, change and share it, including in your own projects, as long as you keep
the copyright notice. See the `LICENSE` file for the full text.

It is built on the Espressif ESP32 Arduino core and the ArduinoJson library,
which have their own open-source licences.

Contributions, bug reports and improvements are welcome.

---

*DMHomeSecurity by Deamonmist*
