#pragma once

#include <Arduino.h>
#include <esp_timer.h>

class WebServer;
class AsyncWebServer;

class EspOtaPortal {
public:
    using Hook = void (*)();

    EspOtaPortal();
    ~EspOtaPortal();
    void setDeviceName(const char* name);
    void setDeviceId(const char* id);
    void setCredentials(const char* username, const char* password);
    void setBasePath(const char* path);
    void setOnStart(Hook hook);
    void setOnEnd(Hook hook);
    void begin(WebServer& server);
#if __has_include(<ESPAsyncWebServer.h>)
    /** Registers routes only; the host application calls server.begin(). */
    void begin(AsyncWebServer& server);
#endif
    String deviceInfoJson() const;

private:
    enum class UploadTarget : uint8_t { Firmware, Filesystem };
    bool beginUpload(UploadTarget target);
    bool writeUpload(const uint8_t* data, size_t length);
    bool finishUpload();
    void abortUpload();
    size_t uploadCapacity(UploadTarget target) const;
    bool hasCredentials() const;
    void scheduleRestart();
    static void restartTimerCallback(void* argument);
    String jsonError() const;
    String jsonSuccess() const;
    static String jsonEscape(const String& value);
    static String normalisePath(const char* path);

    String _deviceName, _deviceId, _username, _password, _basePath = "/ota", _lastError;
    Hook _onStart = nullptr;
    Hook _onEnd = nullptr;
    UploadTarget _uploadTarget = UploadTarget::Firmware;
    bool _uploadActive = false, _uploadFailed = false, _restartPending = false, _restartScheduled = false;
    size_t _uploadedBytes = 0;
    esp_timer_handle_t _restartTimer;
};
