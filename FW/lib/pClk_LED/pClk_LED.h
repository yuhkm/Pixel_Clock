#ifndef PCLK_LED_H_
#define PCLK_LED_H_

#include <Arduino.h>
#include "pClk_config.h"


class PClkLED
{
    private:
        // float _readVoltage_5V();
        // float _readVoltage_3V3();

    public:
        void init();

        void set(uint8_t pin, uint8_t state);
        void allOff();
        void update(bool nightMode, bool apOn, bool staOn, bool alarmOn);
};


#endif
