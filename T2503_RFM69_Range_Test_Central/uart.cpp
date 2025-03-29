#include "main.h"
#include "uart.h"
#include "json.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"

#ifdef BASE_APPLICATION
#define  FRAME_START  ('<')
#define  FRAME_END    ('>')
#endif

#ifdef REMOTE_APPLICATION
#define FRAME_START   '['
#define FRAME_END     ']'
#endif


uart_msg_st         uart;

uart_msg_st *uart_get_data_ptr(void)
{
    return &uart;
}

void uart_initialize(void)
{
    uart.rx.avail = false;
}

void uart_read_uart(void)
{
    if (SerialX.available())
    {
        io_led_flash(LED_INDX_BLUE,20);
        uart.rx.str = SerialX.readStringUntil('\n');
        if (uart.rx.str.length()> 0)
        {
            uart.rx.avail = true;
            //uart.rx.str.remove(uart.rx.str.length()-1);
        }
        #ifdef DEBUG_PRINT
        Serial.println("rx is available");
        #endif        
    } 

}

void uart_report_radio_msg(char *radio_receive_msg, int rssi)
{
    String msg_str = radio_receive_msg;
    // Serial.print(msg_str); Serial.print(" - "); Serial.println(rssi);
    msg_str.trim();
    if ((msg_str.charAt(0) == FRAME_START) || 
        (msg_str.charAt(msg_str.length()-1) == FRAME_END)) 
    {
        uint8_t end_pos = msg_str.indexOf('>');
        String str = msg_str.substring(0,end_pos+1);
        
        uint8_t base_tag   = str.indexOf('S');
        uint8_t remote_tag = str.indexOf('T');

        #ifdef BASE_APPLICATION
        String str1 = str.substring(0,base_tag-1);
        str1 += ",S,";
        str1 += rssi;
        str1 += str.substring(remote_tag-1);
        #endif
        #ifdef REMOTE_APPLICATION
        String str1 = str.substring(0,remote_tag-1);
        str1 += ",T,";
        str1 += rssi;
        str1 += ",>";
        #endif
        Serial.println(str1);
    
    
    
    } 


}

void uart_parse_rx_frame(void)
{
    //rfm_send_msg_st *rx_msg = &send_msg; 
    bool do_continue = true;
    uint8_t len;
    uart.rx.str.trim();
    uart.rx.len = uart.rx.str.length();
    uart.rx.status = UART_RX_MSG_UNDEFINED;

    if (uart.rx.len < 20)  do_continue = false;
    if (do_continue)
    {
        uint16_t radio_pos = uart.rx.str.indexOf('R') + 2;
        uart.rx.radio =  uart.rx.str.charAt(radio_pos) - '0';
        #ifdef DEBUG_PRINT
        Serial.print("Radio ="); Serial.println(uart.rx.radio);
        #endif
    }
    if (do_continue)
    {
        if ((uart.rx.str.charAt(0) == '[') || 
            (uart.rx.str.charAt(uart.rx.len-1) == ']'))  
        {
            uart.rx.status = UART_RX_MSG_ACK_TO_REMOTE;
        }
        else if ((uart.rx.str.charAt(0) == '!') || 
            (uart.rx.str.charAt(uart.rx.len-1) == '!'))
        {
            uart.rx.status = UART_RX_MSG_TO_LOGGER;
        }
        else do_continue = false;
    }
 
    if (do_continue)
    {   


        #ifdef DEBUG_PRINT
        Serial.print("Buffer frame is OK\n");
        Serial.print("RX Status = "); Serial.println(uart.rx.status);
        #endif
    }
}





void uart_rx_send_rfm_from_raw(void)
{
    //String payload = uart.rx.str.substring(6,uart.rx.len - 1);
    String payload = uart.rx.str;
    #ifdef DEBUG_PRINT
    Serial.print("Befor trim ");
    Serial.println(payload.length());
    #endif
    payload.trim();
    #ifdef DEBUG_PRINT
    Serial.print("After trim ");
    Serial.println(payload.length());
    #endif
    memset(uart.rx.radio_msg,0x00, MAX_MESSAGE_LEN);
    payload.toCharArray(uart.rx.radio_msg, MAX_MESSAGE_LEN);
    #ifdef DEBUG_PRINT
    Serial.println(payload);
    Serial.print("char array: ");
    Serial.println(uart.rx.radio_msg);
    #endif
    rfm_send_radiate_msg(uart.rx.radio_msg);
}






void uart_print_rx_metadata(void)
{
    Serial.print("Length      "); Serial.println(uart.rx.len);
    Serial.print("Avail       "); Serial.println(uart.rx.avail);
    Serial.print("Status      "); Serial.println(uart.rx.status);
    Serial.print("Module      "); Serial.println(uart.rx.module);
    Serial.print("Address     "); Serial.println(uart.rx.addr);
    Serial.print("Command     "); Serial.println(uart.rx.cmd);
    Serial.print("Format      "); Serial.println(uart.rx.format);
}    


