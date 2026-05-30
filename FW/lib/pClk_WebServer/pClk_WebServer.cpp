#include "pClk_WebServer.h"
#include "pClk_WebServer_HTML.h"



//--------------------------------------------------------------------------------
//  PClkWebServer
//--------------------------------------------------------------------------------

PClkWebServer::PClkWebServer(PClkWiFi* wifi, sNvsData* nvsData)
    : _pWifi(wifi),
     _psNvsData(nvsData)
{
    _pServer = new WebServer(80);
}


//--------------------------------------------------------------------------------
//  ~PClkWebServer
//--------------------------------------------------------------------------------

PClkWebServer::~PClkWebServer()
{
    if (_pServer)
    {
        _pServer->stop();
        delete _pServer;
    }
}


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkWebServer::init()
{
    _pServer->on("/", std::bind(&PClkWebServer::handleRoot, this));
    _pServer->on("/scan", std::bind(&PClkWebServer::handleScan, this));
    _pServer->on("/save", std::bind(&PClkWebServer::handleSave, this));
    _pServer->on("/ctrl",  std::bind(&PClkWebServer::handleCtrl,  this));
    _pServer->on("/calib", std::bind(&PClkWebServer::handleCalib, this));
    
    Serial.println(">>> [WEB][HTTP] Started.");
}


//--------------------------------------------------------------------------------
//  Run
//--------------------------------------------------------------------------------

void PClkWebServer::run()
{
    if (_pServer)
    {
        _pServer->begin(); 
        Serial.println(">>> [WEB][HTTP] Listening started.");
    }
}


//--------------------------------------------------------------------------------
//  Update
//--------------------------------------------------------------------------------

void PClkWebServer::update()
{
    if (_pServer)
    {
        _pServer->handleClient();
    }
}


//--------------------------------------------------------------------------------
//  Set Control Pointer
//--------------------------------------------------------------------------------

void PClkWebServer::setCtrlPtr(volatile sCtrlData* psCtrl)
{
    _psCtrl = psCtrl;
}


//--------------------------------------------------------------------------------
//  Is Need Reboot
//--------------------------------------------------------------------------------

bool PClkWebServer::isNeedReboot()
{
    return _bNeedReboot;
}


//--------------------------------------------------------------------------------
//  Handle Root
//--------------------------------------------------------------------------------

void PClkWebServer::handleRoot()
{
    String html = String(WIFI_CONFIG_HTML);

    html.replace("{{SSID}}", _psNvsData->sWifi.cSsid);
    html.replace("{{PW}}", _psNvsData->sWifi.cPassword);
    
    html.replace("{{WEATHER_EN}}", _psNvsData->sWeather.nEnable ? "checked" : "");
    html.replace("{{WEATHER_CYCLE}}", String(_psNvsData->sWeather.nSwitchCycle));
    html.replace("{{GRID_X}}", String(_psNvsData->sWeather.nGrid_X));
    html.replace("{{GRID_Y}}", String(_psNvsData->sWeather.nGrid_Y));
    html.replace("{{WEATHER_KEY}}", _psNvsData->sWeather.cApiKey);

    String cityInit = "<script>window.savedCity = {c0:'" + String(_psNvsData->sWeather.cCity_0) +
                      "', c1:'" + String(_psNvsData->sWeather.cCity_1) + "'};</script>";

    html.replace("<body>", "<body>" + cityInit);

    char cRgbColorStr[8];
    sprintf(cRgbColorStr, "#%06lX", (unsigned long)_psNvsData->sRgb.nColor_RGB0_hex);
    html.replace("{{RGB_DSLEEP_EN}}",     _psNvsData->sRgb.nEnable_DSleep ? "checked" : "");
    html.replace("{{RGB_DSLEEP_STR_HH}}", String(_psNvsData->sRgb.nDSleep_str_HH));
    html.replace("{{RGB_DSLEEP_STR_MM}}", String(_psNvsData->sRgb.nDSleep_str_MM));
    html.replace("{{RGB_DSLEEP_END_HH}}", String(_psNvsData->sRgb.nDSleep_end_HH));
    html.replace("{{RGB_DSLEEP_END_MM}}", String(_psNvsData->sRgb.nDSleep_end_MM));
    html.replace("{{RGB_COLOR0}}",        String(cRgbColorStr));
    html.replace("{{RGB_LUMI0_DAY}}",     String(_psNvsData->sRgb.nLumi_RGB0_day));
    html.replace("{{RGB_LUMI0_NIGHT}}",   String(_psNvsData->sRgb.nLumi_RGB0_night));

    uint8_t nD = _psNvsData->sAlarm.nD;
    html.replace("{{ALARM_EN}}",          _psNvsData->sAlarm.nEnable   ? "checked" : "");
    html.replace("{{ALARM_HH}}",          String(_psNvsData->sAlarm.nHH));
    html.replace("{{ALARM_MM}}",          String(_psNvsData->sAlarm.nMM));
    html.replace("{{ALARM_D_SUN}}",       (nD & 0x40) ? "checked" : "");
    html.replace("{{ALARM_D_MON}}",       (nD & 0x20) ? "checked" : "");
    html.replace("{{ALARM_D_TUE}}",       (nD & 0x10) ? "checked" : "");
    html.replace("{{ALARM_D_WED}}",       (nD & 0x08) ? "checked" : "");
    html.replace("{{ALARM_D_THU}}",       (nD & 0x04) ? "checked" : "");
    html.replace("{{ALARM_D_FRI}}",       (nD & 0x02) ? "checked" : "");
    html.replace("{{ALARM_D_SAT}}",       (nD & 0x01) ? "checked" : "");
    html.replace("{{ALARM_TONE_SEL_0}}", _psNvsData->sAlarm.nToneNum == 0 ? "selected" : "");
    html.replace("{{ALARM_TONE_SEL_1}}", _psNvsData->sAlarm.nToneNum == 1 ? "selected" : "");
    html.replace("{{ALARM_TONE_SEL_2}}", _psNvsData->sAlarm.nToneNum == 2 ? "selected" : "");
    html.replace("{{ALARM_VOL}}",         String(_psNvsData->sAlarm.nVolume));

    _pServer->send(200, "text/html", html);
}


//--------------------------------------------------------------------------------
//  Handle Scan
//--------------------------------------------------------------------------------

void PClkWebServer::handleScan()
{
    int nNetCount = WiFi.scanNetworks(false, false, false, 100);

    String cJson = "{";

    cJson += "\"cfg\":{\"s\":\"" + String(_psNvsData->sWifi.cSsid) + "\",";
    cJson += "\"p\":\"" + String(_psNvsData->sWifi.cPassword) + "\"},";

    cJson += "\"list\":[";
    int nDisplayCount = (nNetCount > 7) ? 7 : nNetCount;
    for (int nI = 0; nI < nDisplayCount; ++nI) {
        cJson += "{\"s\":\"" + WiFi.SSID(nI) + "\",\"r\":" + String(WiFi.RSSI(nI)) + "}";
        if (nI < nDisplayCount - 1) cJson += ",";
    }
    cJson += "]}";

    _pServer->send(200, "application/json", cJson);

    WiFi.scanDelete();
}


//--------------------------------------------------------------------------------
//  Handle Save
//--------------------------------------------------------------------------------

void PClkWebServer::handleSave()
{

    if (_pServer->hasArg("ssid"))
    {
        strncpy(_psNvsData->sWifi.cSsid, _pServer->arg("ssid").c_str(), sizeof(_psNvsData->sWifi.cSsid) - 1);
        _psNvsData->sWifi.cSsid[sizeof(_psNvsData->sWifi.cSsid) - 1] = '\0';   

        strncpy(_psNvsData->sWifi.cPassword, _pServer->arg("pw").c_str(), sizeof(_psNvsData->sWifi.cPassword) - 1);
        _psNvsData->sWifi.cPassword[sizeof(_psNvsData->sWifi.cPassword) - 1] = '\0';   
    }

    _psNvsData->sWeather.nEnable = _pServer->hasArg("weather_en") ? 1 : 0;
    
    if (_pServer->hasArg("weather_cycle"))
    {
        _psNvsData->sWeather.nSwitchCycle = _pServer->arg("weather_cycle").toInt();
    }
    
    if (_pServer->hasArg("city0"))
    {
        strncpy(_psNvsData->sWeather.cCity_0, _pServer->arg("city0").c_str(), sizeof(_psNvsData->sWeather.cCity_0) - 1);
        _psNvsData->sWeather.cCity_0[sizeof(_psNvsData->sWeather.cCity_0) - 1] = '\0';  
    }
    
    if (_pServer->hasArg("city1"))
    {
        strncpy(_psNvsData->sWeather.cCity_1, _pServer->arg("city1").c_str(), sizeof(_psNvsData->sWeather.cCity_1) - 1);
        _psNvsData->sWeather.cCity_1[sizeof(_psNvsData->sWeather.cCity_1) - 1] = '\0';
    }

    if (_pServer->hasArg("grid_x"))
    {
        _psNvsData->sWeather.nGrid_X = _pServer->arg("grid_x").toInt();
    }

    if (_pServer->hasArg("grid_y"))
    {
        _psNvsData->sWeather.nGrid_Y = _pServer->arg("grid_y").toInt();
    }

    if (_pServer->hasArg("weather_key"))
    {
        strncpy(_psNvsData->sWeather.cApiKey, _pServer->arg("weather_key").c_str(), sizeof(_psNvsData->sWeather.cApiKey) - 1);
        _psNvsData->sWeather.cApiKey[sizeof(_psNvsData->sWeather.cApiKey) - 1] = '\0';
    }

    _psNvsData->sRgb.nEnable_DSleep = _pServer->hasArg("rgb_dsleep_en") ? 1 : 0;

    if (_pServer->hasArg("rgb_dsleep_str_hh"))
    {
        int nV = _pServer->arg("rgb_dsleep_str_hh").toInt();
        _psNvsData->sRgb.nDSleep_str_HH = (uint8_t)(nV < 0 ? 0 : nV > 23 ? 23 : nV);
    }
    if (_pServer->hasArg("rgb_dsleep_str_mm"))
    {
        int nV = _pServer->arg("rgb_dsleep_str_mm").toInt();
        _psNvsData->sRgb.nDSleep_str_MM = (uint8_t)(nV < 0 ? 0 : nV > 59 ? 59 : nV);
    }
    if (_pServer->hasArg("rgb_dsleep_end_hh"))
    {
        int nV = _pServer->arg("rgb_dsleep_end_hh").toInt();
        _psNvsData->sRgb.nDSleep_end_HH = (uint8_t)(nV < 0 ? 0 : nV > 23 ? 23 : nV);
    }
    if (_pServer->hasArg("rgb_dsleep_end_mm"))
    {
        int nV = _pServer->arg("rgb_dsleep_end_mm").toInt();
        _psNvsData->sRgb.nDSleep_end_MM = (uint8_t)(nV < 0 ? 0 : nV > 59 ? 59 : nV);
    }

    if (_pServer->hasArg("rgb_color0"))
    {
        String cColorStr = _pServer->arg("rgb_color0");
        cColorStr.replace("#", "");
        _psNvsData->sRgb.nColor_RGB0_hex = (uint32_t)strtoul(cColorStr.c_str(), nullptr, 16);
    }
    if (_pServer->hasArg("rgb_lumi0_day"))
        _psNvsData->sRgb.nLumi_RGB0_day   = (uint8_t)_pServer->arg("rgb_lumi0_day").toInt();
    if (_pServer->hasArg("rgb_lumi0_night"))
        _psNvsData->sRgb.nLumi_RGB0_night = (uint8_t)_pServer->arg("rgb_lumi0_night").toInt();

    _psNvsData->sAlarm.nEnable = _pServer->hasArg("alarm_en") ? 1 : 0;

    if (_pServer->hasArg("alarm_hh"))
    {
        int nV = _pServer->arg("alarm_hh").toInt();
        _psNvsData->sAlarm.nHH = (uint8_t)(nV < 0 ? 0 : nV > 23 ? 23 : nV);
    }
    if (_pServer->hasArg("alarm_mm"))
    {
        int nV = _pServer->arg("alarm_mm").toInt();
        _psNvsData->sAlarm.nMM = (uint8_t)(nV < 0 ? 0 : nV > 59 ? 59 : nV);
    }

    {
        uint8_t nD = 0;
        if (_pServer->hasArg("alarm_d_sun")) nD |= 0x40;
        if (_pServer->hasArg("alarm_d_mon")) nD |= 0x20;
        if (_pServer->hasArg("alarm_d_tue")) nD |= 0x10;
        if (_pServer->hasArg("alarm_d_wed")) nD |= 0x08;
        if (_pServer->hasArg("alarm_d_thu")) nD |= 0x04;
        if (_pServer->hasArg("alarm_d_fri")) nD |= 0x02;
        if (_pServer->hasArg("alarm_d_sat")) nD |= 0x01;
        _psNvsData->sAlarm.nD = nD;
    }

    if (_pServer->hasArg("alarm_tone"))
        _psNvsData->sAlarm.nToneNum = (uint8_t)_pServer->arg("alarm_tone").toInt();
    if (_pServer->hasArg("alarm_volume"))
    {
        int nV = _pServer->arg("alarm_volume").toInt();
        _psNvsData->sAlarm.nVolume = (uint16_t)(nV < 0 ? 0 : nV > 100 ? 100 : nV);
    }

    _pServer->send(200, "text/html", "<html><body><h1>Saved! Rebooting...</h1></body></html>");
    _bNeedReboot = true;
}


//--------------------------------------------------------------------------------
//  Handle Ctrl
//--------------------------------------------------------------------------------

void PClkWebServer::handleCtrl()
{
    if (!_psCtrl || !_pServer->hasArg("cmd"))
    {
        _pServer->send(400, "text/plain", "Bad Request");
        return;
    }

    String cCmd = _pServer->arg("cmd");
    String cVal = _pServer->arg("val");

    uint32_t nColorVal = (uint32_t)strtoul(cVal.c_str(), nullptr, 16);
    uint8_t  nLumiVal  = (uint8_t)cVal.toInt();
    uint8_t  nToneVal  = _pServer->hasArg("tone") ? (uint8_t)_pServer->arg("tone").toInt() : 0;

    portENTER_CRITICAL(&g_Spinlock);

    if (cCmd == "color")
    {
        _psCtrl->nColor_hex   = nColorVal;
        _psCtrl->nUpdated    |= 0x01;
    }
    else if (cCmd == "lumi_day")
    {
        _psCtrl->nLumi_day    = nLumiVal;
        _psCtrl->nUpdated    |= 0x02;
    }
    else if (cCmd == "lumi_night")
    {
        _psCtrl->nLumi_night  = nLumiVal;
        _psCtrl->nUpdated    |= 0x04;
    }
    else if (cCmd == "vol_play")
    {
        _psCtrl->nVol_preview  = nLumiVal;
        _psCtrl->nPreview_tone = nToneVal;
        _psCtrl->nVol_play_req = 1;
    }

    portEXIT_CRITICAL(&g_Spinlock);

    _pServer->send(200, "text/plain", "OK");
}


//--------------------------------------------------------------------------------
//  Handle Calib
//--------------------------------------------------------------------------------

void PClkWebServer::handleCalib()
{
    if (!_psCtrl || !_pServer->hasArg("type"))
    {
        _pServer->send(400, "text/plain", "Bad Request");
        return;
    }

    String cType = _pServer->arg("type");

    portENTER_CRITICAL(&g_Spinlock);

    if      (cType == "day")   _psCtrl->nCalib_req = 1;
    else if (cType == "night") _psCtrl->nCalib_req = 2;

    portEXIT_CRITICAL(&g_Spinlock);

    _pServer->send(200, "text/plain", "OK");
}
