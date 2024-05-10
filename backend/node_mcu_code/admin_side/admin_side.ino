#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char *SSID = "Note8";
const char *PASSWORD = "watermelon";

#define RST_PIN D3 // Define the GPIO pin connected to the RFID reader's RST pin
#define SS_PIN D4  // Define the GPIO pin connected to the RFID reader's SS pin

// Your Domain name with URL path or IP address with path
// String serverName = "https://rfid-based-attendance-system-1j2o.onrender.com/admin";
const char* host = "rfid-based-attendance-system-1j2o.onrender.com";
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
  WiFiClientSecure client;
  client.setInsecure();
  connectToWifi();
  
  // Your Domain name with URL path or IP address with path
  if (!client.connect(host, 443)) { //works!
    Serial.println("connection failed");
    return;
  }


  String url = "/admin";

  // Prepare JSON data
  String jsonData = "{\"rfid\":\"" + rfid + "\"}";
  Serial.print("Sending JSON data: ");
  Serial.println(jsonData);

  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + String(jsonData.length()) + "\r\n" +
               "Connection: close\r\n\r\n" +
               jsonData);

  Serial.println();
  Serial.println("closing connection");

  // Wait for response
  while (client.connected()) {
    Serial.print("inside while\n");
    String line = client.readStringUntil('\n');
    Serial.print(line);
    Serial.println();
    if (line.startsWith("HTTP/1.1")) {
      int statusCode = line.substring(9, 12).toInt();
      Serial.print("Response code: ");
      Serial.println(statusCode);
        if(statusCode == 200)
        {
          Serial.print("GREEN LIGHT\n");
          digitalWrite(D2, HIGH);
          delay(500); // 0.5 second delay
          digitalWrite(D2, LOW);
        }
        else {
          digitalWrite(D0, LOW);
          delay(500); // 0.5 second delay
          digitalWrite(D0, HIGH);
        }
      break; // Stop reading after getting the response code
    }
  }
  // http.begin(client, serverName);

  // http.addHeader("Content-Type", "application/json");
  // String data = "{\"rfid\":\"" + rfid + "\"}";
  // Serial.println(data);
  // int httpResponseCode = http.POST(data);

  // Serial.print("HTTP Response code: ");
  // Serial.println(httpResponseCode);





  // green (ok)


  // http.end();
}
