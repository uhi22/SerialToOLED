

/* OLED Display with Serial input
 *  
 *  Hardware: HTIT-WB32, also known as heltec wifi kit 32.
 *  Arduino board name: WIFI_Kit_32
 *  
 *  Regarding Arduino board and library, there are some compatibility issues for Raspbian.
 *  Solving is documented in the boards.txt. In short: take the library from heltech, and add the board config into
 *  the board folder of the original espressif ESP board directory.
 *  
 *  Functionality:
 *   - Listens on the serial port (pin18) and on reception of 0x0A it shows the line on the OLED display. 
 *   - Scrolls the older lines up.
 *     
 *  Changes:
 *     2021-08-24 Uwe:    
 *        - Improvement: Do not show the channel number. This gives more space. 
 *        - Fix: smaller line buffer, to avoid pixel overlay effects
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
  #include "heltec.h"
#endif

int n_loops;
int nDebug;
 #define SER_INBUFFER_SIZE 20
 uint8_t ser_iWrite=0;
 uint8_t ser_iWrite2=0;
 
 char ser_inbuffer[SER_INBUFFER_SIZE];
 char ser_inbuffer2[SER_INBUFFER_SIZE];
 String line1 = "Eins";
 String line2 = "Zwei";
 String line3 = "PreCharge";
 String line4 = "231V";
 
 
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
    Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
    delay(300);
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 0, "Erste Zeile");
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->drawString(0, 10, "Zweite");
    Heltec.display->setFont(ArialMT_Plain_24);
    Heltec.display->drawString(0, 26, "Dritte");
    Heltec.display -> display();
    delay(1000);
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setContrast(255);
    //Heltec.display->invertDisplay();
    //Heltec.display->flipScreenVertically(); /* tut nix ?!? */
    //Heltec.display->mirrorScreen(); /* spiegeln rechts/links */
  #endif  
  Serial.print("Setup finished.\n");

}

void loop() {
  // put your main code here, to run repeatedly:
  handleSerialInput();
  Heltec.display->clear();
  #ifdef OLED_FOUR_LINES
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display -> drawString(0,  0, line1);
    Heltec.display -> drawString(0, 16, line2);
    Heltec.display -> drawString(0, 32, line3);
    Heltec.display -> drawString(0, 48, line4);
  #endif  
  #ifdef OLED_THREE_LINES
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display -> drawString(0,  0, line2);
    Heltec.display->setFont(ArialMT_Plain_24);
    Heltec.display -> drawString(0, 18, line3);
    Heltec.display -> drawString(0, 40, line4);
  #endif  
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display -> drawString(120, 0, (String)(n_loops % 10));
  #ifdef OLED_SHOW_DEBUG_COUNTER
    Heltec.display -> drawString(100,16, (String)(nDebug % 1000));
  #endif  
  Heltec.display -> display();
  delay(100);
  n_loops++;
  if ((n_loops % 30)==0) {
    mySerial.println(n_loops);
  }

}
