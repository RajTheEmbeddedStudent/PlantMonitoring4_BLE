#ifndef ADVMEASDATA_H
#define ADVMEASDATA_H

//External libraries
#include <Arduino.h>
#include <Preferences.h>

//Dependency libraries
#include "DataMeasurement.h"
#include "dataStorage.h"

// The remote service we wish to connect to.
#define servicedUUID "4fafc201-1fb5-459e-8fcc-c5c917000002"
// The characteristic of the remote service we are interested in.
#define datacharacteristicUUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// The characteristic of the remote service we are interested in.
#define commandcharacteristicUUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"

extern Preferences prefs;
// Define the BLE service UUID
//#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c917000001"
//#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f2-ea07361b26a8"

//Function declarations
void advMeasDataInit();
bool advMeasDataRun();
void startAdvData();
void stopAdvData();
TickType_t checkTimegetDelay();
bool advTimeCheck();





#endif