#include <SoftwareSerial.h>

#define SSerialRX        11  //Serial Receive pin
#define SSerialTX        10  //Serial Transmit pin
#define SSerialTxControl 12   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW



/*-----( Declare objects )-----*/
SoftwareSerial RS485Serial(SSerialRX, SSerialTX);


int byteReceived;
int byteSend;

void setup() {
  // put your setup code here, to run once:
// Start the built-in serial port, probably to Serial Monitor
  Serial.begin(19200);
  pinMode(SSerialTxControl, OUTPUT);    
  
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver   
  
  // Start the software serial port, to another device
  RS485Serial.begin(19200);   // set the data rate 

}

void loop() {

  // put your main code here, to run repeatedly:
if (RS485Serial.available() > 0)  //Look for data from other Arduino
   {
    String mot = RS485Serial.readString();
    Serial.print(mot);
   }  

if (Serial.available() > 0)  //Look for data from other Arduino
   {
    String mot = Serial.readString();
         digitalWrite(SSerialTxControl, RS485Transmit);
            RS485Serial.print(mot);
         digitalWrite(SSerialTxControl, RS485Receive);
   }  

   
}
