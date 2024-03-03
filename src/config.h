#pragma once

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Manager
#define WIFI_TIMEOUT_MS 30000 // WiFi connection timeout in milliseconds

// MQTT Broker
#define MQTT_SERVER "165.232.170.5"
#define MQTT_PORT 1883
#define MQTT_USERNAME "farbic_MUC"
#define MQTT_PASSWORD "beonit0138"
#define MQTT_SUBSCRIBE "esp32/fabric"
#define MQTT_TIMEOUT_MS 30000 // MQTT connection timeout in milliseconds

// Serial Port
#define BAUD_RATE 115200

// EEPROM 
#define EEPROM_SIZE 256

// NTP client
#define NTP_SERVER "pool.ntp.org"
#define TIME_OFFSET 7

// HX711 pins
#define LOADCELL_1_DOUT_PIN 5
#define LOADCELL_1_SCK_PIN 18
#define LOADCELL_2_DOUT_PIN 19
#define LOADCELL_2_SCK_PIN 21

// Pump config
#define RXD2 16
#define TXD2 17

#define Door_1 13
#define Door_2 14

#define Pump_1 26
#define Pump_2 27
#define Ready_pump 22

#define Valve_1 32
#define Valve_2 33

// Task config
const TickType_t xDelay100ms = pdMS_TO_TICKS(100);
const TickType_t xDelay1000ms = pdMS_TO_TICKS(1000);

TaskHandle_t MQTT_loop_task = NULL;
TaskHandle_t Data_Task = NULL;
TaskHandle_t NTP_loop = NULL;
TaskHandle_t pump_Task = NULL;
TaskHandle_t swap_Task = NULL;
TaskHandle_t cal_Task = NULL;
TaskHandle_t Check_PumpStatus = NULL;
TaskHandle_t pumping_task = NULL;
TaskHandle_t IdelWeightCheck_task = NULL;
TaskHandle_t drainFluid_task = NULL;

#endif
