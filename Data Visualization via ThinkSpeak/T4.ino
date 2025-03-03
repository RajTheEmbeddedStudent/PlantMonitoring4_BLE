#include <WiFiManager.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <math.h>
#include <ArduinoLowPower.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>

// Wi-Fi credentials
const char* ssid = "SaLaManDer99";       // Replace with your Wi-Fi SSID
const char* password = "potsdamer"; // Replace with your Wi-Fi password

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");

String formattedDate;
String dayStamp;
String timeStamp;

// Sensor Pins and Threshold
#define SOIL_SENSOR_PIN 36
#define MOISTURE_THRESHOLD 30
#define DHTPIN 26      // GPIO pin connected to the DHT sensor
#define DHTTYPE DHT11  // Change to DHT11 if using DHT11 sensor
DHT dht(DHTPIN, DHTTYPE);
int ldrPin1 = 34;

// JSON object for sensor readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

// ThingSpeak API details
const String apiKey = "MYSLFNDFI3NOYGXI";  // Write API Key
const String readApiKey = "B6B3SRQLIX67Q5SF"; // Read API Key
const String channelID = "2764096";
const String tserver = "http://api.thingspeak.com/update";

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect("ESP32-Setup")) {
    Serial.println("Failed to connect to Wi-Fi. Restarting...");
    delay(3000);
    ESP.restart();
  }
  //WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to get timestamp
String getTimeStamp() {
  time_t now = time(NULL);
  struct tm* p_tm = localtime(&now);
  char buffer[20];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
          p_tm->tm_year + 1900,
          p_tm->tm_mon + 1,
          p_tm->tm_mday,
          p_tm->tm_hour,
          p_tm->tm_min,
          p_tm->tm_sec);
  return String(buffer);
}

// Get sensor readings in JSON format
String getSensorReadings() {
  float temperature = dht.readTemperature();    // Celsius by default
  float humidity = dht.readHumidity();
  int sensorValue = analogRead(SOIL_SENSOR_PIN);
  int moisturePercent = map(sensorValue, 0, 4095, 100, 0);

  int ldrValue = analogRead(ldrPin1);
  float V_R = ldrValue / 4096.0 * 5;
  float R_L = (V_R * 10000) / (1 - V_R / 5);
  float lux = pow(50 * 1e3 * pow(10, 0.7) / R_L, (1 / 0.7));

  readings["temperature"] = String(temperature);
  readings["humidity"] = String(humidity);
  readings["moisture"] = String(moisturePercent);
  readings["lux"] = String(lux * 10);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Function to send data to ThingSpeak
void sendToThingSpeak(float temperature, float humidity, int light, int moisture) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = tserver + "?api_key=" + apiKey +
                 "&field1=" + String(temperature) +
                 "&field2=" + String(humidity) +
                 "&field3=" + String(light) +
                 "&field4=" + String(moisture);

    http.begin(url);  // Begin the HTTP request
    int httpCode = http.GET();  // Send GET request

    if (httpCode > 0) {
      Serial.print("ThingSpeak HTTP Response Code: ");
      Serial.println(httpCode);
    } else {
      Serial.println("Error in HTTP request");
    }

    http.end();  // End the HTTP connection
  }
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();
  initLittleFS();
  //initBME();

  // Web Server setup
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start the server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping", NULL, millis());
    String sensorData = getSensorReadings();
    events.send(sensorData.c_str(), "new_readings", millis());

    // Get sensor values
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int sensorValue = analogRead(SOIL_SENSOR_PIN);
    int moisturePercent = map(sensorValue, 0, 4095, 100, 0);

    int ldrValue = analogRead(ldrPin1);
    float V_R = ldrValue / 4096.0 * 5;
    float R_L = (V_R * 10000) / (1 - V_R / 5);
    float lux = pow(50 * 1e3 * pow(10, 0.7) / R_L, (1 / 0.7));

    // Send data to ThingSpeak
    sendToThingSpeak(temperature, humidity, lux*10, moisturePercent);

    lastTime = millis();
  }
}
