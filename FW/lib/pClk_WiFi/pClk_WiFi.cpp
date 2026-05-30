#include "pClk_WiFi.h"



//--------------------------------------------------------------------------------
//  PClkWiFi
//--------------------------------------------------------------------------------

PClkWiFi::PClkWiFi()
{

}


//--------------------------------------------------------------------------------
//  ~PClkWiFi
//--------------------------------------------------------------------------------

PClkWiFi::~PClkWiFi()
{

}


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkWiFi::init()
{
    WiFi.mode(WIFI_OFF);

    _eSt_STA = STA_OFF;
    _eSt_AP = AP_OFF;
}


//--------------------------------------------------------------------------------
//  Loop update
//--------------------------------------------------------------------------------

void PClkWiFi::update() {

    if (_eSt_STA == STA_CONNECTING)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            _eSt_STA = STA_CONNECTED;
        }

        else if (millis() - _nStaStartMillis > _nTimeout)
        {
            _eSt_STA = STA_FAIL;
        }
    }

    else if (_eSt_STA == STA_CONNECTED)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            _eSt_STA = STA_FAIL;
        }
    }
}


//--------------------------------------------------------------------------------
//  STA Run / Stop
//--------------------------------------------------------------------------------

void PClkWiFi::runSta(const char* ssid, const char* pw)
{
    WiFi.begin(ssid, pw);
    _nStaStartMillis = millis();
    _eSt_STA = STA_CONNECTING;

    WiFi.mode(_eSt_AP == AP_RUNNING ? WIFI_AP_STA : WIFI_STA);
}


void PClkWiFi::stopSta()
{
    WiFi.disconnect();

    _eSt_STA = STA_OFF;
    WiFi.mode(_eSt_AP == AP_RUNNING ? WIFI_AP : WIFI_OFF);
}


//--------------------------------------------------------------------------------
//  AP Run / Stop
//--------------------------------------------------------------------------------

void PClkWiFi::runAp(const char* apName)
{
    if (WiFi.softAP(apName))
    {
        _eSt_AP = AP_RUNNING;

        WiFi.mode(_eSt_STA == STA_OFF ? WIFI_AP : WIFI_AP_STA);
        MDNS.begin("setup");    // http://setup.local
    }

    else
    {
        _eSt_AP = AP_FAIL;
    }
}


void PClkWiFi::stopAp()
{
    MDNS.end();
    WiFi.softAPdisconnect(true);
    _eSt_AP = AP_OFF;

    WiFi.mode(_eSt_STA == STA_OFF ? WIFI_OFF : WIFI_STA);
}


//--------------------------------------------------------------------------------
//  Get WiFi state
//--------------------------------------------------------------------------------

bool PClkWiFi::isStaConnected()
{
    return (_eSt_STA == STA_CONNECTED);
}


bool PClkWiFi::isApConnected()
{
    return (_eSt_AP == AP_RUNNING);
}


eState_STA PClkWiFi::getStaState()
{
    return _eSt_STA;
}


eState_AP PClkWiFi::getApState()
{
    return _eSt_AP;
}
