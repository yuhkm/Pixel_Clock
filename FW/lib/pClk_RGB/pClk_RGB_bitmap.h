#ifndef PCLK_RGB_BITMAP_H_
#define PCLK_RGB_BITMAP_H_

#include <Arduino.h>


#define SETUP_SCROLL_TEXT_WIDTH         40



//--------------------------------------------------------------------------------
//  Clock
//--------------------------------------------------------------------------------

// Number
const uint8_t PROGMEM Font_number[10][5] =
{
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b010, 0b010, 0b010, 0b010}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b101, 0b001, 0b001, 0b001}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

// :
const uint8_t PROGMEM Font_colon[5] = {0b0, 0b1, 0b0, 0b1, 0b0}; 



// Axis X (H H : m m)
const int8_t POS_DIGIT_X_4[] = {-1, 3, 7, 9, 13};

// Axis X (H : m m)
const int8_t POS_DIGIT_X_3[] = {2, 6, 8, 12};


// Axis Y
const uint8_t POS_DIGIT_Y[] = {2, 6};


//--------------------------------------------------------------------------------
//  Weather
//--------------------------------------------------------------------------------

// Sun
const uint8_t PROGMEM ICON_SUN[] = {
    0b00000000,
    0b00000000,
    0b00111000,
    0b01111100,
    0b01111100,
    0b01111100,
    0b00111000,
    0b00000000
};

// Cloudy sun layer
const uint8_t PROGMEM ICON_CLOUDY_SUN[] = {
    0b00000000,
    0b00110000,
    0b01111000,
    0b01111000,
    0b00110000,
    0b00000000,
    0b00000000
};

// Cloudy cloud layer
const uint8_t PROGMEM ICON_CLOUDY_CLOUD[] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b10000110,
    0b11001111,
    0b11111111,
    0b01111111,
    0b00000000
};

// Cloude(Overcast, rain...) layer
const uint8_t PROGMEM ICON_CLOUDE[] = {
    0b00000000,
    0b00000000,
    0b01111000,
    0b11111100,
    0b11111110,
    0b11111111,
    0b01111111,
    0b00000000
};

// Rain Cloude layer
const uint8_t PROGMEM ICON_RAIN_CLOUDE[] = {
    0b00000000,
    0b01111000,
    0b11111100,
    0b11111110,
    0b11111111,
    0b01111111,
    0b00000000,
    0b00000000
};

// Rain rain layer
const uint8_t PROGMEM ICON_RAIN_RAIN[] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00101010
};

// Snow
const uint8_t PROGMEM ICON_SNOW[] = {
    0b00000000,
    0b00001000,
    0b00101010,
    0b00011100,
    0b01111111,
    0b00011100,
    0b00101010,
    0b00001000
};


// -
const uint8_t PROGMEM Font_minus[5] = {0b00, 0b00, 0b11, 0b00, 0b00};

//
const uint8_t POS_ICON_Y[] = {0, 7};

//
const uint8_t POS_ICON_X_2[] = {0, 9, 13};
const uint8_t POS_ICON_X_1[] = {0, 11};
const uint8_t POS_ICON_X_m2[] = {0, 8, 10, 13};
const uint8_t POS_ICON_X_m1[] = {0, 9, 12};


// "setup.local" scroll text (40 x 8, row-major, 5 bytes per row, MSB = col 0)
const uint8_t PROGMEM SETUP_SCROLL_TEXT[8][5] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },  // row 0 (blank)
    { 0x00, 0x00, 0x00, 0x00, 0x00 },  // row 1 (blank)
    { 0xEE, 0x4A, 0xE1, 0x3B, 0xB9 },  // row 2: s e t u p . l o c a l
    { 0x8A, 0xEA, 0xA1, 0x2A, 0x09 },  // row 3
    { 0xEE, 0x4A, 0xE1, 0x2A, 0x39 },  // row 4
    { 0x28, 0x4A, 0x81, 0x2A, 0x29 },  // row 5
    { 0xEE, 0x6E, 0x89, 0x3B, 0xB9 },  // row 6
    { 0x00, 0x00, 0x00, 0x00, 0x00 },  // row 7 (blank)
};



#endif