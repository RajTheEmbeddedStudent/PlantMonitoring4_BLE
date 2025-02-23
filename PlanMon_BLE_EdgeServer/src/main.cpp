
#include "main.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_task_wdt.h>



TaskHandle_t scanClientDataTaskHandle;
TaskHandle_t advertisingTimeTaskHandle;
TaskHandle_t webserverTaskHandle;
TaskHandle_t dataAlertsTaskHandle;
SemaphoreHandle_t xButtonSemaphore;
// Semaphore to synchronize Wi-Fi access
SemaphoreHandle_t wifiSemaphore;
// Mutex for BLE stack access
SemaphoreHandle_t bleMutex;
UBaseType_t uxSavedInterruptStatus;
volatile bool isButtonPressRequested = false;
bool firstFiveMinutes = true; // Flag to indicate if the first 5 minutes have passed
unsigned long startTime; // Stores the time when the ESP32 was powered ON

// Function to connect to Wi-Fi
bool connectToWiFi()
{
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < MAX_RETRIES)
  {
    Serial.println(" * * * Connecting to WiFi...");
    WiFi.begin(String(g_userData.wifiSSID), String(g_userData.wifiPassword));

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < CONNECT_TIMEOUT)
    {
      setRGBColor(255, 0, 0);
      vTaskDelay(pdMS_TO_TICKS(1000)); // Yield control every 1 second
      attempt++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      setRGBColor(0, 0, 0);
      Serial.println(" * * * Connected to WiFi!");
      return true;
    }

    retries++;
    Serial.println(" * * * Failed to connect. Retrying...");
    vTaskDelay(pdMS_TO_TICKS(BACKOFF_DELAY * retries)); // Exponential backoff
  }
  
  setRGBColor(255, 0, 0);
  Serial.println(" * * * Failed to connect to WiFi.");
  return false;
}

void disconnectFromWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    // Do nothing
  }
  else
  {
    // Disconnect from the Wi-Fi network
    WiFi.disconnect(true);
    // Set the Wi-Fi mode to OFF
    WiFi.mode(WIFI_OFF);

    Serial.println(F("Disconnected from WiFi!"));
  }
}

void webServerTask(void *pvParameters)
{
  dataVisualisationInit();
  while (1)
  {
    // Reset the watchdog timer to prevent timeout
    esp_task_wdt_reset();

    if (xSemaphoreTake(wifiSemaphore, portMAX_DELAY) == pdTRUE)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        connectToWiFi();
      }
      // Release Wi-Fi semaphore
      xSemaphoreGive(wifiSemaphore);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void dataAlertsTask(void *pvParameters)
{
  bool runOnce = false;
  TickType_t xDelay;
  while (1)
  {
    // Get the current time
    int currentHour = rtc.getHour(true); // 24-hour format
    int currentMinute = rtc.getMinute();

    // Check if it's within the desired time window (21:10 to 21:15)
    if (currentHour == 10 && currentMinute >= 20 && currentMinute < 25 && !runOnce)
    {
      // Wait for Wi-Fi semaphore
      if (xSemaphoreTake(wifiSemaphore, portMAX_DELAY) == pdTRUE)
      {
        // Connect to Wi-Fi if not already connected
        if (WiFi.status() != WL_CONNECTED)
        {
          setRGBColor(255, 0, 0);
          connectToWiFi();
        }
        // Run the data alerts task
        dataAlertsInit();
        dataAlertsRun();
        // Set the flag to indicate the task has run
        runOnce = true;
        // Release the Wi-Fi semaphore
        xSemaphoreGive(wifiSemaphore);
      }
    }
    else
    {
      // Calculate the delay until the next execution window (21:10 the next day)
      if (currentHour > 10 || (currentHour == 10 && currentMinute >= 20))
      {
        // If the current time is past 21:15, schedule for the next day
        xDelay = ((24 - currentHour + 10) * 60 * 60 + (20 - currentMinute) * 60) * 1000 / portTICK_PERIOD_MS;
      }
      else
      {
        // If the current time is before 21:10, schedule for the same day
        xDelay = ((10 - currentHour) * 60 * 60 + (20 - currentMinute) * 60) * 1000 / portTICK_PERIOD_MS;
      }

      Serial.print(F("Next Sensor data alert to customer after: "));
      Serial.print(xDelay / 1000);
      Serial.println(F(" seconds"));

      // Break the delay into smaller chunks to avoid blocking the CPU
      TickType_t startTime = xTaskGetTickCount();
      while ((xTaskGetTickCount() - startTime) < xDelay)
      {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
      }

      // Reset the flag for the next execution
      runOnce = false;
    }
  }
}

void IRAM_ATTR button_isr() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (!isButtonPressRequested) {
    isButtonPressRequested = true;
    xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
  }
  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

void advertisingTimeTask(void *parameter)
{
  bool AdvtimeOver = false;
  bool runOnce = false;
  TickType_t xDelay;
  frequency = (String(g_userData.freq)).toInt();
  while (1)
  {
    if (xSemaphoreTake(bleMutex, portMAX_DELAY) == pdTRUE)
    {
      if (!runOnce)
      {
        startAdvTime(); // Start advertising
        runOnce = true; // Set the flag to indicate advertising has started
      }
      // Check if advertising time is over
      AdvtimeOver = advertiseTimeRun();
      if (AdvtimeOver)
      {
        stopAdvTime();                  // Stop advertising
        xDelay = checkTimegetDelay();   // Calculate delay until next advertising cycle
        xSemaphoreGive(bleMutex);       // Release the mutex before delaying
        vTaskDelay(pdMS_TO_TICKS(200)); // Delay to avoid watchdog trigger
        firstFiveMinutes = false;
        vTaskDelay(xDelay); // Delay until the next advertising cycle
        // Reset flags for the next cycle
        AdvtimeOver = false;
        runOnce = false;
      }
      else
      {
        // Continue advertising
        xSemaphoreGive(bleMutex); // Release the mutex if advertising is still ongoing
      }
    }
    // Delay for 1 second (1000 milliseconds)
    // Reset the watchdog timer
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay to avoid watchdog trigger
  } // End of whie loop
} // End of Task function

void scanClientDataTask(void *parameter)
{
  bool runOnce = false;
  TickType_t xDelay;
  frequency = (String(g_userData.freq)).toInt();
  while (1)
  {
    // Check if the first 5 minutes have passed
    if (!firstFiveMinutes)
    {
      
      if (scanTimeCheck())
      {
        // Acquire the mutex only when scanning is needed
        if (xSemaphoreTake(bleMutex, portMAX_DELAY) == pdTRUE) // Consider using a timeout for xSemaphoreTake() instead of portMAX_DELAY to avoid potential deadlocks.
        {
          //Serial.print("Received bleMutex");
          startScan();
          checkDataRxd();
          runOnce = true;
          xSemaphoreGive(bleMutex);
        }
      }
      else
      {
        if (runOnce)
        { // To skip stopScan being executed in Init phase.
          // Acquire the mutex only when stopping scanning
          if (xSemaphoreTake(bleMutex, portMAX_DELAY) == pdTRUE) // Consider using a timeout for xSemaphoreTake() instead of portMAX_DELAY to avoid potential deadlocks.
          {
            stopScan();
            xSemaphoreGive(bleMutex);
          }
        }
        xDelay = calculateDelay();
        vTaskDelay(pdMS_TO_TICKS(xDelay));
        Serial.print("**** Master starting scannning of Data at -> ");
        Serial.print(rtc.getMinute());
        Serial.print(":");
        Serial.print(rtc.getSecond());
        Serial.println(" ****");
      }
    }
    // Reset the watchdog timer
    esp_task_wdt_reset();
    // Delay for 1 second (1000 milliseconds)
    vTaskDelay(pdMS_TO_TICKS(500)); // Delay to avoid watchdog trigger //1000
  }
}

void setup()
{
  Serial.begin(115200);

  // Configure button pin as input with pull-up resistor
  pinMode(BUTTON_PIN, INPUT);
  // Initialize RGB LED
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Turn off RGB LED initially
  setRGBColor(0, 0, 0); // All colors off

  // Variables
  bool buttonState = false;

  // Check if Button is Pressed
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {
    // Button Pressed: Initialize BLE
    Serial.println("Request received to update Configuration data!");
    setRGBColor(0, 0, 255);
    prefs.begin("masterdata");
    userSetupInit();
    setRGBColor(0, 255, 0);
    delay(5000U);
    setRGBColor(0, 0, 0);
    } 
  else {
    // Button Not Pressed: Skip BLE Initialization
    Serial.println("Button not pressed. Skipping update Configuration data.");
  }

  // Create button semaphore
  xButtonSemaphore = xSemaphoreCreateBinary();
  // Create a semaphore for Wi-Fi access
  wifiSemaphore = xSemaphoreCreateMutex();
  // Create a mutex for BLE stack access
  bleMutex = xSemaphoreCreateMutex();
  
  checkNVMdata();
  // Connect to WiFi to get date & time and set RTC.
  dataStorageInit();
  // initLittleFS();
  Serial.print("Waiting to get connected to: ");
  Serial.println(String(g_userData.wifiSSID));
  timeDateInit();

  // Create the BLE Device
  NimBLEDevice::init("PlanMon_Master");
  advertiseTimeInit(); // Initialize advertising
  scanClientInit();    // Initialize scanning

  // Create the BLE advertising task
  xTaskCreatePinnedToCore(
      advertisingTimeTask,        // Function name of the task
      "advertisingTime",          // Name of the task (for debugging)
      2048,                       // Stack size(bytes) - Observed -> (4096-2148)
      NULL,                       // Parameter to pass
      2,                          // Task Priority
      &advertisingTimeTaskHandle, // Task handle
      0                           // Core 0 handles BLE tasks better
  );

  // Create the BLE advertising task
  xTaskCreatePinnedToCore(
      scanClientDataTask,        // Function name of the task
      "scanClientData",          // Name of the task (for debugging)
      3072,                      // Stack size(bytes) - Observed -> (5092-2748)
      NULL,                      // Parameter to pass
      5,                         // Task Priority
      &scanClientDataTaskHandle, // Task handle
      0                          // Core 0 handles BLE tasks better
  );
/*
  xTaskCreatePinnedToCore(
      buttonTask,           //Function name of the task
      "ButtonTask",         //Name of the task (for debugging)
      3072,                 //Stack size(bytes) - Observed -> () ; 1024 not working
      NULL,                 //Parameter to pass
      5,                    //Task Priority
      NULL,                 //Task handle
      0                     //Core 0 or Core 1
  );
*/
  if(!(String(g_userData.emailid) == "") || !(String(g_userData.emailid) == "STOP")) {
    xTaskCreatePinnedToCore(
      dataAlertsTask,        // Function name of the task
      "dataAlertsTask",      // Name of the task (for debugging)
      5632,                  // Stack size(bytes) - Observed -> ()
      NULL,                  // Parameter to pass
      4,                     // Task Priority
      &dataAlertsTaskHandle, // Task handle
      1                      // Core 1 recommended for WiFi
    );
  }

  xTaskCreatePinnedToCore(
      webServerTask,        // Function name of the task
      "webServerTask",      // Name of the task (for debugging)
      5000,                 // Stack size(bytes) - Observed -> (10240 - 8344)
      NULL,                 // Parameter to pass
      2,                    // Task Priority
      &webserverTaskHandle, // Task handle
      1                     // Core 1 recommended for WiFi
  );

}

void loop() {

}