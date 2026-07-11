#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"

// ログサーバ（TCP:4444）
void logServersetup();
void logServer_task(void *pvParameters);
void logprintln(String log);
void logprintln(String log, bool historyFlg);
String getHistoryData();

// 時刻・セマフォ関連（複数モジュールから利用）
String getSystemTimeStr();
void takeSemaphore(SemaphoreHandle_t xSemaphore);
void giveSemaphore(SemaphoreHandle_t xSemaphore);

#endif // LOGGING_H
