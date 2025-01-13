#include "PH_Sensor.h"

// Constructor: initialize the pH pin
PH_Sensor::PH_Sensor(int pin)
{
    pH_PIN = pin;
}

// Optional setup method, not used in this case but kept for structure
void PH_Sensor::begin()
{
    // Initialize any necessary settings here (if needed)
}

// Simple function to calculate pH from the voltage reading
float PH_Sensor::readPH()
{
    int measurings = 0;

    // Sample readings to average out any noise
    for (int i = 0; i < 10; i++)
    {
        measurings += analogRead(pH_PIN);
        delay(10); // Small delay to stabilize readings
    }

    // Calculate the average voltage
    float voltage = 3.3 / 4095.0 * measurings / 10; // Assuming 12-bit ADC resolution (0-4095)

    // Calculate pH using the provided formula
    return 7 + ((2.5 - voltage) / 0.18); // Adjust according to the calibration (example values)
}