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
void pumpStart(dataPump *handlepump, float currentWeight);
void pumpStop();
// Function to handle swap data
void saveSwapData(void *parameter);
// Function to handle calibration data
void Calibration(void *parameter);
// Fucntion to checking weight
void IdelWeightCheck(void *parameter);

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

  // hx711Reader::getInstance()->FindZeroFactor(1);
  // hx711Reader::getInstance()->FindZeroFactor(2);

  // Create task for mqttloop
  xTaskCreatePinnedToCore(mqttLoop, "MQTT_LOOP", 4096, NULL, 1, &MQTT_loop_task, 0);
  xTaskCreatePinnedToCore(NTPloop, "NTPloop", 2048, NULL, 1, &NTP_loop, 0);
  // xTaskCreatePinnedToCore(IdelWeightCheck, "IdelWeightCheck", 2014, NULL, 1, &IdelWeightCheck_task, 0);
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

// Checking Weight
void IdelWeightCheck(void *parameter)
{
  while (1)
  {
    float calibrationFactor1;
    float calibrationFactor2;
    EEPROM.get(4 + 0x20, calibrationFactor1);
    EEPROM.get(20 + 0x20, calibrationFactor2);
    float weight1 = hx711Reader::getInstance()->readData(1, 0, calibrationFactor1);
    float weight2 = hx711Reader::getInstance()->readData(2, 0, calibrationFactor2);
    Serial.printf("Weight Cell 1: %.2f\n", weight1);
    Serial.printf("Weight Cell 2: %.2f\n", weight2);
    vTaskDelay(xDelay1000ms * 60);
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

bool isDoorOpen(byte door)
{
  return digitalRead(door) == LOW; // ถ้าประตูเปิด จะมีค่า LOW แล้วคืนค่าเป็น true
}

int fluidAvailable(int fluidType, int value)
{
  float totalFluid = 0;
  float EEPROM_get = 0;
  int fluid_number = 0;
  if (fluidType == 1)
    fluid_number = 1;
  else
    fluid_number = 5;
  for (int i = fluid_number; i <= fluid_number + 3; ++i)
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
  float calibrationFactor;
  if (handlePump.cell == 1)
  {
    switch (handlePump.value)
    {
    case 30:
      EEPROM.get(4 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    case 60:
      EEPROM.get(8 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    case 500:
      EEPROM.get(12 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    case 1000:
      EEPROM.get(16 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    }
  }
  else if (handlePump.cell == 2)
  {
    switch (handlePump.value)
    {
    case 30:
      EEPROM.get(20 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    case 60:
      EEPROM.get(24 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    case 500:
      EEPROM.get(28 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    case 1000:
      EEPROM.get(32 + 0x20, calibrationFactor);
      Serial.printf("Calibration Factof: %.02f\n", calibrationFactor);
      break;
    }
  }

  // Set zero for prepare pump
  hx711Reader::getInstance()->setTare(handlePump.cell);

  // Select boxs of fluid
  int fluidType = fluidAvailable(handlePump.cell, handlePump.value);
  if (fluidType != false)
  {
    Serial.printf("Send Serial2 data: %d\n", (handlePump.cell * 10) + fluidType - 1);
    Serial2.println((handlePump.cell * 10) + fluidType - 1);
  }
  else
    vTaskDelete(NULL);

  // Door status checking at begin
  door_check(handlePump.door);
  Serial2.println(201);

  // Check status during pumping
  float previousWeight = hx711Reader::getInstance()->readData(handlePump.cell, 0, calibrationFactor); // เก็บค่าน้ำหนักก่อนหน้า
  unsigned long previousTime = millis();
  unsigned long currentTime; // เก็บเวลาก่อนปั้ม
  bool sendStartMQTT = false;
  bool error = false;
  Serial.printf("Previous Weight: %.2f\n", previousWeight);

  while (1)
  {
    // ตรวจสอบสถานะ Ready_pump และประตูปิด
    bool readyPumpStatus = digitalRead(Ready_pump);
    bool doorClosed = digitalRead(handlePump.door);

    if (!readyPumpStatus && !doorClosed && error == false)
    {
      // ถ้า Ready_pump ต่ำและประตูปิด แสดงว่าปั้มทำงาน
      // ตรวจสอบการไหลของของเหลว
      if (!sendStartMQTT)
      {
        dataSender::getInstance()->sendFlagData("Start");
        sendStartMQTT = true;
      }

      float currentWeight = hx711Reader::getInstance()->readData(handlePump.cell, 0, calibrationFactor);
      Serial.printf("Current Weight: %.3f\n", currentWeight);

      currentTime = millis();
      Serial.printf("currentTime: %d\n", currentTime);
      Serial.printf("Previous Time: %d\n", previousTime);
      Serial.printf("Diff Time: %d\n", currentTime - previousTime);

      pumpStart(&handlePump, currentWeight); // start Pump
      if (handlePump.value - currentWeight < 5)
      {
        delay(100);
        pumpStop();
      }
      else if (handlePump.value - currentWeight < 30)
      {
        delay(500);
        pumpStop();
      }
      else if (handlePump.value - currentWeight < 60)
      {
        delay(1000);
        pumpStop();
      }

      if (currentWeight - previousWeight < 10 && currentTime - previousTime > 60000 && error == false)
      {
        // ถ้าไม่มีการไหลของของเหลวเกิน 10 g เกิน 1 นาที
        pumpStop();
        Serial2.println(200);
        // ตรวจสอบ Ready_pump อีกครั้ง
        if (!digitalRead(Ready_pump))
        {
          Serial2.println(200);
        }
        else
        {
          // ถ้า Ready_pump ยังเป็น low ให้ส่งค่า 200 อีกครั้ง
          // หรือทำสิ่งอื่นตามที่ต้องการ
        }

        // ส่ง error ขึ้น MQTT
        dataSender::getInstance()->sendFlagData("Error01");
        error = true;
      }
      else if (currentWeight - previousWeight > 10 && currentTime - previousTime > 60000)
      {
        previousWeight = hx711Reader::getInstance()->readData(handlePump.cell, 0, calibrationFactor);
        previousTime = millis();
      }

      // Finish pump
      if (currentWeight >= handlePump.value)
      {
        pumpStop();
        delay(5000);
        currentWeight = hx711Reader::getInstance()->readData(handlePump.cell, 0, calibrationFactor);
        Serial.printf("Final Weight: %.2f\n", currentWeight);
        dataSender::getInstance()->sendFlagData("Finished");
        Serial2.println(200);
        vTaskDelete(NULL);
      }
      // อัพเดทค่าน้ำหนักและเวลาก่อนหน้า
      // previousWeight = hx711Reader::getInstance()->readData(handlePump.cell, 0, calibrationFactor);
      // previousTime = millis();
    }
    // ตรวจสอบประตูว่ามีการเปิดหรือไม่
    if (doorClosed)
    {
      // ถ้ามีการเปิดประตู
      // หยุดปั้มทันที
      pumpStop();
      Serial.println("Stop");
      Serial2.println(201);
      // อ่านค่าน้ำหนัก
      float currentWeight = hx711Reader::getInstance()->readData(handlePump.cell, 0, calibrationFactor);
      // ตรวจสอบ error หากมี
      if (currentWeight <= 5 && error == true)
      {
        // ถ้าค่าน้ำหนักอยู่ในช่วง 0 - 5 กิโลกรัม
        Serial2.println(200);
        // ส่ง data ขึ้น MQTT
        dataSender::getInstance()->sendFlagData("Fixed01");
        vTaskDelete(NULL);
      }
      else
      {
        sendStartMQTT = false;
        dataSender::getInstance()->sendFlagData("Stop");
      }
    }
    // รอสักครู่ก่อนที่จะทำการวน loop ต่อ
    vTaskDelay(xDelay100ms);
  }
}

void pumpStart(dataPump *handlepump, float currentWeight)
{
  if (handlepump->cell == 1)
  {
    digitalWrite(Valve_1, LOW);
    if (handlepump->value <= 30)
    {
      analogWrite(handlepump->pump_pin, 80);
    }
    else if (handlepump->value > 30 && currentWeight < handlepump->value - 30)
    {
      analogWrite(handlepump->pump_pin, 150);
    }
    else if (handlepump->value > 30 && currentWeight > handlepump->value - 30)
    {
      analogWrite(handlepump->pump_pin, 80);
    }
  }
  else if (handlepump->value == 2)
  {
    digitalWrite(Valve_2, LOW);
    if (handlepump->value <= 30)
    {
      analogWrite(handlepump->pump_pin, 50);
    }
    else if (handlepump->value > 30 && currentWeight < handlepump->value - 30)
    {
      analogWrite(handlepump->pump_pin, 90);
    }
    else if (handlepump->value > 30 && currentWeight > handlepump->value - 30)
    {
      analogWrite(handlepump->pump_pin, 50);
    }
  }
}

void pumpStop()
{
  digitalWrite(Valve_1, HIGH);
  digitalWrite(Valve_2, HIGH);
  analogWrite(Pump_1, 0);
  analogWrite(Pump_2, 0);
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
    Serial.printf("Set cal @: %d\tCalibrationFactor: %.2f\n", handleCal.cal_msg.toInt() * 4 + 0x20, handelCalConstant);
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
