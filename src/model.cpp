#include "model.h"

// Implementation of myWiFiManager class
myWiFiManager *myWiFiManager::instance = nullptr;

myWiFiManager::myWiFiManager() {}

myWiFiManager *myWiFiManager::getInstance()
{
    if (!instance)
    {
        instance = new myWiFiManager();
    }
    return instance;
}

WiFiClient *myWiFiManager::getClient()
{
    return &wifiClient;
}

bool myWiFiManager::connect()
{
    // Connect to WiFi network
    bool isConnected = wm.autoConnect(); // auto generated AP name from chipid
    return isConnected;
}

void myWiFiManager::setWiFiTimeout(unsigned long timeout)
{
    // Set WiFi connection timeout
    wifiTimeout = timeout;
}

// Implementation of MQTTManager class
MQTTManager *MQTTManager::instance = nullptr;

MQTTManager::MQTTManager() {}

MQTTManager *MQTTManager::getInstance()
{
    if (!instance)
    {
        instance = new MQTTManager();
    }
    return instance;
}

void MQTTManager::setClient(PubSubClient *mqttClient)
{
    // Set MQTT client
    client = mqttClient;
}

void MQTTManager::setServer(const char *server, int port, const char *username, const char *password)
{
    // Set MQTT server and credentials
    mqttServer = server;
    mqttPort = port;
    mqttUsername = username;
    mqttPassword = password;
}

void MQTTManager::connect()
{
    // Connect to MQTT broker
    client->setServer(mqttServer, mqttPort);
    Serial.print("Connecting to MQTT broker...");
    unsigned long startAttemptTime = millis();
    while (!client->connected())
    {
        // Attempt to connect to MQTT broker
        if (client->connect("ESP32Client", mqttUsername, mqttPassword, NULL, 2, NULL, NULL))
        {
            Serial.println("Connected!");
            client->subscribe("esp32/fabric");
            return;
        }
        else
        {
            // If connection failed, retry after delay
            Serial.print("Failed, rc=");
            Serial.print(client->state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }

        // Check connection timeout
        if (millis() - startAttemptTime > mqttTimeout)
        { // 30 seconds timeout
            Serial.println("Connection timeout!");
            return;
        }
    }
}

void MQTTManager::subscribe(const char *topic)
{
    // Subscribe to MQTT topic
    client->subscribe(topic);
}

void MQTTManager::publish(const char *topic, const char *payload)
{
    // Publish message to MQTT topic
    client->publish(topic, payload, MQTTpubQos);
}

void MQTTManager::loop()
{
    if (!client->connected())
    {
        // Reconnect to MQTT Broker if connection lost
        connect();
    }

    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED)
    {
        // Reconnect to WiFi network if disconnected
        myWiFiManager::getInstance()->connect();
    }

    // Maintain MQTT connection
    client->loop();
}

void MQTTManager::setMQTTTimeout(unsigned long timeout)
{
    // Set MQTT connection timeout
    mqttTimeout = timeout;
}

void MQTTManager::setMessageCallback(void (*callback)(char *, byte *, unsigned int))
{
    client->setCallback(callback);
}

// NTP Clieant
NTPClientWrapper *NTPClientWrapper::instance = nullptr;

NTPClientWrapper *NTPClientWrapper::getInstance()
{
    if (!instance)
    {
        instance = new NTPClientWrapper();
    }
    return instance;
}

void NTPClientWrapper::setup(const char *server, int timeOffset)
{
    // Initialize UDP and NTPClient instances
    int offset = timeOffset * 3600;
    udp = new WiFiUDP();
    ntpClient = new NTPClient(*udp, server);

    ntpServer = server;

    // Begin NTPClient
    ntpClient->begin();
    ntpClient->setTimeOffset(offset);
}

void NTPClientWrapper::update()
{
    // Update NTPClient
    ntpClient->update();
}

unsigned long NTPClientWrapper::getEpochTime()
{
    // Get epoch time from NTPClient
    return ntpClient->getEpochTime();
}

String NTPClientWrapper::getFormattedTime()
{
    return ntpClient->getFormattedTime();
}