#include "PH_Sensor.h"

// Constructor: initialize the pH pin and calibration value
PH_Sensor::PH_Sensor(int pin, float calibration)
{
    pH_PIN = pin;
    calibration_value = calibration;
}

// Optional setup method, not used in this case but kept for structure
void PH_Sensor::begin()
{
    // Initialize any necessary settings here (if needed)
}

// Read pH value and return it
float PH_Sensor::readPH()
{
    unsigned long int avgval;
    int buffer_arr[10], temp; // Buffer for averaging the readings

    // Read pH values and average them
    for (int i = 0; i < 10; i++)
    {
        buffer_arr[i] = analogRead(pH_PIN); // Read from the pH sensor
        delay(30);
    }

    // Sort the readings for averaging
    for (int i = 0; i < 9; i++)
    {
        for (int j = i + 1; j < 10; j++)
        {
            if (buffer_arr[i] > buffer_arr[j])
            {
                temp = buffer_arr[i];
                buffer_arr[i] = buffer_arr[j];
                buffer_arr[j] = temp;
            }
        }
    }

    // Calculate the average value (discarding the lowest and highest readings)
    avgval = 0;
    for (int i = 2; i < 8; i++) // Average middle 6 readings
        avgval += buffer_arr[i];

    // Convert the average analog value to voltage
    float volt = (float)avgval * 3.3 / 4095.0;       // Use 3.3V and 12-bit resolution for ESP32
    float ph_act = -5.70 * volt + calibration_value; // Calculate the actual pH value

    return ph_act; // Return the calculated pH value
}
