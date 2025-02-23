#include "initialDeviceSetup.h"
#include "BluetoothSerial.h"

String device_name = "ESP32-BT-Master";

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

userData g_userData; // Declare global instance
thresholdData g_thresholdData;
Preferences prefs;

// Global variables
bool UserdataAvailable = false;
String receivedData = "";

// Initialize Bluetooth and wait for user data
void userSetupInit() {
    SerialBT.begin("PlanMon_MasterModule"); // Bluetooth device name
    Serial.println(F("The device started, now you can pair it with Bluetooth!"));

    while (!UserdataAvailable) {
        ReadUserConfigData();
    }

    //Serial.println(F("User data received successfully!"));
    //Serial.println("Received Data: " + receivedData);
    Serial.print("\n");
    // Process and store the user data
    processData(receivedData);

    thresholdValuesInit();
}

// Read data from Bluetooth
void ReadUserConfigData() {
    while (SerialBT.available()) {
        char incomingChar = SerialBT.read(); // Read one character
        Serial.write(incomingChar);         // Echo the character to Serial Monitor
        if (incomingChar == '$') {         // Check for end of message
            UserdataAvailable = true;       // Mark data as complete
            SerialBT.disconnect();          // Stop Bluetooth communication
            vTaskDelay(pdMS_TO_TICKS(200));
            SerialBT.end();                 // Disable Bluetooth
            vTaskDelay(pdMS_TO_TICKS(2000));
            break;
        }
        receivedData += incomingChar;       // Append character to buffer
    }
}

// Process received data and extract user information
void processData(String data) {
    // Print the raw data received for debugging
    Serial.println(F("Raw Data Received:"));
    Serial.println(data);

    // Split the string using "*,*" as a delimiter
    int delimiterLength = 3; // Length of "*,*"
    int startIndex = 0;
    int endIndex = data.indexOf("*,*");

    // Array to store parsed values
    String parsedData[13]; // 13 fields in the data format
    int fieldIndex = 0;

    while (endIndex != -1 && fieldIndex < 13) {
        parsedData[fieldIndex] = data.substring(startIndex, endIndex);
        startIndex = endIndex + delimiterLength;
        endIndex = data.indexOf("*,*", startIndex);
        fieldIndex++;
    }

    // Extract the last field (before the '$' character)
    if (fieldIndex < 13) {
        parsedData[fieldIndex] = data.substring(startIndex, data.indexOf('$'));
    }

    // Debugging: Print parsed values
    Serial.println(F("Data entered & received from customer: "));
    Serial.println("Frequency: " + parsedData[0]);
    Serial.println("Location: " + parsedData[1]);
    Serial.println("WIFI-SSID: " + parsedData[2]);
    Serial.println("PASSWORD: " + parsedData[3]);
    Serial.println("Email-ID: " + parsedData[4]);
    Serial.println("SoilMoistThreshMin: " + parsedData[5]);
    Serial.println("SoilMoistThreshMax: " + parsedData[6]);
    Serial.println("TempThreshMin: " + parsedData[7]);
    Serial.println("TempThreshMax: " + parsedData[8]);
    Serial.println("HumThreshMin: " + parsedData[9]);
    Serial.println("HumThreshMax: " + parsedData[10]);
    Serial.println("LightThreshMin: " + parsedData[11]);
    Serial.println("LightThreshMax: " + parsedData[12]);

    // Save parsed data into the userData structure
    saveUserData(parsedData[0], parsedData[1], parsedData[2], parsedData[3], parsedData[4],
                 parsedData[5], parsedData[6], parsedData[7], parsedData[8], parsedData[9],
                 parsedData[10], parsedData[11], parsedData[12]);
}

// Save data into the global userData structure
void saveUserData(const String& freq, const String& location, const String& ssid, const String& password, const String& emailid,
                  const String& soilMoistMin, const String& soilMoistMax, const String& tempMin, const String& tempMax,
                  const String& humMin, const String& humMax, const String& lightMin, const String& lightMax) {

    // Convert String to char array and store in g_userData structure
    freq.toCharArray(g_userData.freq, sizeof(g_userData.freq));
    location.toCharArray(g_userData.locData, sizeof(g_userData.locData));
    ssid.toCharArray(g_userData.wifiSSID, sizeof(g_userData.wifiSSID));
    password.toCharArray(g_userData.wifiPassword, sizeof(g_userData.wifiPassword));
    emailid.toCharArray(g_userData.emailid, sizeof(g_userData.emailid));
    soilMoistMin.toCharArray(g_userData.soilMoistMin, sizeof(g_userData.soilMoistMin));
    soilMoistMax.toCharArray(g_userData.soilMoistMax, sizeof(g_userData.soilMoistMax));
    tempMin.toCharArray(g_userData.tempMin, sizeof(g_userData.tempMin));
    tempMax.toCharArray(g_userData.tempMax, sizeof(g_userData.tempMax));
    humMin.toCharArray(g_userData.humMin, sizeof(g_userData.humMin));
    humMax.toCharArray(g_userData.humMax, sizeof(g_userData.humMax));
    lightMin.toCharArray(g_userData.lightMin, sizeof(g_userData.lightMin));
    lightMax.toCharArray(g_userData.lightMax, sizeof(g_userData.lightMax));

    // Store data in non-volatile memory
    if (ssid != "") {
        Serial.println("Updating last ssid data!");
        prefs.putString("ssid", ssid);
    } else {
        Serial.println("Holding on to last ssid data!");
    }

    if (password != "") {
        Serial.println("Updating last password data!");
        prefs.putString("password", password);
    } else {
        Serial.println("Holding on to last password data!");
    }

    if (freq != "") {
        Serial.println("Updating last frequency data!");
        prefs.putString("frequency", freq);
    } else {
        Serial.println("Holding on to last frequency data!");
    }

    if (location != "") {
        Serial.println("Updating last location data!");
        prefs.putString("locData", location);
    } else {
        Serial.println("Holding on to last location data!");
    }

    if (emailid != "") {
        Serial.println("Updating last email-id data!");
        prefs.putString("emailID", emailid);
    } else {
        Serial.println("Holding on to last email-ID data!");
    }

    if (soilMoistMin != "") {
        Serial.println("Updating last SoilMoistThreshMin data!");
        prefs.putString("soilMoistMin", soilMoistMin);
    } else {
        Serial.println("Holding on to last SoilMoistThreshMin data!");
    }

    if (soilMoistMax != "") {
        Serial.println("Updating last SoilMoistThreshMax data!");
        prefs.putString("soilMoistMax", soilMoistMax);
    } else {
        Serial.println("Holding on to last SoilMoistThreshMax data!");
    }

    if (tempMin != "") {
        Serial.println("Updating last TempThreshMin data!");
        prefs.putString("tempMin", tempMin);
    } else {
        Serial.println("Holding on to last TempThreshMin data!");
    }

    if (tempMax != "") {
        Serial.println("Updating last TempThreshMax data!");
        prefs.putString("tempMax", tempMax);
    } else {
        Serial.println("Holding on to last TempThreshMax data!");
    }

    if (humMin != "") {
        Serial.println("Updating last HumThreshMin data!");
        prefs.putString("humMin", humMin);
    } else {
        Serial.println("Holding on to last HumThreshMin data!");
    }

    if (humMax != "") {
        Serial.println("Updating last HumThreshMax data!");
        prefs.putString("humMax", humMax);
    } else {
        Serial.println("Holding on to last HumThreshMax data!");
    }

    if (lightMin != "") {
        Serial.println("Updating last LightThreshMin data!");
        prefs.putString("lightMin", lightMin);
    } else {
        Serial.println("Holding on to last LightThreshMin data!");
    }

    if (lightMax != "") {
        Serial.println("Updating last LightThreshMax data!");
        prefs.putString("lightMax", lightMax);
    } else {
        Serial.println("Holding on to last LightThreshMax data!");
    }

    prefs.end(); // Close the preferences
}

void checkNVMdata() {
  prefs.begin("masterdata");
  // Retrieve data from non-volatile memory and store in g_userData structure
  (prefs.getString("ssid", "")).toCharArray(g_userData.wifiSSID, sizeof(g_userData.wifiSSID));
  (prefs.getString("password", "")).toCharArray(g_userData.wifiPassword, sizeof(g_userData.wifiPassword));
  (prefs.getString("frequency", "")).toCharArray(g_userData.freq, sizeof(g_userData.freq));
  (prefs.getString("locData", "")).toCharArray(g_userData.locData, sizeof(g_userData.locData));
  (prefs.getString("emailID", "")).toCharArray(g_userData.emailid, sizeof(g_userData.emailid));
  (prefs.getString("soilMoistMin", "")).toCharArray(g_userData.soilMoistMin, sizeof(g_userData.soilMoistMin));
  (prefs.getString("soilMoistMax", "")).toCharArray(g_userData.soilMoistMax, sizeof(g_userData.soilMoistMax));
  (prefs.getString("tempMin", "")).toCharArray(g_userData.tempMin, sizeof(g_userData.tempMin));
  (prefs.getString("tempMax", "")).toCharArray(g_userData.tempMax, sizeof(g_userData.tempMax));
  (prefs.getString("humMin", "")).toCharArray(g_userData.humMin, sizeof(g_userData.humMin));
  (prefs.getString("humMax", "")).toCharArray(g_userData.humMax, sizeof(g_userData.humMax));
  (prefs.getString("lightMin", "")).toCharArray(g_userData.lightMin, sizeof(g_userData.lightMin));
  (prefs.getString("lightMax", "")).toCharArray(g_userData.lightMax, sizeof(g_userData.lightMax));

  if (String(g_userData.wifiSSID) == "" || String(g_userData.wifiPassword) == "" || String(g_userData.freq) == "" || String(g_userData.locData) == "" || String(g_userData.emailid) == "")
  {
    Serial.println("No values saved for previously stored data,Starting Bluetooth to take the data");
    userSetupInit();
    
    // prefs.end(); -> Not required as it is called inside userSetupInit()
  }
  else
  {
    prefs.end();
  }
}

void thresholdValuesInit() {
    // Initialize Soil Moisture Thresholds
    if (String(g_userData.soilMoistMin) == "") {
        g_thresholdData.SoilMoistThreshMin = DEFAULT_SOILMOIST_MIN_VAL;
    } else {
        g_thresholdData.SoilMoistThreshMin = String(g_userData.soilMoistMin).toInt();
    }

    if (String(g_userData.soilMoistMax) == "") {
        g_thresholdData.SoilMoistThreshMax = DEFAULT_SOILMOIST_MAX_VAL;
    } else {
        g_thresholdData.SoilMoistThreshMax = String(g_userData.soilMoistMax).toInt();
    }

    // Initialize Temperature Thresholds
    if (String(g_userData.tempMin) == "") {
        g_thresholdData.TempThreshMin = DEFAULT_TEMP_MIN_VAL;
    } else {
        g_thresholdData.TempThreshMin = String(g_userData.tempMin).toInt();
    }

    if (String(g_userData.tempMax) == "") {
        g_thresholdData.TempThreshMax = DEFAULT_TEMP_MAX_VAL;
    } else {
        g_thresholdData.TempThreshMax = String(g_userData.tempMax).toInt();
    }

    // Initialize Humidity Thresholds
    if (String(g_userData.humMin) == "") {
        g_thresholdData.HumThreshMin = DEFAULT_HUM_MIN_VAL;
    } else {
        g_thresholdData.HumThreshMin = String(g_userData.humMin).toInt();
    }

    if (String(g_userData.humMax) == "") {
        g_thresholdData.HumThreshMax = DEFAULT_HUM_MAX_VAL;
    } else {
        g_thresholdData.HumThreshMax = String(g_userData.humMax).toInt();
    }

    // Initialize Light Thresholds
    if (String(g_userData.lightMin) == "") {
        g_thresholdData.LightThreshMin = DEFAULT_LIGHT_MIN_VAL;
    } else {
        g_thresholdData.LightThreshMin = String(g_userData.lightMin).toInt();
    }

    if (String(g_userData.lightMax) == "") {
        g_thresholdData.LightThreshMax = DEFAULT_LIGHT_MAX_VAL;
    } else {
        g_thresholdData.LightThreshMax = String(g_userData.lightMax).toInt();
    }
}