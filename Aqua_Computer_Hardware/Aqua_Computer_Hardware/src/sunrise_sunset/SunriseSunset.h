#ifndef SUNRISE_SUNSET_H
#define SUNRISE_SUNSET_H

#include <Adafruit_NeoPixel.h>

class SunriseSunset
{
public:
    SunriseSunset(uint16_t pin, uint16_t ledCount);
    void sunrise();
    void sunset();
    void clearLEDs();

private:
    Adafruit_NeoPixel strip;
};

#endif // SUNRISE_SUNSET_H