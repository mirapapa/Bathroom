#include "common.h"

WiFiUDP udp;
char packetBuffer[256];
static const int localPort = UDP_LOCAL_PORT;
const int rmoteUdpPort = UDP_REMOTE_PORT;
const char *remoteIpadr = REMOTE_IP_ADDR;

void udpServer_setup(void)
{
}

void udpServer_task(void *pvParameters)
{
  logprintln("udpServer_task START!!");
  delay(100); // 各タスク起動待ち

  while (1)
  {
    int i;
    if (WiFi.status() == WL_CONNECTED)
    {
      udp.begin(localPort);
      break;
    }
    else
    {
      delay(100);
      continue;
    }
  }

  while (1)
  {
    int packetSize = udp.parsePacket();
    if (packetSize)
    {
      memset(packetBuffer, 256, 0x00);
      udp.read(packetBuffer, 256);
      memcpy(&recv_deviceState, &packetBuffer[0], sizeof(SendRecvData));

      manualExhaustfanFlg = (bool)recv_deviceState.deviceState.exhaustfanForced;
      manualExhaustfan();
      manualMusicFlg = (bool)recv_deviceState.deviceState.musicForced;
      manualMusic();

      sendDeviceMode();
    }
    delay(100);
  }

  vTaskDelete(NULL);
}
