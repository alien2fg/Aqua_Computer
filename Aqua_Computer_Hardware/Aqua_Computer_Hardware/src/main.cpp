#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Import secret credentials
#include "secrets.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase sign up OK");
    signupOK = true;
  }
  else{
    Serial.printf("Sign up failed: %s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    // Write temperature (26.0 degrees) to the database path aquariumData
    float temperature = 26.0;
    
    if (Firebase.RTDB.setFloat(&fbdo, "aquariumData/temperature", temperature)){
      Serial.println("Temperature data uploaded successfully.");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED to upload temperature.");
      Serial.println("REASON: " + fbdo.errorReason());

      // Additional diagnostic info
      Serial.println("Error Code: " + String(fbdo.httpCode()));
      Serial.println("Firebase Error: " + fbdo.errorReason());
    }
  }
}
