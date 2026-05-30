#include "pClk_API.h"



//--------------------------------------------------------------------------------
//  PClkAPI
//--------------------------------------------------------------------------------

PClkAPI::PClkAPI()
{

}


//--------------------------------------------------------------------------------
//  ~PClkAPI
//--------------------------------------------------------------------------------

PClkAPI::~PClkAPI()
{

}


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkAPI::init(sInterData* psIntreD)
{
    _psIntreD = psIntreD;
}


//--------------------------------------------------------------------------------
//  Get Weather data
//--------------------------------------------------------------------------------

bool PClkAPI::get_weather(uint8_t h, struct tm* timeinfo)
{
    if (WiFi.status() != WL_CONNECTED || _psIntreD == nullptr) return false;
    if (_psIntreD->sNvsD.sWeather.nEnable != 1) return false;
    if (_psIntreD->sNvsD.sWeather.cApiKey[0] == '\0') return false;

    char cBaseDate[9];
    char cBaseTime[5];
    char cTargetFcstTime[5];

    int nBaseHour = (timeinfo->tm_min < 50) ? (int)h - 1 : (int)h;

    if (nBaseHour < 0)
    {
        nBaseHour = 23;
        time_t tRaw = mktime(timeinfo);
        tRaw -= 86400;
        struct tm sTmYesterday;
        localtime_r(&tRaw, &sTmYesterday);
        sprintf(cBaseDate, "%04d%02d%02d", sTmYesterday.tm_year + 1900, sTmYesterday.tm_mon + 1, sTmYesterday.tm_mday);
    }
    else
    {
        sprintf(cBaseDate, "%04d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    }

    int nTargetHour = nBaseHour + 1;

    if (nTargetHour >= 24)
    {
        nBaseHour--;     // 23 → 22
        nTargetHour = nBaseHour + 1;  // 23
    }

    sprintf(cBaseTime, "%02d00", nBaseHour);
    sprintf(cTargetFcstTime, "%02d00", nTargetHour);

    String cUrl = String(_host) + "?serviceKey=" + String((char*)_psIntreD->sNvsD.sWeather.cApiKey) +
                "&numOfRows=60&pageNo=1&dataType=JSON&base_date=" + String(cBaseDate) +
                "&base_time=" + String(cBaseTime) +
                "&nx=" + String(_psIntreD->sNvsD.sWeather.nGrid_X) +
                "&ny=" + String(_psIntreD->sNvsD.sWeather.nGrid_Y);

    HTTPClient http;
    http.begin(cUrl);
    int nHttpCode = http.GET();

    if (nHttpCode == HTTP_CODE_OK)
    {
        String sPayload = http.getString();
        http.end();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, sPayload);

        if (!error)
        {
            JsonArray items = doc["response"]["body"]["items"]["item"];

            sWeatherData sNewData = {};
            bool bFound = false;

            for (JsonObject item : items)
            {
                if (item["fcstTime"] == String(cTargetFcstTime))
                {
                    String cCategory  = item["category"];
                    String cFcstValue = item["fcstValue"];

                    if (cCategory == "T1H") sNewData.nTemp = (int8_t)cFcstValue.toInt();
                    if (cCategory == "SKY") sNewData.eSky  = (eWeatherSky)cFcstValue.toInt();
                    if (cCategory == "PTY") sNewData.ePty  = (eWeatherPty)cFcstValue.toInt();
                    bFound = true;
                }
            }

            if (bFound)
            {
                sNewData.nTime  = h;
                sNewData.nValid = 1;

                portENTER_CRITICAL(&g_Spinlock);
                memcpy((void*)&_psIntreD->sWeaData, &sNewData, sizeof(sWeatherData));
                portEXIT_CRITICAL(&g_Spinlock);
            }

            return bFound;
        }

        Serial.println(">>> [Weather][API] Failed. Parse error.");
        return false;
    }

    Serial.printf(">>> [Weather][API] Failed. HTTP %d\n", nHttpCode);
    http.end();
    return false;
}
