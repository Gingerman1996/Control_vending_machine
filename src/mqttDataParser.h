// mqttDataParser.h
#ifndef MQTT_DATA_PARSER_H
#define MQTT_DATA_PARSER_H

#include <ArduinoJson.h>
#include <string.h>

// Define callback function type
typedef void (*MQTTDataCallback)(String, int);

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
