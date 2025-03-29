#ifndef __MAIN_H__
#define __MAIN_H__
//#include "WString.h"

//#define DEBUG_PRINT 
//#define SEND_TEST_MSG 
//#define ADA_M0_RFM69
//#define ADA_RFM69_WING
#define PRO_MINI_RFM69
#include <Arduino.h>
#include "rfm69.h"
#if defined(ADA_M0_RFM69) | defined(ADA_RFM69_WING)
#define SerialX  Serial1
#else
#define SerialX Serial
#endif

#define APP_NAME    ((char*)"T2503_RFM69_Range_Test_Central")
#define TASK_NBR_OF  6
// #define LED_INDICATION

typedef enum
{
    UNDEFINED = 0,
    LORA_433 = 1,
    LORA_868 = 2,
    RFM_433  = 3,
} radio_et;


typedef enum
{
    UART_RX_MSG_UNDEFINED = 0,
    UART_RX_MSG_ACK_TO_REMOTE,
    UART_RX_MSG_TO_LOGGER
 } uart_rx_status_et;

typedef struct
{
    radio_et        radio;
    char            module;
    char            addr;         
} module_data_st;


typedef enum
{
    MSG_FORMAT_RAW = 0,
    MSG_FORMAT_SENSOR_JSON,
    MSG_FORMAT_RELAY_JSON
}  msg_format_et;

typedef enum
{
    STATUS_UNDEFINED = 0,
    STATUS_OK_FOR_ME,
    STATUS_NOT_FOR_ME,
    STATUS_UNKNOWN_COMMAND,
    STATUS_CORRECT_FRAME,
    STATUS_INCORRECT_FRAME,
} msg_status_et;



extern module_data_st     me;


#endif