#ifndef ADVERTISETIME_H
#define ADVERTISETIME_H

//External libraries
#include <Arduino.h>
//Dependency libraries
#include "timeDate.h"


// Define the BLE service and Characteristic UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c917000001"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f2-ea07361b2699"



//Function declarations
void advertiseTimeInit();
bool advertiseTimeRun();
void startAdvTime();
void stopAdvTime();
TickType_t checkTimegetDelay();





#endif