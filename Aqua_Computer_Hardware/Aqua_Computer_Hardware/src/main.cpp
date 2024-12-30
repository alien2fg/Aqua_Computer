#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"      // Include the secrets file with WiFi and Firebase credentials
#include "ph/PH_Sensor.h" // pH sensor header
#include <OneWire.h>
#include <DallasTemperature.h>
#include "tds/tds.h"                      // TDS sensor header
#include <time.h>                         // For time handling
#include <Adafruit_NeoPixel.h>            // Adafruit NeoPixel for LED control
#include "sunrise_sunset/SunriseSunset.h" // Sunrise and sunset lighting
#include <ArduinoJson.h>                  // Dodaj bibliotekę do obsługi JSON

#define LED_PIN 27
#define LED_COUNT 40
#define PH_PIN 32
#define ONE_WIRE_BUS_PIN 34
#define FEEDER_PIN 25    // GPIO25 do sterowania karmnikiem
#define WHITE_LED_PIN 26 // GPIO26 do sterowania białą taśmą LED

// Firebase and network configuration
WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(DATABASE_SECRET);

// Global variables for storing times
struct tm currentTime;         // Updated in Firebase task
String sunriseTime = "06:15";  // Default value
String sunsetTime = "20:10";   // Default value
String feedingTimes = "07:00"; // Default value (single feeding time)

// LED Initialization (using Adafruit NeoPixel library)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Initialize sensor settings
float calibration_value = 21.34 - 0.7;
PH_Sensor phSensor(PH_PIN, calibration_value);
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

unsigned long sendDataPrevMillis = 0;
unsigned long lastTempRequestMillis = 0;
unsigned long lastFirebaseFetchMillis = 0;

// Additional variables to handle sunrise and sunset
unsigned long lastFeedingMillis = 0;         // Czas ostatniego karmienia
const unsigned long feedingInterval = 60000; // Czas, po którym karmnik może się włączyć ponownie (1 minuta)
unsigned long feederStartTime = 0;           // Czas rozpoczęcia karmienia
const unsigned long feederDuration = 0;      // Czas działania karmnika w milisekundach (5 sekund)
const long feederOutputRate = 5;             // Wydajność karmnika w gramach na sekundę

unsigned long lightStartTime = 0; // Czas rozpoczęcia włączania światła
unsigned long lightDuration = 0;  // Czas trwania światła, ustawiony jako różnica między godziną wschodu a zachodu
bool feederInProgress = false;    // Flaga wskazująca, czy karmnik jest włączony
bool lightInProgress = false;     // Flaga wskazująca, czy światło jest włączone
bool sunriseInProgress = false;   // Flaga wskazująca, czy wschód słońca jest w trakcie
bool sunsetInProgress = false;    // Flaga wskazująca, czy zachód słońca jest w trakcie

unsigned long sunriseStartTime = 0;          // Czas rozpoczęcia wschodu słońca
unsigned long sunsetStartTime = 0;           // Czas rozpoczęcia zachodu słońca
const unsigned long actionDuration = 300000; // Czas trwania akcji wschodu/zachodu (5 minut)
std::vector<String> feedingTimesList;

void printError(int code, const String &msg)
{
    Serial.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

// Helper function to parse time in "hh:mm" format
bool parseTime(const String &timeStr, int &hour, int &minute)
{
    if (timeStr.indexOf(':') == -1)
        return false; // Check for ":"
    hour = timeStr.substring(0, timeStr.indexOf(':')).toInt();
    minute = timeStr.substring(timeStr.indexOf(':') + 1).toInt();
    return hour >= 0 && hour < 24 && minute >= 0 && minute < 60; // Validate time range
}

// Function to set up time from NTP
void setupTime()
{
    configTime(0, 0, "pl.pool.ntp.org"); // Set NTP server for Poland
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Błąd synchronizacji czasu NTP");
    }
    else
    {
        currentTime = timeinfo;
        Serial.println("Czas zsynchronizowany:");
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }
}

void handleSensors()
{
    unsigned long currentMillis = millis();

    if (currentMillis - lastTempRequestMillis >= 10000 || lastTempRequestMillis == 0)
    {
        lastTempRequestMillis = currentMillis;

        sensors.requestTemperatures();
        float temperatureC = sensors.getTempCByIndex(0);
        float pHValue = phSensor.readPH();
        readTDS();

        Serial.printf("pH: %.2f, Temp: %.2f°C, TDS: %.2f ppm\n", pHValue, temperatureC, tdsValue);

        if (WiFi.status() == WL_CONNECTED)
        {
            bool uploadStatus = Database.set<float>(client, "/aquariumData/ph", pHValue) &&
                                Database.set<float>(client, "/aquariumData/temperature", temperatureC) &&
                                Database.set<float>(client, "/aquariumData/tds", tdsValue);

            if (uploadStatus)
            {
                Serial.println("Sensor data uploaded successfully.");
            }
            else
            {
                printError(client.lastError().code(), client.lastError().message());
            }
        }
    }
}

void parseFeedingTimes(const String &feedingTimesJson, std::vector<String> &feedingTimesList)
{
    DynamicJsonDocument doc(512); // Przygotuj dokument JSON
    DeserializationError error = deserializeJson(doc, feedingTimesJson);

    if (error)
    {
        Serial.println("Error parsing feeding times JSON!");
        return;
    }

    if (doc.is<JsonArray>())
    {
        JsonArray array = doc.as<JsonArray>();
        for (JsonVariant v : array)
        {
            feedingTimesList.push_back(v.as<String>());
        }
    }
    else
    {
        Serial.println("Feeding times JSON is not an array!");
    }
}

void fetchFirebaseData()
{
    unsigned long currentMillis = millis();

    if (currentMillis - lastFirebaseFetchMillis >= 30000 || lastFirebaseFetchMillis == 0)
    {
        lastFirebaseFetchMillis = currentMillis;

        String newSunrise = Database.get<String>(client, "/aquariumData/sunrise");
        String newSunset = Database.get<String>(client, "/aquariumData/sunset");
        String newFeeding = Database.get<String>(client, "/aquariumData/feedingTimes");
        int newFeedNow = Database.get<int>(client, "/aquariumData/feedNow");
        long newFeedAmountGrams = Database.get<long>(client, "/aquariumData/feedAmount");

        if (client.lastError().code() == 0)
        {
            sunriseTime = newSunrise;
            sunsetTime = newSunset;

            feedingTimesList.clear();                        // Wyczyść poprzednią listę
            parseFeedingTimes(newFeeding, feedingTimesList); // Wypełnij nową listą

            // Przelicz czas działania karmnika na podstawie ilości karmy i wydajności
            if (newFeedAmountGrams > 0)
            {
                feederDuration = (newFeedAmountGrams * 1000) / feederOutputRate; // W milisekundach
            }
            else
            {
                feederDuration = 0; // Brak karmienia dla 0 gramów
            }

            Serial.println("Data retrieved successfully.");
            Serial.printf("Feed amount: %ld grams, Feeder duration: %ld ms\n", newFeedAmountGrams, feederDuration);

            Serial.println("Data retrieved successfully.");
            Serial.printf("Feed amount: %.2f grams, Calculated duration: %lu ms\n", feedAmountGrams, calculatedFeederDuration);

            Serial.println("Data retrieved successfully.");
            Serial.print("Sunrise: ");
            Serial.println(sunriseTime);
            Serial.print("Sunset: ");
            Serial.println(sunsetTime);
            Serial.print("Feeding Time: ");
            for (const String &time : feedingTimesList)
            {
                Serial.println(time);
            }

            // Jeśli feedNow = 1, to rozpocznij karmienie
            if (newFeedNow == 1 && !feederInProgress)
            {
                Serial.println("Triggering feeding action based on feedNow!");
                digitalWrite(FEEDER_PIN, LOW); // Włącz karmnik
                feederStartTime = currentMillis;
                feederInProgress = true;
                lastFeedingMillis = currentMillis;

                // Zaktualizuj Firebase, aby ustawiono feedNow na 0
                Database.set<int>(client, "/aquariumData/feedNow", 0); // Wyłącz karmienie po jego rozpoczęciu
            }

            setupTime();
        }
        else
        {
            printError(client.lastError().code(), client.lastError().message());
        }
    }
}

// Pomocnicza funkcja do obliczania różnicy w minutach między godziną wschodu i zachodu
unsigned long calculateLightDuration(const String &sunrise, const String &sunset)
{
    int sunriseHour, sunriseMinute, sunsetHour, sunsetMinute;

    if (parseTime(sunrise, sunriseHour, sunriseMinute) && parseTime(sunset, sunsetHour, sunsetMinute))
    {
        // Oblicz różnicę godzin w minutach
        unsigned long sunriseTotalMinutes = sunriseHour * 60 + sunriseMinute;
        unsigned long sunsetTotalMinutes = sunsetHour * 60 + sunsetMinute;

        // Jeśli czas zachodu słońca jest wcześniejszy niż wschodu słońca, oznacza to, że zachód jest następnego dnia
        if (sunsetTotalMinutes < sunriseTotalMinutes)
        {
            sunsetTotalMinutes += 24 * 60; // Dodaj 24 godziny w minutach do zachodu słońca
        }

        // Odejmij 5 minut od obliczonego czasu trwania
        unsigned long lightDuration = sunsetTotalMinutes - sunriseTotalMinutes;
        if (lightDuration >= 5)
        { // Upewnij się, że różnica nie jest ujemna po odjęciu
            lightDuration -= 5;
        }

        return lightDuration; // Zwróć skorygowaną różnicę w minutach
    }
    return 0; // Zwróć 0, jeśli coś poszło nie tak
}

void handleScheduledActions()
{
    SunriseSunset lighting(LED_PIN, LED_COUNT);

    int currentHour = currentTime.tm_hour;
    int currentMin = currentTime.tm_min;
    unsigned long currentMillis = millis();

    int sunriseHour, sunriseMinute;
    int sunsetHour, sunsetMinute;
    int feedingHour, feedingMinute;

    // Parse times from Firebase
    if (parseTime(sunriseTime, sunriseHour, sunriseMinute) &&
        parseTime(sunsetTime, sunsetHour, sunsetMinute))
    {

        // Calculate the light duration as the difference between sunset and sunrise
        lightDuration = calculateLightDuration(sunriseTime, sunsetTime) * 60000; // convert minutes to milliseconds

        // Trigger sunrise if time matches and action hasn't started yet
        if (currentHour == sunriseHour && currentMin == sunriseMinute && !sunriseInProgress)
        {
            Serial.println("Triggering sunrise action!");
            lighting.sunrise();
            sunriseStartTime = currentMillis;
            sunriseInProgress = true;
        }

        // Trigger sunset if time matches and action hasn't started yet
        if (currentHour == sunsetHour && currentMin == sunsetMinute && !sunsetInProgress)
        {
            Serial.println("Triggering sunset action!");
            lighting.sunset();
            sunsetStartTime = currentMillis;
            sunsetInProgress = true;
        }

        // Trigger feeding action if time matches and enough time has passed since the last feeding
        // Sprawdź, czy zachodzi czas karmienia
        for (const String &feedingTime : feedingTimesList)
        {
            int feedingHour, feedingMinute;
            if (parseTime(feedingTime, feedingHour, feedingMinute))
            {
                if (currentHour == feedingHour && currentMin == feedingMinute)
                {
                    if (currentMillis - lastFeedingMillis >= feedingInterval)
                    {
                        Serial.println("Triggering feeding action!");
                        digitalWrite(FEEDER_PIN, LOW);
                        feederStartTime = currentMillis;
                        feederInProgress = true;
                        lastFeedingMillis = currentMillis;
                    }
                }
            }
        }

        // Włącz światło po zakończeniu wschodu słońca i przed zachodem słońca
        if (sunriseInProgress && !lightInProgress && currentMillis - sunriseStartTime >= 300000)
        { // 5 min po wschodzie słońca
            Serial.println("Włączanie światła...");
            digitalWrite(WHITE_LED_PIN, LOW); // Włącz światło
            lightStartTime = currentMillis;   // Zarejestruj czas rozpoczęcia
            lightInProgress = true;           // Ustaw flagę na true
        }

        // Wyłącz światło po upływie obliczonego czasu świecenia
        if (lightInProgress && currentMillis - lightStartTime >= lightDuration)
        {
            Serial.println("Wyłączanie światła...");
            digitalWrite(WHITE_LED_PIN, HIGH); // Wyłącz światło
            lightInProgress = false;           // Zresetuj flagę
        }

        // End feeding action after 5 seconds
        if (feederInProgress && (currentMillis - feederStartTime >= feederDuration))
        {
            Serial.println("Wyłączanie karmnika.");
            digitalWrite(FEEDER_PIN, HIGH); // Wyłącz karmnik
            feederInProgress = false;       // Zresetuj flagę
        }

        // End sunrise action if it has lasted more than 5 minutes
        if (sunriseInProgress && (currentMillis - sunriseStartTime >= actionDuration))
        {
            sunriseInProgress = false;
            Serial.println("Sunrise action completed!");
        }

        // End sunset action if it has lasted more than 5 minutes
        if (sunsetInProgress && (currentMillis - sunsetStartTime >= actionDuration))
        {
            sunsetInProgress = false;
            Serial.println("Sunset action completed!");
        }

        if (feederDuration == 0)
        {
            Serial.println("Feeder duration is 0. Skipping feeding action.");
            feederInProgress = false;
            return;
        }

        // If it's time for the next day, reset the flags to allow the next sunrise or sunset
        if (currentHour == 0 && currentMin == 0)
        {
            sunriseInProgress = false;
            sunsetInProgress = false;
        }
    }
    else
    {
        Serial.println("Error parsing time from Firebase!");
    }
}

void setup()
{
    Serial.begin(115200);
    strip.begin();
    strip.show(); // Initialize LEDs to OFF

    // Ustawienie pinów jako wyjścia
    pinMode(FEEDER_PIN, OUTPUT);
    pinMode(WHITE_LED_PIN, OUTPUT);

    // Wyłączenie przekaźników na starcie (założenie: HiGHA = wyłączony)
    digitalWrite(FEEDER_PIN, HIGH);
    digitalWrite(WHITE_LED_PIN, HIGH);

    // Wi-Fi setup
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Wi-Fi connected!");

    // Initialize Firebase
    ssl.setInsecure();
    initializeApp(client, app, getAuth(dbSecret));
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
    client.setAsyncResult(result);

    setupTime();
}

void loop()
{
    handleSensors();
    fetchFirebaseData();
    handleScheduledActions(); // Handle scheduled actions (sunrise, sunset, feeding)
    delay(100);               // Small delay to prevent excessive CPU usage
}