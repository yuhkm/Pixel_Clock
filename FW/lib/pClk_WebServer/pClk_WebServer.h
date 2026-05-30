#ifndef PCLK_WEB_SERVER_H_
#define PCLK_WEB_SERVER_H_

#include <WebServer.h>
#include "pClk_WiFi.h"
#include "pClk_gData.h"



class PClkWebServer
{
    private:
        WebServer* _pServer   = nullptr;
        PClkWiFi*  _pWifi     = nullptr;
        sNvsData*  _psNvsData = nullptr;
        volatile sCtrlData* _psCtrl = nullptr;

        bool _bNeedReboot = false;

        // Handle Ap server
        void handleRoot();
        void handleScan();
        void handleSave();
        void handleCtrl();
        void handleCalib();

    public:
        PClkWebServer(PClkWiFi* wifi, sNvsData* nvsData);
        ~PClkWebServer();

        void init();
        void run();
        void update();
        void setCtrlPtr(volatile sCtrlData* psCtrl);

        bool isNeedReboot();
};



#endif