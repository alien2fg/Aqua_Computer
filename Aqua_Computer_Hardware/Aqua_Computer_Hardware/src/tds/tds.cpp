#include <Arduino.h>
#include "tds.h"

#define TdsSensorPin 33
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample points

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void readTDS() {
  static unsigned long analogSampleTimepoint = millis();
  if(millis() - analogSampleTimepoint > 40U) {     //every 40 milliseconds, read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT) { 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for(copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    }
    // Get the median voltage
    int medianVoltage = getMedianNum(analogBufferTemp, SCOUNT);
    averageVoltage = medianVoltage * (float)VREF / 4096.0;   // convert the analog value to voltage
    tdsValue = (133.42 * averageVoltage * averageVoltage * averageVoltage 
               - 255.86 * averageVoltage * averageVoltage 
               + 857.39 * averageVoltage) * 0.5; // calculate TDS value
    Serial.print("TDS Value: ");
    Serial.println(tdsValue);
  }
}
