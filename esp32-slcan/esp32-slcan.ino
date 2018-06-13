#include <ESP32CAN.h>           // v1.0.0     from https://github.com/nhatuan84/arduino-esp32-can-demo
#include <CAN_config.h>         // as above
#include <SPI.h>                // v1.0
#include <Wire.h>               // v1.0
#include <Adafruit_GFX.h>       // v1.2.2     from https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>   // v1.1.2     from https://github.com/adafruit/Adafruit_SSD1306
#include "BluetoothSerial.h"    // v1.0

// CURRENTLY WEMOS LOLIN32
// PIN 4  CANTX to transceiver
// PIN 5  CANRX to transceiver
// PIN 12 BLUETOOTH SWITCH
// PIN 14 NOT IN USE
// PIN 15 10k to ground to remove boot messages
// PIN 21 SDA (4.7k to 5v) for SSD1306
// PIN 22 SCL (4.7k to 5v) for SSD1306
// 3.3v to SSD1306 & CAN transceiver
// GND to SSD1306 & CAN transceiver

CAN_device_t CAN_cfg;
Adafruit_SSD1306 display(2);
BluetoothSerial SerialBT;

boolean working = false;
boolean bluetooth = false;
boolean timestamp = false;
int can_speed = 500;
int ser_speed = 500000;

const int SWITCH_PIN_A = 12;
//const int SWITCH_PIN_B = 14;

static uint8_t hexval[17] = "0123456789ABCDEF";

//----------------------------------------------------------------

void print_error(int canspeed) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.setTextColor(BLACK, WHITE);
  display.print("mintynet.com slcan");
  if (bluetooth) display.println(" B");
  else display.println();
  display.setTextColor(WHITE, BLACK);
  if (canspeed > 0) {
    display.println(" NOT AVAILABLE");
    display.print("  ser: ");
    display.print(ser_speed/1000);
    display.print("kbps");
    if (timestamp) { display.println("    T"); } else { display.println(""); }
    display.print("  can: ");
    display.print(canspeed);
    display.print("kbps");
    if (working) { display.println("   ON"); } else { display.println("   OFF"); }
  } else if (canspeed == -1) {
    display.println();
    display.println("  STOP FIRST");  
  } else {
    display.println();
    display.println("  CANNOT SEND");
  }
  display.display();
  delay(2500);
} //print_error()

//----------------------------------------------------------------

void print_status() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0,0);
  display.print("mintynet.com slcan");
  if (bluetooth) display.println(" B");
  else display.println();
  display.setTextColor(WHITE, BLACK);
  display.print("  ser: ");
  display.print(ser_speed/1000);
  display.print("kbps");
  if (timestamp) { display.println("    T"); } else { display.println(""); }
  display.print("  can: ");
  display.print(can_speed);
  display.print("kbps");
  if (working) { display.println("   ON"); } else { display.println("   OFF"); }
  display.display();
} //print_status()

//----------------------------------------------------------------

void slcan_ack()
{
  if (bluetooth) SerialBT.write('\r');
  else Serial.write('\r');
} // slcan_ack()

//----------------------------------------------------------------

void slcan_nack()
{
  if (bluetooth) SerialBT.write('\a');
  else Serial.write('\a');
} // slcan_nack()

//----------------------------------------------------------------

void pars_slcancmd(char *buf)
{                           // LAWICEL PROTOCOL
  switch (buf[0]) {
    case 'O':               // OPEN CAN
      working=true;
      ESP32Can.CANInit();
      print_status();
      slcan_ack();
      break;
    case 'C':               // CLOSE CAN
      working=false;
      ESP32Can.CANStop();
      print_status();
      slcan_ack();
      break;
    case 't':               // send std frame
      send_canmsg(buf,false,false);
      slcan_ack();
      break;
    case 'T':               // send ext frame
      send_canmsg(buf,false,true);
      slcan_ack();
      break;
    case 'r':               // send std rtr frame
      send_canmsg(buf,true,false);
      slcan_ack();
      break;
    case 'R':               // send ext rtr frame
      send_canmsg(buf,true,true);
      slcan_ack();
      break;
    case 'Z':               // ENABLE TIMESTAMPS
      switch (buf[1]) {
        case '0':           // TIMESTAMP OFF  
          timestamp = false;
          print_status();
          slcan_ack();
          break;
        case '1':           // TIMESTAMP ON
          timestamp = true;
          print_status();
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
      if (working) {
        print_error(-1);
        print_status();
        break;
      }
      switch (buf[1]) {
        case '0': // 10k  
          print_error(10);
          print_status();
          slcan_nack();
          break;
        case '1': // 20k
          print_error(20);
          print_status();
          slcan_nack();
          break;
        case '2': // 50k
          print_error(50);
          print_status();
          slcan_nack();
          break;
        case '3': // 100k
          CAN_cfg.speed=CAN_SPEED_100KBPS;
          can_speed = 100;
          print_status();
          slcan_ack();
          break;
        case '4': // 125k
          CAN_cfg.speed=CAN_SPEED_125KBPS;
          can_speed = 125;
          print_status();
          slcan_ack();
          break;
        case '5': // 250k
          CAN_cfg.speed=CAN_SPEED_250KBPS;
          can_speed = 250;
          print_status();
         slcan_ack();
          break;
        case '6': // 500k
          CAN_cfg.speed=CAN_SPEED_500KBPS;
          can_speed = 500;
          print_status();
          slcan_ack();
          break;
        case '7': // 800k
          CAN_cfg.speed=CAN_SPEED_800KBPS;
          can_speed = 800;
          print_status();
          slcan_nack();
          break;
        case '8': // 1000k
          CAN_cfg.speed=CAN_SPEED_1000KBPS;
          can_speed = 1000;
          print_status();
          slcan_ack();
          break;
        default:
          slcan_nack();
          break;
      }
      break;
    case 'F':               // STATUS FLAGS
      if (bluetooth) SerialBT.print("F00");
      else Serial.print("F00");
      slcan_ack();
      break;
    case 'V':               // VERSION NUMBER
      if (bluetooth) SerialBT.print("V1234");
      else Serial.print("V1234");
      slcan_ack();
      break;
    case 'N':               // SERIAL NUMBER
      if (bluetooth) SerialBT.print("N2208");
      else Serial.print("N2208");
      slcan_ack();
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
  if (bluetooth) {
    if ((ser_length = SerialBT.available()) > 0) {
      for (int i = 0; i < ser_length; i++) {
        char val = SerialBT.read();
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
  } else {
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
  }
} // transfer_tty2can()

//----------------------------------------------------------------

void transfer_can2tty()
{
  CAN_frame_t rx_frame;
  String command = "";
  long time_now = 0;
  //receive next CAN frame from queue
  if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE) {
    //do stuff!
    if(working) {
      if(rx_frame.FIR.B.FF==CAN_frame_ext) {
        if (rx_frame.FIR.B.RTR==CAN_RTR) {
          command = command + "R";
        } else {
          command = command + "T";
        }
        command = command + char(hexval[ (rx_frame.MsgID>>28)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>24)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>20)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>16)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>12)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>8)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>4)&15]);
        command = command + char(hexval[ rx_frame.MsgID&15]);
        command = command + char(hexval[ rx_frame.FIR.B.DLC ]);
      } else {
        if (rx_frame.FIR.B.RTR==CAN_RTR) {
          command = command + "r";
        } else {
          command = command + "t";
        }
        command = command + char(hexval[ (rx_frame.MsgID>>8)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>4)&15]);
        command = command + char(hexval[ rx_frame.MsgID&15]);
        command = command + char(hexval[ rx_frame.FIR.B.DLC ]);
      }
      for(int i = 0; i < rx_frame.FIR.B.DLC; i++){
        command = command + char(hexval[ rx_frame.data.u8[i]>>4 ]);
        command = command + char(hexval[ rx_frame.data.u8[i]&15 ]);
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
    if (bluetooth) SerialBT.print(command);
    else Serial.print(command);
    }
  }
} // transfer_can2tty()

//----------------------------------------------------------------

void send_canmsg(char *buf, boolean rtr, boolean ext) {
  if (!working) {
    print_error(0);
    print_status();
  } else {
    CAN_frame_t tx_frame;
    int msg_id = 0;
    int msg_ide = 0;
    if (rtr) {
      if (ext) {
        sscanf(&buf[1], "%04x%04x", &msg_ide, &msg_id);
        tx_frame.FIR.B.RTR = CAN_RTR;
        tx_frame.FIR.B.FF = CAN_frame_ext;
      } else {
        sscanf(&buf[1], "%03x", &msg_id);
        tx_frame.FIR.B.RTR = CAN_RTR;
        tx_frame.FIR.B.FF = CAN_frame_std;
      }
    } else {
      if (ext) {
        sscanf(&buf[1], "%04x%04x", &msg_ide, &msg_id);
        tx_frame.FIR.B.FF = CAN_frame_ext;
      } else {
        sscanf(&buf[1], "%03x", &msg_id);
        tx_frame.FIR.B.FF = CAN_frame_std;
      }
    }
    tx_frame.MsgID = msg_ide*65536 + msg_id;
    int msg_len = 0;
    if (ext) {
      sscanf(&buf[9], "%01x", &msg_len);
    } else {
      sscanf(&buf[4], "%01x", &msg_len);
    }
    tx_frame.FIR.B.DLC = msg_len;
    int candata = 0;
    if (ext) {
      for (int i = 0; i < msg_len; i++) {
        sscanf(&buf[10 + (i*2)], "%02x", &candata);
        tx_frame.data.u8[i] = candata;
      }
    } else {
      for (int i = 0; i < msg_len; i++) {
        sscanf(&buf[5 + (i*2)], "%02x", &candata);
        tx_frame.data.u8[i] = candata;
      }
    }
    ESP32Can.CANWriteFrame(&tx_frame);
  }
} // send_canmsg()

//----------------------------------------------------------------

void setup() {
  //Wire.begin(21,22);
  pinMode(SWITCH_PIN_A,INPUT_PULLUP);
  //pinMode(SWITCH_PIN_B,INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  Serial.begin(ser_speed);
  delay(100);
  //Serial.println("esp32can-test:iotsharing.com CAN demo");
  CAN_cfg.speed=CAN_SPEED_500KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_4;
  CAN_cfg.rx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
  //display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15,10);
  display.clearDisplay();
  display.println("mintynet");
  display.display();
  delay(2000);
  boolean switchA = digitalRead(SWITCH_PIN_A);
  if (!switchA) {
    SerialBT.begin("SLCAN");
    bluetooth = true;
  } else {
    bluetooth = false;
  }
  if (bluetooth) Serial.println("BLUETOOTH ON");
  print_status();
} // setup()

//----------------------------------------------------------------

void loop() {
  transfer_can2tty();
  transfer_tty2can();
} // loop();
