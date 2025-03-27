#include "main.h"
#include "watchdog.h"
//#include <wdt_samd21.h>


void watchdog_initialize(int wd_countdown_ms)
{
  int countdown_ms = Watchdog.enable(wd_countdown_ms);
  Serial.print("Enabled the watchdog with max countdown of ");
  Serial.print(countdown_ms, DEC);
  Serial.println(" milliseconds!");
    
}


void watchdog_reset(void)
{  
    Watchdog.reset();     
}

void watchdog_disable(void)
{  
    Watchdog.disable();     
}
