#pragma once

#ifndef MODEL_H
#define MODEL_H

#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "mqttDataParser.h"

class myWiFiManager
{
private:
    WiFiClient wifiClient;
    WiFiManager wm;

    myWiFiManager();
    static myWiFiManager *instance;

    unsigned long wifiTimeout; // WiFi connection timeout

public:
    static myWiFiManager *getInstance();
    WiFiClient *getClient();
    bool connect();
    void setWiFiTimeout(unsigned long timeout);
};

class MQTTManager
{
private:
#define MQTTpubQos 2
    MQTTManager();
    static MQTTManager *instance;
    PubSubClient *client;
    const char *mqttServer;
    int mqttPort;
    const char *mqttUsername;
    const char *mqttPassword;
    unsigned long mqttTimeout; // MQTT connection timeout

public:
    static MQTTManager *getInstance();
    void setClient(PubSubClient *mqttClient);
    void setServer(const char *server, int port, const char *username, const char *password);
    void connect();
    void subscribe(const char *topic);
    void publish(const char *topic, const char *payload);
    void loop();
    void setMQTTTimeout(unsigned long timeout);
    void setMessageCallback(void (*callback)(char *, byte *, unsigned int));
};

class NTPClientWrapper
{
private:
    static NTPClientWrapper *instance; // Static instance variable
    WiFiUDP *udp;                      // UDP instance
    NTPClient *ntpClient;              // NTPClient instance
    const char *ntpServer;

    NTPClientWrapper() {}

public:
    static NTPClientWrapper *getInstance();         // Get instance method
    void setup(const char *server, int timeOffset); // Setup method
    void update();                                  // Update method
    unsigned long getEpochTime();                   // Get epoch time method
    String getFormattedTime();                      // Get Fromatted time method
};

#endif