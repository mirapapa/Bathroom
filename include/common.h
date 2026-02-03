#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include "esp_sntp.h"
#include <ArduinoOTA.h>
#include "esp_ota_ops.h"
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>

#define SYSTEM_NAME "浴室自動換気システム"
#define SYSTEM_VERSION "1.1.1"

// --- 構造体の定義 ---
typedef struct version
{
    unsigned char id;
    unsigned char verMejor;
    unsigned char verMinor;
    unsigned char verPatch;
} VERSION;

typedef struct
{
    bool send_flg;
    String data;
    int last_http_code;
} SENDSSDATATOSS;

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

// --- 全ての自作関数のプロトタイプ宣言 ---

// ログ関連
void logServer_task(void *pvParameters);
void logprintln(String log);
void logprintln(String log, bool historyFlg);
void logServersetup();
uint nextnum(uint num);
uint prevnum(uint num);
String getHistoryData();
String getSystemTimeStr();

// ネットワーク関連
void ntp_setup();
int wifisetup();
void wificheck();
void mdnssetup();
void timeavailable(struct timeval *t);
bool isWiFiReallyConnected(); // 新規追加

// OTA関連
void ota_setup();
void verifyFirmware();
void ota_handle(); // 新規追加
String getOtaHtml();
void ota_setup_custom(WebServer &webOtaServer);
String otaProcessor_custom(String html);

// Watchdog関連
void watchdog_setup();
void watchdog_subscribe_task(const char *taskName);
void watchdog_reset();
void watchdog_unsubscribe_task(const char *taskName);

// 再起動ログ関連
void rebootLog_setup();
void loadRebootLog();
void saveRebootLog();
void addRebootRecord(esp_reset_reason_t reason, const char *message);
String getRebootLogJson();
String getRebootLogHtml();
void clearRebootLog();
String getRebootReasonString(esp_reset_reason_t reason);
esp_reset_reason_t getCurrentRebootReason();

// セマフォ関連
void takeSemaphore(SemaphoreHandle_t xSemaphore);
void giveSemaphore(SemaphoreHandle_t xSemaphore);

// センサー関連
void sensor_setup();
void sensor_task(void *pvParameters);
void saveConfig();
void loadConfig();

// udpサーバ関連
void udpServer_setup(void);
void udpServer_task(void *pvParameters);
void sendDeviceMode(void);
extern WiFiUDP udp;
extern const int rmoteUdpPort;
extern const char *remoteIpadr;

// udp送信関連
void udpSend_task(void *pvParameters);

// Alexa関連
void alexasetup();
void alexa_task(void *pvParameters);
String refleshtoken();
void changereport(String access_token, bool detect);

// --- 全ての外部変数の宣言 ---
extern const VERSION version;
extern SENDSSDATATOSS sendHDatatoSS;
extern bool firstTimeNtpFlg;
extern RebootLog rebootLog; // 再起動ログ
extern bool manualExhaustfanFlg;
extern bool manualMusicFlg;
extern bool exhaustfanState;
extern SendRecvData recv_deviceState;
extern SendRecvData send_deviceState;
extern uint16_t judgeOnTime;
extern uint16_t startTime;
extern uint16_t continueTime;
extern DeviceState prev_deviceState;
extern SendRecvData recv_deviceState;
extern SendRecvData send_deviceState;
extern void setExhaustfanState(bool state);
extern byte getDoorState(byte pre_doorState);
extern bool getLightMode();
extern bool getExhaustFanMode();
extern void manualExhaustfan();
extern void manualMusic();
extern bool nowMusicFlg;
extern void alexaChangeReport(bool detected);
extern void detectON();
extern void detectOFF();
extern void migratePhase0();
extern void migratePhase1();
extern void migratePhase2();
extern void migratePhase3();
extern void migratePhase4();
extern void migratePhase5();
extern void migratePhase10();
extern void migratePhase11();
extern byte getIrState();
extern void handleConfigUpdate(uint16_t &var, int delta, uint16_t min, uint16_t max);
extern Preferences preferences;

#endif // COMMON_H