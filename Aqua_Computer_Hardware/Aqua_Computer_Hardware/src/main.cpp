#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "ph/PH_Sensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "secrets.h" // Import secret credentials
#include "addons/TokenHelper.h" // Token generation process info
#include "addons/RTDBHelper.h" // RTDB payload printing info and other helpers
#include "tds/tds.h"

// Firebase configuration
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// pH sensor configuration
#define PH_PIN 32
float calibration_value = 21.34 - 0.7; // Adjust based on your calibration
PH_Sensor phSensor(PH_PIN, calibration_value);

// DS18B20 temperature sensor configuration
const int oneWireBus = 34;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// Timing variables
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
    Serial.begin(115200);
    sensors.begin();
    phSensor.begin();

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    unsigned long startAttemptTime = millis();
    
    // Attempt to connect to Wi-Fi
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
        Serial.print(".");
        delay(300);
    }

    // Check if connection was successful
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to Wi-Fi");
        return; // Exit setup if Wi-Fi connection fails
    }
    
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Firebase configuration
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;

    // Firebase sign-up
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("Firebase sign up OK");
        signupOK = true;
    } else {
        Serial.printf("Sign up failed: %s\n", config.signer.signupError.message.c_str());
    }

    // Initialize Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    // Read TDS value
    readTDS(); // Ensure that this function is defined in "tds.h"
    
    // Read and print pH value
    float pHValue = phSensor.readPH();
    Serial.print("pH Value: ");
    Serial.println(pHValue);
    delay(1000); // Wait for 1 second

    // Debugging: Check the TDS value
    if (tdsValue < 0) { // Assume readTDS() sets tdsValue; adjust as needed
        Serial.println("Failed to read TDS value.");
    } else {
        Serial.print("Current TDS Value: ");
        Serial.println(tdsValue);
    }

    // Read and print temperature
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    Serial.print(temperatureF);
    Serial.println("ºF");
    delay(5000);

    /*
    // Upload data to Firebase if conditions are met
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        float temperature = 26.0;

        // Write temperature to database
        if (Firebase.RTDB.setFloat(&fbdo, "aquariumData/temperature", temperature)) {
            Serial.println("Temperature data uploaded successfully.");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        } else {
            Serial.println("FAILED to upload temperature.");
            Serial.println("REASON: " + fbdo.errorReason());
            Serial.println("Error Code: " + String(fbdo.httpCode()));
            Serial.println("Firebase Error: " + fbdo.errorReason());
        }
    }
    */
}
