#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "common.h"

void sensor_setup();
void sensor_task(void *pvParameters);
void saveConfig();
bool getLightMode();
bool getExhaustFanMode();
void manualExhaustfan();
void manualMusic();

extern bool manualExhaustfanFlg;
extern bool manualMusicFlg;
extern bool exhaustfanState;
extern SendRecvData recv_deviceState;
extern SendRecvData send_deviceState;
extern uint16_t judgeOnTime;
extern uint16_t startTime;
extern uint16_t continueTime;

#endif // SENSOR_TASK_H
