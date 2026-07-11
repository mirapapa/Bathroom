#include "logging.h"

// 現在時刻表示
String getSystemTimeStr()
{
  char str[256];

  time_t t;
  struct tm *tm;
  static const char *wd[7] = {"日", "月", "火", "水", "木", "金", "土"};
  t = time(NULL);
  tm = localtime(&t);
  sprintf(str, " %04d/%02d/%02d(%s) %02d:%02d:%02d : ", tm->tm_year + 1900, tm->tm_mon + 1,
          tm->tm_mday, wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);

  return str;
}

// セマフォ取得（修正版）
void takeSemaphore(SemaphoreHandle_t xSemaphore)
{
  if (xSemaphore == NULL)
  {
    Serial.println("▼▼▼セマフォがNULL▼▼▼");
    return;
  }

  // 5秒待機してセマフォ取得を試みる
  if (xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(5000)) != pdTRUE)
  {
    // 5秒待ってもセマフォが取得できない場合
    Serial.println("▼▼▼セマフォ取得タイムアウト - デッドロック検出▼▼▼");
    Serial.println("タスク名: " + String(pcTaskGetName(NULL)));
    Serial.println("Free heap: " + String(ESP.getFreeHeap()));
    Serial.println("Largest free block: " + String(ESP.getMaxAllocHeap()));

    // 緊急対応：リスタート
    Serial.println("システムを再起動します...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP.restart();
  }
}

// セマフォ解放
void giveSemaphore(SemaphoreHandle_t xSemaphore)
{
  if (xSemaphore != NULL)
  {
    xSemaphoreGive(xSemaphore);
  }
}
