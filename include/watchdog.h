#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "common.h"

void watchdog_setup();
void watchdog_subscribe_task(const char *taskName);
void watchdog_reset();
void watchdog_unsubscribe_task(const char *taskName);

#endif // WATCHDOG_H
