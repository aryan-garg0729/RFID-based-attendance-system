#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>

// CONSTANTS FOR WIFI CONNECTION
const char *SSID = "Note8";
const char *PASSWORD = "watermelon";

unsigned long lastMillis = 0;
const unsigned long interval = 1000; // 1 second interval to update time every second
int DEFAULT_TIME[3] = {0, 0, 0};     // HOURS MINUTES SECONDS
int CURR_TIME[3] = {0, 0, 0};
int CURR_DATE[3] = {1970, 1, 1}; // YEAR MONTH DATE
bool gotCurrDateAndTime = false;

// Constants for file paths
const char *DATABASE_FILE_PATH = "/database.txt"; // RFID EXP_YEAR EXP_MONTH EXP_DATE
const char *LOG_FILE_PATH = "/log.txt";

#define RST_PIN D3                // Define the GPIO pin connected to the RFID reader's RST pin
#define SS_PIN D4                 // Define the GPIO pin connected to the RFID reader's SS pin
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Your Domain name with URL path or IP address with path
String LOCALHOST = "http://192.168.71.144:4000";
String SERVER_NAME = LOCALHOST + "/student";
String STORE = LOCALHOST + "/student/store";
String rfid = ""; // Variable to store the detected RFID UID
JsonArray arr;

bool iswifi = false;
bool gotdata = false;
bool isSpiFFS = false;

// MY FUNCTIONS
bool initializeSPIFFS();
void glowLED(int status);
String getRfid();
int sendToBackend();
int logAndAuth();
void getAllData();
void readAllDataFromFile(String path);
bool connectToWifi();

void setup()
{
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D8, OUTPUT);

  // setting baud rate for serial communication
  Serial.begin(9600);
  delay(10);
  // ISKO INFINITY TIMES NAHI CHALALNA IF WIFI AND GETDATA NAHI MILA TO
  while (!gotdata)
  {
    iswifi = connectToWifi();

    // mandatory to initialize NODEMCU file system
    while (!isSpiFFS)
    {
      isSpiFFS = initializeSPIFFS();
      // delay(500);
    }
    // getalldata from database if wifi is connected
    if (WiFi.status() == WL_CONNECTED)
    {
      getAllData();
      setcurrdatetime();
    }
  }

  // printing database.txt file
  Serial.println("Successfully fetched and stored all data");
  readAllDataFromFile(DATABASE_FILE_PATH);

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

  //  Creating an empty log file
  // File log = SPIFFS.open(LOG_FILE_PATH, "a");
  // if (!log)
  // {
  //   Serial.println("Failed to open file for writing");
  // }
  // else
  // {
  //   log.close();
  //   Serial.println("log file created");
  // }
}

void loop()
{
  // return;
  updateTime();
  // if (CURR_TIME[0] >= 7 && CURR_TIME[0] <= 19)
  if (1)
  {
    // Scan for RFID tags/cards
    int status = 0;
    rfid = getRfid();
    if (rfid == "")
    {
      return;
    }
    // new
    status = logAndAuth();
    // status = 200;
    Serial.println("");
    Serial.println(status);
    glowLED(status);
  }
  else
  {
    sendToBackend();
  }
}

bool connectToWifi()
{
  // Connect to Wi-Fi network try 15 times
  int times = 1;
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
    delay(6000);

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
bool initializeSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to initialize SPIFFS");
    return 0;
  }
  Serial.println("SPIFFS initialized successfully");
  return 1;
}

void getAllData()
{
  WiFiClient client;
  HTTPClient http;
  File localRFIDDataFile;

  // StaticJsonBuffer<300> JSONBuffer;
  JsonDocument doc;
  for (int i = 1;; i++)
  {
    // Convert integer to string
    String pageParam = String(i);
    String serverPath = LOCALHOST + "/allData?page=" + pageParam;

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
    if (!localRFIDDataFile)
    {
      // opening a File in Write Mode
      localRFIDDataFile = SPIFFS.open(DATABASE_FILE_PATH, "w");
      if (!localRFIDDataFile)
      {
        Serial.println("Failed to create local RFID database file");
        return;
      }
    }
    // If you need Node-RED/server authentication, insert user and password below
    // http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

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

        // Write the chunk to the local file
        String dataChunk = obj["rfid"].as<String>() + " " + obj["expiry_date"].as<String>();
        localRFIDDataFile.println(dataChunk);
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
      i--;
    }
  }

  // Free resources
  http.end();
  localRFIDDataFile.close();
}

// FETCH CURRENT DATE AND TIME FROM SERVER IN 5 RETRY
void setcurrdatetime()
{
  // CALL TO BACKEND TO FETCH DATE AND TIME
  WiFiClient client;
  HTTPClient http;

  // WILL TRY 5 TIMES TO FETCH DATE AND TIME
  JsonDocument doc;
  for (int i = 1; i < 6; i++)
  {
    String serverPath = LOCALHOST + "/currentDateTime";
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

// UPDATE TIME EVERY INTERVAL
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
    digitalWrite(D8, HIGH);
    delay(500);
    digitalWrite(D8, LOW);
  }
}

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

int sendToBackend()
{
  String id;
  int year, month, date, hour, minute;
  File file = SPIFFS.open(LOG_FILE_PATH, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return 500;
  }

  // Read data line by line
  while (file.available())
  {

    // Read a line from the file
    String line = file.readStringUntil('\n');
    // Print the data to Serial (you can modify this part as needed)
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
      while (httpResponseCode != 200)
      {
        if (WiFi.status() != WL_CONNECTED)
        {
          connectToWifi();
          continue;
        }
        WiFiClient client;
        HTTPClient http;
        // http.setTimeout(2000);

        // Your Domain name with URL path or IP address with path
        http.begin(client, STORE);

        http.addHeader("Content-Type", "application/json");
        httpResponseCode = http.POST(data);
        http.end();
        Serial.println(httpResponseCode);
      }
    }
  }
  // erase
  // Open the log file in read mode

  // Find the size of the first line (the first entry)
  // size_t firstLineSize = 0;
  // while (file.available()) {
  //   char c = file.read();
  //   if (c == '\n') {
  //     break; // Found the end of the first line
  //   }
  //   firstLineSize++;
  // }
  // file.close();

  // Reopen the file in write mode and position the cursor after the first line
  // file = SPIFFS.open(LOG_FILE_PATH, "w");
  // if (!file) {
  //   Serial.println("Failed to open file for writing");
  //   return 0;
  // }
  // file.seek(firstLineSize + 1); // Position after the first line

  // Copy the rest of the file after the first line
  //       while (file.available()) {
  //         char c = file.read();
  //         file.write(c);
  //       }
  //       file.close();
  //     }
  // } else {
  //     // Handle the case where the line doesn't contain a space character
  //     Serial.println("Invalid format in line: " + line);
  //     file.close();
  // }
  // }

  // Close the file

  // ---------------------------

  // {"rfid": "adsfas", "time": "1231./123/12"}
  // date is static for now (either get dedicated hardware or use wifi)
  // String data = "{\"rfid\":\"" + id + "\", \"" + "time\":\"" + date + "\"}";
  // Serial.println(data);
  // int httpResponseCode = http.POST(data);
  // delay(5000);
  // Serial.print("HTTP Response code: ");
  // Serial.println(httpResponseCode);

  // Free resources
  // Serial.println("LOG FILE HAS BEEN SUCCESSFULLY SENT");
  file.close();
  file = SPIFFS.open(LOG_FILE_PATH, "w");
  if (!file)
  {
    Serial.println("Failed to erase");
    return 500;
  }
  file.close();

  // http.end();

  return 0;
}

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

int logAndAuth()
{
  // Open the file for reading
  File db_file = SPIFFS.open(DATABASE_FILE_PATH, "r");
  if (!db_file)
  {
    Serial.println("Failed to open db_file for reading");
    return 500;
  }

  // Read data line by line
  while (db_file.available())
  {
    // Read a line from the db_file

    String line = db_file.readStringUntil('\n');

    // Find the position of the space character
    int spaceIndex = line.indexOf(' ');

    // Check if the space character exists
    if (spaceIndex != -1)
    {
      // Extract RFID and expiry date using substring
      String id = line.substring(0, spaceIndex);
      // String exp = line.substring(spaceIndex + 1);
      int expDate[3];
      expDate[0] = (line.substring(spaceIndex + 1, spaceIndex + 5)).toInt();  // YEAR
      expDate[1] = (line.substring(spaceIndex + 6, spaceIndex + 8)).toInt();  // MONTH
      expDate[2] = (line.substring(spaceIndex + 9, spaceIndex + 11)).toInt(); // DATE
      // Serial.print("current date: ")
      // Serial.print(CURR_DATE[0]);
      // Serial.print("-");
      // Serial.print(CURR_DATE[1]);
      // Serial.print("-");
      // Serial.println(CURR_DATE[2]);

      if (rfid == id)
      {
        // Close the db_file
        db_file.close();

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
          log.println(rfid + " " + CURR_DATE[0] + " " + CURR_DATE[1] + " " + CURR_DATE[2] + " " + CURR_TIME[0] + " " + CURR_TIME[1] + " " + CURR_TIME[2]);
          Serial.println("log created");
          log.close();
          readAllDataFromFile(LOG_FILE_PATH);
          return 200;
        }
        else
        {
          return 403;
        }
      }
    }
    else
    {
      // Handle the case where the line doesn't contain a space character
      Serial.println("Invalid format in line: " + line);
    }
  }

  // Close the file
  db_file.close();
  return 404;
}

void readAllDataFromFile(String path)
{
  // Open the file for reading
  File file = SPIFFS.open(path, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

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
