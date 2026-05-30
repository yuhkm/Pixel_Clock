#ifndef PCLK_API_H_
#define PCLK_API_H_

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "pClk_gData.h"



class PClkAPI
{
    private:
        sInterData* _psIntreD = nullptr;
        const char* _host = "http://apis.data.go.kr/1360000/VilageFcstInfoService_2.0/getUltraSrtFcst";

    public:
        PClkAPI();
        ~PClkAPI();

        void init(sInterData* psIntreD);
        bool get_weather(uint8_t h, struct tm* timeinfo);
};


    
#endif