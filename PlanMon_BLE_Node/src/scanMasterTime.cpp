#include "scanMasterTime.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

static bool doConnect = false;
static bool connected = false;
static bool doScan = false;
static bool connStatus = false;
static NimBLERemoteCharacteristic *pcommandCharacteristic;
static NimBLERemoteCharacteristic *ptimedataCharacteristic;
static const NimBLEAdvertisedDevice *myDevice;
NimBLERemoteService *pRemoteService;
NimBLEClient *pClient;
NimBLEScan *pBLEScan;
static uint32_t scanTimeMs = 5000U; /** scan time in milliseconds, 0 = scan forever */



class clientConnCallback : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient *pclient) override{
    //connected = true;
    Serial.println("    - onConnect");
  }

  void onDisconnect(NimBLEClient *pclient, int reason) override{
    connected = false;
    //NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    Serial.println("    - onDisconnect");
  }
} clientconnCallbacks;

//Scan for BLE servers and find the first one that advertises the service we are looking for.
class AdvertisedDeviceCheckCallbacks : public NimBLEScanCallbacks {
  //Called for each advertising BLE server.
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override{
    //Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(NimBLEUUID(serviceUUID))) {
      Serial.println("Found our time advertising master!!");
      NimBLEDevice::getScan()->stop();
      myDevice = advertisedDevice;
      doConnect = true;
      doScan = true;
    }  // Found our server
  }  // onResult
}scanCallbacks;  // AdvertisedDeviceCheckCallbacks

bool connectToServer() {
  Serial.print("    - Attempting to connect to: ");
  Serial.println(myDevice->getName().c_str());

  pClient = NimBLEDevice::createClient();

  if (pClient == nullptr) {
    Serial.println("Failed to create BLE client");
    return false;
  }

  pClient->setClientCallbacks(&clientconnCallbacks, false);
  pClient->setConnectionParams(12, 12, 0, 150);
  pClient->setConnectTimeout(5000); // 5 seconds

  if (!pClient->connect(myDevice)) {
    Serial.println("Failed to connect to server. Error code: " + String(pClient->getLastError()));
    NimBLEDevice::deleteClient(pClient);
    Serial.println("    - Failed to connect, deleted client");
    return false;
  }

  //Serial.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

  pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    return false;
  }

  ptimedataCharacteristic = pRemoteService->getCharacteristic(timedatacharacteristicUUID);
  if (ptimedataCharacteristic == nullptr) {
    Serial.println("Failed to find our characteristic UUID");
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    return false;
  }

  if (ptimedataCharacteristic->canRead()) {
    Serial.println("    - All checks complete, now ready for data transfer.");
    connected = true;
    return true;
  } else {
    Serial.println("The characteristic value cannot be read.");
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    return false;
  }
}

void scanMasterTimeInit() {
  // Ensure NimBLE is initialized
  if (!NimBLEDevice::isInitialized()) {
    Serial.println("NimBLE not initialized. Call NimBLEDevice::init() first.");
    return;
  }
  pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setScanCallbacks(&scanCallbacks, false);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(100);
  pBLEScan->setActiveScan(true);
}

void checkTimeRxd() {
  bool timedataRxd = false;
  while (!timedataRxd) {
    if (doConnect) {
      if (connectToServer()) {
        String dataRxd = ptimedataCharacteristic->readValue().c_str();
        if (dataRxd == "") {
          Serial.println("    - No data received, retrying...");
          pBLEScan->clearResults();
          pClient->disconnect();
          NimBLEDevice::deleteClient(pClient);
          doConnect = false;
        } else {
          char databuffer[100];
          Serial.print("    - Time Data received: ");
          Serial.println(dataRxd);
          dataRxd.toCharArray(databuffer, 100);

          if (setLocalClientTime(databuffer)) {
            Serial.print("    - Date & time set successfully. Current time: ");
            Serial.println(rtc.getDateTime());
            NimBLEDevice::getScan()->stop();
            pBLEScan->clearResults();
            pClient->disconnect();
            NimBLEDevice::deleteClient(pClient);
            doConnect = false;
            timedataRxd = true;
          } else {
            Serial.println("    - Incorrect date & time format, retrying...");
            pBLEScan->clearResults();
            pClient->disconnect();
            NimBLEDevice::deleteClient(pClient);
            doConnect = false;
          }
        }
      } else {
        Serial.println("    - Connection failed, retrying...");
        pBLEScan->clearResults();
        doConnect = false;
      }
    } else {
      startScan();
      vTaskDelay(pdMS_TO_TICKS(3000)); // Non-blocking delay for 3 seconds
      //delay(3000); // Add a delay to limit scan frequency
    }
  }
}

void startScan() {
  pBLEScan->start(scanTimeMs, false, true);
}

void stopScan() {
  NimBLEDevice::getScan()->stop();
}
