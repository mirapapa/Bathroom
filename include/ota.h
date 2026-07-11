#ifndef OTA_MODULE_H
#define OTA_MODULE_H

#include "common.h"

// ArduinoOTA + WEB OTA(:8080)
void ota_setup();
void ota_handle();
void verifyFirmware();

// 換気扇システム固有のOTA設定ページ（ota_custom.cppが実装）
String getOtaHtml();
void ota_setup_custom(WebServer &webOtaServer);
String otaProcessor_custom(String html);

// 設定値の増減共通処理（ota.cppが実装、ota_custom.cppから利用）
void handleConfigUpdate(uint16_t &var, int delta, uint16_t min, uint16_t max);

#endif // OTA_MODULE_H
