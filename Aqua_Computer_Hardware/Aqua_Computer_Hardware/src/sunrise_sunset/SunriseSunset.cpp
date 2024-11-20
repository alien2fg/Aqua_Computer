#include "SunriseSunset.h"

// Constructor
SunriseSunset::SunriseSunset(uint16_t pin, uint16_t ledCount)
    : strip(ledCount, pin, NEO_GRB + NEO_KHZ800) {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
}

// Optimized sunrise function
void SunriseSunset::sunrise() {
    for (int brightness = 0; brightness <= 255; brightness += 5) {
        uint8_t red = brightness;
        uint8_t green = brightness / 2;
        uint8_t blue = (brightness < 128) ? brightness / 4 : 0;

        for (uint16_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(red, green, blue));
        }
        strip.show();
        delay(5882); // Smooth transition, adjusted for 5 minutes
    }
}

// Optimized sunset function
void SunriseSunset::sunset() {
    for (int brightness = 255; brightness >= 0; brightness -= 5) {
        uint8_t red = brightness;
        uint8_t green = brightness / 2;
        uint8_t blue = (brightness < 128) ? brightness / 4 : 0;

        for (uint16_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(red, green, blue));
        }
        strip.show();
        delay(5882); // Smooth transition, adjusted for 5 minutes
    }
}

// Optimized clear LEDs function
void SunriseSunset::clearLEDs() {
    strip.clear(); // Clear all LEDs in one command
    strip.show();
}
