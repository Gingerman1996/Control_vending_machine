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

const TickType_t xDelay100ms = pdMS_TO_TICKS(100);
TaskHandle_t MQTT_loop_task = NULL;

#endif
