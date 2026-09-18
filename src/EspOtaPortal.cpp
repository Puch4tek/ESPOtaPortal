#include "EspOtaPortal.h"

#include "FS.h"
#include <LittleFS.h>
#include <Update.h>
#include <WebServer.h>
#include <esp_mac.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

#include "ota_page.h"

#ifndef ESPOTA_PORTAL_DEBUG
#define ESPOTA_PORTAL_DEBUG 0
#endif

#if ESPOTA_PORTAL_DEBUG
#define PORTAL_LOG(format, ...) Serial.printf("[EspOtaPortal] " format "\n", ##__VA_ARGS__)
#else
#define PORTAL_LOG(...) do {} while (false)
#endif

#if __has_include(<ESPAsyncWebServer.h>)
#include <ESPAsyncWebServer.h>
#define ESPOTA_PORTAL_HAS_ASYNC 1
#else
#define ESPOTA_PORTAL_HAS_ASYNC 0
#endif

namespace {

constexpr uint64_t kRestartDelayMicroseconds = 2500000;

const esp_partition_t* filesystemPartition() {
    return esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, nullptr);
}

size_t filesystemUsedBytes() {
    return LittleFS.usedBytes();
}
} // namespace

EspOtaPortal::EspOtaPortal() {}

EspOtaPortal::~EspOtaPortal() {
    if (_restartTimer) esp_timer_delete(_restartTimer);
}
void EspOtaPortal::setDeviceName(const char* name) { _deviceName = name ? name : "ESP32"; }
void EspOtaPortal::setDeviceId(const char* id) { _deviceId = id ? id : ""; }
void EspOtaPortal::setCredentials(const char* user, const char* password) { _username = user ? user : ""; _password = password ? password : ""; }
void EspOtaPortal::setBasePath(const char* path) { _basePath = normalisePath(path); PORTAL_LOG("Base path: %s", _basePath.c_str()); }
void EspOtaPortal::setOnStart(Hook hook) { _onStart = hook; }
void EspOtaPortal::setOnEnd(Hook hook) { _onEnd = hook; }

void EspOtaPortal::begin(WebServer& server) {
    esp_timer_create_args_t timerArgs = {};
    timerArgs.callback = &EspOtaPortal::restartTimerCallback;
    timerArgs.arg = this;
    timerArgs.name = "ota_restart";
    const esp_err_t result = esp_timer_create(&timerArgs, &_restartTimer);
    if (result != ESP_OK) {
        _restartTimer = nullptr;
        PORTAL_LOG("Could not create restart timer: %d", static_cast<int>(result));
    }
    const String infoPath = _basePath + "/api/info";
    const String firmwarePath = _basePath + "/api/firmware";
    const String filesystemPath = _basePath + "/api/filesystem";
    PORTAL_LOG("Attaching WebServer routes at %s", _basePath.c_str());
    PORTAL_LOG("Firmware target: %u bytes; filesystem target: %u bytes", static_cast<unsigned>(uploadCapacity(UploadTarget::Firmware)), static_cast<unsigned>(uploadCapacity(UploadTarget::Filesystem)));
    server.on(_basePath, HTTP_GET, [this, &server]() {
        PORTAL_LOG("GET portal"); if (hasCredentials() && !server.authenticate(_username.c_str(), _password.c_str())) {
            PORTAL_LOG("Portal authentication failed"); server.requestAuthentication();
            return;
        } server.send_P(200, "text/html; charset=utf-8", OTA_PAGE);
    });
    server.on(infoPath, HTTP_GET, [this, &server]() { PORTAL_LOG("GET device info");
        if (hasCredentials() && !server.authenticate(_username.c_str(), _password.c_str())) {
            PORTAL_LOG("Info authentication failed"); server.requestAuthentication();
            return;
        }
        server.send(200, "application/json", deviceInfoJson());
    });
    auto addUpload = [this, &server](const String& path, UploadTarget target) {
        server.on(path, HTTP_POST,
            [this, &server]() { if (hasCredentials() && !server.authenticate(_username.c_str(), _password.c_str())) {
                PORTAL_LOG("Upload authentication failed");
                server.requestAuthentication();
                return;
            } PORTAL_LOG("Upload response: %s", _uploadFailed ? _lastError.c_str() : "success");
                server.send(_uploadFailed ? 400 : 200, "application/json", _uploadFailed ? jsonError() : jsonSuccess());
        },
            [this, &server, target]() {
                if (hasCredentials() && !server.authenticate(_username.c_str(), _password.c_str()))
                    return;
                HTTPUpload& upload = server.upload();
                if (upload.status == UPLOAD_FILE_START) {
                    PORTAL_LOG("Upload start: %s", upload.filename.c_str());
                    beginUpload(target);
                } else if (upload.status == UPLOAD_FILE_WRITE)
                    writeUpload(upload.buf, upload.currentSize);
                else if (upload.status == UPLOAD_FILE_END) {
                    PORTAL_LOG("Upload received: %u bytes", static_cast<unsigned>(upload.totalSize));
                    finishUpload();
                } else if (upload.status == UPLOAD_FILE_ABORTED)
                    abortUpload();
            });
    };
    addUpload(firmwarePath, UploadTarget::Firmware);
    addUpload(filesystemPath, UploadTarget::Filesystem);
}

#if ESPOTA_PORTAL_HAS_ASYNC
void EspOtaPortal::begin(AsyncWebServer& server) {
    esp_timer_create_args_t timerArgs = {};
    timerArgs.callback = &EspOtaPortal::restartTimerCallback;
    timerArgs.arg = this;
    timerArgs.name = "ota_restart";
    const esp_err_t result = esp_timer_create(&timerArgs, &_restartTimer);
    if (result != ESP_OK) {
        _restartTimer = nullptr;
        PORTAL_LOG("Could not create restart timer: %d", static_cast<int>(result));
    }
    const String infoPath = _basePath + "/api/info";
    const String firmwarePath = _basePath + "/api/firmware";
    const String filesystemPath = _basePath + "/api/filesystem";
    auto authorised = [this](AsyncWebServerRequest* request) {
        if (!hasCredentials() || request->authenticate(_username.c_str(), _password.c_str())) return true;
        request->requestAuthentication();
        return false;
    };
    server.on(_basePath.c_str(), HTTP_GET, [this, authorised](AsyncWebServerRequest* request) {
        if (authorised(request)) request->send(200, "text/html; charset=utf-8", kHtml);
    });
    server.on(infoPath.c_str(), HTTP_GET, [this, authorised](AsyncWebServerRequest* request) {
        if (authorised(request)) request->send(200, "application/json", deviceInfoJson());
    });
    auto addUpload = [this, &server, authorised](const String& path, UploadTarget target) {
        server.on(path.c_str(), HTTP_POST, [this, authorised](AsyncWebServerRequest* request) {
            if (authorised(request)) request->send(_uploadFailed ? 400 : 200, "application/json", _uploadFailed ? jsonError() : jsonSuccess());
        }, [this, authorised, target](AsyncWebServerRequest* request, String, size_t index, uint8_t* data, size_t len, bool final) {
            if (!authorised(request)) return;
            if (index == 0) beginUpload(target);
            if (len) writeUpload(data, len);
            if (final) finishUpload();
        });
    };
    addUpload(firmwarePath, UploadTarget::Firmware);
    addUpload(filesystemPath, UploadTarget::Filesystem);
}
#endif

bool EspOtaPortal::beginUpload(UploadTarget target) {
    if (_uploadActive) {
        _lastError = "Another update is already in progress";
        _uploadFailed = true;
        PORTAL_LOG("Upload rejected: %s", _lastError.c_str());
        return false;
    } if (_restartPending) {
        _lastError = "An update has already completed; the device is restarting";
        _uploadFailed = true; PORTAL_LOG("Upload rejected: %s", _lastError.c_str());
        return false;
    } _uploadTarget = target;
    _uploadedBytes = 0;
    _uploadFailed = false;
    _lastError = "";
    const size_t capacity = uploadCapacity(target);
    if (!capacity) {
        _lastError = "Required flash partition was not found";
        _uploadFailed = true;
        PORTAL_LOG("Upload failed: %s", _lastError.c_str());
        return false;
    }
    PORTAL_LOG("Starting %s update, capacity %u bytes", target == UploadTarget::Firmware ? "firmware" : "filesystem", static_cast<unsigned>(capacity));
    _uploadActive = Update.begin(capacity, target == UploadTarget::Firmware ? U_FLASH : U_SPIFFS);
    if (!_uploadActive) {
        _lastError = Update.errorString();
        _uploadFailed = true;
        PORTAL_LOG("Update.begin failed: %s", _lastError.c_str());
    } else if (_onStart) {
        _onStart();
    }
    return _uploadActive;
}
bool EspOtaPortal::writeUpload(const uint8_t* data, size_t length) {
    if (!_uploadActive || _uploadFailed) return false;
    if (Update.write(const_cast<uint8_t*>(data), length) != length) {
        _lastError = Update.errorString();
        _uploadFailed = true;
        PORTAL_LOG("Update.write failed after %u bytes: %s", static_cast<unsigned>(_uploadedBytes), _lastError.c_str());
        return false;
    }
    _uploadedBytes += length; return true;
}
bool EspOtaPortal::finishUpload() {
    if (!_uploadActive || _uploadFailed) {
        abortUpload(); return false;
    } _uploadActive = false;
    if (!Update.end(true)) {
        _lastError = Update.errorString();
        _uploadFailed = true;
        PORTAL_LOG("Update.end failed: %s", _lastError.c_str());
        return false;
    } PORTAL_LOG("Update completed: %u bytes", static_cast<unsigned>(_uploadedBytes));
    if (_onEnd) {
        _onEnd();
    }
    _restartPending = true;
    scheduleRestart();
    return true;
}
void EspOtaPortal::abortUpload() {
    if (_uploadActive) Update.abort();
    _uploadActive = false;
    _uploadFailed = true;
    if (_lastError.isEmpty()) _lastError = "Upload was cancelled";
    PORTAL_LOG("Update aborted: %s", _lastError.c_str());
}
void EspOtaPortal::scheduleRestart() {
    _restartScheduled = false;
    if (!_restartTimer) {
        PORTAL_LOG("Restart timer unavailable; restart the device manually");
        return;
    } esp_timer_stop(_restartTimer);
    const esp_err_t result = esp_timer_start_once(_restartTimer, kRestartDelayMicroseconds);
    _restartScheduled = result == ESP_OK;
    if (_restartScheduled) PORTAL_LOG("Restart scheduled in %u ms", static_cast<unsigned>(kRestartDelayMicroseconds / 1000));
    else PORTAL_LOG("Could not schedule restart: %d", static_cast<int>(result));
}
void EspOtaPortal::restartTimerCallback(void* argument) {
    auto* portal = static_cast<EspOtaPortal*>(argument);
    PORTAL_LOG("Restarting after %s update", portal->_uploadTarget == UploadTarget::Firmware ? "firmware" : "filesystem");

    ESP.restart();
}
size_t EspOtaPortal::uploadCapacity(UploadTarget target) const {
    if (target == UploadTarget::Firmware) {
        const esp_partition_t* partition = esp_ota_get_next_update_partition(nullptr);
        return partition ? partition->size : 0;
    }
    const esp_partition_t* partition = filesystemPartition();
    return partition ? partition->size : 0;
}
bool EspOtaPortal::hasCredentials() const {
    return !_username.isEmpty();
}
String EspOtaPortal::deviceInfoJson() const {
    uint8_t mac[6] = {}; esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char macText[18], fallback[16];
    snprintf(macText, sizeof(macText), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    String id = _deviceId;
    if (id.isEmpty()) {
        snprintf(fallback, sizeof(fallback), "ESP-%02X%02X%02X", mac[3], mac[4], mac[5]); id = fallback;
    } return "{\"name\":\"" + jsonEscape(_deviceName.isEmpty() ? "ESP OTA Portal" : _deviceName) + "\",\"id\":\"" + jsonEscape(id) + "\",\"mac\":\"" + macText + "\",\"firmware\":{\"used\":" + String(ESP.getSketchSize()) + ",\"capacity\":" + String(uploadCapacity(UploadTarget::Firmware)) + "},\"filesystem\":{\"used\":" + String(filesystemUsedBytes()) + ",\"capacity\":" + String(uploadCapacity(UploadTarget::Filesystem)) + "}}";
}
String EspOtaPortal::jsonSuccess() const {
    return String("{\"ok\":true,\"restartRequired\":true,\"restarting\":") + (_restartScheduled ? "true" : "false") + "}";
}
String EspOtaPortal::jsonError() const {
    return "{\"ok\":false,\"error\":\"" + jsonEscape(_lastError.isEmpty() ? "Update failed" : _lastError) + "\"}";
}
String EspOtaPortal::jsonEscape(const String& value) {
    String escaped; escaped.reserve(value.length());
    for (size_t i = 0; i < value.length(); ++i) {
        const char c = value[i]; if (c == '"' || c == '\\') escaped += '\\'; if (c == '\n') escaped += F("\\n"); else if (c == '\r') escaped += F("\\r"); else escaped += c;
    } return escaped;
}
String EspOtaPortal::normalisePath(const char* path) {
    String result = path && *path ? path : "/ota";
    if (!result.startsWith("/")) result = String("/") + result;
    while (result.length() > 1 && result.endsWith("/")) result.remove(result.length() - 1);
    return result;
}
