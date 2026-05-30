#include "pClk_RTC.h"



//--------------------------------------------------------------------------------
//  PClkRTC
//--------------------------------------------------------------------------------

PClkRTC::PClkRTC()
{

}

//--------------------------------------------------------------------------------
//  ~PClkRTC
//--------------------------------------------------------------------------------

PClkRTC::~PClkRTC()
{

}


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

bool PClkRTC::init(int sda, int scl)
{
    return Wire.begin(sda, scl);
}


//--------------------------------------------------------------------------------
//  Private
//--------------------------------------------------------------------------------

uint8_t PClkRTC::_decToBcd(uint8_t val)
{ 
    return ((val / 10) << 4) | (val % 10);
}


uint8_t PClkRTC::_bcdToDec(uint8_t val)
{ 
    return ((val >> 4) * 10) + (val & 0x0F);
}


uint8_t PClkRTC::_readReg(uint8_t reg)
{
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(RTC_ADDR, (uint8_t)1);

    return Wire.available() ? Wire.read() : 0;
}


//--------------------------------------------------------------------------------
//  Set / Get NTP
//--------------------------------------------------------------------------------

bool PClkRTC::set_NTP(time_t tRawData)
{
    struct tm* pTmInfo = localtime(&tRawData);

    if (pTmInfo == nullptr) return false;

    return set_RTC(pTmInfo->tm_year + 1900, pTmInfo->tm_mon + 1, pTmInfo->tm_mday,
                   pTmInfo->tm_hour, pTmInfo->tm_min, pTmInfo->tm_sec, pTmInfo->tm_wday);
}


time_t PClkRTC::get_NTP()
{
    struct tm sTm;

    sTm.tm_sec  = get_RTC_SS();
    sTm.tm_min  = get_RTC_mm();
    sTm.tm_hour = get_RTC_HH_24();
    sTm.tm_mday = get_RTC_DD();
    sTm.tm_mon  = get_RTC_MM() - 1;
    sTm.tm_year = get_RTC_YYYY() - 1900;

    return mktime(&sTm);
}


//--------------------------------------------------------------------------------
//  Set / Get RTC
//--------------------------------------------------------------------------------

bool PClkRTC::readTime(uint8_t* hh, uint8_t* mm, uint8_t* ss, uint8_t* wday)
{
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(RTC_REG_SEC);    // 0x04 — start from seconds
    if (Wire.endTransmission() != 0) return false;

    Wire.requestFrom(RTC_ADDR, (uint8_t)5);  // SS(04), MM(05), HH(06), DD(07), WDAY(08)
    if (Wire.available() < 5) return false;

    *ss   = _bcdToDec(Wire.read() & 0x7F);
    *mm   = _bcdToDec(Wire.read() & 0x7F);
    *hh   = _bcdToDec(Wire.read() & 0x3F);
    Wire.read();                              // DD — skip
    *wday = _bcdToDec(Wire.read() & 0x07);

    return true;
}


bool PClkRTC::set_RTC(int year, int mon, int mday, int hour, int min, int sec, int wday)
{
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(RTC_REG_SEC);
    Wire.write(_decToBcd(sec));
    Wire.write(_decToBcd(min));
    Wire.write(_decToBcd(hour));
    Wire.write(_decToBcd(mday));
    Wire.write(_decToBcd(wday));
    Wire.write(_decToBcd(mon));
    Wire.write(_decToBcd(year % 100));

    return (Wire.endTransmission() == 0);
}


int PClkRTC::get_RTC_YYYY()  
{ 
    return _bcdToDec(_readReg(0x0A)) + 2000;
}


int PClkRTC::get_RTC_MM()
{ 
    return _bcdToDec(_readReg(0x09) & 0x1F); 
}


int PClkRTC::get_RTC_DD()
{ 
    return _bcdToDec(_readReg(0x07) & 0x3F);
}


int PClkRTC::get_RTC_HH_24()
{
    return _bcdToDec(_readReg(0x06) & 0x3F);
}


int PClkRTC::get_RTC_HH_12()
{ 
    int nH = get_RTC_HH_24();
    if (nH == 0)  return 12;
    if (nH > 12)  return nH - 12;

    return nH;
}


int PClkRTC::get_RTC_mm()
{ 
    return _bcdToDec(_readReg(0x05) & 0x7F);
}


int PClkRTC::get_RTC_SS()
{ 
    return _bcdToDec(_readReg(0x04) & 0x7F);
}


int PClkRTC::get_RTC_WDAY() 
{ 
    return _bcdToDec(_readReg(0x08) & 0x07);
}


//--------------------------------------------------------------------------------
//  Interrupt
//--------------------------------------------------------------------------------

bool PClkRTC::set_Int(int hour, int min, int sec)
{
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(RTC_ALARM_SEC);

    Wire.write((sec == 99)  ? 0x80 : (_decToBcd(sec) & 0x7F));
    Wire.write((min == 99)  ? 0x80 : (_decToBcd(min) & 0x7F));
    Wire.write((hour == 99) ? 0x80 : (_decToBcd(hour) & 0x7F));
    Wire.write(0x80); // Day
    Wire.write(0x80); // Weekday

    if (Wire.endTransmission() != 0) return false;

    uint8_t nCtrl2 = _readReg(RTC_CTRL_2);
    nCtrl2 |= RTC_AIE;
    nCtrl2 &= ~RTC_AF;

    Wire.beginTransmission(RTC_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.write(nCtrl2);

    return (Wire.endTransmission() == 0);
}


void PClkRTC::clear_Int()
{
    uint8_t nCtrl2 = _readReg(RTC_CTRL_2);

    nCtrl2 &= ~RTC_AF;

    Wire.beginTransmission(RTC_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.write(nCtrl2);
    Wire.endTransmission();
}


void PClkRTC::disable_Int()
{
    uint8_t nCtrl2 = _readReg(RTC_CTRL_2);

    nCtrl2 &= ~RTC_AIE;  // Disable alarm interrupt
    nCtrl2 &= ~RTC_AF;   // Clear alarm flag

    Wire.beginTransmission(RTC_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.write(nCtrl2);
    Wire.endTransmission();
}


//--------------------------------------------------------------------------------
//  RTC register reset (Factory reset)
//--------------------------------------------------------------------------------

void PClkRTC::reset_register()
{

    Wire.beginTransmission(RTC_ADDR);
    Wire.write(0x00); 
    Wire.write(0x58);
    Wire.endTransmission();
    
    delay(10);

    Wire.beginTransmission(RTC_ADDR);
    Wire.write(0x10);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(RTC_ADDR);
    Wire.write(0x01);
    Wire.write(0x00);
    Wire.endTransmission();
}
