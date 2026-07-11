#ifndef UDP_SYNC_H
#define UDP_SYNC_H

#include "common.h"

// 他機器とのUDP状態同期（受信）
void udpServer_setup(void);
void udpServer_task(void *pvParameters);

// 他機器とのUDP状態同期（送信）
void udpSend_task(void *pvParameters);
void sendDeviceMode(void);

extern WiFiUDP udp;
extern const int rmoteUdpPort;
extern const char *remoteIpadr;

#endif // UDP_SYNC_H
