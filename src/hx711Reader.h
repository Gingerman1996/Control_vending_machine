#pragma once

#ifndef HX711CONTROLLER_H
#define HX711CONTROLLER_H

#include "HX711.h" // Include the HX711 library

class hx711Reader
{
private:
#define DEC_POINT 2
#define STABLE 1
    static hx711Reader *instance; // Static instance variable
    HX711 hx711_1;                // HX711 sensor 1 instance
    HX711 hx711_2;                // HX711 sensor 2 instance
    int HX711_DOUT_PIN[2];        // Data pin for HX711
    int HX711_SCK_PIN[2];         // Clock pin for HX711

    // Private constructor to prevent instantiation
    hx711Reader();
    void begin(int doutPin, int sckPin);
    float get_units_kg(int cell);

public:
    static hx711Reader *getInstance();      // Get instance method
    void setPins1(int doutPin, int sckPin); // Set pins module 1 method
    void setPins2(int doutPin, int sckPin); // Set pins module 2 method
    float readData(int module, long offset);             // Read data method
    long FindZeroFactor(int cell);
    float FindCalibrationFactor(int real_weight, int cell);
};

#endif
