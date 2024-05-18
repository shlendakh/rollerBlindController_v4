#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <CheapStepper.h>
#include <Ticker.h>

extern AsyncWebServer server;
extern Ticker motorTicker;

extern long eeprom1;
extern long eeprom2;
extern long eeprom3;
extern int motorPosition;
extern const int motorStep;
extern const int motorSpeed;
extern const String chipId;
extern CheapStepper stepper;

void setupServer();

#endif // SERVER_H
