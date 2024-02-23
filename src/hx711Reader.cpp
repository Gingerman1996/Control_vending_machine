#include "hx711Reader.h"

// Define the static instance variable
hx711Reader *hx711Reader::instance = nullptr;

// Private constructor to prevent instantiation
hx711Reader::hx711Reader()
{
    // Initialize HX711 sensor here
    hx711_1.begin(HX711_DOUT_PIN[0], HX711_SCK_PIN[0]);
    hx711_2.begin(HX711_DOUT_PIN[1], HX711_SCK_PIN[1]);
}

// Get the instance of the hx711Reader
hx711Reader *hx711Reader::getInstance()
{
    if (!instance)
    {
        instance = new hx711Reader();
    }
    return instance;
}

// Set the HX711 sensor pins
void hx711Reader::setPins1(int doutPin, int sckPin)
{
    HX711_DOUT_PIN[0] = doutPin;
    HX711_SCK_PIN[0] = sckPin;
    // Reinitialize HX711 sensor with new pins
    hx711_1.begin(HX711_DOUT_PIN[0], HX711_SCK_PIN[0]);
}

void hx711Reader::setPins2(int doutPin, int sckPin)
{
    HX711_DOUT_PIN[1] = doutPin;
    HX711_SCK_PIN[1] = sckPin;
    // Reinitialize HX711 sensor with new pins
    hx711_2.begin(HX711_DOUT_PIN[1], HX711_SCK_PIN[1]);
}

// Read data from the HX711 sensor
float hx711Reader::readData(int cell, long offset, float calibrationFactor)
{
    if (cell == 1)
    {
        hx711_1.set_scale(calibrationFactor);
    }
    else if (cell == 2)
    {
        hx711_2.set_scale(calibrationFactor);
    }
    Serial.print("Reading: ");
    String load_cell = String(this->get_units_g(cell) + offset, DEC_POINT);
    Serial.print(load_cell);
    Serial.println(" g");

    return load_cell.toFloat();
}

long hx711Reader::FindZeroFactor(int cell)
{
    Serial.println("Find Zero Factor");
    Serial.println("Please wait .....");

    long zero_factor;
    if (cell == 1)
    {
        hx711_1.set_scale();
        hx711_1.tare();
        zero_factor = hx711_1.read_average(20);
        Serial.print("Zero factor: ");
        Serial.println(zero_factor);
        return (zero_factor);
    }
    else if (cell == 2)
    {
        hx711_2.set_scale();
        hx711_2.tare();
        zero_factor = hx711_2.read_average(20);
        long reading = hx711_2.get_units(10);
        Serial.print("Zero factor: ");
        Serial.println(zero_factor);
        return (zero_factor);
    }
    Serial.print("Zero factor: ");
    Serial.println(zero_factor);
    return (zero_factor);
}

float hx711Reader::FindCalibrationFactor(float real_weight, int cell)
{
    float calibrationFactor;
    unsigned char flag_stable = 0;
    unsigned int decpoint = 1;

    float read_weight;
    long r_weight;
    long int_read_weight;
    long x;
    String data;

    for (unsigned char i = 0; i < DEC_POINT; i++)
        decpoint = decpoint * 10;

    float raw_weight = this->get_units_g(cell);
    calibrationFactor = raw_weight / real_weight;

    Serial.printf("Tare: %.3f\tReal Weight: %.3f\nCalibration Factor: %.3f\n", raw_weight, real_weight, calibrationFactor);

    return calibrationFactor;
}

float hx711Reader::get_units_g(int cell)
{
    if (cell == 1)
    {
        if (hx711_1.is_ready())
        {
            return (hx711_1.get_units(20));
        }
    }
    else if (cell == 2)
    {
        if (hx711_1.is_ready())
        {
            return (hx711_2.get_units(20));
        }
    }
}

void hx711Reader::setTare(int cell)
{
    if (cell == 1)
        hx711_1.tare();
    else if (cell == 2)
        hx711_2.tare();
}