#include "pClk_RGB.h"



//--------------------------------------------------------------------------------
//  PClkRGB
//--------------------------------------------------------------------------------

PClkRGB::PClkRGB(uint8_t pin, uint16_t numLeds)
    : _nPin(pin),
      _nNumLeds(numLeds),
      _nBrightness(128),
      _nLastUpdate(0)
{
    _pixels = new Adafruit_NeoPixel(_nNumLeds, _nPin, NEO_GRB + NEO_KHZ800);
}


//--------------------------------------------------------------------------------
//  ~PClkRGB
//--------------------------------------------------------------------------------

PClkRGB::~PClkRGB()
{
   delete _pixels;
}


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkRGB::init()
{
    _pixels->begin();
    _pixels->setBrightness(_nBrightness);
    _pixels->show();
}


//--------------------------------------------------------------------------------
//  Update
//--------------------------------------------------------------------------------

void PClkRGB::update()
{

}


//--------------------------------------------------------------------------------
//  Set solid color
//--------------------------------------------------------------------------------

void PClkRGB::setSolidColor(uint8_t r, uint8_t g, uint8_t b) 
{
    for (int nI = 0; nI < _nNumLeds; nI++)
    {
        _pixels->setPixelColor(nI, _pixels->Color(r, g, b));
    }

    _pixels->show();
}


//--------------------------------------------------------------------------------
//  Set brightness
//--------------------------------------------------------------------------------

void PClkRGB::setBrightness(uint8_t brightness)
{
    _nBrightness = brightness;

    _pixels->setBrightness(_nBrightness);
    _pixels->show();
}


//--------------------------------------------------------------------------------
//  Clear
//--------------------------------------------------------------------------------

void PClkRGB::clear()
{
    _pixels->clear();
    _pixels->show();
}


//--------------------------------------------------------------------------------
//  Render
//--------------------------------------------------------------------------------

void PClkRGB::renderStart(uint8_t brightness)
{
    _pixels->setBrightness(brightness);
    _pixels->clear();
}


void PClkRGB::renderEnd()
{
    _pixels->show();
}


//--------------------------------------------------------------------------------
//  Draw
//--------------------------------------------------------------------------------

void PClkRGB::drawClock(uint8_t h, uint8_t m, bool tick, uint32_t color, int16_t offset)
{
    const int8_t* pPosX = (h >= 10) ? POS_DIGIT_X_4 : POS_DIGIT_X_3;
    uint8_t nY = POS_DIGIT_Y[0];

    if (h >= 10)
    {
        _drawBitmap(pPosX[0], Font_number[h/10], color, offset, 3, 5, nY);
        _drawBitmap(pPosX[1], Font_number[h%10], color, offset, 3, 5, nY);

        if (tick || offset != 0) _drawBitmap(pPosX[2], Font_colon, color, offset, 1, 5, nY);
    }
    else
    {
        _drawBitmap(pPosX[0], Font_number[h], color, offset, 3, 5, nY);

        if (tick || offset != 0) _drawBitmap(pPosX[1], Font_colon, color, offset, 1, 5, nY);
    }

    uint8_t nMIdx = (h >= 10) ? 3 : 2;

    _drawBitmap(pPosX[nMIdx],   Font_number[m/10], color, offset, 3, 5, nY);
    _drawBitmap(pPosX[nMIdx+1], Font_number[m%10], color, offset, 3, 5, nY);
}


void PClkRGB::drawWeather(int8_t temp, eWeatherSky sky, eWeatherPty pty, uint32_t tempColor, int16_t offset)
{

    if (pty != PTY_NONE)
    {
        switch (pty)
        {
            case PTY_RAIN: case PTY_SHOWER: case PTY_DROP: case PTY_RAIN_SNOW:
                 _drawBitmap(0, ICON_RAIN_CLOUDE, _pixels->Color(150, 150, 150), offset, 8, 8, 0);
                 _drawBitmap(0, ICON_RAIN_RAIN, _pixels->Color(0, 100, 255), offset, 8, 8, 0);
                break;
            case PTY_SNOW: case PTY_SNOW_FLURRY: case PTY_DROP_SNOW:
                _drawBitmap(0, ICON_SNOW, _pixels->Color(255, 255, 255), offset, 8, 7, 0);
                break;
        }
    } 
    else
    {
        switch (sky)
        {
            case SKY_CLEAR:
                _drawBitmap(0, ICON_SUN, _pixels->Color(255, 200, 0), offset, 8, 8, 0);
                break;
            case SKY_CLOUDY:
                _drawBitmap(0, ICON_CLOUDY_SUN, _pixels->Color(255, 200, 0), offset, 8, 8, 0);
                _drawBitmap(0, ICON_CLOUDY_CLOUD, _pixels->Color(255, 255, 255), offset, 8, 8, 0);
                break;
            case SKY_OVERCAST:
                _drawBitmap(0, ICON_CLOUDE, _pixels->Color(150, 150, 150), offset, 8, 8, 0);
                break;
        }
    }

    uint8_t nAbsTemp = (uint8_t)(abs((int)temp));
    if (nAbsTemp > 99) nAbsTemp = 99;  // clamp to displayable range
    uint8_t nY = POS_DIGIT_Y[0];

    if (temp < 0)
    {
        const uint8_t* pPosX = (nAbsTemp >= 10) ? POS_ICON_X_m2 : POS_ICON_X_m1;
        _drawBitmap(pPosX[1], Font_minus, tempColor, offset, 2, 5, nY);
        if (nAbsTemp >= 10)
        {
            _drawBitmap(pPosX[2], Font_number[nAbsTemp/10], tempColor, offset, 3, 5, nY);
            _drawBitmap(pPosX[3], Font_number[nAbsTemp%10], tempColor, offset, 3, 5, nY);
        }
        else _drawBitmap(pPosX[2], Font_number[nAbsTemp], tempColor, offset, 3, 5, nY);
    }
    else
    {
        const uint8_t* pPosX = (nAbsTemp >= 10) ? POS_ICON_X_2 : POS_ICON_X_1;
        if (nAbsTemp >= 10)
        {
            _drawBitmap(pPosX[1], Font_number[nAbsTemp/10], tempColor, offset, 3, 5, nY);
            _drawBitmap(pPosX[2], Font_number[nAbsTemp%10], tempColor, offset, 3, 5, nY);
        }
        else _drawBitmap(pPosX[1], Font_number[nAbsTemp], tempColor, offset, 3, 5, nY);
    }
}


void PClkRGB::drawScrollText(const uint8_t bitmap[][5], uint8_t bitmapWidth, int16_t scrollOff, uint32_t color)
{
    for (int16_t nScreenX = 0; nScreenX < 16; nScreenX++)
    {
        int16_t nSrcX = nScreenX + scrollOff;
        if (nSrcX < 0 || nSrcX >= bitmapWidth) continue;

        uint8_t nByteIdx = nSrcX / 8;
        uint8_t nBitMask = 0x80 >> (nSrcX % 8);

        for (uint8_t nRow = 0; nRow < 8; nRow++)
        {
            if (pgm_read_byte(&bitmap[nRow][nByteIdx]) & nBitMask)
            {
                uint16_t nIdx = _getIndex(nScreenX, nRow);
                if (nIdx != 999) _pixels->setPixelColor(nIdx, color);
            }
        }
    }
}


void PClkRGB::updateColon(uint8_t h, bool tick, uint32_t color, uint8_t brightness)
{
    _pixels->setBrightness(brightness);

    int16_t  nX        = (h >= 10) ? POS_DIGIT_X_4[2] : POS_DIGIT_X_3[1];
    uint16_t nUpperDot = _getIndex(nX, POS_DIGIT_Y[0] + 1);
    uint16_t nLowerDot = _getIndex(nX, POS_DIGIT_Y[0] + 3);

    if (nUpperDot != 999) _pixels->setPixelColor(nUpperDot, tick ? color : 0);

    if (nLowerDot != 999) _pixels->setPixelColor(nLowerDot, tick ? color : 0);

    _pixels->show();
}


//--------------------------------------------------------------------------------
//  Get Index
//--------------------------------------------------------------------------------

uint16_t PClkRGB::_getIndex(int16_t x, int16_t y)
{
    if (x < 0 || x > 15 || y < 0 || y > 7) return 999;
    if (x < 8) return (y * 8) + x;
    return 64 + (y * 8) + (x - 8);
}


void PClkRGB::_drawBitmap(int16_t startX, const uint8_t* bitmap, uint32_t color, 
                    int16_t offset, uint8_t width, uint8_t height, uint8_t startY)
{
    for (int nI = 0; nI < height; nI++)
    {
        uint8_t nRow = pgm_read_byte(&bitmap[nI]);

        for (int nJ = 0; nJ < width; nJ++)
        {
            uint8_t nMask;

            if (width == 1)
            {
                nMask = 0x01;
            }
            else if (width == 2)
            {
                nMask = (0x02 >> nJ);
            }
            else if (width == 3)
            {
                nMask = (0x04 >> nJ);
            }
            else
            {
                nMask = (0x80 >> nJ);
            }

            if (nRow & nMask)
            {
                uint16_t nIdx = _getIndex(startX + nJ - offset, startY + nI);

                if (nIdx != 999) _pixels->setPixelColor(nIdx, color);
            }
        }
    }
}
