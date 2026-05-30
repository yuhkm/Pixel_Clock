
#ifndef PCLK_GDATA_H_
#define PCLK_GDATA_H_

#include <Arduino.h>
#include <time.h>


//--------------------------------------------------------------------------------
//  Define Spinlock
//--------------------------------------------------------------------------------

extern portMUX_TYPE g_Spinlock;


//--------------------------------------------------------------------------------
//  Define system state
//--------------------------------------------------------------------------------

// System sequence
enum eSeq_Core0
{
    CORE0_BOOTING = 0,
    CORE0_IDLE = 1,
    CORE0_STA = 2,
    CORE0_SETUP = 3,
    CORE0_APPLY = 4,
    CORE0_ERROR = -1
};


enum eSeq_Core1
{
    CORE1_BOOTING = 0,
    CORE1_RUN = 1,
    CORE1_STOP = 2,
    CORE1_SETUP = 3,
    CORE1_ERROR = -1
};


// Wi-Fi
enum eState_STA
{
    STA_OFF,
    STA_CONNECTING,
    STA_CONNECTED,
    STA_FAIL
};


enum eState_AP
{
    AP_OFF,
    AP_STARTING,
    AP_RUNNING,
    AP_FAIL
};


// Weather API
enum eWeatherSky
{
    SKY_NONE    = 0,
    SKY_CLEAR   = 1,
    SKY_CLOUDY  = 3,
    SKY_OVERCAST = 4
};


enum eWeatherPty
{
    PTY_NONE        = 0,
    PTY_RAIN        = 1,
    PTY_RAIN_SNOW   = 2,
    PTY_SNOW        = 3,
    PTY_SHOWER      = 4,
    PTY_DROP        = 5,
    PTY_DROP_SNOW   = 6,
    PTY_SNOW_FLURRY = 7 
};


//--------------------------------------------------------------------------------
//  NVS save data
//--------------------------------------------------------------------------------

struct __attribute__((packed)) sConfig_WiFi
{
    char cSsid[32];
    char cPassword[64];
};


struct __attribute__((packed)) sConfig_Weather
{
    char cApiKey[128];
    char cCity_0[32];
    char cCity_1[32];

    uint16_t nEnable;
    uint16_t nSwitchCycle;

    uint16_t nGrid_X;
    uint16_t nGrid_Y;
};


struct __attribute__((packed)) sConfig_RGB
{
    uint16_t nEnable_DSleep;
    // uint16_t nEnable_RGB1;

    uint8_t nDSleep_str_HH;
    uint8_t nDSleep_str_MM;
    uint8_t nDSleep_end_HH;
    uint8_t nDSleep_end_MM;

    uint32_t nColor_RGB0_hex;
    // uint32_t nColor_RGB1_hex;

    uint8_t nLumi_RGB0_day;
    uint8_t nLumi_RGB0_night;
    // uint8_t nLumi_RGB1_day;
    // uint8_t nLumi_RGB1_night;

    uint16_t nCalib_Illu0_day;
    uint16_t nCalib_Illu0_night;
    // uint16_t nCalib_Illu1_day;
    // uint16_t nCalib_Illu1_night;
};


struct __attribute__((packed)) sConfig_Alarm
{
    uint16_t nEnable;

    uint8_t nHH;
    uint8_t nMM;
    uint8_t nD;   //xSMTWTFS

    uint8_t nToneNum;

    uint16_t nVolume;
};


struct __attribute__((packed)) sNvsData
{
    sConfig_WiFi    sWifi;
    sConfig_Weather sWeather;
    sConfig_RGB     sRgb;
    sConfig_Alarm   sAlarm;

    uint32_t crc;   // Checksum
};


struct sBtnState
{
    uint8_t nIsChanged;
    uint8_t nBusy_core0; 

    uint8_t nClick_req;
    uint8_t nCick_ack;
    uint8_t nHold_3s_req;
    uint8_t nHold_3s_ack; 
    uint8_t nHold_10s_req;
    uint8_t nHold_10s_ack; 
};


struct sRtcState
{
    // Core1 -STA ON-> Core0
    uint8_t nRunSta_req;
    uint8_t nRunSta_ack;
    uint8_t nRunSta_HH24;
    uint8_t nRunSta_mm;

    // Core0 -NTP data-> Core1
    uint8_t nGetNtp_req;
    uint8_t nGetNtp_ack;
    time_t tGetNtp_data;
};


struct sWeatherData
{
    uint8_t nValid;
    eWeatherSky eSky;
    eWeatherPty ePty;
    int8_t  nTemp;
    uint8_t nTime;
};


struct sCtrlData
{
    uint32_t nColor_hex;
    uint8_t  nLumi_day;
    uint8_t  nLumi_night;
    uint8_t  nUpdated;

    uint8_t  nCalib_req;
    uint8_t  nCalib_done;

    uint8_t  nVol_preview;
    uint8_t  nVol_play_req;

    uint8_t  nPreview_req;    // Preview request (Core1 set, Task_Buzzer clear)
    uint8_t  nPreview_tone;   // Preview melody index

    uint8_t  nAlarm_pending;   // Alarm active (Core1 set, Core0 clear)

    uint8_t  nBeep_req;        // Beep request (Core0/Core1 set, Task_Buzzer clear)
    uint8_t  nBeep_count;      // Beep count
    uint8_t  nBeep_vol;        // Beep volume (0~100%)

    uint8_t  nNightMode;       // Night mode active (Core1 set)
};


//--------------------------------------------------------------------------------
//  Inter Core data
//--------------------------------------------------------------------------------

struct sInterData
{
    volatile eSeq_Core0 eSeq_C0;
    volatile eSeq_Core1 eSeq_C1;

    volatile sNvsData sNvsD;
    
    volatile eState_STA eSt_Sta;
    volatile eState_AP eSt_AP;

    volatile sBtnState sBtnst;
    volatile sRtcState sRtcSt;
    volatile sWeatherData sWeaData;

    volatile sCtrlData sCtrl;
};




#endif