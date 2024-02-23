#include "mqttDataParser.h"

MQTTDataParser *MQTTDataParser::instance = nullptr;

MQTTDataParser *MQTTDataParser::getInstance()
{
    if (!instance)
    {
        instance = new MQTTDataParser();
    }
    return instance;
}

void MQTTDataParser::parseData(byte *payload, unsigned int length)
{
    // Convert the payload to a JsonObject
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    // Check for parsing errors
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    // Access data in the JsonObject
    String msg = doc["msg"].as<String>();

    // Check msg value and parse data accordingly
    // Check msg value and call the appropriate callback function
    if (msg == "pump1" || msg == "pump2" || msg == "Swap" || msg == "Cal")
    {
        if (dataCallback)
        {
            const char *msg_chr = doc["msg"];
            const char *endNode = doc["EndNode"];
            int value = doc["Value"].as<int>();
            dataCallback(doc); // Call the callback function
        }
    }
    else
    {
        Serial.println("Unknown msg type!");
    }
}

void MQTTDataParser::setCallback(MQTTDataCallback callback)
{
    dataCallback = callback;
}
