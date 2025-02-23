#ifndef DATAALERTS_H
#define DATAALERTS_H

//External dependencies
#include "dataStorage.h"
#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <ESP_Mail_Client.h>

//Internal dependencies
#include "timeDate.h"
#include "initialDeviceSetup.h"

// Structure to hold sensor data
struct SensorData {
  int id;
  int day, month, hour, minute;
  float temperature, humidity, soilMoisture, lightIntensity;
};

// Function prototypes
void smtpCallback(SMTP_Status status);
void processLastDayData();
void getUniqueIDs(const char * path, String *deviceid);
SensorData parseLine(const String& line);
void processDaytimeDataForID(const char*, int lastDay, String& alertMessage);
void sendEmail(const String& message);
void findLastDate(int& lastDay, int& lastMonth);
bool isInvalidData(const SensorData& data, bool dataState[4]);
void extractTime(const String& timeString, int& year, int& month, int& day, int& hour, int& minute);


void dataAlertsStop();
void dataAlertsInit();
void dataAlertsRun();

#endif