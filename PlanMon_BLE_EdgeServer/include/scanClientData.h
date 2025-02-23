#ifndef SCANCLIENTDATA_H
#define SCANCLIENTDATA_H

//External libraries
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLERemoteService.h>
#include <NimBLEClient.h>

//Dependency libraries
#include "dataStorage.h"
#include "timeDate.h"

#define SCAN_PERIOD     5U //Time to scan for Client data once started

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c917000002");
// The characteristic of the remote service we are interested in.
static BLEUUID datacharacteristicUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
// The characteristic of the remote service we are interested in.
static BLEUUID commandcharacteristicUUID("beb5483e-36e1-4688-b7f5-ea07361b26a9");




//Function declarations
void scanClientInit();
void checkDataRxd();
void startScan();
void stopScan();
bool scanTimeCheck();
TickType_t calculateDelay();





#endif