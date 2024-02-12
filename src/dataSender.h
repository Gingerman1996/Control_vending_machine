#pragma once

#ifndef DATA_SENDER_H
#define DATA_SENDER_H

#include <EEPROM.h>
#include "model.h"

class dataSender
{
private:
    static dataSender *instance; // Singleton instance
    MQTTManager *mqttManager;    // MQTTManager instance

    dataSender(); // Constructor

public:
    static dataSender *getInstance();                                        // Get instance method
    void sendSwapEEPROMData(int *numof_box);                                 // Method to send EEPROM data to MQTT
    void sendCalData(int topics, int cal_number, float Weight, float minus); // Method to send cal data to MQTT
};

#endif