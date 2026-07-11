#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include "esp_sntp.h"
#include "esp_ota_ops.h"
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>

#define SYSTEM_NAME "浴室自動換気システム"
#define SYSTEM_VERSION "1.1.1"

// --- モジュール間で共有する構造体の定義 ---
// (各モジュールの関数プロトタイプ／extern変数は、対応する
//  logging.h / wifi.h / ntp.h / watchdog.h / rebootlog.h /
//  ota.h / sensorTask.h / udp.h / alexaTask.h を参照)

typedef struct version
{
    unsigned char id;
    unsigned char verMejor;
    unsigned char verMinor;
    unsigned char verPatch;
} VERSION;

// 再起動記録の構造体
typedef struct
{
    time_t timestamp;     // 再起動時刻
    uint8_t rebootReason; // 再起動理由（esp_reset_reason_t）
    char message[64];     // メッセージ
} RebootRecord;

// リングバッファ形式の再起動ログ
typedef struct
{
    RebootRecord records[10];  // 最大10件
    uint8_t writeIndex;        // 次に書き込む位置
    uint8_t recordCount;       // 現在の記録数
    uint32_t totalRebootCount; // 総再起動回数
} RebootLog;

// 機器状態データ
typedef struct deviceState
{
    unsigned char lightState;
    unsigned char lightForced;
    unsigned char exhaustfanState;
    unsigned char exhaustfanForced;
    unsigned char musicState;
    unsigned char musicForced;
} DeviceState;

typedef struct sendrecvData
{
    VERSION version;
    DeviceState deviceState;
} SendRecvData;

#endif // COMMON_H
