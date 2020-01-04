#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>


#define SSerialRX        0  //Serial Receive pin
#define SSerialTX        1  //Serial Transmit pin
#define SSerialTxControl 2   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW



#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
/*-----( Declare objects )-----*/
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

String str = "";

int openToSend = 2;

int gache = 5;
int ledAccept = 6;
int ledErreur = 7;

int lunchProg = 8;
int tTab = 130;
String userTab[130];
int index = 0;

void setup() {
 
   pinMode(lunchProg, INPUT);
   pinMode(ledAccept, OUTPUT);
   pinMode(gache , OUTPUT);
   pinMode(ledErreur, OUTPUT);
  
   
  // Serial.begin(9600);
   // Start the software serial port, to another device
   RS485Serial.begin(115200);   // set the data rate 

   pinMode(SSerialTxControl, OUTPUT);    
   digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver   
  
   SPI.begin();
   mfrc522.PCD_Init();   
int i = 0;
for(i = 0; i < tTab; i++){
                       userTab[i]="";
}
   
}

void loop() {

  if(digitalRead(lunchProg) == 1){
        digitalWrite(ledAccept, HIGH); 
        digitalWrite(ledErreur, LOW); 
         delay(200);
        digitalWrite(ledAccept, LOW); 
        digitalWrite(ledErreur, HIGH); 
         delay(200);
          if (RS485Serial.available() > 0) {
                          String mot = RS485Serial.readString();
                          prog(mot);
          }
          
  }else{
    if (RS485Serial.available() > 0) {
              //  digitalWrite(SSerialTxControl, RS485Receive);
                String mot = RS485Serial.readString();
                String adrr= readEepromStr(0,2);
                
// ---------------- identification de porte par ADRR-------------//  
                if(mot.substring(0,3).equals(adrr)){
// ---------------- (Acceptation / refus) d'accès ----------
                        //String id = mot.substring(3, 11);
                         //String action = mot.substring(11, mot.length());
                          if(mot.substring(11, mot.length()).equals("ouvre")){
                            // ****************  Accés accepter  ************
                                      openDoor();
                                      if(!mot.substring(3, 11).equals("________")){
                                                    saveIdentifier(mot.substring(3, 11));
                                      }
                                      
                          }else if(mot.substring(3, 11).equals("programe")){
                                                prog(mot.substring(11, mot.length()));
                          }else if(mot.substring(3, 11).equals("supprime")){
                                                // procedure de suppression des droits d'accès.
                                                supp(mot.substring(11, mot.length()));
                            }else{
                            // ****************  Accés refuser  ************
                             int i = 0;
                             if(isIdExist(mot.substring(11, mot.length()))){
                                 supp(mot.substring(11, mot.length()));
                             }
                                     for(i = 0; i < 2; i++ ){
                                        digitalWrite(ledErreur, HIGH); delay(400);
                                        digitalWrite(ledErreur, LOW); 
                                        if(i < 1){
                                           delay(300);
                                        }
                                       
                                    }      
                          }
                }
        }else{
          digitalWrite(ledAccept, LOW); 
          digitalWrite(ledErreur, LOW); 
        }
  }

 
//------lecture sauf si une nouvelle carte RFID est présente sinon retour----------------------------------//
   if(! mfrc522.PICC_IsNewCardPresent()){
    return;
   }


//-----Si pas de carte alors retour sinon lecture est envoi de code vers sur Serial port-------------------//
   if(! mfrc522.PICC_ReadCardSerial()){
    return;
   }

//------lecture contenu complet de la carte et transfere vers le serial port mfrc522.PICC_DumpToSerial(&(mfrc522.uid));-----
str = "";

  // Serial.println("Code de carte : ");
    for(byte i = 0; i < mfrc522.uid.size; i++){
     str = str + String(mfrc522.uid.uidByte[i], HEX);
  
      
    }
     str.toUpperCase();
    
    
     if(isIdExist(str)){
      // ------- Demande de sauvegarde du temps c tout ----------
        str = readEepromStr(0,2)+str+"T";
        openDoor();
     }else{
      // ------- Demande de confirmation + sauvegarde si ok -----
        str = readEepromStr(0,2)+str+"C";
        digitalWrite(ledErreur, HIGH); 
     }
     
     digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit   
     RS485Serial.print(str); 
     digitalWrite(SSerialTxControl, RS485Receive);
         
         digitalWrite(ledAccept, HIGH); 
         delay(200);
         digitalWrite(ledAccept, LOW); 
         digitalWrite(ledErreur, LOW); 
    
  

}

//------------------- ecriture du str dans la position x de l'eeprom----------------------------
void StrToEeprom(String str, int x){
  int str_len = str.length();
  int i = x;
  int j = 0;
  char c = "";
  int s = 0;
  for( i = x; i < x + str_len; i++){
    c = str.charAt(j);
    j++;
    s = c;
    EEPROM.write(i, s);
  }
}


// ----------------- Lecture depuis l'eeprom de la position x jusqu'a la position y-------------
String readEepromStr(int x, int y){
  String str = "";
  int i = 0;
  char c = "";
  for(i = x; i<=y; i++){
    c = char(EEPROM.read(i));
    str = String(str + c);
  }
  return str;
}
//------------------ Existing identifier--------------------------------------------------------
boolean isIdExist(String id){
  boolean ok = false;
  int i = 0;
  for(i = 0; i < tTab; i++){
    if(userTab[i].equals(id)){
       ok = true;
       break;
    }
  }
  return ok;
}
//------------------- save identifier-----------------------------------------------------------

void saveIdentifier(String id){
            boolean ok = false;
            if(index < 0 || index >=tTab) {
                                int cherche = 0;
                                for(cherche = 0; cherche < tTab; cherche++){
                                  if(userTab[cherche].equals("_")){
                                          ok = true;
                                          userTab[cherche] = id;
                                          cherche = tTab;
                                          break;
                                  }
                                }
                              if(!ok){
                                index = 0;
                              }
            }else{
                                index++;
            }
            if(!ok){
                userTab[index] = id;
            }
}


//------------------ Programmation ADRR---------------------------------------------------------
void prog(String adrr){
         StrToEeprom(adrr, 0);
         int i = 0;
         digitalWrite(ledErreur, HIGH);
         for(i = 3; i < EEPROM.length(); i++){
              EEPROM.write(i,255);
         }
    for(i = 0; i<8; i++){
         digitalWrite(ledAccept, HIGH); delay(100);
         digitalWrite(ledAccept, LOW); delay(100);
    }
    
}

//------------------ Suppression IDs ---------------------------------------------------------
void supp(String ids){
  int i = 0;
  int j = 0;
  for(j = 0; j < ids.length(); j=j+8){
                String id = ids.substring(j, j+8);
                
                 for(i = 0; i < tTab ; i++){
                                        if(userTab[i].equals(id)){
                                                                  userTab[i]="_";
                                                                  }
                                            }
                                      }  
}
//------------------ ouvrire la porte-----------------------------------------------------------
void openDoor(){
                                  digitalWrite(gache, HIGH);
                                  int i = 0;
                                                for(i = 0; i < 3; i++ ){
                                                    digitalWrite(ledAccept, HIGH); delay(500);
                                                    digitalWrite(ledAccept, LOW);
                                                    if(i < 2)delay(300);
                                                }
                                  digitalWrite(gache, LOW);
}

