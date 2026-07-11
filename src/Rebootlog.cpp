#include "rebootlog.h"
#include "logging.h"
#include <Preferences.h>
#include "esp_system.h"

Preferences preferences;

#define MAX_REBOOT_RECORDS 10
#define NVS_NAMESPACE "reboot_log"

// リングバッファ形式の再起動ログ
RebootLog rebootLog;

// 再起動理由を文字列に変換
String getRebootReasonString(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_POWERON:
        return "Power ON";
    case ESP_RST_SW:
        return "Software Reset";
    case ESP_RST_PANIC:
        return "Exception/Panic";
    case ESP_RST_INT_WDT:
        return "Interrupt WDT";
    case ESP_RST_TASK_WDT:
        return "Task WDT Timeout";
    case ESP_RST_WDT:
        return "Other WDT";
    case ESP_RST_DEEPSLEEP:
        return "Deep Sleep Wake";
    case ESP_RST_BROWNOUT:
        return "Brownout Reset";
    default:
        return "Unknown";
    }
}

// 現在の再起動理由を取得
esp_reset_reason_t getCurrentRebootReason()
{
    return esp_reset_reason();
}

// NVSから再起動ログを読み込み
void loadRebootLog()
{
    preferences.begin(NVS_NAMESPACE, false); // 読み書きモード

    // 総再起動回数
    rebootLog.totalRebootCount = preferences.getUInt("total_count", 0);

    // リングバッファのインデックス
    rebootLog.writeIndex = preferences.getUChar("write_idx", 0);
    rebootLog.recordCount = preferences.getUChar("record_cnt", 0);

    // 各レコードを読み込み
    for (int i = 0; i < MAX_REBOOT_RECORDS; i++)
    {
        String key = "rec_" + String(i);
        size_t len = preferences.getBytesLength(key.c_str());

        if (len == sizeof(RebootRecord))
        {
            preferences.getBytes(key.c_str(), &rebootLog.records[i], sizeof(RebootRecord));
        }
        else
        {
            // データがない場合は初期化
            memset(&rebootLog.records[i], 0, sizeof(RebootRecord));
        }
    }

    preferences.end();

    logprintln("[REBOOT LOG] Loaded from NVS");
    logprintln("[REBOOT LOG] Total reboots: " + String(rebootLog.totalRebootCount));
    logprintln("[REBOOT LOG] Records: " + String(rebootLog.recordCount));
}

// NVSに再起動ログを保存
void saveRebootLog()
{
    preferences.begin(NVS_NAMESPACE, false);

    // 総再起動回数
    preferences.putUInt("total_count", rebootLog.totalRebootCount);

    // リングバッファのインデックス
    preferences.putUChar("write_idx", rebootLog.writeIndex);
    preferences.putUChar("record_cnt", rebootLog.recordCount);

    // 各レコードを保存
    for (int i = 0; i < MAX_REBOOT_RECORDS; i++)
    {
        String key = "rec_" + String(i);
        preferences.putBytes(key.c_str(), &rebootLog.records[i], sizeof(RebootRecord));
    }

    preferences.end();

    logprintln("[REBOOT LOG] Saved to NVS");
}

// 新しい再起動記録を追加
void addRebootRecord(esp_reset_reason_t reason, const char *message)
{
    // 総再起動回数をインクリメント
    rebootLog.totalRebootCount++;

    // 現在のレコードに書き込み
    RebootRecord *record = &rebootLog.records[rebootLog.writeIndex];

    record->timestamp = time(NULL);
    record->rebootReason = (uint8_t)reason;
    strncpy(record->message, message, sizeof(record->message) - 1);
    record->message[sizeof(record->message) - 1] = '\0'; // NULL終端保証

    // ログ出力
    logprintln("[REBOOT LOG] New record added:");
    logprintln("  Reason: " + getRebootReasonString(reason));
    logprintln("  Message: " + String(message));
    logprintln("  Total count: " + String(rebootLog.totalRebootCount));

    // リングバッファのインデックスを更新
    rebootLog.writeIndex = (rebootLog.writeIndex + 1) % MAX_REBOOT_RECORDS;

    // レコード数を更新（最大MAX_REBOOT_RECORDS）
    if (rebootLog.recordCount < MAX_REBOOT_RECORDS)
    {
        rebootLog.recordCount++;
    }

    // NVSに保存
    saveRebootLog();
}

// 再起動ログをJSON形式で取得（WEB OTA用）
String getRebootLogJson()
{
    String json = "{";

    // 総再起動回数
    json += "\"totalReboots\":" + String(rebootLog.totalRebootCount) + ",";

    // 現在の稼働時間（秒）
    uint64_t uptimeSec = esp_timer_get_time() / 1000000ULL;
    json += "\"uptime\":" + String((unsigned long)uptimeSec) + ",";

    // レコード配列
    json += "\"records\":[";

    // 最新のレコードから順に取得（リングバッファを逆順に読む）
    for (int i = 0; i < rebootLog.recordCount; i++)
    {
        // 最新から古い順に読むためのインデックス計算
        int idx = (rebootLog.writeIndex - 1 - i + MAX_REBOOT_RECORDS) % MAX_REBOOT_RECORDS;
        RebootRecord *record = &rebootLog.records[idx];

        if (i > 0)
            json += ",";

        json += "{";
        json += "\"time\":" + String(record->timestamp) + ",";

        // タイムスタンプを人間が読める形式に変換
        struct tm timeinfo;
        localtime_r(&record->timestamp, &timeinfo);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        json += "\"timeStr\":\"" + String(timeStr) + "\",";

        json += "\"reason\":\"" + getRebootReasonString((esp_reset_reason_t)record->rebootReason) + "\",";
        json += "\"message\":\"" + String(record->message) + "\"";
        json += "}";
    }

    json += "]";
    json += "}";

    return json;
}

// 再起動ログの初期化（setup時に呼び出す）
void rebootLog_setup()
{
    // NVSからログを読み込み
    loadRebootLog();

    // 現在の再起動理由を取得
    esp_reset_reason_t reason = getCurrentRebootReason();

    // メッセージを生成
    char message[64];
    sprintf(message, "Reboot #%lu", rebootLog.totalRebootCount + 1);

    // 新しい再起動記録を追加
    addRebootRecord(reason, message);

    logprintln("[REBOOT LOG] Initialization complete");
}