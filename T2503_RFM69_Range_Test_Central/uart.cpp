#include "main.h"
#include "uart.h"
#include "json.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"

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
    Serial.print(msg_str); Serial.print(" - ");
    Serial.println(rssi);
}

void uart_parse_rx_frame(void)
{
    //rfm_send_msg_st *rx_msg = &send_msg; 
    bool do_continue = true;
    uint8_t len;
    uart.rx.str.trim();
    uart.rx.len = uart.rx.str.length();
    if ((uart.rx.str.charAt(0) != '<') || 
        (uart.rx.str.charAt(1) != '#') || 
        (uart.rx.str.charAt(5) != ':') || 
        (uart.rx.str.charAt(uart.rx.len-1) != '>'))  do_continue = false;
    if (do_continue)
    {   
        #ifdef DEBUG_PRINT
        Serial.print("Buffer frame is OK\n");
        #endif

        uart.rx.status = STATUS_CORRECT_FRAME;
        if ((uart.rx.str.charAt(6)  == '{') && 
            (uart.rx.str.charAt(uart.rx.len-2) == '}'))
        {
            uart.rx.format = MSG_FORMAT_SENSOR_JSON;
        } 
        else 
        {
            uart.rx.format = MSG_FORMAT_RAW;
        }   
        uart.rx.module  = uart.rx.str.charAt(2);
        uart.rx.addr    = uart.rx.str.charAt(3);
        uart.rx.cmd     = (uart_cmd_et)uart.rx.str.charAt(4);

        if((uart.rx.module == me.module) && (uart.rx.addr == me.addr))
            uart.rx.status = STATUS_OK_FOR_ME;
        else 
            uart.rx.status = STATUS_NOT_FOR_ME;
    }
    else uart.rx.status = STATUS_INCORRECT_FRAME;
}

void uart_build_node_from_rx_str(void)
{
    uint8_t indx1;
    uint8_t indx2;
    indx1 = 0;  //uart.rx.str.indexOf(':')
    indx2 = uart.rx.str.indexOf(';');
    uart.node.zone = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    uart.node.name = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    uart.node.value = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    uart.node.remark = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    
}





void uart_build_node_tx_str(void)
{
    rfm_receive_msg_st *receive_p = rfm_receive_get_data_ptr();
    // <#X1N:RMH1;RKOK1;T;->\n   -> {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
    uart.rx.str = (char*) receive_p->radio_msg;  
    uart.tx.str = "<#X1a:";
    json_pick_data_from_rx(&uart);
    #ifdef DEBUG_PRINT
    Serial.print("radio_msg: ");
    Serial.println(uart.rx.str);  
    Serial.println(uart.node.zone);
    Serial.println(uart.node.name);
    Serial.println(uart.node.value);
    Serial.println(uart.node.remark);
    #endif
    uart.tx.str += uart.node.zone;
    uart.tx.str += ';';
    uart.tx.str += uart.node.name;
    uart.tx.str += ';';
    uart.tx.str += uart.node.value;
    uart.tx.str += ';';
    uart.tx.str += uart.node.remark;
    uart.tx.str += '>';
}

void uart_build_raw_tx_str(void)
{
    rfm_receive_msg_st *receive_p = rfm_receive_get_data_ptr();
    uart.tx.str = "<#X1r:";
    uart.tx.str += (char*) receive_p->radio_msg;
    uart.tx.str += '>';
}

void uart_rx_send_rfm_from_raw(void)
{
    String payload = uart.rx.str.substring(6,uart.rx.len - 1);
    payload.toCharArray(uart.rx.radio_msg, MAX_MESSAGE_LEN);
    rfm_send_radiate_msg(uart.rx.radio_msg);
}

void uart_rx_send_rfm_from_node(void)
{
    uart.rx.str = uart.rx.str.substring(6,uart.rx.len - 1);
    uart_build_node_from_rx_str();
    rfm_send_msg_st *send_p = rfm_send_get_data_ptr();
    json_convert_uart_node_to_json(send_p->radio_msg, &uart);
    rfm_send_radiate_msg(send_p->radio_msg);
}

void uart_exec_cmnd(uart_cmd_et ucmd)
{
    String str = "<#Xux:0>";
    str[3] = me.addr;
    switch(ucmd)
    {
        case UART_CMD_TRANSMIT_RAW:
            io_led_flash(LED_INDX_RED, 10);
            uart_rx_send_rfm_from_raw();
            break;
        case UART_CMD_TRANSMIT_NODE:
            io_led_flash(LED_INDX_RED, 20);
            uart_rx_send_rfm_from_node();
            break;
        case UART_CMD_GET_AVAIL:
            str[4] = UART_REPLY_AVAILABLE;
            if(rfm_receive_message_is_avail()) str[6] = '1';
            SerialX.println(str);
            break;
        case UART_CMD_READ_RAW:
            rfm_receive_clr_message_flag();
            uart_build_raw_tx_str();
            SerialX.println(uart.tx.str);          
            break;
        case UART_CMD_READ_NODE:
            rfm_receive_clr_message_flag();
            uart_build_node_tx_str();
            SerialX.println(uart.tx.str);          
          break;

    }
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


