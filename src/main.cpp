#include <Arduino.h>
#include <CheapStepper.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "server.h"

// Change in the .env file
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// Motor configuration
int motorStep = 256;    // Number of steps per revolution
int motorSpeed = 12;    // Motor speed in RPM

// EEPROM configuration
const int EEPROM_SIZE = 12;  // 3 unsigned long integers (4 bytes each)
uint32_t eeprom1 = 0;
uint32_t eeprom2 = 0;
uint32_t eeprom3 = 0;

// Global variables
int motorPosition = 0;  // Current motor position
const String chipId = String(ESP.getChipId(), HEX);
CheapStepper stepper(D1, D2, D5, D6);  // Stepper motor configuration

void setup() {
  stepper.setRpm(motorSpeed);    // Set motor speed
  Serial.begin(115200);  // Start serial communication

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Error mounting LittleFS");
    return;
  }

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Read values from EEPROM
  EEPROM.get(0, eeprom1);
  EEPROM.get(4, eeprom2);
  EEPROM.get(8, eeprom3);

  Serial.println("Initial EEPROM values:");
  Serial.println("eeprom1: " + String(eeprom1));
  Serial.println("eeprom2: " + String(eeprom2));
  Serial.println("eeprom3: " + String(eeprom3));

  if (eeprom3 != 0) {
    motorPosition = eeprom3;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi: " + String(ssid) + "...");
  }
  Serial.println("Connected to WiFi");

  // Configure the server
  setupServer();
}

void loop() {
  // Nothing to do here
}
