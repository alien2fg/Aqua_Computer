#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"        // Include the secrets file with WiFi and Firebase credentials
#include "ph/PH_Sensor.h"   // pH sensor header
#include <OneWire.h>
#include <DallasTemperature.h>
#include "tds/tds.h"        // TDS sensor header
#include <time.h>           // For time handling

// Firebase and network configuration
WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(DATABASE_SECRET);

// Initialize sensor settings
#define PH_PIN 32
float calibration_value = 21.34 - 0.7;
PH_Sensor phSensor(PH_PIN, calibration_value);
const int oneWireBus = 34;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

unsigned long sendDataPrevMillis = 0;

void printError(int code, const String &msg) {
    Serial.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

void setup() {
    Serial.begin(115200);
    sensors.begin();
    phSensor.begin();

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    // Firebase Initialization
    ssl.setInsecure();
    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    initializeApp(client, app, getAuth(dbSecret));
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
    client.setAsyncResult(result);

    // Set the initial values for sunrise, sunset, and feeding times
    Serial.print("Setting initial values for sunrise, sunset, and feeding times... ");
    Database.set<String>(client, "/aquariumData/sunrise", "06:15 AM");
    Database.set<String>(client, "/aquariumData/sunset", "08:10 PM");
    Database.set<String>(client, "/aquariumData/feedingTimes", "07:00 AM");
    
    Serial.println("Initial values set.");
    
    // Initialize NTP for timekeeping (UTC+1 for Poland, with automatic DST handling)
    configTime(3600, 0, "pool.ntp.org", "time.nist.gov"); // 3600 seconds offset for UTC+1
    Serial.println("Time configured to Poland timezone (UTC+1).");
}

void loop() {
    // Update sensor readings
    float pHValue = phSensor.readPH();
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    readTDS();  // Ensure this function is defined in "tds.h"

    // Check Wi-Fi connection before Firebase operations
    if (WiFi.status() == WL_CONNECTED && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();

        // Upload sensor data to Firebase
        Serial.print("Setting aquarium data... ");
        bool status = Database.set<float>(client, "/aquariumData/ph", pHValue) &&
                      Database.set<float>(client, "/aquariumData/temperature", temperatureC) &&
                      Database.set<float>(client, "/aquariumData/tds", tdsValue);
                      
        if (status) {
            Serial.println("Data uploaded successfully.");
        } else {
            printError(client.lastError().code(), client.lastError().message());
        }
    }

    // Retrieve and display data from Firebase
    Serial.print("Retrieving aquarium data... ");
    String sunriseTime = Database.get<String>(client, "/aquariumData/sunrise");
    String sunsetTime = Database.get<String>(client, "/aquariumData/sunset");
    String feedingTimes = Database.get<String>(client, "/aquariumData/feedingTimes");

    if (client.lastError().code() == 0) {
        Serial.println("Data retrieved successfully.");
        Serial.print("Sunrise: ");
        Serial.println(sunriseTime);
        Serial.print("Sunset: ");
        Serial.println(sunsetTime);
        Serial.print("Feeding Time: ");
        Serial.println(feedingTimes);
        
        // Check current time against sunrise and sunset
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 1000)) {
            Serial.println("Failed to obtain time");
            return;
        }

        // Convert current time to HH:MM format
        String currentTime = String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min);
        Serial.print("Current Time: ");
        Serial.println(currentTime);

        // Compare times and trigger actions
        if (currentTime == sunriseTime) {
            Serial.println("Triggering sunrise action!");
            // Add your sunrise action here
        }
        if (currentTime == sunsetTime) {
            Serial.println("Triggering sunset action!");
            // Add your sunset action here
        }
        if (currentTime == feedingTimes) {
            Serial.println("Triggering feeding action!");
            // Add your feeding action here
        }

    } else {
        printError(client.lastError().code(), client.lastError().message());
    }

    delay(5000);  // Delay to avoid frequent Firebase requests
}
