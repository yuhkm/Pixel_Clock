#ifndef PCLK_CORE0_H_
#define PCLK_CORE0_H_

#include "pClk_config.h"
#include "pClk_gData.h"

#include "pClk_NVS.h"
#include "pClk_WiFi.h"
#include "pClk_WebServer.h"
#include "pClk_API.h"
#include "pClk_LED.h"



void Core0_init( sInterData* psIntreD );

void core0_run();



#endif