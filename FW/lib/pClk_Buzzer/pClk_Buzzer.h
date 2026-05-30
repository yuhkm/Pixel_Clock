#ifndef PCLK_BUZZER_H_
#define PCLK_BUZZER_H_

#include <Arduino.h>
#include "pClk_config.h"


//--------------------------------------------------------------------------------
//  Note structure
//--------------------------------------------------------------------------------

struct sNote
{
    uint16_t freq;   // Hz (0 = silence)
    uint16_t ms;     // duration (ms)
};


//--------------------------------------------------------------------------------
//  PClkBuzzer
//--------------------------------------------------------------------------------

class PClkBuzzer
{
    private:
        void _tone(uint16_t freq, uint8_t volume);
        void _silence();
        
    public:
        void init();

        void beep(uint8_t count, uint8_t volume = BUZZER_BEEP_VOL);
        void playMelody(const sNote* melody, uint16_t len, uint8_t volume, volatile uint8_t* stopFlag = nullptr);
        void stop();
};


#endif
