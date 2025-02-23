#ifndef DATAMEASUREMENT_H
#define DATAMEASUREMENT_H

//External libraries
#include <DHT.h>
#include <DHT_U.h>
#include <Preferences.h>

//Internal dependencies
#include "timeDate.h"

//Device ID
#define Device_ID             9999U

//GPIO Pin Configurations
#define DHT_SENSOR_PIN        26
#define LDR_SENSOR_PIN        34
#define SOILMOIST_SENSOR_PIN  36
#define LED_PIN               25

extern Preferences prefs;

//Function declarations
void dataMeasureInit();
bool readSensorData(char *);








#endif