

/* OLED Display with Serial input
 *  
 *  Hardware: HTIT-WB32, also known as heltec wifi kit 32.
 *  Arduino board name: WiFi Kit 32
 *  
 *  Version for Arduino IDE 2.0.4 (March 2023) together with
 *  Heltec board "Heltec ESP32 Series Dev-boards by Heltec" Version 0.0.7
 *  From board manager url
 *  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.7/package_heltec_esp32_index.json
 *
 *  
 *  Functionality:
 *   - Listens on the serial port (pin18) and on reception of 0x0A it shows the line on the OLED display. 
 *   - Scrolls the older lines up.
 *     
 *  Changes:
 *     2021-08-24 Uwe:    
 *        - Improvement: Do not show the channel number. This gives more space. 
 *        - Fix: smaller line buffer, to avoid pixel overlay effects
 *     2023-02-28 Uwe:    
 *        - Improvement: Init text improved. 
 *        - Improvement: Removed the running counter from display. 
 *     2023-03-25 Uwe: 
 *        - Migration to new Arduino IDE and new libraries   
 *     
 */

/* The pins for the second serial port. The first one is used by the USB interface (programming and Arduino console). */
/* see https://quadmeup.com/arduino-esp32-and-3-hardware-serial-ports/ */
#include <HardwareSerial.h>
HardwareSerial mySerial(1); /* 0 would be the USB-Serial, e.g. for Serial.print. 1 is the Serial2. 
                               The pins can be freely chosen, see below. */
#define TX2_PIN 17 /* 17 works fine on the HELTEC ESP32 OLED module for TX */
#define RX2_PIN 18 /* 16 does not work (perhaps used for other purpose on the board), 18 works,  */

 
#define USE_OLED

//#define OLED_FOUR_LINES
#define OLED_THREE_LINES
//#define OLED_SHOW_DEBUG_COUNTER

#ifdef USE_OLED
  // For a connection via I2C using the Arduino Wire include:
  #include <Wire.h>               
  #include "HT_SSD1306Wire.h"

  SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

#endif

int n_loops;
int nDebug;
 #define SER_INBUFFER_SIZE 20
 uint8_t ser_iWrite=0;
 uint8_t ser_iWrite2=0;
 
 char ser_inbuffer[SER_INBUFFER_SIZE];
 char ser_inbuffer2[SER_INBUFFER_SIZE];
 String line1 = "Hello";
 String line2 = "Init...";
 String line3 = "...waiting";
 String line4 = "for data...";
 
 
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


void setup() {
  // put your setup code here, to run once:
  ser_inbuffer[0]=0;
  Serial.begin(9600);
  
  mySerial.begin(19200, SERIAL_8N1, RX2_PIN, TX2_PIN);
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
  Serial.print("Setup finished.\n");

}

void loop() {
  // put your main code here, to run repeatedly:
  handleSerialInput();
  display.clear();
  #ifdef OLED_FOUR_LINES
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display -> drawString(0,  0, line1);
    Heltec.display -> drawString(0, 16, line2);
    Heltec.display -> drawString(0, 32, line3);
    Heltec.display -> drawString(0, 48, line4);
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
    mySerial.println(n_loops);
  }

}
