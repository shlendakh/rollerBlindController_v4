#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <CheapStepper.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <PubSubClient.h>

extern AsyncWebServer server;
extern CheapStepper stepper;
extern Ticker motorTicker;
extern PubSubClient client;

extern long eeprom1;
extern long eeprom2;
extern long eeprom3;
extern int motorPosition;
extern const int motorStep;
extern const int motorSpeed;
extern const String chipId;

extern volatile bool motorRunning;
extern volatile int targetPosition;

void setupServer();
void motorStepControl();

#endif // SERVER_H
