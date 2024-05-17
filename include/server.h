#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <CheapStepper.h>

extern AsyncWebServer server;

extern uint32_t eeprom1;
extern uint32_t eeprom2;
extern uint32_t eeprom3;
extern int motorPosition;
extern int motorStep;
extern int motorSpeed;
extern const String chipId;
extern CheapStepper stepper;

void setupServer();

#endif // SERVER_H