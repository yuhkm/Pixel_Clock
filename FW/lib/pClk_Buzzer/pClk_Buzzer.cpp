#include "pClk_Buzzer.h"


#define BEEP_FREQ           1000    // beep frequency (Hz)
#define BEEP_ON_MS          120    // beep on duration (ms)
#define BEEP_OFF_MS         80    // beep off interval (ms)


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkBuzzer::init()
{
    ledcSetup(BUZZER_PWM_CH, BEEP_FREQ, 8);
    ledcAttachPin(GPIO_BUZZER, BUZZER_PWM_CH);
    _silence();
}


//--------------------------------------------------------------------------------
//  Tone
//--------------------------------------------------------------------------------

void PClkBuzzer::_tone(uint16_t freq, uint8_t volume)
{
    if (freq == 0 || volume == 0)
    {
        _silence();
        return;
    }

    ledcSetup(BUZZER_PWM_CH, freq, 8);

    // 0~100% → 0~128 (50% duty = max volume)
    uint8_t nDuty = (uint8_t)((volume * 128UL) / 100);

    ledcWrite(BUZZER_PWM_CH, nDuty);
}


//--------------------------------------------------------------------------------
//  Silence
//--------------------------------------------------------------------------------

void PClkBuzzer::_silence()
{
    ledcWrite(BUZZER_PWM_CH, 0);
}


//--------------------------------------------------------------------------------
//  Beep
//--------------------------------------------------------------------------------

void PClkBuzzer::beep(uint8_t count, uint8_t volume)
{
    for (uint8_t nI = 0; nI < count; nI++)
    {
        _tone(BEEP_FREQ, volume);
        vTaskDelay(pdMS_TO_TICKS(BEEP_ON_MS));
        _silence();

        if (nI < count - 1)
        {
            vTaskDelay(pdMS_TO_TICKS(BEEP_OFF_MS));
        }
    }
}


//--------------------------------------------------------------------------------
//  Play Melody
//--------------------------------------------------------------------------------

void PClkBuzzer::playMelody(const sNote* melody, uint16_t len, uint8_t volume, volatile uint8_t* stopFlag)
{
    for (uint16_t nI = 0; nI < len; nI++)
    {
        // Check stop flag before each note.
        if (stopFlag && *stopFlag == 0) break;

        if (melody[nI].freq == 0)
            _silence();
        else
            _tone(melody[nI].freq, volume);

        vTaskDelay(pdMS_TO_TICKS(melody[nI].ms));
    }
    
    _silence();
}


//--------------------------------------------------------------------------------
//  Stop
//--------------------------------------------------------------------------------

void PClkBuzzer::stop()
{
    _silence();
}
