#include "dataSender.h"

dataSender *dataSender::instance = nullptr;

dataSender::dataSender() {}

dataSender *dataSender::getInstance() {
  if (!instance) {
    instance = new dataSender();
  }
  return instance;
}

void dataSender::sendSwapEEPROMData(int *numof_box) {
  // Creat JSON doc to handle data
  String chipid_str = "458df8";
  StaticJsonDocument<200> doc_swap;
  doc_swap["msg"] = "Swap";
  doc_swap["EndNode"] = "0x" + chipid_str;
  doc_swap["Box"] = String(*numof_box);

  JsonArray box = doc_swap.createNestedArray("Value");
  // Read data from EEPROM address 0x4 - 0x20
  for (int addr = 4; addr <= 0x20; addr += 4) {
    float value;
    EEPROM.get(addr, value);

    // Convert float value to string
    box.add(String(value, 2));
  }
  // Convert JSON to String
  String payload;
  serializeJson(doc_swap, payload);

  // Generate topic for the data
  String topic = "esp32/fabric/swap";

  // Publish data to MQTT
  MQTTManager::getInstance()->publish(topic.c_str(), payload.c_str());
}

void dataSender::sendCalData(int topics, int cal_number, float Weight,
                             float minus) {
  StaticJsonDocument<200> doc_callback;
  String chipid_str = "458df8";
  doc_callback["msg"] = "call_callback";
  doc_callback["EndNode"] = "0x" + chipid_str;
  doc_callback["Cal_number"] = cal_number;
  doc_callback["Weight"] = Weight;
  doc_callback["Minus"] = minus;

  String JsonOutput;
  serializeJson(doc_callback, JsonOutput);

  char callback[JsonOutput.length() + 1];
  JsonOutput.toCharArray(callback, JsonOutput.length() + 1);

  if (topics == 1) {
    MQTTManager::getInstance()->publish("esp32/fabric/cal_callback1", callback);
  } else if (topics == 2) {
    MQTTManager::getInstance()->publish("esp32/fabric/cal_callback2", callback);
  } else if (topics == 3) {
    MQTTManager::getInstance()->publish("esp32/fabric/minus_callback1",
                                        callback);
  } else if (topics == 4) {
    MQTTManager::getInstance()->publish("esp32/fabric/minus_callback2",
                                        callback);
  }
}

void dataSender::sendFlagData(String flags) {
  int code;

  StaticJsonDocument<200> doc_callback;
  // String chipid_str = String(chipId, HEX);
  String chipid_str = "458df8";

  doc_callback["msg"] = "flag";
  doc_callback["EndNode"] = "0x" + chipid_str;

  if (flags == "Start" || flags == "Stop" || flags == "Finished" ||
      flags == "Error01" || flags == "Fixed01") {
    doc_callback["Status"] = flags;
  }

  String JsonOutput;
  serializeJson(doc_callback, JsonOutput);
  Serial.printf("%S: ", flags);
  Serial.println(JsonOutput);

  char callback[JsonOutput.length() + 1];
  JsonOutput.toCharArray(callback, JsonOutput.length() + 1);
  MQTTManager::getInstance()->publish("tablet/fabric", callback);
  flags = "";
}

void dataSender::sendPumpMessage(int msg_in, float pump_value, bool error,
                                 float liquidLevel[8]) {
  int code;

  StaticJsonDocument<200> doc_callback;
  String chipid_str = "458df8";

  if (msg_in == 1) {
    doc_callback["msg"] = "pump1";
  } else if (msg_in == 2) {
    doc_callback["msg"] = "pump2";
  }

  doc_callback["EndNode"] = "0x" + chipid_str;

  if (msg_in == 1 || msg_in == 2) {
    doc_callback["Value"] = String(pump_value);
    if (!error) {
      doc_callback["Code"] = String(200);
    } else {
      doc_callback["Code"] = String(400);
      error = false;
    }
    JsonArray box = doc_callback.createNestedArray("Box");
    for (int i = 0; i < 8; i++) {
      Serial.printf("Add box: %d to jsonArray Value: %.2f\n", i,
                    liquidLevel[i]);
      box.add(String(liquidLevel[i]));
      vTaskDelay(100);
    }
  }
  vTaskDelay(100);
  String Json_Output;
  serializeJson(doc_callback, Serial);
  serializeJson(doc_callback, Json_Output);

  char callback[Json_Output.length() + 1];
  Json_Output.toCharArray(callback, Json_Output.length() + 1);
  MQTTManager::getInstance()->publish("esp32/fabric/callback", callback);
  msg_in = 0;
}