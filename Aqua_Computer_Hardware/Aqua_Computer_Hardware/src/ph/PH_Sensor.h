#ifndef PH_SENSOR_H
#define PH_SENSOR_H

#include <Arduino.h>

class PH_Sensor {
public:
    PH_Sensor(int pin, float calibration);
    void begin();
    float readPH();

private:
    int pH_PIN;
    float calibration_value;
};

#endif