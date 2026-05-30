#ifndef PCLK_DSLEEP_H_
#define PCLK_DSLEEP_H_

#include <Arduino.h>
#include <esp_sleep.h>

#include "pClk_config.h"
#include "pClk_gData.h"
#include "pClk_RTC.h"
#include "pClk_RGB.h"


// Handles wakeup cause on boot
void DSleep_onBoot(PClkRTC* rtc);

// Checks deep sleep entry condition
void DSleep_check(sInterData* psInterD, PClkRTC* rtc, PClkRGB* rgb, uint8_t cur_HH, uint8_t cur_mm);


#endif
