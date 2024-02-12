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
float hx711Reader::readData(int module, long offset)
{
    
    Serial.print("Reading: ");
    String load_cell = String(this->get_units_kg(module) + offset, DEC_POINT);
    Serial.print(load_cell);
    Serial.println(" kg");
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
    }
    else if (cell == 2)
    {
        hx711_2.set_scale();
        hx711_2.tare();
        zero_factor = hx711_2.read_average(20);
    }
    Serial.print("Zero factor: ");
    Serial.println(zero_factor);
    return (zero_factor);
}

float hx711Reader::FindCalibrationFactor(int real_weight, int cell)
{
    float calibrationFactor = 1;
    unsigned char flag_stable = 0;
    unsigned int decpoint = 1;
    for (unsigned char i = 0; i < DEC_POINT + 1; i++)
        decpoint = decpoint * 10;

    while (1)
    {
        if (cell == 1)
        {
            hx711_1.set_scale(calibrationFactor); // Adjust to this calibration factor
        }
        else if (cell == 2)
        {
            hx711_2.set_scale(calibrationFactor); // Adjust to this calibration factor
        }

        Serial.print("Reading: ");
        float read_weight = this->get_units_kg(cell);
        String data = String(read_weight, DEC_POINT);
        Serial.print(data);
        Serial.print(" kg");
        Serial.print(" calibrationFactor: ");
        Serial.print(calibrationFactor);
        Serial.println();
        long r_weight = (real_weight * decpoint);
        long int_read_weight = read_weight * decpoint;
        Serial.print(r_weight);
        Serial.print(" , ");
        Serial.println(int_read_weight);
        long x;
        if (r_weight == int_read_weight)
        {
            flag_stable++;
            if (flag_stable >= STABLE)
            {
                Serial.print("Calibration Factor is = ");
                Serial.println(calibrationFactor);
                break;
            }
        }
        if (r_weight > int_read_weight)
        {
            x = r_weight - int_read_weight;
            if (x > 100)
                calibrationFactor -= 1000;
            else if (x > 100)
                calibrationFactor -= 10;
            else
                calibrationFactor -= 1;
            flag_stable = 0;
        }
        if (r_weight < int_read_weight)
        {
            x = int_read_weight - r_weight;
            if (x > 100)
                calibrationFactor += 1000;
            else if (x > 10)
                calibrationFactor += 10;
            else
                calibrationFactor += 1;
            flag_stable = 0;
        }
    }
    return calibrationFactor;
}

float hx711Reader::get_units_kg(int cell)
{
    if (cell == 1)
        return (hx711_1.get_units() * 0.453592);
    else if (cell == 2)
        return (hx711_2.get_units() * 0.453592);
}