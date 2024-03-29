#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "B73 G Flor";
const char* password = "hadoop@spark";

#define RST_PIN D3  // Define the GPIO pin connected to the RFID reader's RST pin
#define SS_PIN D4   // Define the GPIO pin connected to the RFID reader's SS pin

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.105:4000/admin";
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
String rfid = ""; // Variable to store the detected RFID UID



void setup() {
  Serial.begin(9600);
  delay(10);

  // Connect to Wi-Fi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");
}



void loop() {
  // Scan for RFID tags/cards
  rfid = getRfid();
  if (rfid == ""){
    return; 
  }
  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  sendToBackend();
  delay(2000);


}

String getRfid(){
  String rfid = "";
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Print UID of the tag/card
    Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    
    // Store UID in a variable
    rfid.toUpperCase();
    Serial.println("Detected UID: " + rfid);
    delay(2000);

    // Halt PICC
    mfrc522.PICC_HaltA();
    return rfid;
  }
  return "";
}




void sendToBackend(){
    WiFiClient client;
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Specify content-type header
    // Data to send with HTTP POST
    
    http.addHeader("Content-Type", "application/json");
    String data = "{\"rfid\":\"" + rfid + "\"}";
    Serial.println(data);
    int httpResponseCode = http.POST(data); 
    
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    // Check for errors
    if (httpResponseCode != HTTP_CODE_OK) {
      Serial.println("Error sending data to backend");
      // Handle error appropriately, e.g., retry, or take corrective action
    }

    // Free resources
    http.end();
}

