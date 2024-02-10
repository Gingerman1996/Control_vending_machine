#include <Arduino.h>
#include "model.h"
#include "config.h"

void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqttLoop(void *pvvalue);

// Function to handle pump1 data
void handlePump1(const char *value);
// Function to handle pump2 data
void handlePump2(const char *value);
// Function to handle swap data
void handleSwap(const char *value);
// Function to handle calibration data
void handleCalibration(const char *value);

// Callback function for handling data
void dataCallback(const char* msg, const char *value)
{
  String msg_str = String(msg);
  // Perform actions based on the received message
  if (msg_str == "pump1")
  {
    // Call function to handle pump1 data
    handlePump1(value);
  }
  else if (msg_str == "pump2")
  {
    // Call function to handle pump2 data
    handlePump2(value);
  }
  else if (msg_str == "swap")
  {
    // Call function to handle swap data
    handleSwap(value);
  }
  else if (msg_str == "cal")
  {
    // Call function to handle calibration data
    handleCalibration(value);
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);

  // Initialize WiFiManager
  myWiFiManager *wifiManager = myWiFiManager::getInstance();
  wifiManager->connect();
  wifiManager->setWiFiTimeout(WIFI_TIMEOUT_MS);

  // Initialize MQTTManager
  WiFiClient *wifiClient = wifiManager->getClient();

  PubSubClient *client = new PubSubClient(*wifiClient);
  MQTTManager *mqttManager = MQTTManager::getInstance();
  mqttManager->setMQTTTimeout(MQTT_TIMEOUT_MS);
  mqttManager->setClient(client);

  mqttManager->setServer(MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
  mqttManager->connect();
  mqttManager->subscribe(MQTT_SUBSCRIBE); // Subscribe topic

  // Initialize MessageHandler and set it to MQTTManager
  mqttManager->setMessageCallback(mqttCallback); // Set message callback

  // Set callback function for MQTTDataParser
    MQTTDataParser::getInstance()->setCallback(dataCallback);

  // xTaskCreatePinnedToCore(mqttLoop, "MQTT_LOOP", 2048, NULL, 1, &MQTT_loop_task, 0);
}

void loop()
{
  MQTTManager::getInstance()->loop();
}

void mqttLoop(void *pvvalue)
{
  // while (1)
  // {
  //   MQTTManager::getInstance()->loop();
  //   vTaskDelay(xDelay100ms);
  // }
}

// For mqttDataParser
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  // เรียกใช้งาน MQTTDataParser เพื่อคัดแยกข้อมูล
  MQTTDataParser::getInstance()->parseData(payload, length);
}

// Function to handle pump1 data
void handlePump1(const char *value)
{
  Serial.printf("Pump1 value: %s\n", value);
}

// Function to handle pump2 data
void handlePump2(const char *value)
{
  // Add your logic here...
}

// Function to handle swap data
void handleSwap(const char *value)
{
  // Add your logic here...
}

// Function to handle calibration data
void handleCalibration(const char *value)
{
  // Add your logic here...
}