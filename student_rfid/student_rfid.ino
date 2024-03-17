#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>

const char* ssid = "B73 G Flor";
const char* password = "hadoop@spark";

// Define constants for file paths
const char* databaseFilePath = "/database.txt";
const char* logFilePath = "/log.txt";

#define RST_PIN D3  // Define the GPIO pin connected to the RFID reader's RST pin
#define SS_PIN D4   // Define the GPIO pin connected to the RFID reader's SS pin
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.104:3000/student";
String rfid = ""; // Variable to store the detected RFID UID
JsonArray arr;
bool iswifi = false;
bool gotdata=false;

void setup() {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D8, OUTPUT);

  Serial.begin(9600);
  delay(10);

  // Connect to Wi-Fi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while(!gotdata){
    int times=0;
    while (WiFi.status() != WL_CONNECTED && times<15) {
      delay(500);
      Serial.print(".");
      times++;
    }
    iswifi = WiFi.status() == WL_CONNECTED;
    Serial.println("");

    if(iswifi){
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

      // getalldata from database
      initializeSPIFFS();
      getAllData();
      // printing database.txt file
      // readAllDataFromFile(databaseFilePath);
    }
    else Serial.println("WiFi not available");
  }
  
  

  // Initialize RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");

  File log = SPIFFS.open(logFilePath, "w");
  if (!log) {
    Serial.println("Failed to open file for writing");
  }
  else{
    log.close();
    Serial.println("log file created");
  }
  
}

void loop() {
  // Scan for RFID tags/cards
  rfid = getRfid();
  if (rfid == ""){
    return; 
  }
  // rfid="B1A8891D";
  int status = 0;
  if(iswifi){
    status = sendToBackend();
  }else{
    // try wifi
    WiFi.begin(ssid, password);
    int times=0;
    while (WiFi.status() != WL_CONNECTED && times<2) {
      delay(500);
      Serial.print(".");
      times++;
    }
    iswifi = WiFi.status() == WL_CONNECTED;
    Serial.println("");

    if(iswifi){
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

      status=sendToBackend();
    }
    else {
      status = logAndAuth();
    }
  }
  // status = logAndAuth();
  // readAllDataFromFile();
  Serial.println("");
  glowLED(status);
}

// Function to initialize SPIFFS
void initializeSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to initialize SPIFFS");
    return;
  }
  Serial.println("SPIFFS initialized successfully");
}

void glowLED(int status){
  
  if(status==404){
    // red (not found)
    digitalWrite(D0, HIGH);
    delay(500);  // 1 second delay
    digitalWrite(D0, LOW);

  }else if(status==403){
    // yellow (fees due)
    digitalWrite(D1, HIGH);
    delay(500);  // 0.5 second delay
    digitalWrite(D1, LOW);

  }else if(status==200){
    // green (ok)
    digitalWrite(D2, HIGH);
    delay(500);  // 1 second delay
    digitalWrite(D2, LOW);

  }else{
    // blue (server error)
    digitalWrite(D8, HIGH);
    delay(500);  // 1 second delay
    digitalWrite(D8, LOW);

  }
}

String getRfid(){
  String rfid = "";
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Print UID of the tag/card
    // Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      // Serial.print(mfrc522.uid.uidByte[i], HEX);
      rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    // Serial.println();
    

    // Store UID in a variable
    rfid.toUpperCase();
    // Serial.println("Detected UID: " + rfid);
    // delay(2000);

    // Halt PICC
    mfrc522.PICC_HaltA();
    return rfid;
  }
  return "";
}

int sendToBackend(){
    WiFiClient client;
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Specify content-type header
    // Data to send with HTTP POST
    
    http.addHeader("Content-Type", "application/json");

    // {"rfid": "adsfas", "time": "1231./123/12"}
    // date is static for now (either get dedicated hardware or use wifi)
    String date = "2012-04-23T18:25:43.511Z";
    String data = "{\"rfid\":\"" + rfid + "\", \"" + "time\":\"" + date + "\"}";
    // Serial.println(data);
    int httpResponseCode = http.POST(data); 
    // delay(5000);
    // Serial.print("HTTP Response code: ");
    // Serial.println(httpResponseCode);
      
    // Free resources
    http.end();

    return httpResponseCode;
}

int logAndAuth(){
  // Open the file for reading
  File file = SPIFFS.open(databaseFilePath, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return 500;
  }

  // Read data line by line
  while (file.available()) {
    // Read a line from the file
    String line = file.readStringUntil('\n');
    // Print the data to Serial (you can modify this part as needed)
    // Serial.println(line);

    // Find the position of the space character
    int spaceIndex = line.indexOf(' ');

    // Check if the space character exists
    if (spaceIndex != -1) {
        // Extract RFID and expiry date using substring
        String id = line.substring(0, spaceIndex);
        String exp = line.substring(spaceIndex + 1);

        // currentDateTime to be written !!!!!!!!!!!!
        String currentDateTime=exp;
        if(rfid==id){
          // Close the file
          file.close();
          
          if(currentDateTime==exp){
            // log
            File log = SPIFFS.open(logFilePath, "a");
            if (!log) {
              Serial.println("Failed to open file for writing");
              return 500;
            }
            log.println(rfid+" "+currentDateTime);
            Serial.println("log created");
            log.close();
            return 200;
          }else {
            return 403;
          }
        }
    } else {
        // Handle the case where the line doesn't contain a space character
        Serial.println("Invalid format in line: " + line);
    }
  }


  // Close the file
  file.close();
  return 404;
}

void getAllData(){
  WiFiClient client;
  HTTPClient http;
  File localRFIDDataFile = SPIFFS.open(databaseFilePath, "w");
  if (!localRFIDDataFile) {
    Serial.println("Failed to create local RFID data file");
    return;
  }
  // StaticJsonBuffer<300> JSONBuffer;
  JsonDocument doc;
  for(int i = 1;;i++){
    // Convert integer to string
    String pageParam = String(i);
    String serverPath ="http://192.168.1.104:3000/allData?page="+pageParam;

    // Your Domain name with URL path or IP address with path
    if(iswifi)http.begin(client, serverPath.c_str());
    else{
      iswifi=false;
      gotdata=false;
      break;
    }

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
    
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    
    if (httpResponseCode==200) {
      // Serial.print("HTTP Response code: ");
      // Serial.println(httpResponseCode);

      String payload = http.getString();
      // Serial.println(payload);

      deserializeJson(doc, payload);
      arr = doc.as<JsonArray>();
      for (JsonObject obj: arr){
        // Serial.println(obj["rfid"].as<String>());
        // Serial.println(obj["expiry_date"].as<String>());
        // Serial.println(arr.size());

        // Write the chunk to the local file
        String dataChunk = obj["rfid"].as<String>() + " " + obj["expiry_date"].as<String>() ;
        localRFIDDataFile.println(dataChunk);
      }
    }
    else if(httpResponseCode==404){gotdata=true;break;}
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
  }
  
  // Free resources
  http.end();
  localRFIDDataFile.close();
}

void readAllDataFromFile(String path) {
  // Open the file for reading
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Read data line by line
  Serial.println("data: ");
  while (file.available()) {
    // Read a line from the file
    String data = file.readStringUntil('\n');

    // Print the data to Serial (you can modify this part as needed)
    Serial.println(data);

  }

  // Close the file
 file.close();
}