# ESP OTA Portal

`EspOtaPortal` is a lightweight Arduino library for adding a web-based OTA portal to ESP32 and ESP8266 projects.

It attaches to a web server your firmware already runs and provides:

- firmware uploads
- LittleFS / SPIFFS uploads
- device info as JSON
- optional hooks when OTA starts and ends

## Features

- Works with `WebServer`
- Supports `ESPAsyncWebServer` when available
- Does not start Wi-Fi or create a server for you
- Keeps filesystem mounting and formatting in your application
- Built-in UI with light/dark mode that follows the user’s OS
- Drag-and-drop upload area with file preview and clear action

## Installation

### PlatformIO

Add the library to `lib_deps`:

```ini
lib_deps =
  https://github.com/Puch4tek/ESPOtaPortal.git
```

### Arduino IDE

Install the library from the ZIP archive or by adding it to your Arduino libraries folder, then include it with:

```cpp
#include <EspOtaPortal.h>
```

## Quick start

```cpp
#include <LittleFS.h>
#include <WebServer.h>
#include <EspOtaPortal.h>

WebServer server(80);
EspOtaPortal portal;

void setup() {
  Serial.begin(115200);
  LittleFS.begin(true);

  // Connect Wi-Fi here.
  portal.setDeviceName("Greenhouse controller");
  portal.setDeviceId("greenhouse-1");
  portal.setCredentials("admin", "change-this-password");
  portal.begin(server);

  server.begin();
}

void loop() {
  server.handleClient();
}
```

Open `/ota` in a browser to use the portal.

## Example

The repository includes a full example at `examples/basic/` showing Wi-Fi setup, LittleFS mounting, authentication, and the OTA hooks.

## API

### `EspOtaPortal`

```cpp
EspOtaPortal portal;
```

### Configuration

```cpp
portal.setDeviceName(const char* name);
portal.setDeviceId(const char* id);
portal.setCredentials(const char* username, const char* password);
portal.setBasePath(const char* path);
portal.setOnStart(void (*)());
portal.setOnEnd(void (*)());
```

- `setDeviceName()` sets the display name shown in the UI.
- `setDeviceId()` sets a custom device ID returned by `/api/info`.
- `setCredentials()` enables HTTP basic auth for the portal and API routes.
- `setBasePath()` changes the portal path, for example `/update`.
- `setOnStart()` runs when an OTA upload starts successfully.
- `setOnEnd()` runs after a successful upload completes, before restart is scheduled.

### Attach to a server

```cpp
portal.begin(server);
```

Use this with either:

- `WebServer`
- `AsyncWebServer` when `ESPAsyncWebServer` is available

The host application still owns calling `server.begin()`.

### Device info

```cpp
String json = portal.deviceInfoJson();
```

Returns the JSON used by the built-in UI. It includes:

- device name
- device ID
- MAC address
- firmware size and capacity
- filesystem size and capacity

## Endpoints

If the base path is `/ota`, the portal exposes:

- `GET /ota` — web UI
- `GET /ota/api/info` — device info JSON
- `POST /ota/api/firmware` — firmware upload
- `POST /ota/api/filesystem` — filesystem upload

## OTA requirements

### Firmware updates

Firmware OTA requires a partition table with:

- `otadata`
- `ota_0`
- `ota_1`

### Filesystem updates

Filesystem uploads target the first standard `data, spiffs` partition used by Arduino ESP32 for SPIFFS and LittleFS.

## Filesystem behavior

The library talks to LittleFS internally and does not take a filesystem object in the constructor.

Your application still owns mounting and formatting. Until `LittleFS.begin()` succeeds, reported filesystem usage stays at `0`.

## Notes

- The portal does not connect Wi-Fi for you.
- The portal does not start the HTTP server for you.
- File uploads are validated against the target partition size in the browser and on the device.

## License

MIT
