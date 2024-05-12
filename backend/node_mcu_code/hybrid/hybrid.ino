#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <FS.h>

#define RST_PIN D3 // Define the GPIO pin connected to the RFID reader's RST pin
#define SS_PIN D4  // Define the GPIO pin connected to the RFID reader's SS pin

// CONSTANTS FOR WIFI CONNECTION
const char *SSID = "Note8";
const char *PASSWORD = "watermelon";

unsigned long lastMillis = 0;
const unsigned long interval = 1000; // 1 second interval to update time every second
int DEFAULT_TIME[3] = {7, 0, 0};     // HOURS MINUTES SECONDS "07:00 AM"
int CURR_TIME[3] = {0, 0, 0};
int CURR_DATE[3] = {1970, 1, 1}; // YEAR MONTH DATE
String masters[3] = {"#", "#", "#"};

// Constants for file paths
const char *DATABASE_FILE_PATH = "/database.txt";   // RFID EXP_YEAR EXP_MONTH EXP_DATE
const char *LOG_FILE_PATH = "/log.txt";             // stores attendance log
const char *FILE_POINTER_FILE = "/filepointer.txt"; // file pointer to log file that contain start line
const char *MASTERS_FILE = "/masters.txt";
const char *DATE_TIME_FILE = "/datetime.txt";

// Domain name with URL path or IP address with path
String LOCALHOST = "https://rfid-based-attendance-system-1j2o.onrender.com";
const char *host = "rfid-based-attendance-system-1j2o.onrender.com"; // for POST requests
String STORE = LOCALHOST + "/student/store";
String MASTER = LOCALHOST + "/admin/master/names";
String rfid = ""; // Variable to store the detected RFID UID
String TOGGLE_RFID = "B1A8891D";
JsonArray arr;

bool iswifi = false;
bool gotdata = false;
bool gotCurrDateAndTime = false;
bool gotmasters = false;
bool isSpiFFS = false;
bool isAdmin = false;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// MY FUNCTIONS USED (IN ORDER)
bool connectToWifi();
void initializeSPIFFS();
void getAllData();
void extractStrings(String payload);
void getMasters();
void fetchPreviousDateTime();
void storeDatetimeToFile();
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
void blinkLEDTimes(String LED, int times, int delay, String returning_state);
void blinkAllLEDs(int times, int dlay);
void sendToBackendAdmin();

// first setup called
void setup()
{
    // Set CPU frequency to 160MHz
    // system_update_cpu_freq(SYS_CPU_160MHZ);

    // Initialize pins
    pinMode(D0, OUTPUT); // red    🔴
    pinMode(D1, OUTPUT); // yellow 🟡
    pinMode(D2, OUTPUT); // green  🟢
    pinMode(D8, OUTPUT); // white  🤍

    // CLEANUP: DANGER TO LOG FILE USE WITH CAUTION!!!
    // formatSPIFFS();

    digitalWrite(D8, HIGH);

    // setting baud rate for serial communication
    Serial.begin(9600);
    delay(10);

    // mandatory to initialize NODEMCU file system
    initializeSPIFFS();

    int loopTimes = 2;

    while (loopTimes > 0 && (!gotdata || !gotCurrDateAndTime || !gotmasters))
    {
        loopTimes--;
        iswifi = connectToWifi();

        // getalldata from database if wifi is connected
        // gotdata = true;
        // gotCurrDateAndTime = true;
        // gotmasters = true;
        if (!gotCurrDateAndTime)
            setcurrdatetime();
        if (!gotmasters)
            getMasters();
        if (!gotdata)
            getAllData();
        yield();
    }

    // printing database.txt file
    Serial.println("Successfully fetched and stored all data");

    Serial.print("current date: ");
    Serial.print(CURR_DATE[0]);
    Serial.print("-");
    Serial.print(CURR_DATE[1]);
    Serial.print("-");
    Serial.println(CURR_DATE[2]);

    // Serial.print("Default time: ");
    // Serial.print(DEFAULT_TIME[0]);
    // Serial.print("-");
    // Serial.print(DEFAULT_TIME[1]);
    // Serial.print("-");
    // Serial.println(DEFAULT_TIME[2]);

    // Printing masters
    Serial.print("Masters : ");
    for (int i = 0; i < 3; i++)
    {
        Serial.print(masters[i]);
        Serial.print(" , ");
    }
    Serial.println();
    // Initialize RFID reader
    SPI.begin();
    mfrc522.PCD_Init();
    delay(100);
    Serial.println("RFID reader initialized");

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
    if (CURR_TIME[0] >= 1 && CURR_TIME[0] <= 14) // MORNING 7AM to EVE 9PM
    // if (1)  //for testing perpose
    {
        // Scan for RFID tags/cards
        int status = 0;
        rfid = getRfid();
        if (rfid == "")
        {
            return;
        }
        if (rfid == TOGGLE_RFID)
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
        // formatSPIFFS();
        // sir se puchna h format file ka
    }
}

// connect to WiFi
bool connectToWifi()
{
    // Connect to Wi-Fi network within 6 trials
    int times = 0;
    digitalWrite(D1, HIGH);
    while (WiFi.status() != WL_CONNECTED && times < 3)
    {
        yield();
        Serial.println();
        Serial.print("Connecting to: ");
        Serial.println(SSID);
        Serial.print("..........");
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
            // yellow light blinks 5 time when connected to wifi else not
            blinkLEDTimes("D1", 5, 500, "LOW");
            break;
        }
    }
    digitalWrite(D1, LOW);
    return (WiFi.status() == WL_CONNECTED);
}

// Function to initialize SPIFFS
void initializeSPIFFS()
{
    while (!SPIFFS.begin())
    {
        delay(500);
        Serial.println("Failed to initialize SPIFFS");
    }
    Serial.println("SPIFFS initialized successfully");
}

// fetch all data from server and save to database.txt file
void getAllData()
{

    Serial.println("Inside getAllData");
    WiFiClientSecure client;
    HTTPClient http;
    client.setInsecure();
    yield();
    http.setTimeout(20000);
    JsonDocument doc;
    for (int i = 1;; i++)
    {

        String pageParam = String(i);
        String serverPath = LOCALHOST + "/student/allData?page=" + pageParam;
        Serial.println("SERVER PATH: ");
        Serial.println(serverPath);
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Connecting to server...");
            http.begin(client, serverPath.c_str());
        }
        else
        {
            Serial.println("WiFi disconnected, can't connect to server");
            iswifi = false;
            gotdata = false;
            break;
        }

        int httpResponseCode = http.GET();
        delay(500);
        // Print headers
        Serial.println("Response Headers:");
        // String headers = http.getString();
        // Serial.println(headers);

        Serial.print("httpResponseCode: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode == 200)
        {
            // Serial.println("Response code 200, data fetched successfully");

            String payload = http.getString();
            DeserializationError jsonError = deserializeJson(doc, payload);
            if (jsonError)
            {
                Serial.print("JSON deserialization error: ");
                Serial.println(jsonError.c_str());
                // Handle the error (e.g., return or break)
                i--;
                yield();
                continue;
            }

            arr = doc.as<JsonArray>();
            for (JsonObject obj : arr)
            {
                String filename = "/" + obj["rfid"].as<String>() + ".txt";
                String dataChunk = obj["expiry_date"].as<String>();
                File rfidFile = SPIFFS.open(filename, "w");
                if (!rfidFile)
                {
                    Serial.println("Failed to create local RFID mini file");
                    rfidFile.close();
                    continue;
                }
                // Serial.println(dataChunk);
                rfidFile.println(dataChunk);
                rfidFile.close();
            }
            Serial.print("Header Length: ");
            Serial.print(arr.size());
            gotdata = true;
        }
        else if (httpResponseCode == 404)
        {
            Serial.println("Data not found (404)");
            gotdata = true;
            blinkAllLEDs(4, 500);
            break;
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
            glowLED(500); // Assuming glowLED function is for an LED indication
            i--;
        }
        http.end();
    }

    Serial.println("Exiting getAllData");
}

// extract strings from payload into masters.txt file and masters array (helping fucntion for getMasters())
void extractStrings(String payload)
{
    File file = SPIFFS.open(MASTERS_FILE, "w"); // Open "masters.txt" in write mode
    if (!file)
    {
        Serial.println("Failed to open master file for writing");
        glowLED(500);
        // return;
    }
    int startIdx = payload.indexOf("\"") + 1; // Find the first occurrence of "
    int endIdx;
    for (int i = 0; i < 3; i++)
    {
        endIdx = payload.indexOf("\"", startIdx); // Find the next occurrence of "
        if (endIdx != -1)
        {
            masters[i] = payload.substring(startIdx, endIdx);
            if (file)
                file.println(masters[i]);
            startIdx = payload.indexOf("\"", endIdx + 1) + 1; // Move to the next string
        }
    }
    file.close();
}

// get masters rfid to perform sendToBackend
void getMasters()
{
    // reading previous masters from masters file
    File file = SPIFFS.open(MASTERS_FILE, "r"); // Open "masters.txt" in read mode
    if (!file)
    {
        Serial.println("Failed to open master file for reading");
        glowLED(500);
    }
    else
    {
        // Read the strings from the file and assign them to the masters array
        for (int i = 0; i < 3; i++)
        {
            if (file.available())
            {
                masters[i] = file.readStringUntil('\n');
            }
            else
            {
                // Serial.println("Unexpected end of file");
                break;
            }
        }
    }
    file.close(); // Close the file

    JsonDocument doc;
    for (int i = 0; i < 3; i++)
    {
        // retry three times
        if (WiFi.status() == WL_CONNECTED)
        {
            HTTPClient http;
            WiFiClientSecure client;
            client.setInsecure();
            yield();

            http.setTimeout(10000);
            http.begin(client, MASTER.c_str());
            // delay(500);
            int httpCode = http.GET();
            delay(1000);
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = http.getString();
                // Deserialize the JSON object into the JSON document
                DeserializationError error = deserializeJson(doc, payload);
                if (error)
                {
                    Serial.print("deserializeJson() failed: ");
                    Serial.println(error.c_str());
                    glowLED(500);
                    gotmasters = false;
                    return;
                    // break;
                }
                // put into master array
                extractStrings(payload);
                gotmasters = true;
            }

            else
            {
                gotmasters = false;
                Serial.print("Error code");
                Serial.println(httpCode);
                Serial.printf(" Cannot get masters, error: %s\n", http.errorToString(httpCode).c_str());
                glowLED(500);
            }

            http.end();
        }
        else
        {
            gotmasters = false;
            Serial.println("WiFi not connected");
        }
    }
}

// helping function for setcurrdatetime()
void fetchPreviousDateTime()
{
    Serial.println("Inside fetchPreviousDateTime");

    int datetimeEntries[6] = {0, 0, 0, 0, 0, 0};
    File file = SPIFFS.open(DATE_TIME_FILE, "r"); // Open "masters.txt" in write mode
    if (!file)
    {
        Serial.println("Failed to open date time file for reading");
        glowLED(500);
    }

    else
    {
        Serial.println("Inside fetchPreviousDateTime else part");
        int entryIndex = 0;
        // Read datetime entries from the file
        while (file.available() && entryIndex < 6)
        {
            yield();
            String line = file.readStringUntil('\n');
            sscanf(line.c_str(), "%d", &datetimeEntries[entryIndex]);
            entryIndex++;
        }
        CURR_DATE[0] = datetimeEntries[0];
        CURR_DATE[1] = datetimeEntries[1];
        CURR_DATE[2] = datetimeEntries[2];
        DEFAULT_TIME[0] = datetimeEntries[3];
        DEFAULT_TIME[1] = datetimeEntries[4];
        DEFAULT_TIME[2] = datetimeEntries[5];
    }

    file.close(); // Close the file
}

// helping function for setcurrdatetime()
void storeDatetimeToFile()
{
    File file = SPIFFS.open(DATE_TIME_FILE, "w"); // Open "datetime.txt" in write mode
    if (!file)
    {
        Serial.println("Failed to open datetime file for writing");
        glowLED(500);
    }
    else
    {
        // Write the date and time values to the file, each in a new line
        file.println(CURR_DATE[0]);    // year
        file.println(CURR_DATE[1]);    // month
        file.println(CURR_DATE[2]);    // day
        file.println(DEFAULT_TIME[0]); // hour
        file.println(DEFAULT_TIME[1]); // minute
        file.println(DEFAULT_TIME[2]); // second
    }
    file.close(); // Close the file
}

// FETCH CURRENT DATE AND TIME FROM SERVER IN 5 RETRY (or use default one)
void setcurrdatetime()
{
    Serial.println("Inside setcurrdatetime");

    // fetch previous fetched date and time from the file
    fetchPreviousDateTime();
    // CALL TO BACKEND TO FETCH DATE AND TIME
    WiFiClientSecure client;
    HTTPClient http;
    client.setInsecure();

    // WILL TRY 5 TIMES TO FETCH DATE AND TIME
    JsonDocument doc;
    for (int i = 1; i < 3; i++)
    {
        yield();
        Serial.println("Inside setcurrdatetime for loop");

        String serverPath = LOCALHOST + "/student/currentDateTime";
        // Your Domain name with URL path or IP address with path
        if (WiFi.status() == WL_CONNECTED)
        {
            http.setTimeout(10000);
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
        delay(1000);
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
            storeDatetimeToFile();
            break;
        }
        else
        {
            Serial.print("Server Error code: ");
            Serial.println(httpResponseCode);
            glowLED(500);
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
        delay(500); // 0.5 second delay
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
            String jsonData = "{\"rfid\":\"" + id + "\", \"year\":\"" + year + "\", \"month\":\"" + month + "\", \"date\":\"" + date + "\", \"hour\":\"" + hour + "\", \"minute\":\"" + minute + "\"}";
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
                // sending data to server
                WiFiClientSecure client;
                client.setInsecure();

                // Your Domain name with URL path or IP address with path
                if (!client.connect(host, 443))
                { // works!
                    Serial.println("connection failed");
                    return 500;
                }

                String url = "/student/store";

                // Prepare JSON data
                // String jsonData = "{\"rfid\":\"" + rfid + "\"}";
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
                while (client.connected())
                {
                    Serial.print("inside while\n");
                    String line = client.readStringUntil('\n');
                    Serial.print(line);
                    Serial.println();
                    if (line.startsWith("HTTP/1.1"))
                    {
                        httpResponseCode = line.substring(9, 12).toInt();
                        Serial.print("Response code: ");
                        Serial.println(httpResponseCode);
                        if (httpResponseCode == 200)
                        {
                            digitalWrite(D8, LOW);
                            delay(500);
                            digitalWrite(D8, HIGH);
                        }
                        else
                        {
                            digitalWrite(D0, HIGH);
                            delay(500);
                            digitalWrite(D0, LOW);
                        }
                        break; // Stop reading after getting the response code
                    }
                }
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

// blick a led multiple times
void blinkLEDTimes(String LED, int times, int dlay = 300, String returning_state = "LOW")
{
    int ledPin;

    // Determine the GPIO pin based on the LED string
    if (LED == "D0")
    {
        ledPin = D0;
    }
    else if (LED == "D1")
    {
        ledPin = D1;
    }
    else if (LED == "D2")
    {
        ledPin = D2;
    }
    else if (LED == "D8")
    {
        ledPin = D8;
    }
    else
    {
        Serial.println("Invalid LED");
        return;
    }

    // Set pin mode to output
    pinMode(ledPin, OUTPUT);

    // Save the initial state of the LED
    // returning_state = returning_state.toUpperCase(); //not working
    bool returningState = (returning_state == "HIGH");

    // Blink the LED
    for (int i = 0; i < times; i++)
    {
        digitalWrite(ledPin, HIGH); // Toggle LED state
        delay(dlay);
        digitalWrite(ledPin, LOW); // Toggle LED state back
        delay(dlay);
    }

    digitalWrite(ledPin, returningState);
    return;
}

// blink all leds multiple times
void blinkAllLEDs(int times, int dlay = 300)
{
    // Blink all LEDs simultaneously
    for (int i = 0; i < times; i++)
    {
        digitalWrite(D0, HIGH);
        digitalWrite(D1, HIGH);
        digitalWrite(D2, HIGH);
        digitalWrite(D8, HIGH);
        delay(dlay);
        digitalWrite(D0, LOW);
        digitalWrite(D1, LOW);
        digitalWrite(D2, LOW);
        digitalWrite(D8, LOW);
        delay(dlay);
    }
}

void sendToBackendAdmin()
{
    WiFiClientSecure client;
    client.setInsecure();
    connectToWifi();

    // Your Domain name with URL path or IP address with path
    if (!client.connect(host, 443))
    {
        Serial.println("connection failed");
        return;
    }

    String URL = "/admin";

    // Prepare JSON data
    String jsonData = "{\"rfid\":\"" + rfid + "\"}";
    Serial.print("Sending JSON data: ");
    Serial.println(jsonData);

    // This will send the request to the server
    client.print(String("POST ") + URL + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + String(jsonData.length()) + "\r\n" +
                 "Connection: close\r\n\r\n" +
                 jsonData);

    // Serial.println("closing connection");

    // Wait for response
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line.startsWith("HTTP/1.1"))
        {
            int statusCode = line.substring(9, 12).toInt();
            Serial.print("Response code: ");
            Serial.println(statusCode);
            if (statusCode == 200)
            {
                // Serial.print("GREEN LIGHT\n");
                digitalWrite(D2, HIGH);
                delay(500); // 0.5 second delay
                digitalWrite(D2, LOW);
            }
            else
            {
                digitalWrite(D0, LOW);
                delay(500); // 0.5 second delay
                digitalWrite(D0, HIGH);
            }
            break; // Stop reading after getting the response code
        }
    }
    client.stop(); // disconnection
}
