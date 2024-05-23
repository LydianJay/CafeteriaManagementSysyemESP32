#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include "SDFunctions.h"


// LiquidCrystal_I2C lcd(0x27,16,2);
// LiquidCrystal_I2C lcd2(0x26,16,2);
File root;


constexpr uint8_t RST_PIN = 0;         
constexpr uint8_t SS_PIN = 4;         

constexpr uint8_t SD_MISO = 16;         
constexpr uint8_t SD_MOSI = 17;      
constexpr uint8_t SD_SCK = 2;
constexpr uint8_t SD_SS =  5;          

SPIClass SPIsd;


MFRC522 rfid;  // Create MFRC522 instance
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];


void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
void setup() {
  
  Serial.begin(115200);
  SPI.begin(18, 19, 23, SS_PIN);
  SPIsd.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS);
 
  if (!SD.begin(SD_SS, SPIsd)) {

    Serial.println("initialization failed!");
   
  }
  if(SD.cardType() == CARD_NONE){
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

  // lcd.init();                      // initialize the lcd 
  // lcd.backlight();
  // lcd.clear();
  // lcd.setCursor(0,0);
  // lcd.print("LCD 1");

  // lcd2.init();                      // initialize the lcd 
  // lcd2.backlight();
  // lcd2.clear();
  // lcd2.setCursor(0,0);
  // lcd2.print("LCD 2");
  
  // root = SD.open("/");
  // printDirectory(root, 0);

  rfid.PCD_Init(SS_PIN, RST_PIN); 
  
  rfid.PCD_DumpVersionToSerial();
  Serial.println(' ');
  
}





void loop() { 

   // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent()) {
    //Serial.println("No new card and not reading shit");
    return;
  }
   
  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial()){
    Serial.println("No reading shit");
    return;

  }

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
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

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

}