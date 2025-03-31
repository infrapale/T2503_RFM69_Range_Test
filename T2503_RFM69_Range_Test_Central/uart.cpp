#include "main.h"
#include "uart.h"
#include "json.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"

#define  FRAME_START  ('<')
#define  FRAME_END    ('>')

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


msg_type_et uart_get_msg_type(char msg_type)
{
    msg_type_et mtype = MSG_UNDEFINED;
    if  ((msg_type == MSG_SET_BASE_NODE) ||
        (msg_type == MSG_ACK_BASE_TO_REMOTE) ||
        (msg_type == MSG_SEND_BASE_TO_LOGGER) ||
        (msg_type == MSG_SET_REMOTE_NODE) ||
        (msg_type == MSG_SEND_REMOTE_TO_BASE))

    {
        mtype = msg_type;
    }    
    return mtype;
}

void uart_report_radio_msg(char *radio_receive_msg, int rssi)
{
    String msg_str = radio_receive_msg;
    msg_type_et msg_type = MSG_UNDEFINED;
    bool do_report_msg = false;

    // Serial.print(msg_str); Serial.print(" - "); Serial.println(rssi);
    msg_str.trim();
    if ((msg_str.charAt(0) == FRAME_START) &&
        (msg_str.charAt(msg_str.length()-1) == FRAME_END)) 
    {
        uint8_t end_pos = msg_str.indexOf('>');
        String str = msg_str.substring(0,end_pos+1);

        msg_type = uart_get_msg_type(str.charAt(2));
 
        uint8_t base_tag   = str.indexOf('S');
        uint8_t remote_tag = str.indexOf('T');

        switch(me.node_type)
        {
            case  NODE_TYPE_BASE:
                if  (msg_type == MSG_SEND_REMOTE_TO_BASE)
                {
                    String str1 = str.substring(0,base_tag-1);
                    str1 += ",S,";
                    str1 += rssi;
                    str1 += str.substring(remote_tag-1);  
                    Serial.println(str1);                 
                }    
                break;
            case NODE_TYPE_REMOTE:
                if  (msg_type == MSG_ACK_BASE_TO_REMOTE)
                {
                    String str1 = str.substring(0,remote_tag-1);
                    str1 += ",T,";
                    str1 += rssi;
                    str1 += ",>";
                    Serial.println(str1);
                }
                break;
        }
    } 
}


// Parse message recived by the UART 
void uart_parse_rx_frame(void)
{
    //rfm_send_msg_st *rx_msg = &send_msg; 
    bool do_continue = true;
    uint8_t len;
    uint8_t step_cntr = 0;
    uart.rx.str.trim();
    uart.rx.len = uart.rx.str.length();
    uart.rx.msg_type = MSG_UNDEFINED;
  
    if (uart.rx.len < 20)
    {
        do_continue = false;
        step_cntr++;
    }  

    if ((uart.rx.str.charAt(0) == '<') && 
            (uart.rx.str.charAt(uart.rx.len-1) == '>'))  
        step_cntr++;
    else    
        do_continue = false;     


    if (do_continue)
    {
        uart.rx.msg_type = uart_get_msg_type(uart.rx.str.charAt(2));
        if (uart.rx.msg_type == MSG_UNDEFINED ) 
            do_continue = false;
        else
            step_cntr = 10;
    }

    if (do_continue)
    {   
        step_cntr++;
        int16_t radio_pos = uart.rx.str.indexOf('R') + 2;
        if(radio_pos > 0)
            uart.rx.radio =  uart.rx.str.charAt(radio_pos) - '0';
        else 
          do_continue = false;    
        #ifdef DEBUG_PRINT
        Serial.print("Radio ="); Serial.println(uart.rx.radio);
        #endif
    }
    if (do_continue)
    {
        step_cntr++;
        int16_t pwr_pos = uart.rx.str.indexOf('P') + 2;
        if(pwr_pos > 0)
        {
            int16_t pwr_pos2 = uart.rx.str.indexOf(',',pwr_pos);
            if (pwr_pos2 > 0)
            {
                uart.rx.radio_pwr =  uart.rx.str.substring(pwr_pos,pwr_pos2).toInt();
                //Serial.print("Power value:"); Serial.println(uart.rx.str.substring(pwr_pos,pwr_pos2));
            }
            else
              do_continue = false;
        }
        else 
          do_continue = false;    
    }

    if (do_continue)
    {
        step_cntr = 20;
        switch(me.node_type)
        {
            case  NODE_TYPE_BASE:
                uart.rx.radio_pwr = 20;  //allways transmit with full power
                switch(uart.rx.msg_type) 
                {
                    case MSG_ACK_BASE_TO_REMOTE:
                        if(me.radio != uart.rx.radio ) do_continue = false;
                        break;
                    case MSG_SEND_BASE_TO_LOGGER:
                        break;
                    case MSG_SET_BASE_NODE:
                        me.node_type = NODE_TYPE_BASE;
                        break;
                    default:
                        do_continue = false;
                }
                break;
            case NODE_TYPE_REMOTE:
                switch(uart.rx.msg_type)
                {
                    case MSG_SEND_REMOTE_TO_BASE:
                        if(me.radio != uart.rx.radio ) do_continue = false;
                        break;
                    case MSG_SET_REMOTE_NODE:
                        me.node_type = NODE_TYPE_REMOTE;
                        break;    
                    default:
                        do_continue = false;  
                        break;
                } 
                break;
            case NODE_TYPE_UNDEFINED:
                switch(uart.rx.msg_type)
                {
                    case MSG_SET_BASE_NODE:
                        me.node_type = NODE_TYPE_BASE;
                        break;
                    case MSG_SET_REMOTE_NODE:
                        me.node_type = NODE_TYPE_REMOTE;
                        break;    
                    default:
                        do_continue = false;  
                        break;
                } 
                break;    
            default:
                do_continue = false;
                break;
        }
    }    

    if (!do_continue) 
    {
        step_cntr = 30;
        uart.rx.msg_type = MSG_UNDEFINED;
    }

    //if (do_continue)
    {   
        #ifdef DEBUG_PRINT
        Serial.print("Node type: "); Serial.println(me.node_type);
        Serial.print("Buffer frame is OK\n");
        Serial.print("Msg Type = "); Serial.println(uart.rx.msg_type);
        Serial.print("Step Cntr = "); Serial.println(step_cntr);
        
        #endif
    }
}





void uart_rx_send_rfm_from_raw(void)
{
    //String payload = uart.rx.str.substring(6,uart.rx.len - 1);
    String payload = uart.rx.str;
    payload.trim();
    memset(uart.rx.radio_msg,0x00, MAX_MESSAGE_LEN);
    payload.toCharArray(uart.rx.radio_msg, MAX_MESSAGE_LEN);
    #ifdef DEBUG_PRINT
    Serial.print(payload); Serial.print(" > char array: ");
    Serial.println(uart.rx.radio_msg);
    #endif
    rfm_send_radiate_msg(&uart);
}






void uart_print_rx_metadata(void)
{
    Serial.print("Msg Type    "); Serial.println(uart.rx.msg_type);
    Serial.print("Radio       "); Serial.println(uart.rx.radio);
    Serial.print("Length      "); Serial.println(uart.rx.len);
    Serial.print("Avail       "); Serial.println(uart.rx.avail);
    Serial.print("Module      "); Serial.println(uart.rx.module);
    Serial.print("Address     "); Serial.println(uart.rx.addr);
    Serial.print("Command     "); Serial.println(uart.rx.cmd);
    Serial.print("Format      "); Serial.println(uart.rx.format);

}    


