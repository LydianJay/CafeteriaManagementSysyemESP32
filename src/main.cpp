#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include "SDFunctions.h"
#include <Keypad.h>
LiquidCrystal_I2C lcd(0x27,16,2);
LiquidCrystal_I2C lcd2(0x26,16,2);
File root;




const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
{'1','2','3', 'A'},
{'4','5','6', 'B'},
{'7','8','9', 'C'},
{'*','0','#', 'D'}
};
// 27, 14, 12, 13
byte rowPins[ROWS] = {26, 25, 33, 32}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {13, 12, 14, 27}; //connect to the column pinouts of the kpd
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
// KeyPadPinOut pinout = {32, 33, 25, 26, 27, 14, 12, 13};


uint8_t cursorPos = 0;

constexpr uint8_t RST_PIN = 0;         
constexpr uint8_t SS_PIN = 4;         

constexpr uint8_t SD_MISO = 16;         
constexpr uint8_t SD_MOSI = 17;      
constexpr uint8_t SD_SCK = 2;
constexpr uint8_t SD_SS =  5;          



SPIClass SPIsd;


MFRC522 rfid;  // Create MFRC522 instance
MFRC522::MIFARE_Key key; 
byte nuidPICC[4] = {0x0, 0x0, 0x0, 0x0};
String strRFID = "";

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void resetPICC() {
  for (size_t i = 0; i < 4; i++)
  {
    nuidPICC[i] = 0x0;
  }
  
}

void setup() {
  
  Serial.begin(115200);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("LCD 1");

  lcd2.init();                      // initialize the lcd 
  lcd2.backlight();
  lcd2.clear();
  lcd2.setCursor(0,0);
  lcd2.print("Welcome");
  
  SPI.begin(18, 19, 23, SS_PIN);
  SPIsd.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS);
 
  if (!SD.begin(SD_SS, SPIsd)) {

    Serial.println("initialization failed!");

    while (true) {
      lcd.clear();
      lcd.print("SD Card ERROR");
      delay(1500);
    }
    
   
  }
  if(SD.cardType() == CARD_NONE){
    lcd2.clear();
    Serial.println("No SD card attached");
  }

  Serial.print("SD Card Type: ");
  if(SD.cardType() == CARD_MMC){
    Serial.println("MMC");
  } else if(SD.cardType() == CARD_SD){
    Serial.println("SDSC");
  } else if(SD.cardType() == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  
  // root = SD.open("/");
  // printDirectory(root, 0);
  uint8_t buffer[32];
  readFile(SD, "/Database/1474385245.txt", buffer, 32);

  rfid.PCD_Init(SS_PIN, RST_PIN); 
  
  rfid.PCD_DumpVersionToSerial();
  Serial.println(' ');

}




void readRFID() {


  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }
   
  if ( ! rfid.PICC_ReadCardSerial()){
    return;
  }

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && 
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
      strRFID += nuidPICC[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    
  }
  else Serial.println(F("Card read previously."));


  rfid.PICC_HaltA();


  rfid.PCD_StopCrypto1();


}





// ============= Main Menu stuff =================


#define MAIN_MENU 1
#define PAY_SCREEN 2
#define CASH_IN 3
#define BALANCE 4


int state = 1;


bool updateLCD = true;



void displayMenu() {

  if(updateLCD){
    lcd.clear();

    if (cursorPos >= 2){
      lcd.setCursor(0,1);
      lcd.print("->");
      lcd.setCursor(2, 0);
      lcd.print("Pay");
      lcd.setCursor(2, 1);
      lcd.print("Balance");
    }
    else {
      lcd.setCursor(0, cursorPos);
      lcd.print("->");
      lcd.setCursor(2, 0);
      lcd.print("Cash In");
      lcd.setCursor(2, 1);
      lcd.print("Pay");
    }
    
  

    lcd2.clear();
    lcd2.print("Welcome!");

    updateLCD = false;
  }

 

}

void responToMenuKeys() {

  if(kpd.getKeys()) {

    for (int i=0; i < LIST_MAX; i++)   
    {
        if ( kpd.key[i].stateChanged )  
        {
            switch (kpd.key[i].kstate) {  
                case PRESSED:

                  switch (kpd.key[i].kchar)
                  {
                  case '#': // select
                      updateLCD = true;
                      if(cursorPos == 0)
                        state = 3;
                      if(cursorPos == 1)
                        state = PAY_SCREEN;
                      if(cursorPos == 2)
                        state = BALANCE;

                    break;
                  
                  case 'A': // up
                    if(cursorPos > 0){
                      cursorPos--;
                      updateLCD = true;
                    }

                  break;

                  case 'B': // down
                    if(cursorPos < 2) {
                      cursorPos++;
                      updateLCD = true;
                    }
                  break;

                  case '*': // back

                  break;


                  default:
                    break;
                }
            break;
            
            }
        }
    }
  }

}

// ===================================================


// ==================== Pay Screen =====================

uint32_t totalAmmountToPay = 0;
uint32_t lastState = 0;
String ammount;
bool scanState = false;

void payScreen() {


  if(updateLCD){
    lcd.clear();
    lcd.print("Ammount");
    lcd.setCursor(0, 1);
    lcd.print(ammount);

    lcd2.clear();
    lcd2.print("To Pay:");
    lcd2.setCursor(0, 1);
    lcd2.print(ammount);
    updateLCD = false;
  }
  


  if (kpd.getKeys())
  {
    for (int i=0; i < LIST_MAX; i++)   
    {
        if ( kpd.key[i].stateChanged && kpd.key[i].kstate == PRESSED)  
        {
          if(kpd.key[i].kchar >= '0' && kpd.key[i].kchar <= '9'){
            if(ammount.isEmpty() && kpd.key[i].kchar == '0') {
              continue;
            }
            ammount += kpd.key[i].kchar;
            updateLCD = true;
          }
          // if(kpd.key[i].kchar == 'A'){
          //   lastState = totalAmmountToPay;
          //   totalAmmountToPay += ammount.toInt();
          //   updateLCD = true;
          // }
          // else if(kpd.key[i].kchar == 'B'){
          //   lastState = totalAmmountToPay;
          //   totalAmmountToPay = lastState;
          //   updateLCD = true;
          // }
          else if(kpd.key[i].kchar == '#'){
            scanState = true;
            updateLCD = true;
          }
        }
    }
  }
}


void scanScreen() {
  strRFID = "";
  lcd2.clear();
  
  lcd2.print("To Pay:" + String(ammount));
  delay(5000);
  uint64_t timer = millis() + 15000; // 15 second timer

  while (strRFID.isEmpty() && timer > millis()) {
    readRFID();
    lcd2.clear();
    lcd2.print("Swipe When Ready");
    lcd2.setCursor(0,1);
    lcd2.print("Time: " + String( (timer - millis()) / 1000) + "s");
    delay(250);
  }

  if(strRFID.isEmpty()) {
    lcd2.clear();
    lcd2.print("Error");
    lcd2.setCursor(0,1);
    lcd2.print("No ID Detected");
    state = 1;
    delay(3500);
  }
  else {


    
      String filePath = "/Database/" + strRFID + ".txt";
      uint8_t buffer[32];
      memset(buffer, 0, 32);

      if(readFile(SD, filePath.c_str(), buffer, 32)){


        if(String((char*)buffer).toInt() < ammount.toInt()){

          lcd2.clear();
          lcd2.print("Not Enough");
          lcd2.setCursor(0,1);
          lcd2.print("Balance");
          state = 1;
          ammount = "";
          scanState = false; 
          updateLCD = true;
          resetPICC();
          delay(5000);
          
          return;
        }
        else {
          int bal = String((char*)buffer).toInt() - ammount.toInt();
          Serial.printf("To be written: %s \n", String(bal).c_str());
          Serial.printf("Read: %s \n", buffer);
          String strBal = String(bal);
          writeFile(SD, filePath.c_str(), strBal);
          lcd2.clear();
          lcd2.print("Bal: " + strBal);
          lcd2.setCursor(0,1);
          lcd2.print("Success!");
          state = 1;
          updateLCD = true;
          delay(5000);
        }

        
      }
      else {
        
        state = 1;
        updateLCD = true;
        lcd2.clear();
        lcd2.print("Unregistered");
        lcd2.setCursor(0,1);
        lcd2.print("User Detected!");
        delay(5000);
      }



  }
  ammount = "";
  scanState = false; 
  resetPICC();
}

// ====================================================


// ================= CASH IN =====================


uint16_t ammountToCashIn = 0;
String ammountCashIn;

void displayCashInScreen() {

 
  strRFID = String();


  
  if(updateLCD){
    lcd.clear();
    lcd.print("Ammount");
    lcd.setCursor(0, 1);
    lcd.print(ammountCashIn);
    updateLCD = false;
  }
  


  if (kpd.getKeys())
  {
    for (int i=0; i < LIST_MAX; i++)   
    {
        if ( kpd.key[i].stateChanged && kpd.key[i].kstate == PRESSED)  
        {
          if(kpd.key[i].kchar >= '0' && kpd.key[i].kchar <= '9'){
            ammountCashIn += kpd.key[i].kchar;
            updateLCD = true;
          }
          if(kpd.key[i].kchar == 'A'){
            ammountToCashIn = ammountCashIn.toFloat();
            updateLCD = true;
          }
          else if(kpd.key[i].kchar == '#') {

            if(ammountCashIn.toInt() <= 0 || ammountCashIn.isEmpty()) {
              lcd2.clear();
              lcd2.print("Invalid Value");
              lcd.clear();
              lcd.print("Invalid Value");
              delay(3500);
              resetPICC();
              updateLCD = true;
              ammountCashIn = "";
              return;
            }


            lcd2.clear();
            lcd2.println("Swipe When Ready");
            
            uint64_t timer = millis() + 15000; // 15 second timer

            while (strRFID.isEmpty() && timer > millis()) {
              
              readRFID();
              lcd2.clear();
              lcd2.print("Swipe When Ready");
              lcd2.setCursor(0,1);
              lcd2.print("Time: " + String( (timer - millis()) / 1000) + "s");
              delay(250);
            }
            


            if(strRFID.isEmpty()) {
              lcd2.clear();
              lcd2.print("Error");
              lcd2.setCursor(0,1);
              lcd2.print("No ID Detected");
              state = 1;
              updateLCD = true;
              delay(3000);
            }
            else {
              

              String filePath = "/Database/" + strRFID + ".txt";
              uint8_t buffer[32];
              memset(buffer, 0, 32);

              if(readFile(SD, filePath.c_str(), buffer, 32)){
                int bal = String((char*)buffer).toInt() + ammountCashIn.toInt();
                Serial.printf("To be written: %s \n", String(bal).c_str());
                Serial.printf("Read: %s \n", buffer);
                String strBal = String(bal);
                writeFile(SD, filePath.c_str(), strBal);
                lcd2.clear();
                lcd2.print("Bal: " + strBal);
                lcd2.setCursor(0,1);
                lcd2.print("Success!");
                state = 1;
                updateLCD = true;
                delay(5000);
              }
              else {
                Serial.printf("To be written: %s \n", ammountCashIn.c_str());
                writeFile(SD, filePath.c_str(), ammountCashIn);
                state = 1;
                updateLCD = true;
                lcd2.clear();
                lcd2.print("Bal: " + ammountCashIn);
                lcd2.setCursor(0,1);
                lcd2.print("Success!");
                delay(5000);
              }
              resetPICC();
            }
            
            resetPICC();
            ammountCashIn = "";
          }
        }
    }
  }

}



// ===============================================



void displayBalance() {

  lcd.clear();
  lcd2.clear();

  lcd.print("Waiting swipe");
  strRFID = "";
  

  uint64_t timer = millis() + 15000; // 15 second timer

    while (strRFID.isEmpty() && timer > millis()) {
      
      readRFID();
      lcd2.clear();
      lcd2.print("Swipe When Ready");
      lcd2.setCursor(0,1);
      lcd2.print("Time: " + String( (timer - millis()) / 1000) + "s");
      delay(250);
    }



    if(strRFID.isEmpty()) {
      lcd2.clear();
      lcd2.print("Error");
      lcd2.setCursor(0,1);
      lcd2.print("No ID Detected");
      state = 1;
      updateLCD = true;
      delay(3500);
    }
    else {
      

      String filePath = "/Database/" + strRFID + ".txt";
      uint8_t buffer[32];
      memset(buffer, 0, 32);

      if(readFile(SD, filePath.c_str(), buffer, 32)){

        
        lcd2.clear();
        lcd2.print("Bal: " + String((char*)buffer));
        
        state = 1;
        updateLCD = true;
        delay(6000);
      }
      else {
        state = 1;
        updateLCD = true;
        lcd2.clear();
        lcd2.print("Unregistered");
        lcd2.setCursor(0,1);
        lcd2.print("User Detected");
        delay(5000);
      }
      
    }


  resetPICC();
}

//


void loop() { 




  switch (state) {
    case MAIN_MENU:
      displayMenu();
      responToMenuKeys();
    break;

    case PAY_SCREEN:
      if(!scanState){
        payScreen();
      }
      else {
        scanScreen();
      }
    break;

    case CASH_IN:
      displayCashInScreen();
    break;


    case BALANCE:
      displayBalance();
    break;
  
  default:
    break;
  }

  

  

  

}