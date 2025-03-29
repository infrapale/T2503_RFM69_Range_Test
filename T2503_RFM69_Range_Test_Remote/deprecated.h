
void rfm_send_parse_message(rfm_send_msg_st *rx_msg);

void rfm_send_transmit(rfm_send_msg_st *rx_msg);

void rfm_send_read_uart(void);





void xxsend_rfm69_message(String str)
{
  char cbuff[MAX_MESSAGE_LEN];
    str.toCharArray(cbuff,MAX_MESSAGE_LEN);
    //Serial.println(cbuff);
    rfm_send_radiate_msg(cbuff);
}


void xxrfm_send_parse_message(void)
{
    //rfm_send_msg_st *rx_msg = &send_msg; 
    bool do_continue = true;
    uint8_t len;
    send_msg.msg.trim();
    send_msg.len = send_msg.msg.length();
    if ((send_msg.msg.charAt(0) != '<') || 
        (send_msg.msg.charAt(1) != '#') || 
        (send_msg.msg.charAt(send_msg.len-1) != '>'))  do_continue = false;
    if (do_continue)
    {   
        Serial.print("Buffer frame is OK\n");
        send_msg.status = STATUS_CORRECT_FRAME;
        if ((send_msg.msg.charAt(6)  == '{') && 
            (send_msg.msg.charAt(send_msg.len-2) == '}'))
        {
            send_msg.rx_format = MSG_FORMAT_SENSOR_JSON;
        } 
        else 
        {
            send_msg.rx_format = MSG_FORMAT_RAW;
        }   
    }
    else send_msg.status = STATUS_INCORRECT_FRAME;

    if (do_continue)
    {
        send_msg.module = send_msg.msg.charAt(2);
        send_msg.addr   = send_msg.msg.charAt(3);
        if((send_msg.module == me.module) &&
           (send_msg.addr == me.addr))
        {
            send_msg.status = STATUS_OK_FOR_ME;
        }   
        else 
        {
            do_continue = false;
            send_msg.status = STATUS_NOT_FOR_ME;
        }
    }
    
    if (do_continue)
    { 
        switch( send_msg.msg.charAt(5))
        {
            case '-':
                send_msg.cmd_format = MSG_FORMAT_RAW;
                break;
            case 'S':
                send_msg.cmd_format = MSG_FORMAT_SENSOR_JSON;
                break;
        }
    }
    
    if (do_continue)
    {
        switch(send_msg.msg.charAt(4))
        {
            case '?':
              send_msg.cmd = UART_CMD_GET_AVAIL;
              break;
            case 'T':
              send_msg.cmd = UART_CMD_TRANSMIT;
              break;
            case 'R':
              send_msg.cmd = UART_CMD_READ_MSG;
              break;
            default:
              send_msg.cmd = UART_CMD_UNKNOWN; 
              send_msg.status = STATUS_UNKNOWN_COMMAND;
              break; 
        }
    }

    if (do_continue)
    {
      Serial.print("Good so far \n");
    }
    else 
    {
        Serial.print("NOK \n");
    }
    
    Serial.print("Status      "); Serial.println(send_msg.status);
    Serial.print("Module      "); Serial.println(send_msg.module);
    Serial.print("Address     "); Serial.println(send_msg.addr);
    Serial.print("Command     "); Serial.println(send_msg.cmd);
    Serial.print("Rx Format   "); Serial.println(send_msg.rx_format);
    Serial.print("Cmd Format  "); Serial.println(send_msg.cmd_format);
    Serial.print("Length      "); Serial.println(send_msg.len);
    Serial.print("Avail       "); Serial.println(send_msg.avail);
    

    switch(send_msg.cmd)
    {
        case UART_CMD_GET_AVAIL:
          {
              Serial.print("<#");
              Serial.print(me.module);
              Serial.print(me.addr);
              if(receive_msg.avail)
                  Serial.print('1');
              else
                  Serial.print('0');    
              Serial.println("F>");
          }
          break;
        case UART_CMD_TRANSMIT:
          break;
        case UART_CMD_READ_MSG:
            Serial.println(receive_msg.radio_msg);
            receive_msg.avail = false;
            break;
        case UART_CMD_UNKNOWN:
          break;
    }
    
}


void xxrfm_send_transmit(rfm_send_msg_st *rx_msg)
{
    char zone[8];
    char name[8]; 
    float value;
    char rem[8];

    String payload = rx_msg->msg.substring(6,rx_msg->len-1);
    switch(rx_msg->cmd_format)
    {
        case MSG_FORMAT_RAW:
            //<#X1T-{"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}>\n

            payload.toCharArray(send_msg.radio_msg, MAX_MESSAGE_LEN);
            rfm_send_radiate_msg(send_msg.radio_msg);
            break;
        case MSG_FORMAT_SENSOR_JSON:
            //<#X1TSOD_1;Temp;23.1;->\n
            //json_decode_to_tab_str(&payload, send_msg.radio_msg);
            rfm_send_radiate_msg(send_msg.radio_msg);
            break;
    }
}

void xxrfm_send_read_uart(void)
{
    send_msg.msg = Serial.readStringUntil('\n');
    if (send_msg.msg.length()> 0)
    {
        send_msg.msg.remove(send_msg.msg.length()-1);
        Serial.println(send_msg.msg);
        // rfm_send_parse_message();
        //parse_uart_rx_message(&uart_rx_msg);
        if (send_msg.status == STATUS_OK_FOR_ME)
        {
            switch (send_msg.cmd)
            {
                case UART_CMD_TRANSMIT:
                    rfm_send_transmit(&send_msg);
                    break;
                case UART_CMD_GET_AVAIL:
                    // <#Xu?F>\n  -> reply:  <#XuNF>\n 
                    if (send_msg.avail)
                    {
                        String reply = "<#Xu1F>";
                        reply.setCharAt(3, me.addr);
                        reply.setCharAt(4, '1');
                        reply.setCharAt(5, '1');
                        Serial.println(reply);
                    }
                    break;
                case UART_CMD_READ_MSG:
                    Serial.println(send_msg.radio_msg);
                    break;
            }
        }
        //send_rfm69_message(uart_rx_msg.msg);
    }
}

