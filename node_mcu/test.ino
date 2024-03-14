// #include <ESP8266WiFi.h>
// #include <SPI.h>
// #include <MFRC522.h>

// const char* ssid = "NSUT_WIFI";
// const char* password = "";

// #define RST_PIN D3  // Define the GPIO pin connected to the RFID reader's RST pin
// #define SS_PIN D4   // Define the GPIO pin connected to the RFID reader's SS pin

// MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
// String rfid = ""; // Variable to store the detected RFID UID

// void sendToBackend(String uid);

// void setup() {
//   Serial.begin(9600);
//   delay(10);

//   // Connect to Wi-Fi network
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());

//   // Initialize RFID reader
//   SPI.begin();
//   mfrc522.PCD_Init();
//   Serial.println("RFID reader initialized");
// }

// void loop() {
//   // Scan for RFID tags/cards
//   /*
//   if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
//     // Print UID of the tag/card
//     Serial.print("Card UID: ");
//     for (byte i = 0; i < mfrc522.uid.size; i++) {
//       Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
//       Serial.print(mfrc522.uid.uidByte[i], HEX);
//       rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
//       rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
//     }
//     Serial.println();
    
//     // Store UID in a variable
//     rfid.toUpperCase();
//     Serial.println("Detected UID: " + rfid);
//     delay(2000);

//     // Halt PICC
//     mfrc522.PICC_HaltA();
//     rfid = ""; // Clear the variable for the next detection*/

//   }
