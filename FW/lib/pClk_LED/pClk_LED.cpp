#include "pClk_LED.h"



static const uint8_t LED_PINS[] = {
    GPIO_LED_5V, GPIO_LED_3V3, GPIO_LED_AP,
    GPIO_LED_STA, GPIO_LED_STATUS, GPIO_LED_TX
    };

static const uint8_t LED_PIN_COUNT = sizeof(LED_PINS) / sizeof(LED_PINS[0]);


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkLED::init()
{
    // LED off (Active LOW)
    for (uint8_t nI = 0; nI < LED_PIN_COUNT; nI++)
    {
        pinMode(LED_PINS[nI], OUTPUT);
        digitalWrite(LED_PINS[nI], HIGH);
    }

    // ADC voltage check (NU)
    set(GPIO_LED_5V,  1);
    set(GPIO_LED_3V3, 1);

    // ADC voltage check
    /*
    float v5 = _readVoltage_5V();

    if (v5 >= ADC_5V_MIN * ADC_5V_CORR && v5 <= ADC_5V_MAX * ADC_5V_CORR)
        set(GPIO_LED_5V, 1);

    float v33 = _readVoltage_3V3();

    if (v33 >= ADC_33V_MIN * ADC_33V_CORR && v33 <= ADC_33V_MAX * ADC_33V_CORR)
        set(GPIO_LED_3V3, 1);
    */
}


//--------------------------------------------------------------------------------
//  Set
//--------------------------------------------------------------------------------

void PClkLED::set(uint8_t pin, uint8_t state)
{
    digitalWrite(pin, state ? LOW : HIGH);
}


//--------------------------------------------------------------------------------
//  All Off
//--------------------------------------------------------------------------------

void PClkLED::allOff()
{
    for (uint8_t nI = 0; nI < LED_PIN_COUNT; nI++)
    {
        digitalWrite(LED_PINS[nI], HIGH);
    }
}


//--------------------------------------------------------------------------------
//  Update
//--------------------------------------------------------------------------------

void PClkLED::update(bool nightMode, bool apOn, bool staOn, bool alarmOn)
{
    if (nightMode)
    {
        allOff();
        return;
    }

    set(GPIO_LED_5V,     1);
    set(GPIO_LED_3V3,    1);
    set(GPIO_LED_AP,     apOn    ? 1 : 0);
    set(GPIO_LED_STA,    staOn   ? 1 : 0);
    set(GPIO_LED_STATUS, alarmOn ? 1 : 0);
}


//--------------------------------------------------------------------------------
//  ADC voltage check
//--------------------------------------------------------------------------------
/*
float PClkLED::_readVoltage_5V()
{
    // Divider 1/2: Vin = ADC * (3.3V / 4095) * 2
    uint32_t raw = analogRead(GPIO_ADC_5V);
    return (float)raw / 4095.0f * 3.3f * 2.0f;
}


float PClkLED::_readVoltage_3V3()
{
    // Divider 2/3: Vin = ADC * (3.3V / 4095) * 1.5
    uint32_t raw = analogRead(GPIO_ADC_3V3);
    return (float)raw / 4095.0f * 3.3f * 1.5f;
}
*/
