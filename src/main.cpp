#include <Arduino.h>
#include "model.h"
#include "config.h"

void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqttLoop(void *pvvalue);
void PumpWork(void *parameter);

struct dataPump
{
  String msg;
  int value;
};

// Function to handle pump data
void handlePump(String msg, int value);
// Function to handle swap data
void handleSwap(int value);
// Function to handle calibration data
void handleCalibration(int value);

// Callback function for handling data
void dataCallback(String msg, int value)
{
  // Perform actions based on the received message
  if (msg == "pump1" || msg == "pump2")
  {
    // Create task to work pump
    dataPump *datapump = new dataPump;                                                 // สร้างข้อมูลใหม่
    datapump->msg = msg;                                                               // กำหนดข้อมูล
    datapump->value = value;                                                           // กำหนดข้อมูล
    xTaskCreatePinnedToCore(PumpWork, "PumpTask", 4026, (void *)datapump, 1, NULL, 0); // ส่ง pointer ไปยัง task
  }
  else if (msg == "swap")
  {
    // Call function to handle swap data
    handleSwap(value);
  }
  else if (msg == "cal")
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

  xTaskCreatePinnedToCore(mqttLoop, "MQTT_LOOP", 8192, NULL, 1, &MQTT_loop_task, 0);
}

void loop()
{
}

// Task mqtt loop
void mqttLoop(void *pvvalue)
{
  while (1)
  {
    MQTTManager::getInstance()->loop();
    vTaskDelay(xDelay100ms);
  }
}

// Task pump work
void PumpWork(void *parameter)
{
  // Retrieve the value passed from dataCallback
  dataPump datapump = *((dataPump *)parameter);

  // Add your logic here to process the received data...

  // Task finished, delete itself
  vTaskDelete(NULL);
}

// For mqttDataParser
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  // เรียกใช้งาน MQTTDataParser เพื่อคัดแยกข้อมูล
  MQTTDataParser::getInstance()->parseData(payload, length);
}

// Function to handle pump data
void handlePump(String msg, int value)
{
  Serial.printf("Pump1 value: %d\n", value);
}

// Function to handle swap data
void handleSwap(int value)
{
  // Add your logic here...
}

// Function to handle calibration data
void handleCalibration(int value)
{
  // Add your logic here...
}