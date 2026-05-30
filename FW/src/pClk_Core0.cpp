#include "pClk_Core0.h"
#include <esp_sntp.h>



static PClkNVS nvs;
static PClkWiFi wifi;
static PClkAPI api;
static PClkLED led;

static sNvsData _sNvsD_temp;

static PClkWebServer webServer(&wifi, &_sNvsD_temp);

static sInterData* _psIntreD = nullptr;

static bool     bWebServerStarted = false;
static bool     bNtpInitDone      = false;
static uint32_t nNtpStartMs       = 0;


//--------------------------------------------------------------------------------
//  Btn Handshaking
//--------------------------------------------------------------------------------

void _Btn_handshaking()
{
    if ((_psIntreD->sBtnst.nIsChanged == 1) && (_psIntreD->sBtnst.nBusy_core0 == 0))
    {
        // Single click (stop alarm)
        if ((_psIntreD->sBtnst.nClick_req == 1) && (_psIntreD->sBtnst.nCick_ack == 0))
        {
            portENTER_CRITICAL(&g_Spinlock);
            _psIntreD->sBtnst.nBusy_core0 = 1;
            portEXIT_CRITICAL(&g_Spinlock);

            // Clear alarm
            portENTER_CRITICAL(&g_Spinlock);
            if (_psIntreD->sCtrl.nAlarm_pending == 1)
                _psIntreD->sCtrl.nAlarm_pending = 0;
            portEXIT_CRITICAL(&g_Spinlock);

            portENTER_CRITICAL(&g_Spinlock);
            _psIntreD->sBtnst.nCick_ack    = 1;
            _psIntreD->sBtnst.nBusy_core0  = 0;
            portEXIT_CRITICAL(&g_Spinlock);
        }

        // Enter setup mode (3s hold, 2 beeps)
        else if ((_psIntreD->sBtnst.nHold_3s_req == 1) && (_psIntreD->sBtnst.nHold_3s_ack == 0))
        {
            portENTER_CRITICAL(&g_Spinlock);
            _psIntreD->sBtnst.nBusy_core0 = 1;

            _psIntreD->eSeq_C0 = CORE0_SETUP;
            _psIntreD->eSeq_C1 = CORE1_SETUP;

            // stop alarm
            _psIntreD->sCtrl.nAlarm_pending = 0;

            _psIntreD->sCtrl.nBeep_count = 2;
            _psIntreD->sCtrl.nBeep_vol   = BUZZER_BEEP_VOL;
            _psIntreD->sCtrl.nBeep_req   = 1;

            _psIntreD->sBtnst.nHold_3s_ack = 1;
            _psIntreD->sBtnst.nBusy_core0  = 0;
            portEXIT_CRITICAL(&g_Spinlock);

            bWebServerStarted = false;
        }

        // Factory reset (10s hold)
        else if ((_psIntreD->sBtnst.nHold_10s_req == 1) && (_psIntreD->sBtnst.nHold_10s_ack == 0))
        {
            nvs.clearMem();
            Serial.println(">>> [Core0][RESET] Factory reset complete. Rebooting...");
            Serial.flush();
            ESP.restart();
        }
    }
}


//--------------------------------------------------------------------------------
//  Core0 Init
//--------------------------------------------------------------------------------

void Core0_init(sInterData* psIntreD)
{
    _psIntreD = psIntreD;

    memset((void*)_psIntreD, 0, sizeof(sInterData));
    _psIntreD->eSeq_C0 = CORE0_BOOTING;
    _psIntreD->eSeq_C1 = CORE1_STOP;

    bWebServerStarted = false;

    led.init();
    nvs.init();
    wifi.init();
    api.init(_psIntreD);
    webServer.init();
    webServer.setCtrlPtr(&_psIntreD->sCtrl);
}


//--------------------------------------------------------------------------------
//  Core0 Run
//--------------------------------------------------------------------------------

void core0_run()
{
    wifi.update();

    led.update(
        _psIntreD->sCtrl.nNightMode,
        wifi.getApState() == AP_RUNNING,
        wifi.getStaState() == STA_CONNECTING || wifi.getStaState() == STA_CONNECTED,
        _psIntreD->sNvsD.sAlarm.nEnable
    );

    _Btn_handshaking();


    switch (_psIntreD->eSeq_C0)
    {
        case CORE0_BOOTING:

            if (nvs.open())
            {
                bool bHasData = nvs.load(&_sNvsD_temp);
                nvs.close();

                if (bHasData && nvs.isConfigValid(&_sNvsD_temp))
                {
                    Serial.println(">>> [Core0][BOOT] Config valid. Moving to IDLE.");

                    portENTER_CRITICAL(&g_Spinlock);
                    memcpy((void*)&_psIntreD->sNvsD, &_sNvsD_temp, sizeof(sNvsData));
                    _psIntreD->eSeq_C0 = CORE0_IDLE;
                    _psIntreD->eSeq_C1 = CORE1_BOOTING;
                    portEXIT_CRITICAL(&g_Spinlock);
                }
                else
                {
                    Serial.println(">>> [Core0][BOOT] Config invalid. Moving to SETUP.");

                    portENTER_CRITICAL(&g_Spinlock);
                    _psIntreD->eSeq_C0 = CORE0_SETUP;
                    _psIntreD->eSeq_C1 = CORE1_SETUP;
                    portEXIT_CRITICAL(&g_Spinlock);

                    bWebServerStarted = false;
                }
            }

            break;

        case CORE0_IDLE:

            if ((_psIntreD->sRtcSt.nRunSta_req == 1) && (_psIntreD->sRtcSt.nRunSta_ack == 0))
            {
                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->eSeq_C0 = CORE0_STA;
                _psIntreD->sRtcSt.nRunSta_ack = 1;
                portEXIT_CRITICAL(&g_Spinlock);
            }

            if (_psIntreD->eSeq_C1 == CORE1_BOOTING)
            {
                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->eSeq_C1 = CORE1_RUN;
                portEXIT_CRITICAL(&g_Spinlock);
            }

            break;

        case CORE0_STA:

            if (wifi.getStaState() == STA_OFF)
            {
                Serial.println(">>> [WiFi][STA] Connecting...");

                const char* pcRunSSID = (const char*)_psIntreD->sNvsD.sWifi.cSsid;
                const char* pcRunPW   = (const char*)_psIntreD->sNvsD.sWifi.cPassword;

                wifi.runSta(pcRunSSID, pcRunPW);
            }

            // WiFi connection failed
            if (wifi.getStaState() == STA_FAIL)
            {
                Serial.println(">>> [WiFi][STA] Failed. Returning to IDLE.");
                wifi.stopSta();

                portENTER_CRITICAL(&g_Spinlock);
                _psIntreD->eSeq_C0 = CORE0_IDLE;
                portEXIT_CRITICAL(&g_Spinlock);

                bNtpInitDone = false;
                nNtpStartMs  = 0;
            }

            // WiFi connected — handle NTP
            if (wifi.getStaState() == STA_CONNECTED)
            {
                if (!bNtpInitDone)
                {
                    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
                    configTime(32400, 0, "pool.ntp.org", "time.google.com");
                    bNtpInitDone = true;
                    nNtpStartMs  = millis();
                    Serial.println(">>> [NTP][NTP] Service initialized.");
                }

                struct tm sTmInfo;

                if (_psIntreD->sRtcSt.nGetNtp_req == 0 &&
                    sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED &&
                    getLocalTime(&sTmInfo))
                {
                    Serial.println(">>> [NTP][NTP] Data received.");

                    time_t ntpRaw = mktime(&sTmInfo);

                    // Update RTC
                    portENTER_CRITICAL(&g_Spinlock);
                    _psIntreD->sRtcSt.tGetNtp_data = ntpRaw;
                    _psIntreD->sRtcSt.nGetNtp_req  = 1;
                    portEXIT_CRITICAL(&g_Spinlock);

                    bool bWeatherOk = false;
                    for (int i = 0; i < DSLEEP_WEATHER_RETRY; i++)
                    {
                        bWeatherOk = api.get_weather(sTmInfo.tm_hour, &sTmInfo);
                        if (bWeatherOk) break;
                        delay(2000);
                    }
                    if (bWeatherOk) Serial.println(">>> [Weather][API] Updated.");
                }
                else if (millis() - nNtpStartMs > 30000UL && _psIntreD->sRtcSt.nGetNtp_req == 0)
                {
                    Serial.println(">>> [NTP][NTP] Timeout. Returning to IDLE.");
                    wifi.stopSta();

                    portENTER_CRITICAL(&g_Spinlock);
                    _psIntreD->eSeq_C0 = CORE0_IDLE;
                    portEXIT_CRITICAL(&g_Spinlock);

                    bNtpInitDone = false;
                    nNtpStartMs  = 0;
                }

                // NTP ack (Core1)
                if ((_psIntreD->sRtcSt.nGetNtp_req == 1) && (_psIntreD->sRtcSt.nGetNtp_ack == 1))
                {
                    Serial.println(">>> [WiFi][STA] Success. Entering IDLE.");
                    wifi.stopSta();

                    portENTER_CRITICAL(&g_Spinlock);
                    _psIntreD->sRtcSt.nGetNtp_req = 0;
                    _psIntreD->sRtcSt.nGetNtp_ack = 0;
                    _psIntreD->eSeq_C0 = CORE0_IDLE;
                    portEXIT_CRITICAL(&g_Spinlock);

                    bNtpInitDone = false;
                    nNtpStartMs  = 0;
                }
            }

            break;

        case CORE0_SETUP:

            // Save calibration data (Core1)
            if (_psIntreD->sCtrl.nCalib_done == 1)
            {
                portENTER_CRITICAL(&g_Spinlock);
                _sNvsD_temp.sRgb.nCalib_Illu0_day   = _psIntreD->sNvsD.sRgb.nCalib_Illu0_day;
                _sNvsD_temp.sRgb.nCalib_Illu0_night = _psIntreD->sNvsD.sRgb.nCalib_Illu0_night;
                _psIntreD->sCtrl.nCalib_done = 0;
                portEXIT_CRITICAL(&g_Spinlock);

                _sNvsD_temp.crc = nvs.generateCRC(&_sNvsD_temp);

                if (nvs.open())
                {
                    nvs.save(&_sNvsD_temp);
                    nvs.close();
                }
            }

            if (wifi.getApState() == AP_OFF)
            {
                wifi.runAp("PixelClock");
            }

            if (wifi.getApState() == AP_RUNNING)
            {
                if (!bWebServerStarted)
                {
                    webServer.run();
                    bWebServerStarted = true;
                    Serial.println(">>> [Core0][SETUP] Server started.");
                }

                webServer.update();

                if (webServer.isNeedReboot())
                {
                    Serial.println(">>> [WiFi][AP] Success. Applying setup.");
                    wifi.stopAp();

                    portENTER_CRITICAL(&g_Spinlock);
                    _psIntreD->eSeq_C0 = CORE0_APPLY;
                    portEXIT_CRITICAL(&g_Spinlock);

                    bWebServerStarted = false;
                }
            }

            break;

        case CORE0_APPLY:

            Serial.println(">>> [Core0][APPLY] Saving config...");

            _sNvsD_temp.crc = nvs.generateCRC(&_sNvsD_temp);

            if (nvs.open())
            {
                nvs.save(&_sNvsD_temp);
                nvs.close();
                Serial.println(">>> [Core0][APPLY] Done. Rebooting...");
            }

            portENTER_CRITICAL(&g_Spinlock);
            _psIntreD->eSeq_C0 = CORE0_BOOTING;
            _psIntreD->eSeq_C1 = CORE1_STOP;
            portEXIT_CRITICAL(&g_Spinlock);

            Serial.flush();
            ESP.restart();

            break;

        case CORE0_ERROR:
            break;

        default:
            break;
    }
}
