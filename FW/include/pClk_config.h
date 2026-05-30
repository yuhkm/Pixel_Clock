#ifndef PCLK_CONFIG_H_
#define PCLK_CONFIG_H_



//--------------------------------------------------------------------------------
//  Parameter
//--------------------------------------------------------------------------------

// Serial
#define PCLK_BAUD_RATE              115200

// RGB count
#define RGB_CNT_MAT                 128

// STA active time (triggers every hour)
#define STA_ACTIVE_mm               0
  
// ADC
#define ADC_SAMPLE_PERIOD_MS        3000

// Deep Sleep
#define DSLEEP_PREWAKE_MIN          2

// Alarm auto-stop (5 Min)
#define ALARM_AUTO_STOP_MS          (5UL * 60UL * 1000UL)  

// Weather API retry
#define DSLEEP_WEATHER_RETRY        3

// Buzzer (default beep volume 0~100%)
#define BUZZER_PWM_CH               0
#define BUZZER_BEEP_VOL             5

// Voltage ADC lower, upper limit
#define ADC_5V_MIN                  4.0f
#define ADC_5V_MAX                  6.0f
#define ADC_33V_MIN                 2.8f
#define ADC_33V_MAX                 3.8f

// ADC nonlinear correction factors
#define ADC_5V_CORR                 0.89f
#define ADC_33V_CORR                0.77f


//--------------------------------------------------------------------------------
//  GPIO Pin map
//--------------------------------------------------------------------------------

// Boot
#define GPIO_BOOT                   0

// RTC
#define GPIO_RTC_SDA                17
#define GPIO_RTC_SCL                18
#define GPIO_RTC_INT                7

// RGB
#define GPIO_RGB_MAT                10

// Illuminance
#define GPIO_ILLU_0                 4

// Button
#define GPIO_BTN_0                  13

// Buzzer
#define GPIO_BUZZER                 1

// Voltage ADC
#define GPIO_ADC_5V                 3
#define GPIO_ADC_3V3                6

// Indicator LED
#define GPIO_LED_5V                 2
#define GPIO_LED_3V3                16
#define GPIO_LED_AP                 12
#define GPIO_LED_STA                14
#define GPIO_LED_STATUS             11
#define GPIO_LED_TX                 15



#endif