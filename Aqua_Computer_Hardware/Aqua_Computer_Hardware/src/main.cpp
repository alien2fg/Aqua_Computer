#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "ph/PH_Sensor.h"

// Import secret credentials
#include "secrets.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#include "tds/tds.h"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Define your pH sensor pin and calibration value
#define PH_PIN 32
float calibration_value = 21.34 - 0.7; // Adjust based on your calibration

PH_Sensor phSensor(PH_PIN, calibration_value); // Create a PH_Sensor object

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
    Serial.begin(115200);

    phSensor.begin();
    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    
    unsigned long startAttemptTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
        Serial.print(".");
        delay(300);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to Wi-Fi");
        return; // Exit setup if Wi-Fi connection fails
    }
    
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Assign the API key (required)
    config.api_key = API_KEY;

    // Assign the RTDB URL (required)
    config.database_url = DATABASE_URL;

    // Sign up
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("Firebase sign up OK");
        signupOK = true;
    } else {
        Serial.printf("Sign up failed: %s\n", config.signer.signupError.message.c_str());
    }

    // Assign the callback function for the long-running token generation task
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    // Read TDS value
    readTDS(); // Ensure that this function is defined in "tds.h"
    float pHValue = phSensor.readPH(); // Read pH value
    Serial.print("pH Value: ");
    Serial.println(pHValue); // Print pH value to Serial Monitor
    delay(1000); // Wait for 1 second before the next reading
    
    // Debugging: Check the TDS value
    if (tdsValue < 0) { // Assume readTDS() sets tdsValue; adjust as needed
        Serial.println("Failed to read TDS value.");
    } else {
        Serial.print("Current TDS Value: ");
        Serial.println(tdsValue);
    }
/* 
    // Upload data if Firebase is ready and we have successfully signed up
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();

        // Write temperature (26.0 degrees) to the database path aquariumData
        float temperature = 26.0;

        if (Firebase.RTDB.setFloat(&fbdo, "aquariumData/temperature", temperature)) {
            Serial.println("Temperature data uploaded successfully.");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        } else {
            Serial.println("FAILED to upload temperature.");
            Serial.println("REASON: " + fbdo.errorReason());
            // Additional diagnostic info
            Serial.println("Error Code: " + String(fbdo.httpCode()));
            Serial.println("Firebase Error: " + fbdo.errorReason());
        }
    } */
}
