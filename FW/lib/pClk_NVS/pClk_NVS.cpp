#include "pClk_NVS.h"



//--------------------------------------------------------------------------------
//  PClkNVS
//--------------------------------------------------------------------------------

PClkNVS::PClkNVS()
{

}


PClkNVS::~PClkNVS()
{

}


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkNVS::init()
{
    
}


//--------------------------------------------------------------------------------
//  NVS open / close
//--------------------------------------------------------------------------------

bool PClkNVS::open()
{
    return _prefs.begin(_namespace, false);
}


void PClkNVS::close()
{
    _prefs.end();
}


//--------------------------------------------------------------------------------
//  NVS data load / save
//--------------------------------------------------------------------------------

bool PClkNVS::load(sNvsData* data)
{
    size_t nRes = _prefs.getBytes("config", data, sizeof(sNvsData));

    return (nRes == sizeof(sNvsData));
}


bool PClkNVS::save(const sNvsData* data)
{
    size_t nRes = _prefs.putBytes("config", data, sizeof(sNvsData));

    return (nRes == sizeof(sNvsData));
}


//--------------------------------------------------------------------------------
//  NVS clear
//--------------------------------------------------------------------------------

void PClkNVS::clearMem()
{
    sNvsData sInvalid = {};

    if (open())
    {
        save(&sInvalid);
        close();
    }
}


//--------------------------------------------------------------------------------
//  CRC
//--------------------------------------------------------------------------------

uint32_t PClkNVS::generateCRC(sNvsData* data)
{
    const uint8_t* pData = (const uint8_t*)data;
    size_t nLen = offsetof(sNvsData, crc);
    uint32_t nCrc = 0xFFFFFFFF;

    while (nLen--) {
        nCrc ^= *pData++;
        for (int nI = 0; nI < 8; nI++) {
            nCrc = (nCrc >> 1) ^ (0xEDB88320 & -(nCrc & 1));
        }
    }
    return ~nCrc;
}


bool PClkNVS::isConfigValid(sNvsData* data)
{
    uint32_t nCalculated = generateCRC(data);

    if (nCalculated != data->crc)
    {
        Serial.printf(">>> [NVS][CRC] Mismatch. (Calc: 0x%08X, Saved: 0x%08X)\n", nCalculated, data->crc);
        return false;
    }

    return true;
}
