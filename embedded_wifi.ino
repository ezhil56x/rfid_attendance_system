#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>

const char *GScriptId = "DEPLOYMENT_dID";

const char* ssid     = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

String student_Rollno;
String uid;

int blocks[] = {4, 5};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))

#define RST_PIN  0  //D3
#define SS_PIN   2  //D4
#define BUZZER   4  //D2

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;

byte bufferLen = 18;
byte readBlockData[18];

void setup() {
  pinMode(D8, OUTPUT);
  Serial.begin(9600);        
  delay(10);
  Serial.println('\n');
  SPI.begin();
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  delay(5000);
  
  Serial.print("Connecting to ");
  Serial.println(host);
  
  bool flag = false;
  for(int i = 0; i < 5; i++) { 
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      delay(2000);
      break;
    } else
      Serial.println("Connection failed. Retrying...");
  }
  
  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
  }
  
  delete client;
  client = nullptr;
}

void loop() {
  static bool flag = false;
  if (!flag) {
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  
  if (client != nullptr) {
    if (!client->connected())
      client->connect(host, httpsPort);
  } else {
    Serial.println("Error creating client object!");
  }
  
  mfrc522.PCD_Init();
  
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;
  
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));  
  
  String values = "", data;

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  
  for (byte i = 0; i < total_blocks; i++) {
    ReadDataFromBlock(blocks[i], readBlockData);
    
    if (i == 0) {
      data = String((char*)readBlockData);
      data.trim();
      student_Rollno = data;
      values = data + ",";
    } else if (i == total_blocks - 1) {
      data = String((char*)readBlockData);
      data.trim();
      values += data;
    } else {
      data = String((char*)readBlockData);
      data.trim();
      values += data + ",";
    }
  }
  
  payload = payload_base + "\"" + uid + "," + values + "\"}";
  
  Serial.println("Publishing data...");
  Serial.println(payload);
  
  if (client->POST(url, host, payload)) { 
    digitalWrite(D8, HIGH);
    delay(2000);
    digitalWrite(D8, LOW);
    delay(2000);
  } else {
    Serial.println("Error while connecting");
  }
  
  delay(5000);
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
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
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");  
  }
}
