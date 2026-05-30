#ifndef PCLK_RGB_H_
#define PCLK_RGB_H_

#include <Adafruit_NeoPixel.h>
#include "pClk_RGB_bitmap.h"
#include "pClk_config.h"
#include "pClk_gData.h"


class PClkRGB
{
    private :
        Adafruit_NeoPixel* _pixels;

        uint8_t  _nPin;
        uint16_t _nNumLeds;
        uint8_t  _nBrightness;
        uint32_t _nLastUpdate;

        uint16_t _getIndex(int16_t x, int16_t y);
        void _drawBitmap(int16_t startX, const uint8_t* bitmap, uint32_t color,
                    int16_t offset, uint8_t width, uint8_t height, uint8_t startY);

    public:

        PClkRGB(uint8_t pin, uint16_t numLeds);
        ~PClkRGB();

        void init();

        void update(); 
        
        void setSolidColor(uint8_t r, uint8_t g, uint8_t b);
        void setBrightness(uint8_t brightness);

        void clear();

        void renderStart(uint8_t brightness);
        void renderEnd();

        void drawClock(uint8_t h, uint8_t m, bool tick, uint32_t color, int16_t offset = 0);
        void drawWeather(int8_t temp, eWeatherSky sky, eWeatherPty pty, uint32_t tempColor, int16_t offset);
        void drawScrollText(const uint8_t bitmap[][5], uint8_t bitmapWidth, int16_t scrollOff, uint32_t color);
        void updateColon(uint8_t h, bool tick, uint32_t color, uint8_t brightness);
};     



#endif