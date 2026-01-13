#include "driver/twai.h"
#include "Arduino.h"
// FOR USE WITH ESP32 Board def 3.x as of Jan 2026
// NO BLUETOOTH CAN

// CURRENTLY ESP32S3 Dev Module Board Definition
// NOTE ESP32C3 Does NOT seem to work with TWAI module, it initiates but no CAN is seen.
// USB CDC on boot enabled
// PIN 4  CANTX 
// PIN 5  CANRX 
// 3.3v 

#define CAN_TX            4
#define CAN_RX            5

boolean working           = false;
boolean timestamp         = false;
boolean cr                = false;
int can_speed             = 500;
int ser_speed             = 500000;

static uint8_t hexval[17] = "0123456789ABCDEF";

QueueHandle_t             tx_queue;
QueueHandle_t             rx_queue;
twai_general_config_t     g_config;
twai_timing_config_t      t_config;
twai_filter_config_t      f_config;

//----------------------------------------------------------------

void slcan_ack()
{
  Serial.write('\r');
} // slcan_ack()

//----------------------------------------------------------------

void slcan_nack()
{
  Serial.write('\a');
} // slcan_nack()

//----------------------------------------------------------------

void pars_slcancmd(char *buf)
{                           // LAWICEL PROTOCOL
  switch (buf[0]) {
    case 'O':               // OPEN CAN
      if (!working) {
        g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX, (gpio_num_t)CAN_RX, TWAI_MODE_NORMAL);  // TWAI_MODE_NORMAL, TWAI_MODE_NO_ACK, TWAI_MODE_LISTEN_ONLY
        f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
        if(twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
          if (twai_start() != ESP_OK) {
            slcan_nack();
            working=false;
          } else {
            working=true;
            slcan_ack();
            delay(100);
          }
        } else {
          slcan_nack();
        }
      }
      break;
    case 'C':               // CLOSE CAN
      if(working) {
        if(twai_stop() != ESP_OK) {
          slcan_nack();
        } else {
          if(twai_driver_uninstall()!=ESP_OK) {
            slcan_nack();
          } else {
            working=false;
            slcan_ack();
          }
        }
      }
      break;
    case 't':               // SEND STD FRAME
      send_canmsg(buf,false,false);
      slcan_ack();
      break;
    case 'T':               // SEND EXT FRAME
      send_canmsg(buf,false,true);
      slcan_ack();
      break;
    case 'r':               // SEND STD RTR FRAME
      send_canmsg(buf,true,false);
      slcan_ack();
      break;
    case 'R':               // SEND EXT RTR FRAME
      send_canmsg(buf,true,true);
      slcan_ack();
      break;
    case 'Z':               // ENABLE TIMESTAMPS
      switch (buf[1]) {
        case '0':           // TIMESTAMP OFF  
          timestamp = false;
          slcan_ack();
          break;
        case '1':           // TIMESTAMP ON
          timestamp = true;
          slcan_ack();
          break;
        default:
          break;
      }
      break;
    case 'M':               ///set ACCEPTANCE CODE ACn REG
      slcan_ack();
      break;
    case 'm':               // set ACCEPTANCE CODE AMn REG
      slcan_ack();
      break;
    case 's':               // CUSTOM CAN bit-rate
      slcan_nack();
      break;
    case 'S':               // CAN bit-rate
      switch (buf[1]) {
        case '0':           // 10k  
          can_speed = 10;
          t_config = TWAI_TIMING_CONFIG_10KBITS();
          slcan_ack();
          break;
        case '1':           // 20k
          can_speed = 20;
          t_config = TWAI_TIMING_CONFIG_20KBITS();
          slcan_ack();
          break;
        case '2':           // 50k
          can_speed = 50;
          t_config = TWAI_TIMING_CONFIG_50KBITS();
          slcan_ack();
          break;
        case '3':           // 100k
          can_speed = 100;
          t_config = TWAI_TIMING_CONFIG_100KBITS();
          slcan_ack();
          break;
        case '4':           // 125k
          can_speed = 125;
          t_config = TWAI_TIMING_CONFIG_125KBITS();
          slcan_ack();
          break;
        case '5':           // 250k
          can_speed = 250;
          t_config = TWAI_TIMING_CONFIG_250KBITS();
          slcan_ack();
          break;
        case '6':           // 500k
          can_speed = 500;
          t_config = TWAI_TIMING_CONFIG_500KBITS();
          slcan_ack();
          break;
        case '7': // 800k
          can_speed = 800;
          t_config = TWAI_TIMING_CONFIG_800KBITS();
          slcan_ack();
          break;
        case '8':           // 1000k
          can_speed = 1000;
          t_config = TWAI_TIMING_CONFIG_1MBITS();
          slcan_ack();
          break;
        default:
          slcan_nack();
          break;
      }
      break;
    case 'F':               // STATUS FLAGS
      Serial.print("F00");
      slcan_ack();
      break;
    case 'V':               // VERSION NUMBER
      Serial.print("V1234");
      slcan_ack();
      break;
    case 'N':               // SERIAL NUMBER
      Serial.print("N2233");
      slcan_ack();
      break;
    case 'l':               // (NOT SPEC) TOGGLE LINE FEED ON SERIAL
      cr = !cr;
      slcan_nack();
      break;
    case 'h':               // (NOT SPEC) HELP SERIAL
      Serial.println();
      Serial.println("mintynet.com - slcan esp32");
      Serial.println();
      Serial.println("O\t=\tStart slcan");
      Serial.println("C\t=\tStop slcan");
      Serial.println("t\t=\tSend std frame");
      Serial.println("r\t=\tSend std rtr frame");
      Serial.println("T\t=\tSend ext frame");
      Serial.println("R\t=\tSend ext rtr frame");
      Serial.println("Z0\t=\tTimestamp Off");
      Serial.println("Z1\t=\tTimestamp On");
      Serial.println("snn\t=\tSpeed 0xnnk N/A");
      Serial.println("S0\t=\tSpeed 10k");
      Serial.println("S1\t=\tSpeed 20k");
      Serial.println("S2\t=\tSpeed 50k");
      Serial.println("S3\t=\tSpeed 100k");
      Serial.println("S4\t=\tSpeed 125k");
      Serial.println("S5\t=\tSpeed 250k");
      Serial.println("S6\t=\tSpeed 500k");
      Serial.println("S7\t=\tSpeed 800k");
      Serial.println("S8\t=\tSpeed 1000k");
      Serial.println("F\t=\tFlags N/A");
      Serial.println("N\t=\tSerial No");
      Serial.println("V\t=\tVersion");
      Serial.println("-----NOT SPEC-----");
      Serial.println("h\t=\tHelp");
      Serial.print("l\t=\tToggle CR ");
      if (cr) {
        Serial.println("ON");
      } else {
        Serial.println("OFF");
      }
      Serial.print("CAN_SPEED:\t");
      switch(can_speed) {
        case 10:
          Serial.print("10");
          break;
        case 20:
          Serial.print("20");
          break;
        case 50:
          Serial.print("50");
          break;
        case 100:
          Serial.print("100");
          break;
        case 125:
          Serial.print("125");
          break;
        case 250:
          Serial.print("250");
          break;
        case 500:
          Serial.print("500");
          break;
        case 800:
          Serial.print("800");
          break;
        case 1000:
          Serial.print("1000");
          break;
        default:
          break;
      }
      Serial.print("kbps");
      if (timestamp) {
        Serial.print("\tT");
      }
      if (working) {
        Serial.print("\tON");
      } else {
        Serial.print("\tOFF");
      }
      Serial.println();
      slcan_nack();
      break;
    default:
      slcan_nack();
      break;
  }
} // pars_slcancmd()

//----------------------------------------------------------------

void transfer_tty2can()
{
  int ser_length;
  static char cmdbuf[32];
  static int cmdidx = 0;
  if ((ser_length = Serial.available()) > 0) {
    for (int i = 0; i < ser_length; i++) {
      char val = Serial.read();
      cmdbuf[cmdidx++] = val;
      if (cmdidx == 32)
      {
        slcan_nack();
        cmdidx = 0;
      } else if (val == '\r')
      {
        cmdbuf[cmdidx] = '\0';
        pars_slcancmd(cmdbuf);
        cmdidx = 0;
      }
    }
  }
} // transfer_tty2can()

//----------------------------------------------------------------

void transfer_can2tty()
{
  twai_message_t rx_frame;
  String command = "";
  long time_now = 0;
  //receive next CAN frame from queue
  uint32_t result = twai_receive(&rx_frame,1);
  if(result==ESP_OK) {
    //do stuff!
    if(working) {
      if(rx_frame.extd==1) {
        if (rx_frame.rtr==1) {
          command = command + "R";
        } else {
          command = command + "T";
        }
        command = command + char(hexval[ (rx_frame.identifier>>28)&1]);
        command = command + char(hexval[ (rx_frame.identifier>>24)&15]);
        command = command + char(hexval[ (rx_frame.identifier>>20)&15]);
        command = command + char(hexval[ (rx_frame.identifier>>16)&15]);
        command = command + char(hexval[ (rx_frame.identifier>>12)&15]);
        command = command + char(hexval[ (rx_frame.identifier>>8)&15]);
        command = command + char(hexval[ (rx_frame.identifier>>4)&15]);
        command = command + char(hexval[ rx_frame.identifier&15]);
        command = command + char(hexval[ rx_frame.data_length_code ]);
      } else {
        if (rx_frame.rtr==1) {
          command = command + "r";
        } else {
          command = command + "t";
        }
        command = command + char(hexval[ (rx_frame.identifier>>8)&15]);
        command = command + char(hexval[ (rx_frame.identifier>>4)&15]);
        command = command + char(hexval[ rx_frame.identifier&15]);
        command = command + char(hexval[ rx_frame.data_length_code ]);
      }
      for(int i = 0; i < rx_frame.data_length_code; i++){
        command = command + char(hexval[ rx_frame.data[i]>>4 ]);
        command = command + char(hexval[ rx_frame.data[i]&15 ]);
        //printf("%c\t", (char)rx_frame.data.u8[i]);
      }
    if (timestamp) {
      time_now = millis() % 60000;
      command = command + char(hexval[ (time_now>>12)&15 ]);
      command = command + char(hexval[ (time_now>>8)&15 ]);
      command = command + char(hexval[ (time_now>>4)&15 ]);
      command = command + char(hexval[ time_now&15 ]);
    }
    command = command + '\r';
    Serial.print(command);
    if (cr) Serial.println("");
    }
  }
} // transfer_can2tty()

//----------------------------------------------------------------

void send_canmsg(char *buf, boolean rtr, boolean ext) {
  if (!working) {

  } else {
    twai_message_t tx_frame;
    int msg_id = 0;
    int msg_ide = 0;
    if (rtr) {
      if (ext) {
        sscanf(&buf[1], "%04x%04x", &msg_ide, &msg_id);
        tx_frame.rtr = 1;
        tx_frame.extd = 1;
      } else {
        sscanf(&buf[1], "%03x", &msg_id);
        tx_frame.rtr = 1;
        tx_frame.extd = 1;
      }
    } else {
      if (ext) {
        sscanf(&buf[1], "%04x%04x", &msg_ide, &msg_id);
        tx_frame.rtr = 0;
        tx_frame.extd = 1;
      } else {
        sscanf(&buf[1], "%03x", &msg_id);
        tx_frame.rtr = 0;
        tx_frame.extd = 0;
      }
    }
    tx_frame.identifier = msg_ide*65536 + msg_id;
    int msg_len = 0;
    if (ext) {
      sscanf(&buf[9], "%01x", &msg_len);
    } else {
      sscanf(&buf[4], "%01x", &msg_len);
    }
    tx_frame.data_length_code = msg_len;
    int candata = 0;
    if (ext) {
      for (int i = 0; i < msg_len; i++) {
        sscanf(&buf[10 + (i*2)], "%02x", &candata);
        tx_frame.data[i] = candata;
      }
    } else {
      for (int i = 0; i < msg_len; i++) {
        sscanf(&buf[5 + (i*2)], "%02x", &candata);
        tx_frame.data[i] = candata;
      }
    }
    twai_transmit(&tx_frame,1);
  }
} // send_canmsg()

//----------------------------------------------------------------

void setup() {
  Serial.begin(ser_speed);
  while(!Serial);
  t_config = TWAI_TIMING_CONFIG_500KBITS();
  delay(1000);
} // setup()

//----------------------------------------------------------------

void loop() {
  if(working) {
    transfer_can2tty();
  }
  transfer_tty2can();
} // loop();



