
#include "main.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void advertisingDataTask(void *parameter) {
  bool Advstop = false;
  bool runOnce = false;
  bool advertisingStarted = false; // New flag to track if advertising has started
  TickType_t xDelay;
  //checkTimegetDelay();
  while(1) {
    if(advTimeCheck() && !runOnce) {
      startAdvData();
      runOnce=true;
      advertisingStarted = true; // Set flag to indicate advertising has started
    }

    // Handle data transmission only if advertising has started
    if (advertisingStarted) {
      Advstop = advMeasDataRun();
    }

    if(Advstop && runOnce) {
      stopAdvData();
      //Calculate the delay time until next periodic time for Advertisement
      xDelay = checkTimegetDelay();
      vTaskDelay(pdMS_TO_TICKS(xDelay));
      Advstop = false;
      runOnce = false;
      advertisingStarted = false;
    }
    else {
      //Continue advertising
    }
    // Delay for 1 second (1000 milliseconds)
    // 250 => 250ms
    vTaskDelay(pdMS_TO_TICKS(500)); //Delay to avoid watchdog trigger

  } //End of whie loop
} //End of Task function



void setup() {
  Serial.begin(115200);

  //dataStorageInit();
  Serial.println("Waiting for Master to deliver Time data...");
 // Create the BLE Device
  NimBLEDevice::init("PlanMon_9999");
  advMeasDataInit(); //Due to some reason (unknown) - Calling advMeasDataInit(); (i.e createServer()) after the scanMasterTimeInit() crashes the program with an error.
  scanMasterTimeInit();
  startScan();
  checkTimeRxd();
  dataMeasureInit();

  // Create the BLE advertising task
  xTaskCreatePinnedToCore(
      advertisingDataTask, //Function name of the task
      "advertisingData",   //Name of the task (for debugging)
      3072,               //Stack size(bytes) - Observed -> (4096-2148)
      NULL,               //Parameter to pass
      2,                  //Task Priority
      NULL,               //Task handle
      0                   //Core 0 or Core 1
  );
}

void loop() {

}