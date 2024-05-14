#include <Arduino.h>
#include <CheapStepper.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

AsyncWebServer server(80);

const int EEPROM_SIZE = 8;  // 2 unsigned long integers (4 bytes each)
uint32_t value1 = 0;
uint32_t value2 = 0;
int motorPosition = 0;  // Pozycja silnika
int motorStep = 256;  // Ilość stopni obrotu silnika

CheapStepper stepper(D5, D6, D7, D8);  // Konfiguracja pinów do sterowania silnikiem

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

void setup() {
  stepper.setRpm(16);  // Ustawienie prędkości obrotowej silnika

  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Read values from EEPROM
  value1 = EEPROM.read(0) | (EEPROM.read(1) << 8) | (EEPROM.read(2) << 16) | (EEPROM.read(3) << 24);
  value2 = EEPROM.read(4) | (EEPROM.read(5) << 8) | (EEPROM.read(6) << 16) | (EEPROM.read(7) << 24);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi: " + String(ssid) + "...");
  }
  Serial.println("Connected to WiFi");

  // Serve HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = readFile(LittleFS, "/index.html");
    html.replace("{{value1}}", String(value1));
    html.replace("{{value2}}", String(value2));
    request->send(200, "text/html", html);
  });

  // Handle form submission
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value1", true) && request->hasParam("value2", true)) {
      value1 = request->getParam("value1", true)->value().toInt();
      value2 = request->getParam("value2", true)->value().toInt();

      // Write to EEPROM
      EEPROM.write(0, value1 & 0xFF);
      EEPROM.write(1, (value1 >> 8) & 0xFF);
      EEPROM.write(2, (value1 >> 16) & 0xFF);
      EEPROM.write(3, (value1 >> 24) & 0xFF);
      EEPROM.write(4, value2 & 0xFF);
      EEPROM.write(5, (value2 >> 8) & 0xFF);
      EEPROM.write(6, (value2 >> 16) & 0xFF);
      EEPROM.write(7, (value2 >> 24) & 0xFF);
      EEPROM.commit();

      request->redirect("/");
    } else {
      request->send(400, "text/plain", "Invalid Input");
    }
  });

  // Handle motor control
  server.on("/stepCW", HTTP_GET, [](AsyncWebServerRequest *request) {
    stepper.moveCW(motorStep);  // Obrót o 1 stopień zgodnie z ruchem wskazówek zegara
    motorPosition += motorStep;
    String response = "{\"position\":" + String(motorPosition) + "}";
    request->send(200, "application/json", response);
  });

  server.on("/stepCCW", HTTP_GET, [](AsyncWebServerRequest *request) {
    stepper.moveCCW(motorStep);  // Obrót o 1 stopień przeciwnie do ruchu wskazówek zegara
    motorPosition -= motorStep;
    String response = "{\"position\":" + String(motorPosition) + "}";
    request->send(200, "application/json", response);
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing to do here
}
