#ifndef PCLK_RTC_H_
#define PCLK_RTC_H_


#include <Arduino.h>
#include <Wire.h>
#include <time.h>


#define RTC_ADDR        ((uint8_t)0x51)
#define RTC_REG_SEC     ((uint8_t)0x04)
#define RTC_CTRL_2      ((uint8_t)0x01)
#define RTC_ALARM_SEC   ((uint8_t)0x0B)
#define RTC_AIE         ((uint8_t)0x80)
#define RTC_AF          ((uint8_t)0x40)



class PClkRTC
{
    private:
        uint8_t _decToBcd(uint8_t val);
        uint8_t _bcdToDec(uint8_t val);
        uint8_t _readReg(uint8_t reg);

    public:

        PClkRTC();
        ~PClkRTC();

        bool init(int sda, int scl);

        bool set_NTP(time_t tRawData);
        time_t get_NTP();

        // Set / Get RTC
        bool set_RTC(int year, int mon, int mday, int hour, int min, int sec, int wday);
        bool readTime(uint8_t* hh, uint8_t* mm, uint8_t* ss, uint8_t* wday); // Burst read SS/MM/HH/WDAY
        int get_RTC_YYYY();
        int get_RTC_MM();
        int get_RTC_DD();
        int get_RTC_HH_24();
        int get_RTC_HH_12();
        int get_RTC_mm();
        int get_RTC_SS();
        int get_RTC_WDAY(); // 0:SUN, 6:SAT

        // Interrupt
        bool set_Int(int hour, int min, int sec=0);
        void clear_Int();
        void disable_Int();

        // RTC register reset (Factory reset)
        void reset_register();
};



#endif