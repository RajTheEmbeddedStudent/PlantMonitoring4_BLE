#include "timeDate.h"
#include "initialDeviceSetup.h"
#include <WiFiClient.h>

const char* ssidchar = g_userData.wifiSSID;
const char* passwordchar = g_userData.wifiPassword;

const unsigned long wifiTimeout = 10000; // Timeout after 10 seconds
const int maxRetries = 3;              // Maximum number of connection attempts

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600; /*---------------------------------------------------PROBLEM: HOW TO HANDLE DAYLIGHT SAVING TIME OFFSET ?----------------------------------------------------------*/

ESP32Time rtc(0);

void timeDateInit() {
  int retryCount = 0;

  while (retryCount < maxRetries) {
    Serial.println("Connecting to Wi-Fi...");

    unsigned long startTime = millis(); // Record the start time

    WiFi.begin(ssidchar, passwordchar); // Start the Wi-Fi connection

    // Wait for the connection with a timeout
    while (WiFi.status() != WL_CONNECTED) {
      setRGBColor(255,0,0);
      vTaskDelay(pdMS_TO_TICKS(500));
      Serial.print(".");

      // Check if the timeout has been reached
      if (millis() - startTime > wifiTimeout) {
        Serial.println("\nWi-Fi connection timed out.");
        break;
      }
    }

    // Check if the connection was successful
    if (WiFi.status() == WL_CONNECTED) {
      setRGBColor(0,0,0);
      Serial.println("\nWi-Fi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      getLocalTime();
      return; // Exit the function on successful connection
    } else {
      retryCount++;
      Serial.print("Retry attempt: ");
      Serial.println(retryCount);
    }
  }
  
  setRGBColor(255,0,0);
  // If all retries fail, reset the ESP32
  Serial.println("Failed to connect to Wi-Fi after maximum retries. Resetting ESP32...");
  vTaskDelay(pdMS_TO_TICKS(1000)); // Small delay before resetting
  ESP.restart(); // Reset the ESP32

  // Disconnect from the Wi-Fi network
  //WiFi.disconnect(true); 

  // Set the Wi-Fi mode to OFF
  //WiFi.mode(WIFI_OFF);

} //end of function

void getLocalTime() {
  struct tm timeinfo;
  time_t now;
  bool timeupdated = false;
  Serial.println("Attempting to extract local time");

  int retryCount = 0;

  while (retryCount < maxRetries) {
    Serial.println("Connecting to NTP server...");

    unsigned long startTime = millis(); // Record the start time

    // Wait for the connection with a timeout
    while (!getLocalTime(&timeinfo)) {
      vTaskDelay(pdMS_TO_TICKS(500));
      Serial.print(".");
      // Check if the timeout has been reached
      if (millis() - startTime > wifiTimeout) {
        Serial.println("\nTime Server connection timed out.");
        break;
      }
    }

    // Check if the connection was successful
    if(getLocalTime(&timeinfo)) {
      rtc.setTimeStruct(timeinfo);
      return; // Exit the function on successful connection
    } else {
      timeupdated = false;
      Serial.print(".");
      retryCount++;
      Serial.print("Retry attempt: ");
      Serial.println(retryCount);
    }
  }
  // If all retries fail, reset the ESP32
  Serial.println("Failed to connect to Time Server after maximum retries. Resetting ESP32...");
  vTaskDelay(pdMS_TO_TICKS(1000));// Small delay before resetting
  ESP.restart(); // Reset the ESP32
} //end of function



void getTimeoftheday(char *buffer) {
  time_t rawtime;
  struct tm * timeinfo;
  char freq_str[1]; // Adjust size as needed
    
  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,25,"%F,%T,",timeinfo); //2001-01-11,14:02:54,
  sprintf(freq_str, "%d,", (String(g_userData.freq).toInt()));
  //Serial.print("freq_str to be sent is: ");
  //Serial.print(freq_str);
  strcat(buffer, freq_str);
  //Serial.print("Time to be sent is: ");
  //Serial.print(buffer);

} //end of function

// Function to Set RGB LED Color (for Common Cathode)
void setRGBColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);   // Direct PWM value for common cathode
  analogWrite(GREEN_PIN, green); // Direct PWM value for common cathode
  analogWrite(BLUE_PIN, blue);  // Direct PWM value for common cathode
}