

/*
     
 * CAN Bus options for the ESP32:
   (A)  http://www.iotsharing.com/2017/09/how-to-use-arduino-esp32-can-interface.html refers to   
      https://github.com/nhatuan84/esp32-can-protocol-demo  but this is quite old and does not
      use interrupts. So it has the risk to run into the fifo-full-bug.
   (B) The Arduino Library manager contains ACAN_ESP32, which refers to
      https://github.com/pierremolinaro/acan-esp32 This is still active (last commit 2022).
      But no interrupts uses. Risk to run into the fifo-full-bug.
   (C) Arduino Library manager: CAN by Sandeep Mistry, https://github.com/sandeepmistry/arduino-CAN
   (D) Arduino Library manager: CAN Adafruit fork by Sandeep Mistry  , see https://github.com/adafruit/arduino-CAN
       It looks promising, because it uses a callback (interrupt?), but also
       crash is reported on ESP32 (https://github.com/adafruit/arduino-CAN/issues/13)

  We go with option (D).
 */


#define USE_CAN_INPUT /* if this is defined, the board uses input fom CAN instead of serial line */

#ifdef USE_CAN_INPUT
  #include <CAN.h>
  #define CAN_RX_PIN 22 /* The default would be 4, but this is occupied by the OLED */
  #define CAN_TX_PIN 5
#else
  #define USE_SERIAL_INPUT
  /* The pins for the second serial port. The first one is used by the USB interface (programming and Arduino console). */
  /* see https://quadmeup.com/arduino-esp32-and-3-hardware-serial-ports/ */
  #include <HardwareSerial.h>
  HardwareSerial mySerial(1); /* 0 would be the USB-Serial, e.g. for Serial.print. 1 is the Serial2. 
                               The pins can be freely chosen, see below. */
  #define TX2_PIN 17 /* 17 works fine on the HELTEC ESP32 OLED module for TX */
  #define RX2_PIN 18 /* 16 does not work (perhaps used for other purpose on the board), 18 works,  */
#endif
 
#define USE_OLED

#define OLED_FOUR_LINES
//#define OLED_THREE_LINES
//#define OLED_SHOW_DEBUG_COUNTER

#ifdef USE_OLED
  // For a connection via I2C using the Arduino Wire include:
  #include <Wire.h>               
  #include "HT_SSD1306Wire.h"

  SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

#endif

int n_loops;
int nDebug;
#ifdef USE_SERIAL_INPUT
 #define SER_INBUFFER_SIZE 20
 uint8_t ser_iWrite=0;
 uint8_t ser_iWrite2=0;
 
 char ser_inbuffer[SER_INBUFFER_SIZE];
 char ser_inbuffer2[SER_INBUFFER_SIZE];
#endif
 String line1 = "Hello";
 String line2 = "Init...";
 String line3 = "...waiting";
 String line4 = "for data...";
 

#ifdef USE_CAN_INPUT
uint32_t CAN_RX_uptime;
uint16_t CAN_RX_checkpoint;
uint16_t CAN_RX_EVSEPresentVoltage;
int16_t CAN_RX_InletVoltage;
uint8_t canRxData[8];
uint32_t canRxId;
uint32_t canCounterRxOverall;
uint32_t canCounterRxUsed;


void translateCanInputVariablesToStrings(void) {
/* this runs in task context. It takes the variables which are
set by the interrupt, and translates them into strings. */
line1 = "up "+ String(CAN_RX_uptime) + "  in " + String(CAN_RX_InletVoltage) + "V";
line2 = "chckpt " + String(CAN_RX_checkpoint);
line3 = "EVSEPrV " + String(CAN_RX_EVSEPresentVoltage);
line4 = "cnt " + String(canCounterRxOverall) + " " + String(canCounterRxUsed);
}

void decodeReceivedPacket(void) {
 /* attention: This runs in callback/interrupt context. Do not use serial.print here. */
 canCounterRxOverall++;
 if (canRxId==0x567) {
    CAN_RX_uptime = canRxData[0];
    CAN_RX_uptime <<=8;
    CAN_RX_uptime += canRxData[1];
    CAN_RX_uptime <<=8;
    CAN_RX_uptime += canRxData[2];
    CAN_RX_checkpoint = canRxData[3];
    CAN_RX_checkpoint <<=8;
    CAN_RX_checkpoint += canRxData[4];
    canCounterRxUsed++;
 }
 if (canRxId==0x568) {
    CAN_RX_EVSEPresentVoltage = canRxData[0];
    CAN_RX_EVSEPresentVoltage <<=8;
    CAN_RX_EVSEPresentVoltage += canRxData[1];
    CAN_RX_InletVoltage = (int8_t)(canRxData[2]);
    CAN_RX_InletVoltage <<=8;
    CAN_RX_InletVoltage |= (int8_t)(canRxData[3]);
    canCounterRxUsed++;
 }
}

void onCanReceive(int packetSize) {
  uint8_t i;
  // received a packet
  /* attention: do not use Serial.print here. We are in interrupt context.
     See https://github.com/adafruit/arduino-CAN/issues/13 */  
  if (CAN.packetExtended()) {
    /* extended */
  } else {
    /* standard */
    canRxId=CAN.packetId();
    if (packetSize==8) {
      for (i=0; i<8; i++) {     
        if (CAN.available()) {
          canRxData[i]=CAN.read();
        }
      }
      decodeReceivedPacket();              
    }
  }
}
#endif


#ifdef USE_SERIAL_INPUT 
void handleSerialInput(void) {
  /* read the serial data.
   *  This reads from TWO different serial inputs.
   */
   int incomingByte = 0;
   while (Serial.available() > 0) {
      incomingByte = Serial.read();
      if (incomingByte==0x0A) { /* The sentence ends with LineFeed (0x0A) */
        //ser_processTheSerialInbuffer();
        line1=line2;
        line2=line3;
        line3=line4;
        ser_inbuffer[ser_iWrite]=0; /* terminating zero */       
        line4=(String)"1:"+ser_inbuffer;
        ser_iWrite=0;
      } else {
        ser_inbuffer[ser_iWrite]=incomingByte;
        if (ser_iWrite<SER_INBUFFER_SIZE-2) { ser_iWrite++; }
      }
   }
   while (mySerial.available() > 0) {
      nDebug++;
      incomingByte = mySerial.read();
      if (incomingByte==0x0A) { /* The sentence ends with lineFeed (0x0A) */
        if ((ser_inbuffer2[0]=='l') && (ser_inbuffer2[1]=='c')) {
          //ser_processTheSerialInbuffer();
          line1=line2;
          line2=line3;
          line3=line4;
          ser_inbuffer2[ser_iWrite2]=0; /* terminating zero */       
          line4=(String) /* "2:"+ */ &ser_inbuffer2[2];
        }  
        Serial.println(line4); /* auf USB-Serielle weiterleiten zum long-debugging auf PC */
        ser_iWrite2=0;
      } else {
        ser_inbuffer2[ser_iWrite2]=incomingByte;
        if (ser_iWrite2<SER_INBUFFER_SIZE-2) { ser_iWrite2++; }
      }
   }        
}
#endif

void setup() {
  // put your setup code here, to run once:
#ifdef USE_SERIAL_INPUT  
  ser_inbuffer[0]=0;
#endif  
  Serial.begin(9600);
  
#ifdef USE_SERIAL_INPUT
  mySerial.begin(19200, SERIAL_8N1, RX2_PIN, TX2_PIN);
#endif
  Serial.print("Hello World.\n");
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH); /* Die weiße LED neben dem OLED-Display */
  delay(500);
  digitalWrite(LED,LOW);
  
  #ifdef USE_OLED
    //Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
    display.init();
    display.setFont(ArialMT_Plain_10);    
    delay(300);
    //Heltec.display->clear();
    //#ifdef USE_TEXT_TESTS
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 0, "Erste Zeile");
      //Heltec.display->setFont(ArialMT_Plain_16);
      //Heltec.display->drawString(0, 10, "Zweite");
      //Heltec.display->setFont(ArialMT_Plain_24);
      //Heltec.display->drawString(0, 26, "Dritte");
      //Heltec.display -> display();
      delay(1000);
    //#endif  
    //Heltec.display->setFont(ArialMT_Plain_16);
    //Heltec.display->setContrast(255);
    //Heltec.display->invertDisplay();
    //Heltec.display->flipScreenVertically(); /* tut nix ?!? */
    //Heltec.display->mirrorScreen(); /* spiegeln rechts/links */
  #endif

  #ifdef USE_CAN_INPUT
      //Serial.println("default pins: " + String(DEFAULT_CAN_RX_PIN) + " " + String(DEFAULT_CAN_TX_PIN));
      /* configure the pins for the CAN: */
      CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
      /* start the CAN bus at 500 kbps */
      if (!CAN.begin(500E3)) {
        Serial.println("Starting CAN failed!");
        while (1);
      }

      // register the receive callback
      CAN.onReceive(onCanReceive);
  #endif      
  Serial.print("Setup finished.\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef USE_SERIAL_INPUT
    handleSerialInput();
  #endif
  #ifdef USE_CAN_INPUT  
    translateCanInputVariablesToStrings();
  #endif
  display.clear();
  #ifdef OLED_FOUR_LINES
    display.setFont(ArialMT_Plain_16);
    display.drawString(0,  0, line1);
    display.drawString(0, 16, line2);
    display.drawString(0, 32, line3);
    display.drawString(0, 48, line4);
  #endif  
  #ifdef OLED_THREE_LINES
    display.setFont(ArialMT_Plain_16);
    display.drawString(0,  0, line2);
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 18, line3);
    display.drawString(0, 40, line4);
  #endif  
  display.setFont(ArialMT_Plain_16);
  #ifdef OLED_SHOW_LOOPS
    display.drawString(120, 0, (String)(n_loops % 10));
  #endif  
  #ifdef OLED_SHOW_DEBUG_COUNTER
    display.drawString(100,16, (String)(nDebug % 1000));
  #endif  
  display.display();
  delay(100);
  n_loops++;
  if ((n_loops % 30)==0) {
    #ifdef USE_SERIAL_INPUT
      mySerial.println(n_loops);
    #endif
  }

}
