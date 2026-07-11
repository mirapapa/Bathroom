#ifndef NTP_H
#define NTP_H

#include "common.h"

void ntp_setup();
void timeavailable(struct timeval *t);

extern bool firstTimeNtpFlg;

#endif // NTP_H
