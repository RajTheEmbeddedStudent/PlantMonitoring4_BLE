#include "advertiseTime.h"
#include "timeDate.h"
#include <NimBLEDevice.h>
//#include <NimBLELog.h>
//#include <NimBLEServer.h>
//#include <NimBLEAdvertising.h>

static int startMin; 
static int startSec;
NimBLEServer* pServer;
NimBLEService* pService;
NimBLECharacteristic* ptimedateCharacteristic;
NimBLEAdvertising* pAdvertising;

bool clientconnected = false;
char buffer[25];
int Adv_period = 5U;

// Callback for server events
class ConnectStatusCallbacks: public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    clientconnected = true;
    Serial.println("-> Client connected to get time!");
    //pServer->getAdvertising()->start();
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override{
    clientconnected = false;
    pServer->getAdvertising()->start(0, nullptr);
    Serial.println("-> Client disconnected after getting time!");
  }
}serverCallbacks;

void startAdvTime() {
  // Start advertising
  pServer->getAdvertising()->start(0, nullptr);
  Serial.print("**** Starting Advertisement of Time at -> ");
  startMin = rtc.getMinute();
  startSec = rtc.getSecond();
  Serial.print(startMin);
  Serial.print(":");
  Serial.print(startSec);
  Serial.println(" ****");
}

void stopAdvTime() {
  // Stop advertising
  pServer->getAdvertising()->stop();
  Serial.print("**** Stopping Advertisement of Time at -> ");
  startMin = 0U;
  startSec = 0U;
  Serial.print(rtc.getMinute());
  Serial.print(":");
  Serial.print(rtc.getSecond());
  Serial.println(" ****");
}



void advertiseTimeInit() {
  // Create the BLE Server
  pServer = NimBLEDevice::createServer();

  pServer->setCallbacks(&serverCallbacks);

  // Create the BLE Service
  pService = pServer->createService(SERVICE_UUID);

  // Create the BLE Characteristic
  ptimedateCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::NOTIFY
  );

  pAdvertising = NimBLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(SERVICE_UUID); 

  //When a BLE peripheral advertises, it sends out a small packet of data called the advertising packet.

  //If the central device requests more information, the peripheral can send an additional packet called the scan response.

  //The scan response is typically used to provide extra details, such as the device name, service UUIDs, or other custom data, that couldn't fit in the initial advertising packet.
  // Set scan response data
  NimBLEAdvertisementData scanResponseData;
  scanResponseData.setName("PlanMon_Master");
  pAdvertising->setScanResponseData(scanResponseData);
    /**
     *  If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
  pAdvertising->enableScanResponse(false);

  // Start the service
  pService->start();

}

bool advertiseTimeRun() {
    // Get the current time
    uint8_t currentMinute = rtc.getMinute();
    uint8_t currentSecond = rtc.getSecond();

    // Calculate the end time of the advertising period
    uint8_t endMinute = startMin + Adv_period;
    uint8_t endSecond = startSec;

    // Handle rollover if endMinute exceeds 60
    if (endMinute >= 60) {
        endMinute -= 60;
    }

    // Determine if the current time is past the end time
    bool isAdvertisingPeriodOver;
    if (startMin <= endMinute) {
        // Case 1: No rollover (e.g., startMin = 10, startSec = 30, Adv_period = 15)
        isAdvertisingPeriodOver = (currentMinute > endMinute || 
                                  (currentMinute == endMinute && currentSecond >= endSecond));
    } else {
        // Case 2: Rollover (e.g., startMin = 55, startSec = 30, Adv_period = 10)
        isAdvertisingPeriodOver = ((currentMinute < startMin && currentMinute >= endMinute) || 
                                  (currentMinute == endMinute && currentSecond >= endSecond));
    }

    // Advertise the current time over BLE if the period is still active
    if (!isAdvertisingPeriodOver) {
        getTimeoftheday(buffer); // Get the current time as a string
        ptimedateCharacteristic->setValue(buffer); // Update the BLE characteristic
    }

    return isAdvertisingPeriodOver;
}



TickType_t checkTimegetDelay()
{
  // Function to check the current time and determine the next advertising time period.
  uint32_t xDelay = 0U;
  int CurrMin = rtc.getMinute();
  if (CurrMin >= 05U && CurrMin < 10U)
  {
    xDelay = (1) * configTICK_RATE_HZ;    // 1 sec -> To avoid milli second executions
    Adv_period = (10U - rtc.getMinute()); // Adjust the Adv_period to time left after startup time during Adv_time_range - 05U to 10U
    Serial.print("Next advertisement of time immediately ");
  }
  else if (CurrMin >= 35U && CurrMin < 40U)
  {
    xDelay = (1) * configTICK_RATE_HZ;    // 1 sec -> To avoid milli second executions
    Adv_period = (40U - rtc.getMinute()); // Adjust the Adv_period to time left after startup time during Adv_time_range - 35U to 40U
    Serial.print("Next advertisement of time immediately ");
  }
  else
  {
    if (CurrMin >= 10U && CurrMin <= 35U)
    { // Case when current time is between 10 and 35
      xDelay = ((((34 - rtc.getMinute()) * 60) + (60 - rtc.getSecond())) * 1000);
      Adv_period = 05U;
      Serial.print("Next advertisement of time after: ");
      Serial.print(xDelay / 1000);
      Serial.println(" seconds");
    }
    else if (CurrMin >= 40U && CurrMin <= 59U)
    { // Case when current time is between 40 and 59
      xDelay = ((((64 - rtc.getMinute()) * 60) + (60 - rtc.getSecond())) * 1000);
      Adv_period = 05U;
      Serial.print("Next advertisement of time after: ");
      Serial.print(xDelay / 1000);
      Serial.println(" seconds");
    }
    else
    { // Case when current time is between 00 and 05
      xDelay = ((((04 - rtc.getMinute()) * 60) + (60 - rtc.getSecond())) * 1000);
      Adv_period = 05U;
      Serial.print("Next advertisement of time after: ");
      Serial.print(xDelay / 1000);
      Serial.println(" seconds");
    }
  }
  return xDelay;
}