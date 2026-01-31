#include "common.h"

// #include <Audio.h>
// Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);
//  Audio audio;

#define DEBUG
TaskHandle_t thp[5];

void setup()
{
  Serial.begin(115200);

  logServersetup(); // logServerのセットアップ
  sensor_setup();   // sensorのセットアップ

  logprintln(F(""));
  logprintln("***********************************");
  logprintln("** " SYSTEM_NAME "          **");
  logprintln("**   (ver" SYSTEM_VERSION ")                  **");
  logprintln("***********************************");
#ifdef CONFIG_APP_ROLLBACK_ENABLE
  logprintln(F("CONFIG_APP_ROLLBACK_ENABLE"));
#endif // CONFIG_APP_ROLLBACK_ENABLE
  logprintln(F(""));

  ntp_setup();  // NTPクライアントのセットアップ
  wifisetup();  // wifiのセットアップ
                //  webserversetup(); // webserverのセットアップ
  alexasetup(); // alexaのセットアップ
  // speaker_setup(); // speakerのセットアップ

  xTaskCreatePinnedToCore(sensor_task, "SENSOR_TASK", 4096, NULL, 10, &thp[0], APP_CPU_NUM);      // sensorタスク起動
  xTaskCreatePinnedToCore(alexa_task, "ALEXA_TASK", 4096, NULL, 4, &thp[1], APP_CPU_NUM);         // alexaタスク起動
  xTaskCreatePinnedToCore(logServer_task, "LOGSERVER_TASK", 4096, NULL, 3, &thp[2], APP_CPU_NUM); // logServerタスク起動
  xTaskCreatePinnedToCore(udpServer_task, "UDPSERVER_TASK", 4096, NULL, 8, &thp[3], APP_CPU_NUM); // udpサーバータスク起動
  xTaskCreatePinnedToCore(udpSend_task, "UDPSEND_TASK", 4096, NULL, 8, &thp[4], APP_CPU_NUM);     // udp送信タスク起動
  // xTaskCreatePinnedToCore(speaker_task, "SPEAKER_TASK", 4096, NULL, 1, &thp[4], APP_CPU_NUM);     // スピーカータスク起動

  ota_setup(); // otaのセットアップ

  logprintln("<<浴室自動換気システム再起動>>", 1);
}

void loop() // メインCPU(Core1)で実行するプログラム
{
  // wifi接続判定
  wificheck();

  // OTA処理（ArduinoOTA + WEB OTA）
  ota_handle();

  delay(100);

  // char msg_buffer[2048];
  // vTaskList(msg_buffer);

  // Serial.printf("%s\n", msg_buffer);
}

void showChipInfo()
{
  logprintln("// Internal RAM");
  logprintln("total heap size = " + String(ESP.getHeapSize()));
  logprintln("available heap = " + String(ESP.getFreeHeap()));
  logprintln("lowest level of free heap since boot = " + String(ESP.getMinFreeHeap()));
  logprintln("largest block of heap that can be allocated at once = " + String(ESP.getMaxAllocHeap()));
}
