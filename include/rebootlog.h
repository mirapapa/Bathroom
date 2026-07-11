#ifndef REBOOTLOG_H
#define REBOOTLOG_H

#include "common.h"

void rebootLog_setup();
void loadRebootLog();
void saveRebootLog();
void addRebootRecord(esp_reset_reason_t reason, const char *message);
String getRebootLogJson();
String getRebootLogHtml();
void clearRebootLog();
String getRebootReasonString(esp_reset_reason_t reason);
esp_reset_reason_t getCurrentRebootReason();

extern RebootLog rebootLog;
extern Preferences preferences;

#endif // REBOOTLOG_H
