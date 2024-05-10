#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>

// CONSTANTS FOR WIFI CONNECTION
const char *SSID = "Airtel_isha_6157";
const char *PASSWORD = "air04221";

unsigned long lastMillis = 0;
const unsigned long interval = 1000; // 1 second interval to update time every second
int DEFAULT_TIME[3] = {7, 0, 0};     // HOURS MINUTES SECONDS
int CURR_TIME[3] = {0, 0, 0};
int CURR_DATE[3] = {1970, 1, 1}; // YEAR MONTH DATE

// Constants for file paths
const char *DATABASE_FILE_PATH = "/database.txt"; // RFID EXP_YEAR EXP_MONTH EXP_DATE
const char *LOG_FILE_PATH = "/log.txt";
const char *FILE_POINTER_FILE = "/filepointer.txt";

#define RST_PIN D3                // Define the GPIO pin connected to the RFID reader's RST pin
#define SS_PIN D4                 // Define the GPIO pin connected to the RFID reader's SS pin
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Your Domain name with URL path or IP address with path
String LOCALHOST = "https://rfid-based-attendance-system-1j2o.onrender.com";
// Your Domain name with URL path or IP address with path
String admin_serverName = LOCALHOST + "/admin";
String SERVER_NAME = LOCALHOST + "/student";
String STORE = LOCALHOST + "/student/store";
String MASTER = LOCALHOST + "/admin/master/names";
String rfid = ""; // Variable to store the detected RFID UID
JsonArray arr;

bool iswifi = false;
bool gotdata = false;
bool gotCurrDateAndTime = false;
bool gotmasters = false;
bool isSpiFFS = false;
bool isAdmin = false;

String masters[3];
String TOGGLE_RFID = "B1A8891D";

// MY 14 FUNCTIONS (IN ORDER)
bool connectToWifi();
void initializeSPIFFS();
void getAllData();
void extractStrings(String payload);
void getMasters();
void setcurrdatetime();
void updateTime();
String getRfid();
void glowLED(int status);
void extract_rfid_date_time(String &line, String &id, int &year, int &month, int &date, int &hour, int &minute);
int readLastFilePointer();
void writeCurrentFilePointer(int lineNumber);
int sendToBackend();
bool checkExpiry(int expDate[]);
int logAndAuth();
void readAllDataFromFile(String path);
void formatSPIFFS();
void sendToBackendAdmin();

void setup()
{
  // Set CPU frequency to 160MHz
  // system_update_cpu_freq(SYS_CPU_160MHZ);

  pinMode(D0, OUTPUT); // red
  pinMode(D1, OUTPUT); // yellow
  pinMode(D2, OUTPUT); // green
  pinMode(D8, OUTPUT); // blue (white)
  // digitalWrite(D8,HIGH);

  // CLEANUP: DANGER TO LOG FILE USE WITH CAUTION!!!
  formatSPIFFS();

  digitalWrite(D8, HIGH);

  // setting baud rate for serial communication
  Serial.begin(9600);
  delay(10);
  // ISKO INFINITY TIMES NAHI CHALALNA IF WIFI AND GETDATA NAHI MILA TO
  while (!gotdata || !gotCurrDateAndTime || !gotmasters)
  {
    iswifi = connectToWifi();

    // mandatory to initialize NODEMCU file system
    initializeSPIFFS();

    // getalldata from database if wifi is connected
    if (WiFi.status() == WL_CONNECTED)
    {
      if (!gotdata)
        getAllData();
      // gotdata = 1;
      if (!gotCurrDateAndTime)
        setcurrdatetime();
      if (!gotmasters)
        getMasters();
    }
  }

  // printing database.txt file
  Serial.println("Successfully fetched and stored all data");

  Serial.print("current date: ");
  Serial.print(CURR_DATE[0]);
  Serial.print("-");
  Serial.print(CURR_DATE[1]);
  Serial.print("-");
  Serial.println(CURR_DATE[2]);

  // Initialize RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  delay(100);
  Serial.println("RFID reader initialized");

  // // Creating an empty log file
  // File log = SPIFFS.open(LOG_FILE_PATH, "w");
  // if (!log)
  // {
  //   Serial.println("Failed to open file for writing");
  // }
  // else
  // {
  //   log.close();
  //   Serial.println("log file created");
  // }

  digitalWrite(D8, LOW);
}

void loop()
{
  if (isAdmin)
  {
    digitalWrite(D0, HIGH);
    // Scan for RFID tags/cards
    rfid = getRfid();
    if (rfid == "")
    {
      return;
    }
    if (rfid == TOGGLE_RFID)
    {
      digitalWrite(D0, LOW);
      isAdmin = !isAdmin;
      return;
    }
    sendToBackendAdmin();
    return;
  }
  updateTime();
  if (CURR_TIME[0] >= 7 && CURR_TIME[0] <= 19)   //MORNING 7AM to EVE 7PM
  // if (1) // for testing perpose
  {
    // Scan for RFID tags/cards
    int status = 0;
    rfid = getRfid();
    if (rfid == "")
    {
      return;
    }
    if (rfid == "B1A8891D")
    {
      isAdmin = !isAdmin;
      return;
    }
    // compare masters
    for (int i = 0; i < 3; i++)
    {
      if (masters[i] == rfid)
      {
        digitalWrite(D8, HIGH);
        sendToBackend();
        digitalWrite(D8, LOW);
        return;
      }
    }

    status = logAndAuth();
    Serial.println("");
    Serial.println(status);
    glowLED(status);
  }
  else
  {
    digitalWrite(D8, HIGH);
    sendToBackend();
    digitalWrite(D8, LOW);
    // CLEANUP: DANGER TO LOG FILE USE WITH CAUTION!!!
    formatSPIFFS();
    // sir se puchna h format file ka
  }
}

bool connectToWifi()
{
  // Connect to Wi-Fi network try 15 times
  int times = 0;
  while (WiFi.status() != WL_CONNECTED && times < 6)
  {
    Serial.println();
    Serial.print("Connecting to: ");
    Serial.println(SSID);
    Serial.println("..........");
    Serial.print("Retrying Times: ");
    Serial.println(times);
    // connecting to wifi
    WiFi.begin(SSID, PASSWORD);
    delay(7000);
    times++;
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

  return (WiFi.status() == WL_CONNECTED);
}

// Function to initialize SPIFFS
void initializeSPIFFS()
{
  while (!SPIFFS.begin())
  {
    delay(1000);
    Serial.println("Failed to initialize SPIFFS");
  }
  Serial.println("SPIFFS initialized successfully");
}

// fetch all data from server and save to database.txt file
void getAllData()
{
  WiFiClient client;
  HTTPClient http;

  // StaticJsonBuffer<300> JSONBuffer;
  JsonDocument doc;
  for (int i = 1;; i++)
  {
    // initializeSPIFFS();
    // Convert integer to string
    String pageParam = String(i);
    String serverPath = LOCALHOST + "/student/allData?page=" + pageParam;

    // Your Domain name with URL path or IP address with path
    if (WiFi.status() == WL_CONNECTED)
      http.begin(client, serverPath.c_str());
    else
    {
      Serial.println("Wifi disconnected can't connect to server");
      iswifi = false;
      gotdata = false;
      break;
    }

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {

      String payload = http.getString();

      deserializeJson(doc, payload);
      arr = doc.as<JsonArray>();
      for (JsonObject obj : arr)
      {
        // Serial.println(obj["rfid"].as<String>());
        // Serial.println(obj["expiry_date"].as<String>());
        // Serial.println(arr.size());

        // creating file for each rfid name record
        String filename = "/" + obj["rfid"].as<String>() + ".txt";
        String dataChunk = obj["expiry_date"].as<String>();
        File rfidFile = SPIFFS.open(filename, "w");
        if (!rfidFile)
        {
          Serial.println("Failed to create local RFID mini file");
          rfidFile.close();
          continue;
        }
        rfidFile.println(dataChunk);
        rfidFile.close();
      }
    }
    else if (httpResponseCode == 404)
    {
      gotdata = true;
      break;
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      // glow led
      glowLED(500);
      i--;
    }
  }

  // Free resources
  http.end();
}

void extractStrings(String payload)
{
  int startIdx = payload.indexOf("\"") + 1; // Find the first occurrence of "
  int endIdx;
  for (int i = 0; i < 3; i++)
  {
    endIdx = payload.indexOf("\"", startIdx); // Find the next occurrence of "
    if (endIdx != -1)
    {
      masters[i] = payload.substring(startIdx, endIdx);
      startIdx = payload.indexOf("\"", endIdx + 1) + 1; // Move to the next string
    }
  }
}

void getMasters()
{
  JsonDocument doc;
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, MASTER.c_str());

    int httpCode = http.GET();

    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        // Deserialize the JSON object into the JSON document
        DeserializationError error = deserializeJson(doc, payload);
        Serial.println("Response:");
        Serial.println(payload);
        // put into master array
        extractStrings(payload);
        gotmasters = true;
      }
    }
    else
    {
      gotmasters = false;
      Serial.printf("[HTTP] GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  else
  {
    gotmasters = false;
    Serial.println("WiFi not connected");
  }
}

// FETCH CURRENT DATE AND TIME FROM SERVER IN 5 RETRY (or use default one)
void setcurrdatetime()
{
  // CALL TO BACKEND TO FETCH DATE AND TIME
  WiFiClient client;
  HTTPClient http;

  // WILL TRY 5 TIMES TO FETCH DATE AND TIME
  JsonDocument doc;
  for (int i = 1; i < 6; i++)
  {
    String serverPath = LOCALHOST + "/student/currentDateTime";
    // Your Domain name with URL path or IP address with path
    if (WiFi.status() == WL_CONNECTED)
    {
      http.begin(client, serverPath.c_str());
    }
    else
    {
      Serial.println("Wifi disconnected can't connect to server");
      iswifi = false;
      gotCurrDateAndTime = false;
      break;
    }
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    delay(100);
    if (httpResponseCode == 200)
    {

      String payload = http.getString();

      // Deserialize the JSON object into the JSON document
      DeserializationError error = deserializeJson(doc, payload);
      // Check for parsing errors
      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        continue;
      }
      gotCurrDateAndTime = true;
      CURR_DATE[0] = doc["year"];
      CURR_DATE[1] = doc["month"];
      CURR_DATE[2] = doc["date"];
      DEFAULT_TIME[0] = doc["hours"];
      DEFAULT_TIME[1] = doc["minutes"];
      DEFAULT_TIME[2] = doc["seconds"];
      Serial.println("Successfully fetched current date and time");
      break;
    }
    else
    {
      Serial.print("Server Error code: ");
      Serial.println(httpResponseCode);
    }
  }
  if (gotCurrDateAndTime == false)
  {
    Serial.println("Failed to fetch current Date and Time");
  }

  // Free resources
  http.end();
}

// UPDATE CURRETN TIME
void updateTime()
{
  unsigned long milliseconds = millis();
  // Convert milliseconds to seconds
  unsigned long totalSeconds = milliseconds / 1000;

  // Calculate hours
  int hours = totalSeconds / 3600;

  // Calculate remaining seconds after removing hours
  unsigned long remainingSeconds = totalSeconds % 3600;

  // Calculate minutes
  int minutes = remainingSeconds / 60;

  // Calculate remaining seconds after removing minutes
  int seconds = remainingSeconds % 60;

  CURR_TIME[0] = DEFAULT_TIME[0] + hours;
  CURR_TIME[1] = DEFAULT_TIME[1] + minutes;
  CURR_TIME[2] = DEFAULT_TIME[2] + seconds;

  CURR_TIME[1] += CURR_TIME[2] / 60;
  CURR_TIME[2] %= 60;

  CURR_TIME[0] += CURR_TIME[1] / 60;
  CURR_TIME[1] %= 60;

  CURR_TIME[0] %= 24;

  // Print current time
  // Serial.print("Current time: ");
  // Serial.print(CURR_TIME[0]);
  // Serial.print(":");
  // Serial.print(CURR_TIME[1]);
  // Serial.print(":");
  // Serial.println(CURR_TIME[2]);
}

// scan rfid reader
String getRfid()
{
  String rfid = "";
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    // Print UID of the tag/card
    // Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      // Serial.print(mfrc522.uid.uidByte[i], HEX);
      rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    // Serial.println();

    // Store UID in a variable
    rfid.toUpperCase();
    Serial.println("Detected UID: " + rfid);
    // delay(2000);

    // Halt PICC to scan new tags
    mfrc522.PICC_HaltA();
    return rfid;
  }
  return "";
}

// blink led based on status
void glowLED(int status)
{

  if (status == 404)
  {
    // red (not found)
    digitalWrite(D0, HIGH);
    delay(500); // 1 second delay
    digitalWrite(D0, LOW);
  }
  else if (status == 403)
  {
    // yellow (fees due)
    digitalWrite(D1, HIGH);
    delay(500); // 0.5 second delay
    digitalWrite(D1, LOW);
  }
  else if (status == 200)
  {
    // green (ok)
    digitalWrite(D2, HIGH);
    delay(500); // 0.5 second delay
    digitalWrite(D2, LOW);
  }
  else
  {
    // blue (server error)
    digitalWrite(D8, LOW);
    delay(100);
    digitalWrite(D8, HIGH);
  }
}

// extract rfid, year, month, date, hour, minute from "log.file" line to send to server
void extract_rfid_date_time(String &line, String &id, int &year, int &month, int &date, int &hour, int &minute)
{
  // LINE : RFID YYYY MM DD HH MM
  //  Find the index of the first space to separate RFID from the date-time components
  int spaceIndex = line.indexOf(' ');

  // Extract RFID
  id = line.substring(0, spaceIndex);
  // Serial.println("RFID: " + id);

  // Extract year
  int startIndex = spaceIndex + 1;
  int endIndex = line.indexOf(' ', startIndex);
  String yearStr = line.substring(startIndex, endIndex);
  year = yearStr.toInt();
  // Serial.println("Year: " + String(year));

  // Extract month
  startIndex = endIndex + 1;
  endIndex = line.indexOf(' ', startIndex);
  String monthStr = line.substring(startIndex, endIndex);
  month = monthStr.toInt();
  // Serial.println("Month: " + String(month));

  // Extract date
  startIndex = endIndex + 1;
  endIndex = line.indexOf(' ', startIndex);
  String dateStr = line.substring(startIndex, endIndex);
  date = dateStr.toInt();
  // Serial.println("Date: " + String(date));

  // Extract hour
  startIndex = endIndex + 1;
  endIndex = line.indexOf(' ', startIndex);
  String hourStr = line.substring(startIndex, endIndex);
  hour = hourStr.toInt();
  // Serial.println("Hour: " + String(hour));

  // Extract minute
  startIndex = endIndex + 1;
  endIndex = line.indexOf(' ', startIndex);
  String minuteStr = line.substring(startIndex, endIndex);
  minute = minuteStr.toInt();
  // Serial.println("Minute: " + String(minute));
}

// Function to read the last stored file pointer position from filepointer.txt
int readLastFilePointer()
{
  File file = SPIFFS.open(FILE_POINTER_FILE, "r");
  if (!file)
  {
    return 0; // File pointer file does not exist
  }
  int lineNumber = 0;
  file.readBytes((char *)&lineNumber, sizeof(int)); // Read the integer value
  file.close();
  return lineNumber;
}
// Function to write the current file pointer position to filepointer.txt
void writeCurrentFilePointer(int lineNumber)
{
  File file = SPIFFS.open(FILE_POINTER_FILE, "w");
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.write((const uint8_t *)&lineNumber, sizeof(int)); // Write the integer value
  file.close();
}

// send line by line data to server from log.txt file
int sendToBackend()
{
  int startLine = readLastFilePointer(); // StartLine from where data is going to be send
  String id;
  int year, month, date, hour, minute;
  File file = SPIFFS.open(LOG_FILE_PATH, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return 500;
  }

  // Set the file pointer to the specified line number
  for (int i = 0; i < startLine; i++)
  {
    file.readStringUntil('\n');
  }

  // Read data line by line
  while (file.available())
  {

    // Read a line from the file
    String line = file.readStringUntil('\n');
    // Serial.println(line);

    int spaceIndex = line.indexOf(' ');
    extract_rfid_date_time(line, id, year, month, date, hour, minute);
    // Check if the space character exists
    if (spaceIndex != -1)
    {
      // Extract RFID and expiry date using substring
      String data = "{\"rfid\":\"" + id + "\", \"year\":\"" + year + "\", \"month\":\"" + month + "\", \"date\":\"" + date + "\", \"hour\":\"" + hour + "\", \"minute\":\"" + minute + "\"}";
      int httpResponseCode = 0;
      // Serial.println(data);
      // Serial.println(startLine);
      while (httpResponseCode != 200)
      {

        if (WiFi.status() != WL_CONNECTED)
        {
          connectToWifi();
          continue;
        }
        WiFiClient client;
        HTTPClient http;

        // Your Domain name with URL path or IP address with path
        http.begin(client, STORE);

        http.addHeader("Content-Type", "application/json");
        httpResponseCode = http.POST(data);
        http.end();
        Serial.println(httpResponseCode);
        glowLED(500);
      }

      // udpating log filepointer startline for each line
      startLine++;
      writeCurrentFilePointer(startLine);
      delay(300);
    }
  }

  // Free resources
  // LOG FILE HAS BEEN SUCCESSFULLY SENT
  file.close();
  // truncating log file
  file = SPIFFS.open(LOG_FILE_PATH, "w");
  if (!file)
  {
    Serial.println("Failed to erase");
    return 500;
  }
  file.close();

  // resetting startline file pointer to zero
  writeCurrentFilePointer(0);
  // http.end();

  return 0;
}

// check card is expired or not
bool checkExpiry(int expDate[])
{
  // Compare the date components YEAR MONTH DATE
  if ((CURR_DATE[0] > expDate[0]) ||
      (CURR_DATE[0] == expDate[0] && CURR_DATE[1] > expDate[1]) ||
      (CURR_DATE[0] == expDate[0] && CURR_DATE[1] == expDate[1] && CURR_DATE[2] > expDate[2]))
  {
    Serial.println("Card is Expired");
    return 0;
  }
  else
  {
    Serial.println("Card is Valid");
    return 1;
  }
}

// check card is valid and store it into log.txt file with current date and time
int logAndAuth()
{
  // Open the file for reading
  String id = rfid;
  String filename = "/" + id + ".txt";
  File rfidFile = SPIFFS.open(filename, "r");
  if (!rfidFile)
  {
    // user not found
    Serial.println("Failed to open rfidFile for reading");
    return 404;
  }
  else
  {
    Serial.println("Inside logAndAuth else part found the file");

    String line = rfidFile.readStringUntil('\n');
    if (line.length() == 0)
    {
      Serial.println("Invalid line format");
      return 500;
    }
    int expDate[3];
    expDate[0] = (line.substring(0, 4)).toInt();  // YEAR
    expDate[1] = (line.substring(5, 7)).toInt();  // MONTH
    expDate[2] = (line.substring(8, 10)).toInt(); // DATE

    if (checkExpiry(expDate))
    {
      // log
      File log = SPIFFS.open(LOG_FILE_PATH, "a");
      if (!log)
      {
        Serial.println("Failed to open file for writing");
        return 500;
      }
      updateTime();
      // log file contains data in the format (RFID CURR_DATE(YEAR, MONTH, DATE) CURR_TIME(HOURS, MINUTES, SECONDS))
      log.println(id + " " + CURR_DATE[0] + " " + CURR_DATE[1] + " " + CURR_DATE[2] + " " + CURR_TIME[0] + " " + CURR_TIME[1] + " " + CURR_TIME[2]);
      Serial.println("log created");
      log.close();
      readAllDataFromFile(LOG_FILE_PATH);
      rfidFile.close();
      // valid card
      return 200;
    }

    rfidFile.close();
    // card is expired
    return 403;
  }
}

// Prints all data of a file
void readAllDataFromFile(String path)
{
  // Open the file for reading
  File file = SPIFFS.open(path, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading inside readAllDataFromFile");
    return;
  }

  Serial.println("reading inside readAllDataFromFile");
  // Read data line by line
  Serial.println("data: ");
  while (file.available())
  {
    // Read a line from the file
    String data = file.readStringUntil('\n');

    // Print the data to Serial (you can modify this part as needed)
    Serial.println(data);
  }

  // Close the file
  file.close();
}

// format all files and data
void formatSPIFFS()
{
  Serial.println("Formatting SPIFFS...");
  if (SPIFFS.format())
  {
    Serial.println("SPIFFS formatted successfully.");
  }
  else
  {
    Serial.println("Error formatting SPIFFS!");
  }
}

void sendToBackendAdmin()
{
  WiFiClient client;
  HTTPClient http;

  connectToWifi();
  // Your Domain name with URL path or IP address with path
  http.begin(client, admin_serverName);

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
  // Free resources
  http.end();
}
