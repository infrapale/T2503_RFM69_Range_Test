/*****************************************************************************
T2501_RFM69_BT_Monitor.git
*******************************************************************************

HW: Adafruit M0 RFM69 Feather or Arduino Pro Mini + RFM69

Send and receive data via UART

*******************************************************************************
https://github.com/infrapale/T2501_RFM69_BT_Monitor.git

https://learn.adafruit.com/adafruit-feather-m0-radio-with-rfm69-packet-radio
https://learn.sparkfun.com/tutorials/rfm69hcw-hookup-guide/all
*******************************************************************************

Module = 'X'
Address = '1'

******** UART ***************** Transmit Raw ********* Radio ********************
                                  --------
 <#X1T:Hello World>\n             |      |
--------------------------------->|      | Hello World
                                  |      |-------------------------------------->
                                  |      |
<---------------------------------|      |
                                  |      |<-------------------------------------
                                  --------

******** UART ***************** Transmit Node ********* Radio ********************
                                  --------
 <#X1N:RMH1;RKOK1;T;->\n          |      |
--------------------------------->|      | {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
                                  |      |-------------------------------------->
                                  |      |
<---------------------------------|      |
                                  |      |<-------------------------------------
                                  --------

******** UART *************** Check Radio Data ********* Radio ********************
                                  --------
<#X1A:>\n                         |      |
--------------------------------->|      | 
<#X1a:0>\n                        |      |
<---------------------------------|      | {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                                  |      |<-------------------------------------
<#X1A:>\n                         |      |
--------------------------------->|      | 
<#X1a:1>\n                        |      |
<---------------------------------|      | 
                                  |      |
                                  --------

******** UART ************ Read Radio Raw Data ********* Radio ********************
                                  --------
                                  |      | {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                                  |      |<-------------------------------------
<#X1R:>\n                         |      |
--------------------------------->|      | 
<#X1r:{"Z":"OD_1","S":"Temp",     |      |
"V":23.1,"R":"-"}>                |      |
<---------------------------------|      | 
                                  |      |
                                  --------

******** UART ************ Read Radio Node Data ********* Radio ********************
                                  --------
                                  |      | {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                                  |      |<-------------------------------------
<#X1O:>\n                         |      |
--------------------------------->|      | 
<#X1o:OD_1;Temp;23.1;->\n         |      |
<---------------------------------|      | 
                                  |      |
                                  --------

UART Commands
  UART_CMD_TRANSMIT_RAW   = 'T',
  UART_CMD_TRANSMIT_NODE  = 'N',
  UART_CMD_GET_AVAIL      = 'A',
  UART_CMD_READ_RAW       = 'R',
  UART_CMD_READ_NODE      = 'O' 

UART Replies
  UART_REPLY_AVAILABLE    = 'a',
  UART_REPLY_READ_RAW     = 'r',
  UART_REPLY_READ_NODE    = 'o' 

*******************************************************************************
Sensor Radio Message:   {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                        {"Z":"Dock","S":"T_dht22","V":"8.7","R":"-"}
Relay Radio Message     {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
Sensor Node Rx Mesage:  <#X1N:OD1;Temp;25.0;->
Sensor Node Rx Mesage:  <#X1N:Dock;T_bmp180;5.1;->
Sensor Node Rx Mesage:  <#X1N:Dock;T_Water;3.2;->

Relay Node Rx Mesage:   <#X1N:RMH1;RKOK1;T;->

Relay Mesage      <#R12=x>   x:  0=off, 1=on, T=toggle

*******************************************************************************
<,R,1,P,10,N,233,T,-88,>
<,R,2,P,20,N,7,T,-69,>


*******************************************************************************
**/

#include <Arduino.h>
#include "main.h"
#if defined(ADA_M0_RFM69) | defined(ADA_RFM69_WING)
#include <wdt_samd21.h>
#endif
#ifdef PRO_MINI_RFM69
// #include "avr_watchdog.h"
#endif
#include "secrets.h"
#include <RH_RF69.h>
#include "atask.h"
#include "watchdog.h"
#include "json.h"
#include "rfm69.h"
#include "uart.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"

#define ZONE  "OD_1"
//*********************************************************************************************
#define SERIAL_BAUD   9600
#define ENCRYPTKEY    RFM69_KEY   // defined in secret.h


RH_RF69         rf69(RFM69_CS, RFM69_INT);
RH_RF69         *rf69p;
module_data_st  me = {'X','1'};


#define NBR_TEST_MSG  4
#define LEN_TEST_MSG  64

// bus_msg = "<,R,{},P,{},#,{},>\n".format(msg['radio'], msg['pwr'], msg['nbr'])
const char test_msg[NBR_TEST_MSG][LEN_TEST_MSG] =
{  //12345678901234567890123456789012
    "<,B,R,1,P,14,#,345,S,-88,S,-99,>,\n",
    "<,B,R,2,P,5,#,346,S,-80,S,-99,>,\n",
    "<,B,R,3,P,20,#,347,S,-112,S,-99,>,\n",
    "<,B,R,1,P,10,#,348,S,-100,S,-99,>,\n"
};

void debug_print_task(void);
void run_100ms(void);
void send_test_data_task(void);
void rfm_receive_task(void); 


atask_st debug_print_handle        = {"Debug Print    ", 5000,0, 0, 255, 0, 1, debug_print_task};
atask_st clock_handle              = {"Tick Task      ", 100,0, 0, 255, 0, 1, run_100ms};
atask_st rfm_receive_handle        = {"Receive <- RFM ", 100,0, 0, 255, 0, 1, rfm_receive_task};

#ifdef SEND_TEST_MSG
atask_st send_test_data_handle     = {"Send Test Task ", 10000,0, 0, 255, 0, 1, send_test_data_task};
#endif

#ifdef PRO_MINI_RFM69
//AVR_Watchdog watchdog(4);
#endif

rfm_receive_msg_st  *receive_p;
rfm_send_msg_st     *send_p;
uart_msg_st         *uart_p;



void initialize_tasks(void)
{
  atask_initialize();
  atask_add_new(&debug_print_handle);
  atask_add_new(&clock_handle);
  atask_add_new(&rfm_receive_handle);

  #ifdef SEND_TEST_MSG
  atask_add_new(&send_test_data_handle);
  #endif
  #ifdef DEBUG_PRINT
  Serial.print("Tasks initialized "); Serial.println(TASK_NBR_OF);
  #endif
}


void setup() 
{
    //while (!Serial); // wait until serial console is open, remove if not tethered to computer
    delay(2000);
    Serial.begin(9600);
    //SerialX.begin(9600);

    #ifdef DEBUG_PRINT
    Serial.print("T2503_RFM69_Range_Test_Remote"); Serial.print(" Compiled: ");
    Serial.print(__DATE__); Serial.print(" ");
    Serial.print(__TIME__); Serial.println();
    #endif
    //watchdog_initialize(8000);

    
    uart_initialize();
    uart_p = uart_get_data_ptr();
    send_p = rfm_send_get_data_ptr();

    rf69p = &rf69;
    rfm69_initialize(&rf69);
    rfm_receive_initialize();
    io_initialize();
    // Hard Reset the RFM module
    
    initialize_tasks();

    #if defined(ADA_M0_RFM69) | defined(ADA_RFM69_WING)
    // Initialze WDT with a 2 sec. timeout
    // wdt_init ( WDT_CONFIG_PER_16K );
    #endif
    #ifdef PRO_MINI_RFM69
    //watchdog.set_timeout(4);
    #endif
}

void remote_state_machine(void)
{
    static uint16_t state = 0;
    static uint16_t new_state = 0;
    static uint32_t timeout = millis();

    if(state != new_state)
    {
        Serial.print(state); Serial.print("-->"); Serial.println(new_state);
        state = new_state;
    }
    switch(state)
    {
      case 0:
        new_state = 10;
        break;
      case 10:
        //Serial.println(test_msg[0]);
        rfm_send_radiate_msg( test_msg[0]);
        new_state = 20;
        break;
      case 20:
        timeout = millis() + 4000;
        new_state = 30;
        break;
      case 30:
        if (rfm_receive_message_is_avail())
        {
          rfm_receive_message();
          new_state = 40;
          Serial.println("Ack received!");
        }
        else
        {
          if (millis() > timeout)
          {
            Serial.println("Timeout");
            new_state = 100;
          }
        }

        break;
      case 40:
        timeout = millis() + 6000;
        new_state = 50;
        break;
      case 50:
        if (millis() > timeout)
        {
          new_state = 0;
        }
        break;
      case 100:
        new_state = 0;
        break;

    }

}

void loop() 
{
    //SerialX.println("Hello World"); delay(4000);
    atask_run();  
    remote_state_machine();
}


void rfm_receive_task(void) 
{
    //SerialX.print("@");
    uart_read_uart();    // if available -> uart->prx.str uart->rx.avail
    if(uart_p->rx.avail)
    {
        uart_parse_rx_frame();

        #ifdef DEBUG_PRINT
        Serial.println(uart_p->rx.str);
        uart_print_rx_metadata();
        #endif
        if ( uart_p->rx.status == STATUS_OK_FOR_ME)
        {
            uart_exec_cmnd(uart_p->rx.cmd);
        }
        uart_p->rx.avail = false;
    }
    rfm_receive_message();
    #if defined(ADA_M0_RFM69) | defined(ADA_RFM69_WING)
    //wdt_reset();
    #endif
    #ifdef PRO_MINI_RFM69
    // watchdog.clear();
    #endif
}


void run_100ms(void)
{
    static uint8_t ms100 = 0;
    if (++ms100 >= 10 )
    {
    }
    io_run_100ms();
}

void debug_print_task(void)
{
  #ifdef DEBUG_PRINT 
  atask_print_status(true);
  #endif
}

#ifdef SEND_TEST_MSG
void send_test_data_task(void)
{
    if  (send_test_data_handle.state >= NBR_TEST_MSG ) send_test_data_handle.state = 0;

    Serial.print(test_msg[send_test_data_handle.state]);
    send_test_data_handle.state++;
}
#endif

