#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <LiquidCrystal_I2C.h>
//-----------------------------------------
#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D8
//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;      
//-----------------------------------------
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  
/* Create another array to read data from Block */
/* Length of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];
//-----------------------------------------
String card_holder_name;
const String sheet_url = "https://script.google.com/macros/s/AKfycbycXadDuoeXIkC6zrAVWvkHha1MOlMfbr3mrbh380QWudsmFbJPniEtMuXno1iIY6g/exec?name=";  //Enter Google Script URL

//-----------------------------------------
// WiFi credentials
#define WIFI_SSID "Amjid_2.4g"  //Enter WiFi Name
#define WIFI_PASSWORD "qwerttrewq"  //Enter WiFi Password
//-----------------------------------------

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Change LCD Address to 0x27 if 0x3F doesn't work

/****************************************************************************************************
 * setup() function
 ****************************************************************************************************/
void setup() {
  //--------------------------------------------------
  /* Initialize serial communications with the PC */
  Serial.begin(9600);

  // Initialize the LCD with specified dimensions
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  
  //--------------------------------------------------
  // WiFi Connectivity
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //--------------------------------------------------
  /* Set BUZZER as OUTPUT */
  pinMode(BUZZER, OUTPUT);
  //--------------------------------------------------
  /* Initialize SPI bus */
  SPI.begin();
  //--------------------------------------------------
}

/****************************************************************************************************
 * loop() function
 ****************************************************************************************************/
void loop() {
  //--------------------------------------------------
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Scan your Card ");
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /* Select one of the cards */
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  /* Read data from the same block */
  //--------------------------------------------------
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  
  /* Print the data read from block */
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  for (int j = 0; j < 16; j++) {
    Serial.write(readBlockData[j]);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hey " + String((char*)readBlockData) + "!");
  }
  Serial.println();
  //--------------------------------------------------
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  //--------------------------------------------------
  
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    
    card_holder_name = sheet_url + String((char*)readBlockData);
    card_holder_name.trim();
    Serial.println(card_holder_name);

    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
    if (https.begin(*client, card_holder_name)) {
      Serial.print(F("[HTTPS] GET...\n"));
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        lcd.setCursor(0, 1);
        lcd.print(" Data Recorded ");
        delay(2000);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
      delay(1000);
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}

/****************************************************************************************************
 * ReadDataFromBlock() function
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[]) { 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Authentication success");
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Block was read successfully");  
  }
}
