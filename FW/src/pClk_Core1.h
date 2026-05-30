#ifndef PCLK_CORE1_H_
#define PCLK_CORE1_H_

#include "pClk_config.h"
#include "pClk_gData.h"

#include "pClk_RGB.h"



void Core1_init( sInterData* psIntreD );
void Core1_run();

void init_btn(sInterData* psShared);
void run_btn();



#endif