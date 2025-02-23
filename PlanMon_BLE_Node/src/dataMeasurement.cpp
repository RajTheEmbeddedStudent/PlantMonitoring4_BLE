#include "dataMeasurement.h"


Preferences prefs;


//Sensor specific configurations
#define DHTTYPE DHT11
DHT dht(DHT_SENSOR_PIN, DHTTYPE);

void dataMeasureInit() 
{
  //Store the device ID in Flash memory
  Preferences prefs;
  prefs.begin("clientdata");
  prefs.putString("deviceid", String(Device_ID));
  prefs.end();

  //Initialize sensor inputs
  pinMode(DHT_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Initialize DHT sensor
  dht.begin();
}

bool readSensorData(char *sensorData)
{
    char buffer[100];
    float sumTemp = 0.0;
    float sumHum = 0.0;
    float sumLight = 0;
    float R_L = 0;
    float lux = 0.0;
    float sumMoist = 0;
    float sensorVal = 0.0;
    String dataMessage;

    for(int i=0;i<10;i++)
      {
        //Read Temperature data
        sumTemp = sumTemp + dht.readTemperature();
        //Read Humidity data
        sumHum = sumHum + dht.readHumidity();
        //Read Light intensity data
        sensorVal = analogRead(LDR_SENSOR_PIN);
        R_L = (((sensorVal / 4096.0) * 5) * 10000) / (1 - ((sensorVal / 4096.0) * 5) / 5);
        lux = pow(50 * 1e3 * pow(10, 0.7) / R_L, (1 / 0.7));
        sumLight = sumLight + lux;
        sensorVal = 0;
        //Read Soil Moisture data
        sensorVal = analogRead(SOILMOIST_SENSOR_PIN);
        sumMoist = sumMoist + map(sensorVal,0,4095,100,0);
      }
  
  getTimeoftheday(buffer);
  dataMessage = String(Device_ID)                  + ","
              + String(buffer)                     //Time data comes with , at the end so no "," added
              + String((random(5, 40)*10)/10.0)    + ","         //Temperature
              + String((random(0, 100)*10)/10.0)   + ","         //Humidity
              + String((random(0, 100)*10)/10.0) + ","           //Soil moisture
              + String((random(0, 12000)*10)/10.0)   + "\r\n" ;  //(sumLight/10.0) * 10;

  dataMessage.toCharArray(sensorData,100);

  return true;
}
