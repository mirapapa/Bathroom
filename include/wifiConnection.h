#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include "common.h"

int wifisetup();
void wificheck();
void mdnssetup();
bool isWiFiReallyConnected();

#endif // WIFI_CONNECTION_H
