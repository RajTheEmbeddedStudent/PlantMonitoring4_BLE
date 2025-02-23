#include "scanClientData.h"
#include "initialDeviceSetup.h"


static bool doConnect = false;
static bool connected = false;
static bool doScan = false;
static bool connStatus = false;
int frequency = 4U;
static NimBLERemoteCharacteristic *pcommandCharacteristic;
static NimBLERemoteCharacteristic *pdataCharacteristic;
static const NimBLEAdvertisedDevice *myDevice;
NimBLERemoteService *pRemoteService;
NimBLEClient *pClient;
NimBLEScan *pBLEScan;
static uint32_t scanTimeMs = 5000U; /** scan time in milliseconds, 0 = scan forever */

/*
void reinitializeNimBLE() {
  // Deinitialize the NimBLE stack
  NimBLEDevice::deinit(true);  // Pass `true` to release all resources

  // Reinitialize the NimBLE stack with the given device name
  NimBLEDevice::init("PlanMon_Master");
  // Optional: Set the MTU size or other BLE parameters if needed
  // NimBLEDevice::setMTU(517); // Example: Set MTU size to 517 bytes

  Serial.println("NimBLE stack reinitialized.");
}
*/

class clientConnCallback : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient *pclient) override{
    //connected = true;
    Serial.print("    - Connection established @ ");
    Serial.println(millis());
  }

  void onDisconnect(NimBLEClient *pclient, int reason) override{
    connected = false;
    Serial.print("    - Connection disconnected @ ");
    Serial.println(millis());
  }
}clientconnCallbacks;

//Scan for BLE servers and find the first one that advertises the service we are looking for.
class AdvertisedDeviceCheckCallbacks : public NimBLEScanCallbacks {
  //Called for each advertising BLE server.
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override{
    //Serial.print("Found an advertising device with name:");
    //Serial.println(advertisedDevice.getName().c_str());
    //Serial.print("with UUID");
    //Serial.println(advertisedDevice.getServiceUUID().toString().c_str());
    //Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID)) {
      Serial.println("**Found a Client!!");
      Serial.println("  Phase1: Connection progress details - ");
      Serial.printf("    - Client Device data: %s \n", advertisedDevice->toString().c_str());
      NimBLEDevice::getScan()->stop();
      //Serial.println("  Advertising stopped!! ");
      myDevice = advertisedDevice;
      doConnect = true;
      //doScan = true;
    }  // Found our server
    //Serial.println("Exiting onResult!! ");
  }  // onResult
}scanCallbacks;  // AdvertisedDeviceCheckCallbacks


bool connectToServer()
{
  //Serial.print("    - Attempting to connect to: ");
  //Serial.println(myDevice->getName().c_str());

  //Serial.print("    - Attempting to connect to: " + String(myDevice->getName().c_str()));
  Serial.print("    - Attempting to connect @ ");
  Serial.println(millis());
  pClient = NimBLEDevice::createClient();

  if (pClient == nullptr) {
    Serial.println("    - Failed to create BLE client");
    return false;
  }

  pClient->setClientCallbacks(&clientconnCallbacks, false);
  //Try-1//pClient->setConnectionParams(24, 24, 2, 150); // Connection interval = 30 ms, latency = 2, timeout = 1500 ms
  //Try-2//pClient->setConnectTimeout(5000);             // 5 seconds
  pClient->setConnectionParams(48, 48, 2, 300); // Adjusted connection parameters
  pClient->setConnectTimeout(10000);            // Increased connection timeout to 10 seconds

  int retryCount = 3;
  while (retryCount > 0) {
    if (pClient->connect(myDevice)) {
        break; // Connection successful
    }
    Serial.println("    - Retrying connection...");
    retryCount--;
    vTaskDelay(pdMS_TO_TICKS(500)); // Wait 2 seconds before retrying
  }
  if (retryCount == 0) {
    Serial.println("    - Failed to connect to server. Error code: " + String(pClient->getLastError()));
    NimBLEDevice::deleteClient(pClient);
    return false;
  }

  //pClient->getLastError()
  //Error code: 000 ->
  //Error code: 003 ->
  //Error code: 013 ->
  //Error code: 574 -> 

  pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("    - Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    return false;
  }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pdataCharacteristic = pRemoteService->getCharacteristic(datacharacteristicUUID);
    pcommandCharacteristic = pRemoteService->getCharacteristic(commandcharacteristicUUID);

  if (pdataCharacteristic == nullptr || pcommandCharacteristic == nullptr) {
    Serial.println("    - Failed to find our characteristic UUID");
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    return false;
  }

  // Try to read the value of the characteristic.
  if (pdataCharacteristic->canRead() && pcommandCharacteristic->canRead() &&
      pdataCharacteristic->canWrite() && pcommandCharacteristic->canWrite()) {
    Serial.print("    - Ready for data transfer @ ");
    Serial.println(millis());
    connected = true;
    return true;
  } else {
    Serial.println("    - The characteristic value cannot be read/written.");
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    return false;
  }
}

void scanClientInit()
{
  // Ensure NimBLE is initialized
  if (!NimBLEDevice::isInitialized()) {
    Serial.println("NimBLE not initialized. Call NimBLEDevice::init() first.");
    return;
  }
  pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setScanCallbacks(&scanCallbacks, false);
  pBLEScan->setInterval(200);  // Scan interval = 200 * 0.625 = 125 ms
  pBLEScan->setWindow(150);    // Scan window = 150 * 0.625 = 93.75 ms  
  pBLEScan->setActiveScan(true);
}

// Function to process data from a device
void checkDataRxd() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.print("    - Sending SEND_DATA @ ");
      Serial.println(millis());
      pcommandCharacteristic->writeValue("SEND_DATA");
      vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for client to process the command //500 (NOT OK) -> 1500 -> 2000 

      String commandRxd = pcommandCharacteristic->readValue().c_str();
      if (commandRxd == "DATA_WRITTEN") {
        String dataRxd = pdataCharacteristic->readValue().c_str();
        Serial.print("    - Data received is: ");
        Serial.print(dataRxd);

        // Log data to SD card
        // Copy the String to a char buffer and log it
        char buffer[100]; // Adjust the size as needed
        dataRxd.toCharArray(buffer, sizeof(buffer));
        logSDCard(buffer);

        Serial.println("    - Sending DATA_RECEIVED");
        pcommandCharacteristic->writeValue("DATA_RECEIVED");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for client to process the 
        
        commandRxd = pcommandCharacteristic->readValue().c_str();
        Serial.print("    - Client has updated the data to: ");
        Serial.println(commandRxd);

        // Clear the characteristic value before disconnecting
        pcommandCharacteristic->writeValue(""); // Clear the value //||
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for client to process the ACK
      }
      else {
        Serial.println("    - Client did not respond with DATA_WRITTEN.");
      }

      pClient->disconnect();
      NimBLEDevice::deleteClient(pClient);
      connected = false;
      doConnect = false;
    } else {
      Serial.println("    - Couldn't connect, will try again.");
      pBLEScan->clearResults();
      doConnect = false;
    }
  }
}


void startScan() {
  pBLEScan->start(scanTimeMs, false, true);
}

void stopScan() {
  NimBLEDevice::getScan()->stop();
  Serial.print("**** Master stopped scannning of Data at -> ");
  Serial.print(rtc.getMinute());
  Serial.print(":");
  Serial.print(rtc.getSecond());
  Serial.println(" ****");
}

bool scanTimeCheck()
{
  int Min = rtc.getMinute(); // Get the current minute
  frequency = (String(g_userData.freq)).toInt();
  // Define scanning windows based on frequency
  switch (frequency)
  {
    case FREQ_1: // Frequency = 1 (50-55 minutes)
      return (Min >= MIN_50 && Min < MIN_55);

    case FREQ_2: // Frequency = 2 (00-05 and 30-35 minutes)
      return (Min >= MIN_00 && Min < MIN_05) || (Min >= MIN_30 && Min < MIN_35);

    case FREQ_3: // Frequency = 3 (00-05, 20-25, and 40-45 minutes)
      return (Min >= MIN_00 && Min < MIN_05) || (Min >= MIN_20 && Min < MIN_25) || (Min >= MIN_40 && Min < MIN_45);

    case FREQ_4: // Frequency = 4 (00-05, 15-20, 30-35, and 45-50 minutes)
      return (Min >= MIN_00 && Min < MIN_05) || (Min >= MIN_15 && Min < MIN_20) ||
             (Min >= MIN_30 && Min < MIN_35) || (Min >= MIN_45 && Min < MIN_50);

    default: // Default case (Frequency = 1) (50-55 minutes)
      return (Min >= MIN_50 && Min < MIN_55);
  }
}

TickType_t calculateDelay() {
  //Function to determine the sleep time until the next advertising time period.
  uint32_t xDelay = MIN_00;

  if(frequency == FREQ_1) {
    if((MIN_50 - rtc.getMinute())<=MIN_00) { // current Time between 50 and 59 
      //xDelay = (60-(50 - rtc.getMinute())) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_59-(MIN_50 - rtc.getMinute())) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else { //current time between 01 and 49
      //xDelay = (50 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_49 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
  }
  else if(frequency == FREQ_2) {
    if((rtc.getMinute()>=MIN_00 && rtc.getMinute()<=MIN_29)) {
      //xDelay = (30 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_29 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else { //current time between 35 and 59
      //xDelay = (60 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_59 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
  }
  else if(frequency == FREQ_3) {
    if((rtc.getMinute()>=MIN_00 && rtc.getMinute()<=MIN_19)) {
      //xDelay = (20 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_19 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=MIN_20 && rtc.getMinute()<=MIN_39)) {
      //xDelay = (40 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_39 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=MIN_40 && rtc.getMinute()<=MIN_59)) {
      //xDelay = (60 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_59 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else {
      Serial.print("Scan_Client: I dont know something is wrong, You shouldn't be here!");
      xDelay = 5000U;
    }
  }
  else if(frequency == FREQ_4) {
    if((rtc.getMinute()>=MIN_00 && rtc.getMinute()<=MIN_14)) {
      //xDelay = (15 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_14 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=MIN_15 && rtc.getMinute()<=MIN_29)) {
      //xDelay = (30 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_29 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=MIN_30 && rtc.getMinute()<=MIN_44)) {
      //xDelay = (45 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_44 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=MIN_45 && rtc.getMinute()<=MIN_59)) {
      //xDelay = (60 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_59 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else {
      Serial.print("Scan_Client: I dont know something is wrong, You shouldn't be here!");
      xDelay = 5000U;
    }
  }
  else {
   if((MIN_50 - rtc.getMinute())<=MIN_00) { // current Time between 50 and 59 
      //xDelay = (60-(50 - rtc.getMinute())) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_59-(MIN_50 - rtc.getMinute())) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else { //current time between 01 and 49
      //xDelay = (50 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((MIN_49 - rtc.getMinute()) * 60) + (SEC_60 - rtc.getSecond()))* 1000);
      Serial.print("Next client scanning for data after: ");
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
  }
  return xDelay;
}