#include <Arduino.h>
#include "pClk_config.h"

#include "pClk_gData.h"
#include "pClk_Core0.h"
#include "pClk_Core1.h"
#include "pClk_Buzzer.h"
#include "pClk_Buzzer_Alarm.h"



portMUX_TYPE g_Spinlock = portMUX_INITIALIZER_UNLOCKED;

sInterData g_sInterData;


//--------------------------------------------------------------------------------
//  Task Buzzer
//--------------------------------------------------------------------------------

void Task_Buzzer(void *pvParameters)
{
    static PClkBuzzer buzzer;
    buzzer.init();

    for (;;)
    {
        // Button beep request
        if (g_sInterData.sCtrl.nBeep_req == 1)
        {
            portENTER_CRITICAL(&g_Spinlock);
            uint8_t nCount = g_sInterData.sCtrl.nBeep_count;
            uint8_t nVol   = g_sInterData.sCtrl.nBeep_vol;
            g_sInterData.sCtrl.nBeep_req = 0;
            portEXIT_CRITICAL(&g_Spinlock);

            buzzer.beep(nCount, nVol);
        }

        // Melody preview (1 cycle)
        else if (g_sInterData.sCtrl.nPreview_req == 1)
        {
            portENTER_CRITICAL(&g_Spinlock);
            uint8_t nIdx = g_sInterData.sCtrl.nPreview_tone;
            uint8_t nVol = g_sInterData.sCtrl.nVol_preview;
            g_sInterData.sCtrl.nPreview_req = 0;
            portEXIT_CRITICAL(&g_Spinlock);

            if (nIdx >= ALARM_MELODY_COUNT) nIdx = 0;
            buzzer.playMelody(ALARM_MELODIES[nIdx].notes, ALARM_MELODIES[nIdx].len, nVol, nullptr);
        }

        // Alarm melody playback
        else if (g_sInterData.sCtrl.nAlarm_pending == 1)
        {
            uint8_t nIdx = g_sInterData.sNvsD.sAlarm.nToneNum;
            if (nIdx >= ALARM_MELODY_COUNT) nIdx = 0;
            uint8_t nVol = (uint8_t)g_sInterData.sNvsD.sAlarm.nVolume;

            buzzer.playMelody(
                ALARM_MELODIES[nIdx].notes,
                ALARM_MELODIES[nIdx].len,
                nVol,
                &g_sInterData.sCtrl.nAlarm_pending
            );
        }

        else
        {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}


//--------------------------------------------------------------------------------
//  Task Button
//--------------------------------------------------------------------------------

void Task_Button(void *pvParameters)
{
    init_btn(&g_sInterData);

    for (;;)
    {
        run_btn();
    }
}


//--------------------------------------------------------------------------------
//  Task Core1
//--------------------------------------------------------------------------------

void Task_Core1(void *pvParameters)
{
    for(;;)
    {
        uint32_t nStart = millis();
        Core1_run();
        uint32_t nElapsed = millis() - nStart;

        if (g_sInterData.eSeq_C1 != CORE1_SETUP)
        {
            if (nElapsed < 500)
            {
                vTaskDelay(pdMS_TO_TICKS(500 - nElapsed));
            }
        }
    }
}


//--------------------------------------------------------------------------------
//  Setup
//--------------------------------------------------------------------------------

void setup()
{
    Serial.begin(PCLK_BAUD_RATE);

    Core0_init(&g_sInterData);
    Core1_init(&g_sInterData);

    // Core 1 main task
    xTaskCreatePinnedToCore(Task_Core1, "Task_Core1", 4096, NULL, 1, NULL, 1);

    // Button task (Core 1)
    xTaskCreatePinnedToCore(Task_Button, "BtnTask", 3072, NULL, 2, NULL, 1);

    // Buzzer task (Core 1)
    xTaskCreatePinnedToCore(Task_Buzzer, "BuzzerTask", 2048, NULL, 1, NULL, 1);
}


//--------------------------------------------------------------------------------
//  Loop
//--------------------------------------------------------------------------------

void loop()
{
    core0_run();
    yield();
}
