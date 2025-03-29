#ifndef __IO_H__
#define __IO_H__

#ifdef  ADA_M0_RFM69
#define RFM69_CS      8
#define RFM69_INT     3
// #define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     4
#endif


#ifdef  ADA_RFM69_WING
#define RFM69_CS      18
#define RFM69_INT     5
// #define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     18
#endif

#ifdef PRO_MINI_RFM69
#define RFM69_CS      10
#define RFM69_INT     2
#define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     9
#endif

#define LOGGER_SD_CS  6 
#define TFT_CS        11
#define TFT_DC        12


// LED Definitions
#define PIN_LED_ONBOARD 13  // onboard blinky
#define LED_NBR_OF      4
#define PIN_LED_RED     PIN_LED_ONBOARD
#define PIN_LED_GREEN   PIN_LED_ONBOARD
#define PIN_LED_BLUE    PIN_LED_ONBOARD

// https://learn.adafruit.com/adafruit-feather/rtc-datalogging-wings
// https://www.adafruit.com/product/2922
// The RTC we'll be using is the PCF8523



typedef enum
{
    LED_INDX_ONBOARD = 0,
    LED_INDX_RED,
    LED_INDX_GREEN,
    LED_INDX_BLUE,
    LED_INDX_NBR_OF
} led_index_et;

void io_initialize(void);

void io_led_flash(led_index_et led_indx, uint16_t nbr_ticks );

void io_run_100ms(void);


#endif