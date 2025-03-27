#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
#include <Adafruit_SleepyDog.h>


void watchdog_initialize(int wd_countdown_ms);


void watchdog_reset(void);

void watchdog_disable(void);

#endif