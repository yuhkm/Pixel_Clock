#ifndef PCLK_WIFI_H_
#define PCLK_WIFI_H_


#include <WiFi.h>
#include <ESPmDNS.h>
#include <functional>
#include "pClk_gData.h"



//--------------------------------------------------------------------------------
//  Class WiFi
//--------------------------------------------------------------------------------

class PClkWiFi
{
    private:
        eState_STA _eSt_STA = STA_OFF;
        eState_AP _eSt_AP = AP_OFF;

        uint32_t _nStaStartMillis = 0;
        uint32_t _nTimeout = 30000;   // Timeout 30s

    public:
        PClkWiFi();
        ~PClkWiFi();

        void init();

        void update();

        // STA Run / Stop
        void runSta(const char* ssid, const char* pw);
        void stopSta();

        // AP Run / Stop
        void runAp(const char* apName);
        void stopAp();
        
        // Get WiFi state
        bool isStaConnected();
        bool isApConnected();
        eState_STA getStaState();
        eState_AP  getApState();
};



#endif