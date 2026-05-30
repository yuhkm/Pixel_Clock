#ifndef PCLK_NVS_H_
#define PCLK_NVS_H_

#include <Preferences.h>
#include "pClk_gData.h"



class PClkNVS
{
    private:
        Preferences _prefs;
        const char* _namespace = "NvsData";

    public:
        PClkNVS();
        ~PClkNVS();

        void init();

        bool open();
        void close();

        bool load(sNvsData* data);          // Flash -> RAM
        bool save(const sNvsData* data);    // RAM -> Flash

        // Clear memory
        void clearMem();

        // Checksum
        uint32_t generateCRC(sNvsData* data);
        bool isConfigValid(sNvsData* data);
};



#endif