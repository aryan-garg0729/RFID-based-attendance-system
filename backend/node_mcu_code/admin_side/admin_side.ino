#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char *SSID = "S10";
const char *PASSWORD = "fomn1596";

#define RST_PIN D3 // Define the GPIO pin connected to the RFID reader's RST pin
#define SS_PIN D4  // Define the GPIO pin connected to the RFID reader's SS pin

// Your Domain name with URL path or IP address with path
String serverName = "http://192.168.72.108:4000/admin";
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
String rfid = "";                 // Variable to store the detected RFID UID

bool connectToWifi();
String getRfid();
void sendToBackend();

void setup()
{
  Serial.begin(9600);
  delay(10);
  pinMode(D0, OUTPUT); // red : server issue
  pinMode(D1, OUTPUT); // yellow : connecting to wifi
  pinMode(D2, OUTPUT); // green : successfully send to server
  // Connect to Wi-Fi network
  connectToWifi();
  digitalWrite(D0, HIGH);
  // Initialize RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");
}

void loop()
{
  // Scan for RFID tags/cards
  rfid = getRfid();
  if (rfid == "")
  {
    return;
  }
  sendToBackend();
}

bool connectToWifi()
{
  // Connect to Wi-Fi network
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(D1, HIGH); // turn on yellow light
    Serial.println();
    Serial.print("Connecting to: ");
    Serial.println(SSID);
    Serial.println("..........");

    // connecting to wifi
    WiFi.begin(SSID, PASSWORD);
    delay(6000);

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      Serial.print("Successfully Connected to: ");
      Serial.println(SSID);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      break;
    }
  }
  digitalWrite(D1, LOW); // turn off yellow ilght
  return (WiFi.status() == WL_CONNECTED);
}

String getRfid()
{
  String rfid = "";
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    // Print UID of the tag/card
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    // Serial.println();

    // Store UID in a variable
    rfid.toUpperCase();
    Serial.println("Detected UID: " + rfid);

    // Halt PICC to scan new tags
    mfrc522.PICC_HaltA();
    return rfid;
  }
  return "";
}

void sendToBackend()
{
  WiFiClient client;
  HTTPClient http;

  connectToWifi();
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);

  http.addHeader("Content-Type", "application/json");
  String data = "{\"rfid\":\"" + rfid + "\"}";
  Serial.println(data);
  int httpResponseCode = http.POST(data);

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  // green (ok)
  digitalWrite(D2, HIGH);
  delay(500); // 0.5 second delay
  digitalWrite(D2, LOW);

  http.end();
}
