#include "common.h"

void udpSend_setup(void)
{
}

void udpSend_task(void *pvParameters)
{
    logprintln("udpSend_task START!!");
    delay(100); // 各タスク起動待ち

    while (1)
    {
        sendDeviceMode();
        delay(1000);
    }

    vTaskDelete(NULL);
}

// 動作モード送信処理
void sendDeviceMode(void)
{
#ifdef DEBUG
#else
    if (WiFi.status() == WL_CONNECTED)
    {
        // 現在の機器状況設定
        send_deviceState.deviceState.lightState = (unsigned char)getLightMode();
        send_deviceState.deviceState.lightForced = 0;
        send_deviceState.deviceState.exhaustfanState = (unsigned char)getExhaustFanMode();
        send_deviceState.deviceState.exhaustfanForced = (unsigned char)manualExhaustfanFlg;
        send_deviceState.deviceState.musicState = (unsigned char)nowMusicFlg;
        send_deviceState.deviceState.musicForced = manualMusicFlg;

        udp.beginPacket(remoteIpadr, rmoteUdpPort);
        udp.write((uint8_t *)&send_deviceState, sizeof(SendRecvData));
        udp.endPacket();
    }
#endif
}
