#ifndef scanMasterTime_H
#define scanMasterTime_H

//External libraries
#include <Arduino.h>
#include <NimBLEDevice.h>
//#include <NimBLERemoteService.h>
//#include <NimBLEClient.h>

//Dependency libraries
#include "dataStorage.h"
#include "timeDate.h"

// The remote service we wish to connect to.
static NimBLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c917000001");
// The characteristic of the remote service we are interested in.
static NimBLEUUID timedatacharacteristicUUID("beb5483e-36e1-4688-b7f2-ea07361b2699");



//Function declarations
void scanMasterTimeInit();
void checkTimeRxd();
void startScan();
void stopScan();
bool scanTimeCheck();





#endif