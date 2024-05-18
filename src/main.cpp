#include <Arduino.h>
#include <CheapStepper.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <Ticker.h>

#include "server.h"

// Change in the .env file
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *mqtt_server = MQTT_SERVER;
const uint16_t mqtt_port = MQTT_PORT;
const char *mqtt_user = MQTT_USER;
const char *mqtt_password = MQTT_PASSWORD;

// Motor configuration
const int motorStep = 256;  // Number of steps per revolution
const int motorSpeed = 12;  // Motor speed in RPM

// EEPROM configuration
const int EEPROM_SIZE = 12;  // 3 unsigned long integers (4 bytes each)
long eeprom1 = 0;
long eeprom2 = 0;
long eeprom3 = 0;

// Global variables
int motorPosition = 0;  // Current motor position
const String chipId = String(ESP.getChipId(), HEX);
CheapStepper stepper(D1, D2, D5, D6);  // Stepper motor configuration

Ticker motorTicker;         // Ticker instance
AsyncWebServer server(80);  // Server instance

WiFiClient espClient;
PubSubClient client(espClient);

// Function to handle MQTT messages
void callback(char *topic, byte *payload, unsigned int length) {
  String messageTemp;

  for (unsigned int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }

  if (String(topic) == "roller_blinder/moveTo") {
    int position = messageTemp.toInt();
    motorRunning = true;
    targetPosition = position;
    motorTicker.attach_ms(1, motorStepControl);
  }

  if (String(topic) == "roller_blinder/stop") {
    motorRunning = false;
    motorTicker.detach();
    EEPROM.put(8, motorPosition);
    EEPROM.commit();
  }
}

// Function to reconnect to MQTT
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      client.subscribe("roller_blinder/moveTo");
      client.subscribe("roller_blinder/stop");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  stepper.setRpm(motorSpeed);  // Set motor speed
  Serial.begin(115200);        // Start serial communication

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

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Configure the server
  setupServer();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}