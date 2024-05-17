#include "server.h"
#include <CheapStepper.h>
#include <EEPROM.h>

// Declaration of global variables
extern uint32_t eeprom1;
extern uint32_t eeprom2;
extern uint32_t eeprom3;
extern int motorPosition;
extern int motorStep;
extern int motorSpeed;
extern const String chipId;
extern CheapStepper stepper;

// Server initialization on port 80
AsyncWebServer server(80); 

// Function for reading a file from LittleFS
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

// Function for configuring the server
void setupServer() {
    // Serving HTML file
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = readFile(LittleFS, "/index.html");
        html.replace("{{eeprom1}}", String(eeprom1));
        html.replace("{{eeprom2}}", String(eeprom2));
        html.replace("{{eeprom3}}", String(eeprom3));
        html.replace("{{chipId}}", String(chipId));
        request->send(200, "text/html", html);
    });

    // Handling motor control
    server.on("/stepCW", HTTP_GET, [](AsyncWebServerRequest *request) {
        stepper.moveCW(motorStep); // Rotate 1 degree clockwise
        motorPosition += motorStep;
        String response = "{\"position\":" + String(motorPosition) + "}";
        request->send(200, "application/json", response);
    });

    server.on("/stepCCW", HTTP_GET, [](AsyncWebServerRequest *request) {
        stepper.moveCCW(motorStep); // Rotate 1 degree counterclockwise
        motorPosition -= motorStep;
        String response = "{\"position\":" + String(motorPosition) + "}";
        request->send(200, "application/json", response);
    });

    // Handling EEPROM POST
    server.on("/eeprom", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool updated = false;

        if (request->hasParam("eeprom1", true)) {
            uint32_t value = request->getParam("eeprom1", true)->value().toInt();
            if (value != eeprom1) {
                eeprom1 = value;
                EEPROM.put(0, value);
                updated = true;
                Serial.println("Updated eeprom1: " + String(eeprom1));
            }
        }

        if (request->hasParam("eeprom2", true)) {
            uint32_t value = request->getParam("eeprom2", true)->value().toInt();
            if (value != eeprom2) {
                eeprom2 = value;
                EEPROM.put(4, value);
                updated = true;
                Serial.println("Updated eeprom2: " + String(eeprom2));
            }
        }

        if (request->hasParam("eeprom3", true)) {
            uint32_t value = request->getParam("eeprom3", true)->value().toInt();
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

    // Start the server
    server.begin();
}
