#ifndef PCLK_ILLU_H_
#define PCLK_ILLU_H_

#include <Arduino.h>



class PClkIllu
{
    private:
        int _nPin;

        // 12-bit ADC
        const int _nAdcResolution = 4095;

        // ESP32 reference voltage
        const float _fVRef = 3.3;          

    public:

        PClkIllu(int pin);
        ~PClkIllu();

        void init();
        int get();
};



#endif