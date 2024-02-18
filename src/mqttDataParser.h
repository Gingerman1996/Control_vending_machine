#pragma once

#ifndef MQTT_DATA_PARSER_H
#define MQTT_DATA_PARSER_H

#include <ArduinoJson.h>
#include <string.h>

// Define callback function type
typedef void (*MQTTDataCallback)(StaticJsonDocument<200>);

struct dataPump
{
    String msg;
    int cell;
    int value;
    int door;
    int pump_pin;
    int valve_pin;
};

struct dataSwap
{
    String msg;
    int box;
    float value;
};

struct dataCal
{
    String msg;
    String cal_msg;
    float value;
};

class MQTTDataParser
{
private:
    MQTTDataParser() {}
    static MQTTDataParser *instance;
    MQTTDataCallback dataCallback; // Callback function

public:
    static MQTTDataParser *getInstance();
    void parseData(byte *payload, unsigned int length);
    void setCallback(MQTTDataCallback callback); // Set callback function
};

#endif
