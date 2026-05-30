#include "pClk_Core1.h"
#include "pClk_config.h"
#include "pClk_gData.h"

#include "pClk_RTC.h"
#include "pClk_RGB.h"
#include "pClk_Illu.h"
#include "pClk_Btn.h"
#include "pClk_DSleep.h"



static sInterData* _psIntreD = nullptr;

static PClkRTC rtc;
static PClkRGB rgb_mat (GPIO_RGB_MAT, RGB_CNT_MAT);
static PClkIllu illu(GPIO_ILLU_0);
static PClkButton btn(GPIO_BTN_0);

static bool bNvsApplied;
static bool bStaRun_done;
static bool bManualBoot;
static bool bTickState;
static bool bWeatherMode;

static uint8_t nRTC_HH12_cur;
static uint8_t nRTC_HH24_cur;
static uint8_t nRTC_mm_cur;
static uint8_t nRTC_mm_prev;
static uint8_t nRTC_SS_cur;
static uint8_t nRTC_WDAY_cur;

static bool     bNightMode;
static uint32_t nLastAdcMs;
static uint32_t nClockColor;
static uint8_t  nBrightness;
static uint8_t  nAlarm_fired_mm;
static uint32_t nAlarmStartMs;


//--------------------------------------------------------------------------------
//  Core1 Init
//--------------------------------------------------------------------------------

void Core1_init( sInterData* psIntreD )
{
    _psIntreD = psIntreD;
    
    nRTC_HH24_cur  = 99;
    nRTC_mm_cur    = 99;
    nRTC_mm_prev   = 99;
    nRTC_SS_cur    = 99;
    nRTC_WDAY_cur  = 0;

    bNvsApplied = false;
    bStaRun_done = false;
    bManualBoot = true;
    bTickState = true;
    bWeatherMode = false;
    bNightMode      = false;
    nLastAdcMs       = 0;
    nAlarm_fired_mm = 255;
    nAlarmStartMs   = 0;

    if (!rtc.init(GPIO_RTC_SDA, GPIO_RTC_SCL))
        Serial.println(">>> [RTC][I2C] Init failed.");
    DSleep_onBoot(&rtc);

    rgb_mat.init();
    nClockColor = Adafruit_NeoPixel::Color(255, 255, 255);
    nBrightness = 64;

    illu.init();
}


//--------------------------------------------------------------------------------
//  Core1 Run
//--------------------------------------------------------------------------------

void Core1_run()
{
    static eSeq_Core1 ePrevSeq_C1   = CORE1_STOP;
    static bool       bScrollDone   = false;
    static int16_t    nScrollOffset = 0;
    static uint8_t    nScrollTick   = 0;

    switch ( _psIntreD->eSeq_C1 )
    {
        case CORE1_BOOTING:
        
            bManualBoot = true;

            break;

        case CORE1_RUN:

            if (!bNvsApplied)
            {
                if (_psIntreD->sNvsD.sRgb.nColor_RGB0_hex != 0)
                    nClockColor  = _psIntreD->sNvsD.sRgb.nColor_RGB0_hex;
                if (_psIntreD->sNvsD.sRgb.nLumi_RGB0_day != 0)
                    nBrightness = _psIntreD->sNvsD.sRgb.nLumi_RGB0_day;
                bNvsApplied = true;
                Serial.println(">>> [Core1][NVS] Settings applied.");
            }

            uint8_t nNewHH, nNewMM, nNewSS, nNewWDAY;
            if (rtc.readTime(&nNewHH, &nNewMM, &nNewSS, &nNewWDAY))
            {
                nRTC_HH24_cur = nNewHH;
                nRTC_mm_cur   = nNewMM;
                nRTC_SS_cur   = nNewSS;
                nRTC_WDAY_cur = nNewWDAY;
                nRTC_HH12_cur = nRTC_HH24_cur % 12;
                if (nRTC_HH12_cur == 0) nRTC_HH12_cur = 12;
            }

            bTickState = (nRTC_SS_cur % 2 == 0);

            // STA active
            if(((bManualBoot)|| (nRTC_mm_cur == (uint8_t)STA_ACTIVE_mm)) 
                && (_psIntreD->sRtcSt.nRunSta_ack == 0) && (bStaRun_done == false))
            {
                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->sRtcSt.nRunSta_HH24 = nRTC_HH24_cur;
                _psIntreD->sRtcSt.nRunSta_mm = nRTC_mm_cur;
                _psIntreD->sRtcSt.nRunSta_req = 1;
                portEXIT_CRITICAL(&g_Spinlock);
                
                if(bManualBoot) bManualBoot = false;
            }
            else if((_psIntreD->sRtcSt.nRunSta_req == 1) && (_psIntreD->sRtcSt.nRunSta_ack == 1))
            {
                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->sRtcSt.nRunSta_req = 0;
                _psIntreD->sRtcSt.nRunSta_ack = 0;
                portEXIT_CRITICAL(&g_Spinlock);

                bStaRun_done = true;
            }

            if(nRTC_mm_cur != (uint8_t)STA_ACTIVE_mm)   
            {
                bStaRun_done = false;
            }


            // Update NTP
            if((_psIntreD->sRtcSt.nGetNtp_req == 1) && (_psIntreD->sRtcSt.nGetNtp_ack == 0))
            {
                rtc.set_NTP(_psIntreD->sRtcSt.tGetNtp_data);
                Serial.println(">>> [Core1][RTC] NTP synced.");

                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->sRtcSt.nGetNtp_ack = 1;
                portEXIT_CRITICAL(&g_Spinlock);
            }

            // Deep Sleep check
            DSleep_check(_psIntreD, &rtc, &rgb_mat, nRTC_HH24_cur, nRTC_mm_cur);

            // Alarm check
            if (_psIntreD->sNvsD.sAlarm.nEnable == 1)
            {
                uint8_t nWday = nRTC_WDAY_cur;   // 0=SUN, 6=SAT
                bool isAlarmDay = (_psIntreD->sNvsD.sAlarm.nD & (0x40 >> nWday)) != 0;

                if (isAlarmDay
                    && nRTC_HH24_cur == _psIntreD->sNvsD.sAlarm.nHH
                    && nRTC_mm_cur   == _psIntreD->sNvsD.sAlarm.nMM)
                {
                    if (nAlarm_fired_mm != nRTC_mm_cur)
                    {
                        portENTER_CRITICAL(&g_Spinlock);
                        _psIntreD->sCtrl.nAlarm_pending = 1;
                        portEXIT_CRITICAL(&g_Spinlock);

                        nAlarm_fired_mm = nRTC_mm_cur;
                        nAlarmStartMs   = millis();
                        Serial.println(">>> [Core1][ALARM] Fired.");
                    }
                }
                else
                {
                    nAlarm_fired_mm = 255;  // Reset re-fire guard outside alarm time
                }
            }

            // Alarm auto-stop
            if (_psIntreD->sCtrl.nAlarm_pending == 1 &&
                nAlarmStartMs != 0 &&
                millis() - nAlarmStartMs >= ALARM_AUTO_STOP_MS)
            {
                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->sCtrl.nAlarm_pending = 0;
                portEXIT_CRITICAL(&g_Spinlock);
                Serial.println(">>> [Core1][ALARM] Auto-stopped.");
            }

            // Illuminance ADC
            if (millis() - nLastAdcMs >= ADC_SAMPLE_PERIOD_MS)
            {
                nLastAdcMs = millis();

                uint16_t nCalDay   = _psIntreD->sNvsD.sRgb.nCalib_Illu0_day;
                uint16_t nCalNight = _psIntreD->sNvsD.sRgb.nCalib_Illu0_night;

                if (nCalDay > nCalNight)
                {
                    uint16_t nRange = nCalDay - nCalNight;
                    uint16_t nUpper = nCalNight + (nRange * 7) / 10;
                    uint16_t nLower = nCalNight + (nRange * 2) / 10;

                    uint16_t nIlluVal = (uint16_t)illu.get();

                    if (!bNightMode && nIlluVal < nLower)
                    {
                        bNightMode   = true;
                        nBrightness  = _psIntreD->sNvsD.sRgb.nLumi_RGB0_night;
                        nRTC_mm_prev = 99;

                        portENTER_CRITICAL(&g_Spinlock);
                        _psIntreD->sCtrl.nNightMode = 1;
                        portEXIT_CRITICAL(&g_Spinlock);
                        Serial.println(">>> [Core1][ILLU] Night mode ON.");
                    }
                    else if (bNightMode && nIlluVal > nUpper)
                    {
                        bNightMode   = false;
                        nBrightness  = _psIntreD->sNvsD.sRgb.nLumi_RGB0_day;
                        nRTC_mm_prev = 99;

                        portENTER_CRITICAL(&g_Spinlock);
                        _psIntreD->sCtrl.nNightMode = 0;
                        portEXIT_CRITICAL(&g_Spinlock);
                        Serial.println(">>> [Core1][ILLU] Night mode OFF.");
                    }
                }
            }

            // RGB Matrix
            if(_psIntreD->sNvsD.sWeather.nEnable == 1 && _psIntreD->sWeaData.nValid == 1)
            {
                sWeatherData sWeaSnap;
                portENTER_CRITICAL(&g_Spinlock);
                memcpy(&sWeaSnap, (const void*)&_psIntreD->sWeaData, sizeof(sWeatherData));
                portEXIT_CRITICAL(&g_Spinlock);

                uint16_t nSwitchStartSec = 60 - _psIntreD->sNvsD.sWeather.nSwitchCycle;
                bool bCurrentModeIsWeather = (nRTC_SS_cur >= nSwitchStartSec);

                if (bWeatherMode != bCurrentModeIsWeather)
                {
                    for (int16_t nOffset = 0; nOffset <= 16; nOffset++)
                    {
                        rgb_mat.renderStart(nBrightness);
                        if (bCurrentModeIsWeather)
                        {
                            rgb_mat.drawClock(nRTC_HH12_cur, nRTC_mm_cur, bTickState, nClockColor, nOffset);
                            rgb_mat.drawWeather(sWeaSnap.nTemp, sWeaSnap.eSky, sWeaSnap.ePty, nClockColor, nOffset - 16);
                        }
                        else
                        {
                            rgb_mat.drawWeather(sWeaSnap.nTemp, sWeaSnap.eSky, sWeaSnap.ePty, nClockColor, nOffset);
                            rgb_mat.drawClock(nRTC_HH12_cur, nRTC_mm_cur, bTickState, nClockColor, nOffset - 16);
                        }
                        rgb_mat.renderEnd();
                        vTaskDelay(pdMS_TO_TICKS(30));
                    }
                    bWeatherMode = bCurrentModeIsWeather;
                }

                rgb_mat.renderStart(nBrightness);

                if (bWeatherMode)
                {
                    rgb_mat.drawWeather(sWeaSnap.nTemp, sWeaSnap.eSky, sWeaSnap.ePty, nClockColor, 0);
                }
                else
                {
                    rgb_mat.drawClock(nRTC_HH12_cur, nRTC_mm_cur, bTickState, nClockColor, 0);
                }

                rgb_mat.renderEnd();
            }
            else
            {
                if( nRTC_mm_cur != nRTC_mm_prev )
                {
                    rgb_mat.renderStart(nBrightness);
                    rgb_mat.drawClock(nRTC_HH12_cur, nRTC_mm_cur, bTickState, nClockColor, 0);
                    rgb_mat.renderEnd();
                } 
                else
                {
                    rgb_mat.updateColon(nRTC_HH12_cur, bTickState, nClockColor, nBrightness);
                }
            }
            nRTC_mm_prev = nRTC_mm_cur;

            break;

        case CORE1_STOP:
            vTaskDelay(pdMS_TO_TICKS(100));
            break;

        case CORE1_SETUP:

            if (ePrevSeq_C1 != CORE1_SETUP)
            {
                bScrollDone   = false;
                nScrollOffset = 0;
                nScrollTick   = 0;
            }

            if (_psIntreD->sCtrl.nUpdated)
            {
                portENTER_CRITICAL(&g_Spinlock);
                uint8_t  nUpdated      = _psIntreD->sCtrl.nUpdated;
                uint32_t nNewColor     = _psIntreD->sCtrl.nColor_hex;
                uint8_t  nNewLumiDay   = _psIntreD->sCtrl.nLumi_day;
                uint8_t  nNewLumiNight = _psIntreD->sCtrl.nLumi_night;
                _psIntreD->sCtrl.nUpdated = 0;
                portEXIT_CRITICAL(&g_Spinlock);

                if (nUpdated & 0x01) nClockColor  = nNewColor;
                if (nUpdated & 0x02) nBrightness = nNewLumiDay;
                if (nUpdated & 0x04) nBrightness = nNewLumiNight;

                bScrollDone = true;  // First update received — stop scrolling

                uint8_t nHh = rtc.get_RTC_HH_24() % 12;
                if (nHh == 0) nHh = 12;
                uint8_t nMm = rtc.get_RTC_mm();

                rgb_mat.renderStart(nBrightness);
                rgb_mat.drawClock(nHh, nMm, bTickState, nClockColor, 0);
                rgb_mat.renderEnd();
            }
            else if (!bScrollDone)
            {
                // setup.local
                rgb_mat.renderStart(nBrightness);
                rgb_mat.drawScrollText(SETUP_SCROLL_TEXT, SETUP_SCROLL_TEXT_WIDTH, nScrollOffset, nClockColor);
                rgb_mat.renderEnd();

                if (++nScrollTick >= 4)
                {
                    nScrollTick = 0;
                    nScrollOffset++;
                    if (nScrollOffset >= SETUP_SCROLL_TEXT_WIDTH) nScrollOffset = -16;
                }
            }

            if (_psIntreD->sCtrl.nCalib_req)
            {
                portENTER_CRITICAL(&g_Spinlock);
                uint8_t nReq = _psIntreD->sCtrl.nCalib_req;
                _psIntreD->sCtrl.nCalib_req = 0;
                portEXIT_CRITICAL(&g_Spinlock);

                uint16_t nVal = (uint16_t)illu.get();

                portENTER_CRITICAL(&g_Spinlock);
                if (nReq == 1)
                    _psIntreD->sNvsD.sRgb.nCalib_Illu0_day   = nVal;
                else
                    _psIntreD->sNvsD.sRgb.nCalib_Illu0_night = nVal;
                _psIntreD->sCtrl.nCalib_done = 1;
                portEXIT_CRITICAL(&g_Spinlock);
            }

            if (_psIntreD->sCtrl.nVol_play_req)
            {
                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->sCtrl.nVol_play_req = 0;
                _psIntreD->sCtrl.nPreview_req  = 1;
                portEXIT_CRITICAL(&g_Spinlock);
            }

            vTaskDelay(pdMS_TO_TICKS(50));
            break;

        case CORE1_ERROR:
            vTaskDelay(pdMS_TO_TICKS(100));
            break;

        default:
            break;
    }

    ePrevSeq_C1 = _psIntreD->eSeq_C1;
}


//--------------------------------------------------------------------------------
//  Button Wrapper
//--------------------------------------------------------------------------------

void init_btn(sInterData* psShared)
{
    btn.init(psShared);
}


void run_btn()
{
    btn.run();
}
