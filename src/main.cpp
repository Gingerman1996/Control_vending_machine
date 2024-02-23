#include <Arduino.h>
#include <EEPROM.h>
#include "dataSender.h"
#include "config.h"
#include <HX711.h>
#include "hx711Reader.h"

#define LOAD_CELL_ACTIVE

// time dulation
unsigned long start;
unsigned long end;
unsigned long delta;

// Function for mqtt process
void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqttLoop(void *parameter);

// NTP client loop
void NTPloop(void *parameter);

// Function to handle pump data
void getPumpReady(void *parameter);
// Function to check pump status
void CheckPumpStatus(void *parameter);
// Function to pumping
void pumpStart(void *parameter);
// Function to handle swap data
void saveSwapData(void *parameter);
// Function to handle calibration data
void Calibration(void *parameter);

// Callback function for handling data
void dataCallback(StaticJsonDocument<200> doc)
{
  // Perform actions based on the received message
  if (doc["msg"] == "pump1" || doc["msg"] == "pump2")
  {
    // handle pump data
    dataPump *handlepump = new dataPump;        // สร้างข้อมูลใหม่
    handlepump->msg = doc["msg"].as<String>();  // กำหนดข้อมูล
    handlepump->value = doc["Value"].as<int>(); // กำหนดข้อมูล
    // Create task to work pump
    xTaskCreatePinnedToCore(getPumpReady, "handlePumpData", 4096, (void *)handlepump, 1, &pump_Task, 0); // ส่ง pointer ไปยัง task
  }
  else if (doc["msg"] == "Swap")
  {
    dataSwap *handleSwap = new dataSwap;
    handleSwap->box = doc["Box"];
    handleSwap->value = doc["Value"];
    // Create task for swap
    xTaskCreatePinnedToCore(saveSwapData, "saveSwapData", 4096, (void *)handleSwap, 1, &swap_Task, 0);
  }
  else if (doc["msg"] == "Cal")
  {
    // Call function to handle calibration data
    dataCal *handleCal = new dataCal;
    handleCal->cal_msg = doc["cal_msg"].as<String>();
    handleCal->value = doc["Value"];
    // // Create task for calibration
    xTaskCreatePinnedToCore(Calibration, "Calibration", 4096, (void *)handleCal, 1, &cal_Task, 0);
  }
}

void setup()
{
  start = micros();
  Serial.begin(BAUD_RATE);
  Serial2.begin(BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
  EEPROM.begin(EEPROM_SIZE);

  // Pin set
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Door_1, INPUT_PULLUP);
  pinMode(Door_2, INPUT_PULLUP);
  pinMode(Pump_1, OUTPUT);
  pinMode(Pump_2, OUTPUT);
  pinMode(Ready_pump, INPUT_PULLUP);
  pinMode(Valve_1, OUTPUT);
  pinMode(Valve_2, OUTPUT);

  analogWrite(Pump_1, 0);
  analogWrite(Pump_2, 0);
  digitalWrite(Valve_1, HIGH);
  digitalWrite(Valve_2, HIGH);

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

  // Initialize HX711
  hx711Reader *HX711Reader = hx711Reader::getInstance();
  HX711Reader->setPins1(LOADCELL_1_DOUT_PIN, LOADCELL_1_SCK_PIN);
  HX711Reader->setPins2(LOADCELL_2_DOUT_PIN, LOADCELL_2_SCK_PIN);

  // Initialize NTP server
  NTPClientWrapper *ntpClient = NTPClientWrapper::getInstance();
  ntpClient->setup(NTP_SERVER, TIME_OFFSET); // Set NTP Server and Port

  // Create task for mqttloop
  xTaskCreatePinnedToCore(mqttLoop, "MQTT_LOOP", 4096, NULL, 1, &MQTT_loop_task, 0);
  xTaskCreatePinnedToCore(NTPloop, "NTPloop", 2048, NULL, 1, &NTP_loop, 0);
  end = micros();
  delta = end - start;
  Serial.printf("Dulation Time for void setup: %.2f\n", float(delta) / 1000000.00);
}

void loop()
{
}

// Task mqtt loop
void mqttLoop(void *parameter)
{
  while (1)
  {
    MQTTManager::getInstance()->loop();
    vTaskDelay(xDelay100ms);
  }
}

// NTP client loop
void NTPloop(void *parameter)
{
  while (1)
  {
    NTPClientWrapper *ntpClient = NTPClientWrapper::getInstance();
    ntpClient->update();
    String time = ntpClient->getFormattedTime();
    unsigned long time_long = ntpClient->getEpochTime();
    // Serial.printf("TIME: %s\n", time);
    // Serial.printf("EPOCH: %lu\n", time_long);
    vTaskDelay(xDelay1000ms);
  }
}

// For mqttDataParser
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  // เรียกใช้งาน MQTTDataParser เพื่อคัดแยกข้อมูล
  MQTTDataParser::getInstance()->parseData(payload, length);
}
// Task pump work
void getPumpReady(void *parameter)
{
  // Retrieve the value passed from dataCallback
  dataPump handlePump = *((dataPump *)parameter);

  Serial.printf("msg: %s\nvalue: %d\n", handlePump.msg, handlePump.value);
  if (handlePump.msg == "pump1")
  {
    handlePump.cell = 1;
    handlePump.door = Door_1;
    handlePump.pump_pin = Pump_1;
    handlePump.valve_pin = Valve_1;
  }
  else if (handlePump.msg == "pump2")
  {
    handlePump.cell = 2;
    handlePump.door = Door_2;
    handlePump.pump_pin = Pump_2;
    handlePump.valve_pin = Valve_2;
  }

  xTaskCreatePinnedToCore(CheckPumpStatus, "CheckPumpStatus", 4096, reinterpret_cast<void *>(&handlePump), 1, &cal_Task, 0);

  Serial.printf("Cell: %d\n//////////////////////////\n", handlePump.cell);
  // #ifdef LOAD_CELL_ACTIVE
  //   float calibration_factor;
  //   EEPROM.get(20 + 0x20, calibration_factor);
  //   Serial.printf("Calibration Factof: %.02f\n", calibration_factor);
  //   hx711Reader::getInstance()->readData(handlePump.cell, 0, calibration_factor);
  // #endif
  // Task finished, delete itself
  vTaskDelete(NULL);
}

void door_check(byte door)
{
  bool previousDoorStatus = digitalRead(door);

  while (1)
  {
    // Read the status of the door
    bool currentDoorStatus = digitalRead(door);

    // Check if the door has changed from closed to open
    if (!previousDoorStatus && currentDoorStatus)
    {
      // Wait until the door closes again
      while (digitalRead(door))
      {
        vTaskDelay(xDelay100ms);
      }
      // Update the previous door status
      previousDoorStatus = currentDoorStatus;
      // Exit the while loop
      break;
    }
    // Check if the door has changed from open to closed
    else if (previousDoorStatus && !currentDoorStatus)
    {
      // Wait until the closes again
      while (digitalRead(door))
      {
        vTaskDelay(xDelay100ms);
      }
      // Update the previous door status
      previousDoorStatus = currentDoorStatus;
      // Exit the while loop
      break;
    }
    // If the door is closed, wait a short while before checking again
    else if (!currentDoorStatus)
    {
      vTaskDelay(xDelay100ms);
    }
  }
}

int fluidAvailable(int fluidType, int value)
{
  float totalFluid = 0;
  float EEPROM_get = 0;
  for (int i = fluidType; i <= fluidType * 4; ++i)
  {
    EEPROM.get(i * 4, totalFluid);
    totalFluid += EEPROM_get;
    if (totalFluid >= value)
    {
      return i; // มีของเหลวพอที่จะปั้ม
    }
  }
  return false; // ไม่มีของเหลวพอที่จะปั้ม
}

void CheckPumpStatus(void *parameter)
{
  dataPump handlePump = *((dataPump *)parameter);
  bool door_status;

  // Select boxs of fluid
  int fluidType = (handlePump.cell, handlePump.value);
  Serial2.println((handlePump.cell * 10) + fluidType - 1);

  // Door status checking at begin
  door_check(handlePump.door);
  Serial2.println(201);

  // Set zero for prepare pump
  hx711Reader::getInstance()->setTare(handlePump.cell);

  // Create task for pump
  xTaskCreatePinnedToCore(pumpStart, "pumpStart", 4096, reinterpret_cast<void *>(&handlePump), 1, &pump_Task, 0);

  // Check status during pumping
  while (1)
  {

    vTaskDelay(xDelay100ms);
  }
}

void pumpStart(void *parameter)
{
}

// Function to handle swap data
void saveSwapData(void *parameter)
{
  // Retrieve the value passed from dataCallback
  dataSwap handleSwap = *((dataSwap *)parameter);

  Serial.printf("Box: %d\nvalue: %.2f\n//////////////////////////\n", handleSwap.box, handleSwap.value);

  EEPROM.put(handleSwap.box * 4, handleSwap.value);
  EEPROM.commit();

  // Send EEPROM data to MQTT
  dataSender::getInstance()->sendSwapEEPROMData(&handleSwap.box);

  // Task finished, delete itself
  vTaskDelete(NULL);
}

// Function to handle calibration data
void Calibration(void *parameter)
{
  start = micros();
  dataCal handleCal = *((dataCal *)parameter);

  float handelCalConstant;
  float containner[2];
  int cell;

  switch (handleCal.cal_msg.toInt())
  {
  case 1:
  case 2:
  case 3:
  case 4:
  case 9:
    cell = 1;
    break;
  case 5:
  case 6:
  case 7:
  case 8:
  case 10:
    cell = 2;
    break;
  default:
    break;
  }
  if (handleCal.cal_msg.toInt() == 9 || handleCal.cal_msg.toInt() == 10)
  {
    hx711Reader::getInstance()->FindZeroFactor((handleCal.cal_msg.toInt() % 9) + 1);
  }
  else
  {
#ifdef LOAD_CELL_ACTIVE
    handelCalConstant = hx711Reader::getInstance()->FindCalibrationFactor(handleCal.value, cell);
    EEPROM.put(handleCal.cal_msg.toInt() * 4 + 0x20, handelCalConstant);
    EEPROM.commit();
#else
    Serial.printf("Cal msg: %d\ncell: %d\nvalue: %.2f\n//////////////////////////\n", handleCal.cal_msg.toInt(), cell, handleCal.value);
#endif
  }
  dataSender::getInstance()->sendCalData(cell, handleCal.cal_msg.toInt(), handleCal.value, 0);
  end = micros();
  delta = end - start;
  Serial.printf("Dulation Time for cal function: %.2f ms\n", float(delta) / 1000.00);
  vTaskDelete(NULL);
}
