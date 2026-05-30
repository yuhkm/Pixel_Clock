#include "pClk_DSleep.h"
#include "pClk_LED.h"



//--------------------------------------------------------------------------------
//  All LED Off
//--------------------------------------------------------------------------------

// Turn off all LEDs — Active LOW, so HIGH = OFF
static void _allLedOff()
{
    const uint8_t pins[] = {
        GPIO_LED_5V, GPIO_LED_3V3, GPIO_LED_AP,
        GPIO_LED_STA, GPIO_LED_STATUS, GPIO_LED_TX
    };

    for (uint8_t nI = 0; nI < sizeof(pins); nI++)
    {
        digitalWrite(pins[nI], HIGH);
    }
}


//--------------------------------------------------------------------------------
//  Helper
//--------------------------------------------------------------------------------

static bool _isInSleepPeriod(int16_t timeMin, int16_t startMin, int16_t endMin)
{
    // No midnight crossing
    if (startMin < endMin)
    {
        return (timeMin > startMin && timeMin < endMin);
    }
    // Midnight crossing
    else
    {
        return (timeMin > startMin || timeMin < endMin);
    }
}


//--------------------------------------------------------------------------------
//  DSleep_onBoot
//--------------------------------------------------------------------------------

void DSleep_onBoot(PClkRTC* rtc)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    // PCF85063A INT
    if (cause == ESP_SLEEP_WAKEUP_EXT0)
    {
        rtc->clear_Int();
    }
    // Button force-wake
    else if (cause == ESP_SLEEP_WAKEUP_EXT1)
    {
        rtc->clear_Int();
        rtc->disable_Int();
    }
}


//--------------------------------------------------------------------------------
//  DSleep_check
//--------------------------------------------------------------------------------

void DSleep_check(sInterData* psInterD, PClkRTC* rtc, PClkRGB* rgb, uint8_t cur_HH, uint8_t cur_mm)
{
    // Deep sleep disabled
    if (psInterD->sNvsD.sRgb.nEnable_DSleep != 1) return;

    uint8_t nStrHH = psInterD->sNvsD.sRgb.nDSleep_str_HH;
    uint8_t nStrMM = psInterD->sNvsD.sRgb.nDSleep_str_MM;
    uint8_t nEndHH = psInterD->sNvsD.sRgb.nDSleep_end_HH;
    uint8_t nEndMM = psInterD->sNvsD.sRgb.nDSleep_end_MM;

    // Invalid if start 
    if (nStrHH == nEndHH && nStrMM == nEndMM) return;

    // Check if current time matches sleep start time
    if (cur_HH != nStrHH || cur_mm != nStrMM) return;

    int16_t nSleepStartMin = (int16_t)(nStrHH * 60 + nStrMM);
    int16_t nSleepEndMin   = (int16_t)(nEndHH * 60 + nEndMM);
    int16_t nWakeMin       = nSleepEndMin;

    // Check if alarm falls within the sleep window
    if (psInterD->sNvsD.sAlarm.nEnable == 1)
    {
        // Check if today is an alarm day
        uint8_t nWday = (uint8_t)rtc->get_RTC_WDAY();
        bool isAlarmDay = (psInterD->sNvsD.sAlarm.nD & (0x40 >> nWday)) != 0;

        if (isAlarmDay)
        {
            int16_t nAlarmMin = (int16_t)(psInterD->sNvsD.sAlarm.nHH * 60 + psInterD->sNvsD.sAlarm.nMM);

            if (_isInSleepPeriod(nAlarmMin, nSleepStartMin, nSleepEndMin))
            {
                // Calculate pre-wake time
                int16_t nPreWakeMin = nAlarmMin - DSLEEP_PREWAKE_MIN;
                if (nPreWakeMin < 0) nPreWakeMin += 1440;

                // Check if pre-wake time is within the sleep window
                if (!_isInSleepPeriod(nPreWakeMin, nSleepStartMin, nSleepEndMin))
                {
                    return;
                }

                nWakeMin = nPreWakeMin;
            }
        }
    }

    // Set PCF85063A alarm (seconds ignored: sec=99)
    rtc->set_Int((int)(nWakeMin / 60), (int)(nWakeMin % 60), 99);

    // Clear display and turn off all LEDs
    rgb->clear();
    _allLedOff();

    // Register wakeup sources
    esp_sleep_enable_ext0_wakeup((gpio_num_t)GPIO_RTC_INT, 0);                 // PCF85063A INT (active LOW)
    esp_sleep_enable_ext1_wakeup(1ULL << GPIO_BTN_0, ESP_EXT1_WAKEUP_ANY_LOW); // Button

    // Enter deep sleep (no return)
    Serial.printf(">>> [DSleep][RTC] Entering sleep. Wake at %02d:%02d.\n", nWakeMin / 60, nWakeMin % 60);
    Serial.flush();
    esp_deep_sleep_start();
}
