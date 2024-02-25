# README

## Code Documentation

This document describes the purpose and functionality of the provided code.

## Purpose

The code is designed to control a fluid dispensing system using pumps and load cells. It receives commands via MQTT, manages pump operations, monitors fluid levels, and sends status updates.

## Key Features

* **MQTT Communication:** Subscribes to MQTT topics for commands and publishes status updates.
* **Pump Control:** Manages pump activation, speed, and valve opening/closing.
* **Load Cell Integration:** Uses HX711 load cells to measure fluid weight for accurate dispensing.
* **Fluid Management:** Tracks fluid levels in multiple containers and selects appropriate containers for dispensing.
* **Calibration:** Supports calibration of load cells for precise measurements.
* **NTP Time Sync:** Synchronizes with an NTP server for accurate timestamps.
* **Error Handling:** Detects and handles potential errors, such as pump malfunctions or fluid shortages.

## Code Structure

* **Setup:** Initializes hardware components, connects to WiFi and MQTT, sets up NTP time sync, and calibrates load cells.
* **Main Loop:** 
    - Empties, relying on tasks for continuous operations.
* **Tasks:** 
    - `mqttLoop`: Handles MQTT communication in a separate thread.
    - `NTPloop`: Updates time from the NTP server periodically.
    - `getPumpReady`: Prepares a pump for dispensing based on MQTT commands.
    - `CheckPumpStatus`: Monitors pump status, fluid flow, door opening, and handles errors.
    - `pumpStart`: Activates the pump and adjusts speed based on fluid weight.
    - `pumpStop`: Stops the pump and closes valves.
    - `saveSwapData`: Saves fluid level data to EEPROM and sends it via MQTT.
    - `Calibration`: Handles calibration commands for load cells.

## Additional Notes

- The code utilizes FreeRTOS for task management.
- EEPROM is used to store fluid level data and calibration constants.
- Serial communication is used for debugging and logging.

## Dependencies

- Arduino libraries: MQTT, NTPClient, HX711
- FreeRTOS

## Usage

1. Compile and upload the code to your Arduino board.
2. Configure WiFi and MQTT settings in the `config.h` file.
3. Connect the hardware components as specified in the code.
4. Send MQTT commands to control the system (see `dataCallback` function for supported commands).
