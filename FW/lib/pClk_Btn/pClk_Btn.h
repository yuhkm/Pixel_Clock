#ifndef PCLK_BTN_H_
#define PCLK_BTN_H_

#include <Arduino.h>
#include "pClk_gData.h"



class PClkButton 
{
    private:
        uint8_t _nPin;
        sInterData* _psIntreD = nullptr;
        QueueHandle_t _btnQueue = nullptr;
        uint32_t _nPressStartTime = 0;
        uint32_t _nLastIsrTime = 0;

        enum class RawType { PRESS, RELEASE };
        
        struct RawEvt
        {
            RawType  eType;
            uint32_t nTime;
        };

        static void IRAM_ATTR _isr_handler(void* arg);

    public:
        PClkButton(uint8_t pin);
        void init(sInterData* psShared);
        void run();
};



#endif