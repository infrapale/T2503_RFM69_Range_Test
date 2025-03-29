#ifndef __UART_H__
#define __UART_H__
#include "Arduino.h"
#define UART_MAX_BLOCK_LEN  8

typedef enum
{
    UART_CMD_TRANSMIT_RAW   = 'T',
    UART_CMD_TRANSMIT_NODE  = 'N',
    UART_CMD_GET_AVAIL      = 'A',
    UART_CMD_READ_RAW       = 'R',
    UART_CMD_READ_NODE      = 'O' 
} uart_cmd_et;

typedef enum
{
    UART_REPLY_AVAILABLE    = 'a',
    UART_REPLY_READ_RAW     = 'r',
    UART_REPLY_READ_NODE    = 'o' 
} uart_reply_et;


typedef struct
{
    String          str;
    char            radio_msg[MAX_MESSAGE_LEN];
    uint8_t         len;
    bool            avail;
    char            module;
    char            addr;         
    uart_cmd_et     cmd;
    msg_format_et   format;
    //msg_format_et   cmd_format;
    uart_rx_status_et   status;
    radio_et        radio;
} uart_rx_st;

typedef struct
{
    String          str;
    //char            radio_msg[MAX_MESSAGE_LEN];
    uint8_t         len;
    bool            avail;
    char            module;
    char            addr;         
    uart_cmd_et     cmd;
    msg_format_et   format;
    msg_format_et   cmd_format;
    msg_status_et   status;
} uart_tx_st;

typedef struct
{
    String zone;
    String name; 
    String value;
    String remark;
} uart_node_st;

typedef struct
{
    uart_rx_st      rx;
    uart_tx_st      tx;
    uart_node_st    node;
} uart_msg_st;

/// @brief Clear rx available
/// @param  -
/// @return -
void uart_initialize(void);

void uart_report_radio_msg(char *radio_receive_msg, int rssi);

void uart_rx_send_rfm_from_raw(void);

/// @brief  Get pointer to module data
/// @param
/// @return data pointer
uart_msg_st *uart_get_data_ptr(void);

/// @brief  Read uart
/// @note   Save mesage in uart.rx.str
/// @param  -
/// @return -
void uart_read_uart(void);

/// @brief  Parse Rx frame, 
/// @note   check that the frame is valid and addressed to me
/// @param  -
/// @return -
void uart_parse_rx_frame(void);

/// @brief  Print rx metadat for debugging
/// @param  -
/// @return -
void uart_print_rx_metadata(void);

/// @brief  Build radio mesasge
/// @param  -
/// @return -
void uart_rx_build_rfm_array(void);

/// @brief  Execute command
/// @param  UART command
/// @return
void uart_exec_cmnd(uart_cmd_et ucmd);


#endif