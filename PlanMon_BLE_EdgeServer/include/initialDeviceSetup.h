#ifndef INITIALDEVICESETUP_H
#define INITIALDEVICESETUP_H

#include <Arduino.h>
#include <Preferences.h>

// Variables to store received data
extern int frequency;
extern String locationData;
extern String ssid;
extern String password;
extern Preferences prefs;

void ReadUserConfigData();
void userSetupInit();
void processData(String data);
void saveUserData(const String& freq, const String& location, const String& ssid, const String& password, const String& emailid,
                  const String& soilMoistMin, const String& soilMoistMax, const String& tempMin, const String& tempMax,
                  const String& humMin, const String& humMax, const String& lightMin, const String& lightMax);
void thresholdValuesInit();
void checkNVMdata();

//Structure to hold Received data from User (char format)
struct userData {
    char freq[20];              // Frequency
    char locData[50];           // Location
    char wifiSSID[50];          // Wi-Fi SSID
    char wifiPassword[50];      // Wi-Fi Password
    char emailid[50];           // Email ID
    char soilMoistMin[10];      // Soil Moisture Threshold Minimum
    char soilMoistMax[10];      // Soil Moisture Threshold Maximum
    char tempMin[10];           // Temperature Threshold Minimum
    char tempMax[10];           // Temperature Threshold Maximum
    char humMin[10];            // Humidity Threshold Minimum
    char humMax[10];            // Humidity Threshold Maximum
    char lightMin[10];          // Light Threshold Minimum
    char lightMax[10];          // Light Threshold Maximum
};


//Below threshold values are derived for indoor Ficus plant.
enum DefaultValues {    
    DEFAULT_TEMP_MIN_VAL = 18,      // int8_t (-128 to 127)  // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
    DEFAULT_TEMP_MAX_VAL = 24,      // int8_t (-128 to 127)  // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
    DEFAULT_SOILMOIST_MIN_VAL = 40, // uint8_t (0 to 255)    // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
    DEFAULT_SOILMOIST_MAX_VAL = 60, // uint8_t (0 to 255)    // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
    DEFAULT_HUM_MIN_VAL = 50,       // uint8_t (0 to 255)    // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
    DEFAULT_HUM_MAX_VAL = 60,       // uint8_t (0 to 255)    // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
    DEFAULT_LIGHT_MIN_VAL = 20000,  // uint16_t (0 to 65535) // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
    DEFAULT_LIGHT_MAX_VAL = 50000   // uint16_t (0 to 65535) // *******IMPORTANT: RE_ORDER THE VALUES FROM LOW TO HIGH IF CHANGED!!******** //
};

struct thresholdData {
    uint8_t SoilMoistThreshMin = DEFAULT_SOILMOIST_MIN_VAL;  // Default minimum soil moisture threshold    (Units: %)
    uint8_t SoilMoistThreshMax = DEFAULT_SOILMOIST_MAX_VAL;  // Default maximum soil moisture threshold    (Units: %)
    int8_t TempThreshMin = DEFAULT_TEMP_MIN_VAL;             // Default minimum temperature threshold      (Units: deg C)
    int8_t TempThreshMax = DEFAULT_TEMP_MAX_VAL;             // Default maximum temperature threshold      (Units: deg C)
    uint8_t HumThreshMin = DEFAULT_HUM_MIN_VAL;              // Default minimum humidity threshold         (Units: %)
    uint8_t HumThreshMax = DEFAULT_HUM_MAX_VAL;              // Default maximum humidity threshold         (Units: %)
    uint32_t LightThreshMin = DEFAULT_LIGHT_MIN_VAL;         // Default minimum light intensity threshold  (Units: Lux)
    uint32_t LightThreshMax = DEFAULT_LIGHT_MAX_VAL;         // Default maximum light intensity threshold  (Units: Lux)
};

extern userData g_userData; // Declare global instance
extern thresholdData g_thresholdData;







#endif