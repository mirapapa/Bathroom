#ifndef ALEXA_TASK_H
#define ALEXA_TASK_H

#include "common.h"

void alexasetup();
void alexa_task(void *pvParameters);
void alexaChangeReport(bool detected);

extern bool nowMusicFlg;

#endif // ALEXA_TASK_H
