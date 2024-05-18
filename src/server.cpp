#include "server.h"

#include <CheapStepper.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Ticker.h>

// Global variable declarations
extern long eeprom1;
extern long eeprom2;
extern long eeprom3;
extern int motorPosition;
extern const int motorStep;
extern const int motorSpeed;
extern const int stepsPerRevolution;
extern const String chipId;

extern CheapStepper stepper;
extern Ticker motorTicker;

// Flag to control motor operation
volatile bool motorRunning = false;
volatile int targetPosition = 0;

const int msDelay = 2;

// Function to read a file from LittleFS
String readFile(fs::FS &fs, const char *path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  return fileContent;
}

// Function to control a single motor step
void motorStepControl() {
  if (!motorRunning) {
    return;
  }

  if (motorPosition < targetPosition) {
    stepper.stepCW();
    motorPosition++;
  } else if (motorPosition > targetPosition) {
    stepper.stepCCW();
    motorPosition--;
  } else {
    motorRunning = false;
    motorTicker.detach();
    EEPROM.put(8, motorPosition);  // Save current position to EEPROM
    EEPROM.commit();
    eeprom3 = motorPosition;
    Serial.println("Motor position saved: " + String(motorPosition));
  }
}

// Function to configure the server
void setupServer() {
  // Serve HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = readFile(LittleFS, "/index.html");
    html.replace("{{eeprom1}}", String(eeprom1));
    html.replace("{{eeprom2}}", String(eeprom2));
    html.replace("{{eeprom3}}", String(eeprom3));
    html.replace("{{chipId}}", String(chipId));
    request->send(200, "text/html", html);
  });

  // Handle motor control
  // Move to a specific position (with full rotation if needed)
  server.on("/moveTo", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("position", true)) {
      int position = request->getParam("position", true)->value().toInt();
      Serial.println("Moving to position: " + String(position));
      targetPosition = position;
      motorRunning = true;
      motorTicker.attach_ms(msDelay, motorStepControl);
      String response = "{\"position\":" + String(motorPosition) + "}";
      request->send(200, "application/json", response);
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing position parameter\"}");
    }
  });

  // Handle motor stop
  server.on("/stop", HTTP_POST, [](AsyncWebServerRequest *request) {
    motorRunning = false;  // Set the stop flag
    motorTicker.detach();
    EEPROM.put(8, motorPosition);  // Save current position to EEPROM
    EEPROM.commit();
    eeprom3 = motorPosition;
    Serial.println("Motor stopped and position saved: " + String(motorPosition));

    request->send(200, "application/json", "{\"status\":\"Motor stopped\"}");
  });

  // Handle EEPROM POST
  server.on("/eeprom", HTTP_POST, [](AsyncWebServerRequest *request) {
    bool updated = false;

    if (request->hasParam("eeprom1", true)) {
      long value = request->getParam("eeprom1", true)->value().toInt();
      if (value != eeprom1) {
        eeprom1 = value;
        EEPROM.put(0, value);
        updated = true;
        Serial.println("Updated eeprom1: " + String(eeprom1));
      }
    }

    if (request->hasParam("eeprom2", true)) {
      long value = request->getParam("eeprom2", true)->value().toInt();
      if (value != eeprom2) {
        eeprom2 = value;
        EEPROM.put(4, value);
        updated = true;
        Serial.println("Updated eeprom2: " + String(eeprom2));
      }
    }

    if (request->hasParam("eeprom3", true)) {
      long value = request->getParam("eeprom3", true)->value().toInt();
      if (value != eeprom3) {
        eeprom3 = value;
        EEPROM.put(8, value);
        updated = true;
        Serial.println("Updated eeprom3: " + String(eeprom3));
      }
    }

    if (updated) {
      EEPROM.commit();
      Serial.println("EEPROM committed");
    }
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // Handle EEPROM GET
  server.on("/eeprom", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "{\"eeprom1\":" + String(eeprom1) + ",\"eeprom2\":" + String(eeprom2) + ",\"eeprom3\":" + String(eeprom3) + "}";
    request->send(200, "application/json", response);
  });

  // Start the server
  server.begin();
}