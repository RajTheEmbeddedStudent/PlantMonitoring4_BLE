#include "advMeasData.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEAdvertising.h>

static int startMin; 
static int startSec;
NimBLEServer* pServer;
NimBLEService* pService;
NimBLECharacteristic* pdataCharacteristic;
NimBLECharacteristic* pcommandCharacteristic;
NimBLEAdvertising* pAdvertising;

static bool clientconnected = false;
char buffer[100];
int Adv_period = 5U;

void startAdvData() {
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.print("**** Starting Advertisement of Data at -> ");
  startMin = rtc.getMinute();
  startSec = rtc.getSecond();
  Serial.print(startMin);
  Serial.print(":");
  Serial.print(startSec);
  Serial.println(" ****");
}

void stopAdvData() {
  // Stop advertising
  pServer->getAdvertising()->stop();
  Serial.print("**** Stopping Advertisement of Data at -> ");
  startMin = 0U;
  startSec = 0U;
  Serial.print(rtc.getMinute());
  Serial.print(":");
  Serial.print(rtc.getSecond());
  Serial.println(" ****");
}

// Callback for server events
class ConnectStatusCallbacks: public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override{
    clientconnected = true;
    Serial.print("     - Master connected @ ");
    Serial.println(millis());
    // Stop advertising when a client connects
    //pServer->getAdvertising()->stop();
    //pServer->getAdvertising()->start();
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    clientconnected = false;
    Serial.print("     - Master disconnected @ ");
    Serial.println(millis());
  }
}serverCallbacks;

void advMeasDataInit() {

  // Ensure NimBLE is initialized
  if (!NimBLEDevice::isInitialized()) {
    Serial.println("NimBLE not initialized. Call NimBLEDevice::init() first.");
    return;
  }

  // Create the BLE Server
  pServer = NimBLEDevice::createServer();

  pServer->setCallbacks(&serverCallbacks);

  // Create the BLE Service
  pService = pServer->createService(servicedUUID);

  // Create the BLE Characteristic
  pcommandCharacteristic = pService->createCharacteristic(
      commandcharacteristicUUID,
      NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::WRITE
  );
  pdataCharacteristic = pService->createCharacteristic(
      datacharacteristicUUID,
      NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::WRITE
  );

  pAdvertising = NimBLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(servicedUUID);

  NimBLEAdvertisementData scanResponseData;
  scanResponseData.setName("PlanMon_9999");
  //scanResponseData.setManufacturerData("PlanMonTeam");
  pAdvertising->setScanResponseData(scanResponseData);
    /**
     *  If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
  pAdvertising->enableScanResponse(false); //Required for Active scan

  uint32_t baseAdvertisingInterval = 320; // 200 ms
  uint32_t randomOffset = rand() % 32; // Random offset up to 20 ms (32 * 0.625)

  uint32_t advertisingInterval = baseAdvertisingInterval + randomOffset;
  pAdvertising->setMinInterval(advertisingInterval);
  pAdvertising->setMaxInterval(advertisingInterval);

  // Start the service
  pService->start();

}

bool advMeasDataRun() {
  bool stopAdv = false;
  if(clientconnected) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    String command = pcommandCharacteristic->getValue().c_str();
    Serial.print("     - Command is ");
    Serial.println(command);
    if(command == "SEND_DATA") {
      //Check if any pending data is there to send.----------------------------??????????
      readSensorData(buffer);
      //logSDCard(buffer);
      Serial.print("     - Logging done & Data going to be sent is:");
      Serial.print(buffer);
      pdataCharacteristic->setValue(buffer);
      pcommandCharacteristic->setValue("DATA_WRITTEN");
    }
    else if(command == "DATA_RECEIVED") {
      //stopAdvData(); ------Will be done once the return stopAdv sends true value
      pcommandCharacteristic->setValue("--");
      vTaskDelay(pdMS_TO_TICKS(1000)); //Let Master also check that the data is cleared.
      command = pcommandCharacteristic->getValue().c_str();
      Serial.print("     - Command after update is : ");
      Serial.println(command);
      stopAdv = true;
    }
    else {
      //Do nothing for now
      Serial.print(" ** ");
    }
  }
  else {
/*---------------------------------------------------PROBLEM: Implement logic for data to be written with a mark to be sent later if time period is over-------------------------------*/
    //Keep waiting until Master connects or data gets
    //Serial.println(".");
  }


  if(stopAdv) {
    Serial.println("     - Data sent successfully, Advertisement will be stopped now!");
    //Need not check for Adv_period as we have finished the data sending process
    return true;
  }
  else {
    if((5U + startMin)>=60) { //Adv time is around 55 to 00
      if((rtc.getMinute()>=0U) && (rtc.getMinute()<=54U)) {
        return (((5U + startMin) <= (rtc.getMinute()+60)) ? true : false);
        //Serial.print("Comparing data1: ");
        //Serial.print((5U + startMin));
        //Serial.print("<=");
        //Serial.println((rtc.getMinute()+60));
      }
      else { //Handle like rest of the cases (55 to 59)
        return (((5U + startMin) <= (rtc.getMinute())) ? true : false);
        //Serial.print("Comparing data2: ");
        //Serial.print((5U + startMin));
        //Serial.print("<=");
        //Serial.println(rtc.getMinute());
      }
    }
    else {
      return (((5U + startMin) <= (rtc.getMinute())) ? true : false);
        //Serial.print("Comparing data3: ");
        //Serial.print((5U + startMin));
        //Serial.print("<=");
        //Serial.println(rtc.getMinute());
    }
  }
}


bool advTimeCheck() {
  //Serial.print("Frequency value advTimeCheck is: ");
  //Serial.println(frequency);
  int Min = rtc.getMinute();
  if(frequency == 1U) {
    if(Min>=50U && Min<55U) {
        return true;
    }
    else {
        return false;
    }
  }
  else if(frequency == 2U) {
    if((Min>=00U && Min<05U) || (Min>=30U && Min<35U)) {
        return true;
    }
    else {
        return false;
    }
  }
  else if(frequency == 3U) {
    if((Min>=20U && Min<25U) || (Min>=40U && Min<45U) || (Min>=00U && Min<05U)) {
        return true;
    }
    else {
        return false;
    }
  }
  else if(frequency == 4U) {
    if((Min>=00U && Min<05U) || (Min>=15U && Min<20U) || (Min>=30U && Min<35U) || (Min>=45U && Min<50U)) {
        return true;
    }
    else {
        return false;
    }
  }
  else {
    //Default case of 1
    if(Min>=50U && Min<55U) {
        return true;
    }
    else {
        return false;
    }
  }
}

TickType_t checkTimegetDelay() {
  //Function to determine the sleep time until the next advertising time period.
  uint32_t xDelay = 0U;

  if(frequency == 1U) {
    if((50 - rtc.getMinute())<=0) { // current Time between 50 and 59 
      //xDelay = (60-(50 - rtc.getMinute())) * 60 * configTICK_RATE_HZ;
      xDelay = ((((59-(50 - rtc.getMinute())) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else { //current time between 01 and 49
      //xDelay = (50 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((49 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
  }
  else if(frequency == 2U) {
    if((rtc.getMinute()>=00U && rtc.getMinute()<=29U)) {
      //xDelay = (30 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((29 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else { //current time between 30 and 59
      //xDelay = (60 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((59 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
  }
  else if(frequency == 3U) {
    if((rtc.getMinute()>=00U && rtc.getMinute()<=19U)) {
      //xDelay = (20 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((19 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=20U && rtc.getMinute()<=39U)) {
      //xDelay = (40 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((39 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=40U && rtc.getMinute()<=59U)) {
      //xDelay = (60 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((59 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else {
      Serial.print("I dont know something is wrong, You shouldn't be here!");
      xDelay = 5000U;
    }
  }
  else if(frequency == 4U) {
    if((rtc.getMinute()>=00U && rtc.getMinute()<=14U)) {
      //xDelay = (15 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      //xDelay = ((Minutes left in seconds) + (Seconds left))*1000 - Data in Millseconds
      xDelay = ((((14 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=15U && rtc.getMinute()<=29U)) {
      //xDelay = (30 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((29 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=30U && rtc.getMinute()<=44U)) {
      //xDelay = (45 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((44 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else if((rtc.getMinute()>=45U && rtc.getMinute()<=59U)) {
      //xDelay = (60 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((59 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else {
      Serial.print("I dont know something is wrong, You shouldn't be here!");
      xDelay = 5000U;
    }
  }
  else {
   if((50 - rtc.getMinute())<=0) { // current Time between 50 and 59 
      //xDelay = (60-(50 - rtc.getMinute())) * 60 * configTICK_RATE_HZ;
      xDelay = ((((59-(50 - rtc.getMinute())) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
    else { //current time between 01 and 49
      //xDelay = (50 - rtc.getMinute()) * 60 * configTICK_RATE_HZ;
      xDelay = ((((49 - rtc.getMinute()) * 60) + (60 - rtc.getSecond()))* 1000);
      Serial.print(F("NOTE :- Next sensor data advertisement after: "));
      Serial.print(xDelay/1000);
      Serial.println(" seconds");
    }
  }
  return xDelay;
}