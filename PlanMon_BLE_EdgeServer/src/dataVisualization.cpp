#include "dataVisualization.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Variables to store start and end dates
String startDate = "";
String endDate = "";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

char fileName[20] = "/default.csv";
const char *devicename = "/slave_devices.csv";
String newFileName;
const char* hostname = "planmon";


String filterDataFromFile(const char* start, const char* end) {
  Serial.println("Entered filterDataFromFile!");
  File file = SD.open(fileName, FILE_READ);

  if (!file) {
    Serial.println("Failed to open file!");
    return "[]";  // Return empty array if file fails to open
  }

  // Extract date, month, and year from start and end dates
  int Stday = atoi(start);
  int Stmon = atoi(start + 3);
  int Styear = atoi(start + 6);

  int Endday = atoi(end);
  int Endmon = atoi(end + 3);
  int Endyear = atoi(end + 6);

  Serial.print("Start Date: ");
  Serial.print(Stday);
  Serial.print("-");
  Serial.print(Stmon);
  Serial.print("-");
  Serial.print(Styear);
  Serial.println();

  Serial.print("End Date: ");
  Serial.print(Endday);
  Serial.print("-");
  Serial.print(Endmon);
  Serial.print("-");
  Serial.print(Endyear);
  Serial.println();

  // Use a String object to dynamically build the JSON response
  String jsonResponse = "[";  // Start of the JSON array
  bool firstLine = true;      // Flag to manage commas in JSON

  // Read all lines from the file
  while (file.available()) {
    char line[100];  // Fixed-size buffer for reading lines
    int len = file.readBytesUntil('\n', line, sizeof(line) - 1);
    line[len] = '\0';  // Null-terminate the string

    // Skip empty lines
    if (len == 0) continue;

    // Split the line by commas into fields (timestamp, temperature, humidity, moisture, lux)
    char* fields[5];
    int index = 0;
    char* token = strtok(line, ",");
    while (token != nullptr && index < 5) {
      fields[index++] = token;
      token = strtok(nullptr, ",");
    }

    if (strcmp(fields[0], "Timestamp") == 0) {  // Skip the header row
      continue;
    }

    // Extract date, month, and year from timestamp
    char Tdate[11];
    strncpy(Tdate, fields[0], 10);
    Tdate[10] = '\0';
    int Tday = atoi(Tdate);
    int Tmon = atoi(Tdate + 3);
    int Tyear = atoi(Tdate + 6);

    //Serial.print("Parsed Date: ");
    //Serial.print(Tday);
    //Serial.print("-");
    //Serial.print(Tmon);
    //Serial.print("-");
    //Serial.print(Tyear);
    //Serial.println();

    // Check if the parsed date exceeds the end date
    if (Tyear > Endyear || (Tyear == Endyear && Tmon > Endmon) || (Tyear == Endyear && Tmon == Endmon && Tday > Endday)) {
      Serial.println("Parsed date exceeds end date. Stopping processing.");
      break;  // Exit the loop immediately
    }

    // Filter the data based on the date range
    if (((Tday >= Stday && Tday <= Endday)) && ((Tmon >= Stmon && Tmon <= Endmon)) && ((Tyear >= Styear && Tyear <= Endyear))) {
      if (!firstLine) {
        jsonResponse += ",";  // Add a comma before each object after the first one
      }

      // Construct the JSON object for the current line
      jsonResponse += "{\"D\":\"";
      jsonResponse += fields[0];
      jsonResponse += "\",\"T\":";
      jsonResponse += fields[1];
      jsonResponse += ",\"H\":";
      jsonResponse += fields[2];
      jsonResponse += ",\"M\":";
      jsonResponse += fields[3];
      jsonResponse += ",\"L\":";
      jsonResponse += fields[4];
      jsonResponse += "}";

      firstLine = false;  // After the first line, set the flag to false
    }
  }

  file.close();  // Close the file after reading
  jsonResponse += "]";  // End of the JSON array

  return jsonResponse;  // Return the constructed JSON response
}


/*
// Function to filter data from the file based on date range
String filterDataFromFile(String start, String end) {
  File file = SD.open(fileName, FILE_READ);
  
  if (!file) {
    Serial.println("Failed to open file!");
    return "[]";  // Return empty array if file fails to open
  }

  String jsonResponse = "[";  // Start of the JSON array
  bool firstLine = true;  // Flag to manage commas in JSON
  String line;
  bool continueScan = false;
  
  // Read all lines from the file
  while (file.available()) {
    line = file.readStringUntil('\n');
    line.trim();  // Remove extra whitespace
    
    if (line.length() == 0) continue;  // Skip empty lines

    // Split the line by commas into fields (timestamp, temperature, humidity, moisture, lux)
    String fields[5];
    int index = 0;
    
    while (line.length() > 0 && index < 5) {
      int commaIndex = line.indexOf(',');
      if (commaIndex == -1) {
        fields[index++] = line;
        break;
      } else {
        fields[index++] = line.substring(0, commaIndex);
        line = line.substring(commaIndex + 1);
      }
    }

    if(fields[0] == "Timestamp") { //Skip the first row of the file as it has header names
      continue;
    }

    //1. Extract date, month and year from Start and end date // Format -> DD/MM/YYYY
    int Stday = start.substring(0,2).toInt();
    int Stmon = start.substring(3,5).toInt();
    int Styear = start.substring(6,10).toInt();

    //Serial.printf("%d,%d,%d,",Stday,Stmon,Styear);

    int Endday = end.substring(0,2).toInt();  // / Format -> DD-MM-YYYY
    int Endmon = end.substring(3,5).toInt();  // / Format -> DD-MM-YYYY
    int Endyear = end.substring(6,10).toInt();  // / Format -> DD-MM-YYYY
    
    //Serial.printf("%d,%d,%d,",Endday,Endmon,Endyear);
    
    
    //2. Extract date, month and year timestamp 
    String timestamp = fields[0];
    //Serial.println(timestamp);
    String Tdate = timestamp.substring(0,10); //timestamp.indexOf(' '));  // / Format -> DD-MM-YYYY
    //Serial.println(Tdate);
    int Tday = Tdate.substring(0,2).toInt();  // / Format -> DD-MM-YYYY
    int Tmon = Tdate.substring(3,5).toInt();  // / Format -> DD-MM-YYYY
    int Tyear = Tdate.substring(6,10).toInt();  // / Format -> DD-MM-YYYY   

    //Serial.printf("%d,%d,%d,",Tday,Tmon,Tyear);

    // Filter the data based on the date range
    if (((Tday >= Stday && Tday <= Endday)) && ((Tmon >= Stmon && Tmon <= Endmon)) && ((Tyear >= Styear && Tyear <= Endyear))) {
      if (!firstLine) {
        jsonResponse += ",";  // Add a comma before each object after the first one
      }
      jsonResponse += "{";
      jsonResponse += "\"D\":\"" + fields[0] + "\",";
      jsonResponse += "\"T\":" + fields[1] + ",";
      jsonResponse += "\"H\":" + fields[2] + ",";
      jsonResponse += "\"M\":" + fields[3] + ",";
      jsonResponse += "\"L\":" + fields[4];
      jsonResponse += "}";
      
      firstLine = false;  // After the first line, set the flag to false
    }

    // Check if the parsed date exceeds the end date
    if (Tyear > Endyear || (Tyear == Endyear && Tmon > Endmon) || (Tyear == Endyear && Tmon == Endmon && Tday > Endday)) {
      Serial.println("Parsed date exceeds end date. Stopping processing.");
      break;  // Exit the loop immediately
    }
  }
  file.close();  // Close the file after reading
  jsonResponse += "]";  // End of the JSON array

  return jsonResponse;
}

*/

// Initialize LittleFS
void initLittleFS() {
  Serial.println("Initializing LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
}

String getSensorReadings() {
  Serial.println("Getting sensor readings from the last entry in the file...");
  Serial.println(fileName);
  // Open the file
  if (!SD.exists(fileName)) {
    Serial.println("File does not exist. Returning default readings.");
    return "{\"error\":\"No data available\"}";
  }

  File file = SD.open(fileName, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file!");
    return "{\"error\":\"Failed to read file\"}";
  }

  // Variables to store the last line
  String lastLine = "";
  while (file.available()) {
    lastLine = file.readStringUntil('\n');  // Keep reading until the last line
    lastLine.trim();  // Remove any extra whitespace
  }
  file.close();

  // If no valid data, return an error
  if (lastLine.length() == 0) {
    Serial.println("No valid data in file.");
    return "{\"error\":\"No valid data\"}";
  }

  // Parse the last line
  String fields[5];  // Expected fields: timestamp, temperature, humidity, soil moisture, lux
  int index = 0;
  while (lastLine.length() > 0 && index < 5) {
    int commaIndex = lastLine.indexOf(',');
    if (commaIndex == -1) {
      fields[index++] = lastLine;
      break;
    } else {
      fields[index++] = lastLine.substring(0, commaIndex);
      lastLine = lastLine.substring(commaIndex + 1);
    }
  }

  // Ensure the line has all fields
  if (index < 5) {
    Serial.println("Incomplete data in last line.");
    return "{\"error\":\"Incomplete data\"}";
  }

  // Build the JSON response
  String jsonString = "{";
  jsonString += "\"temperature\":\"" + fields[1] + "\",";  // Temperature
  jsonString += "\"humidity\":\"" + fields[2] + "\",";     // Humidity
  jsonString += "\"moisture\":\"" + fields[3] + "\",";     // Soil Moisture
  jsonString += "\"lux\":\"" + fields[4] + "\"";           // Light Intensity
  jsonString += "}";

  Serial.println("Sensor readings fetched from file successfully.");
  return jsonString;
}

String thresholdjson()
{
  String jsonString = "{";
  jsonString += "\"tempmin\":\"" + String(g_thresholdData.TempThreshMin) + "\",";
  jsonString += "\"tempmax\":\"" + String(g_thresholdData.TempThreshMax) + "\",";
  jsonString += "\"hummin\":\"" + String(g_thresholdData.HumThreshMin) + "\",";
  jsonString += "\"hummax\":\"" + String(g_thresholdData.HumThreshMax) + "\",";
  jsonString += "\"soilmin\":\"" + String(g_thresholdData.SoilMoistThreshMin) + "\",";
  jsonString += "\"soilmax\":\"" + String(g_thresholdData.SoilMoistThreshMax) + "\",";
  jsonString += "\"lgtmin\":\"" + String(g_thresholdData.LightThreshMin) + "\",";
  jsonString += "\"lgtmax\":\"" + String(g_thresholdData.LightThreshMax) + "\"";
  jsonString += "}";

  return jsonString;
}


String slavedevicesjson() {
  String jsonResponse = "{\"devices\":[";  // Start of JSON object with key "devices" and begin array

  if (SD.exists(devicename)) {
    File file = SD.open(devicename, FILE_READ);
    if (file) {
      bool firstLine = true;  // Flag to handle the first line

      // Read the file line by line
      while (file.available()) {
        String line = file.readStringUntil('\n');  // Read line until newline
        
        // Trim any leading/trailing whitespace
        line.trim();

        if (line.length() > 0) {  // Check if line is not empty
          // Split the line into columns by comma
          int delimiterIndex = line.indexOf(',');  // Find the position of the first comma
          if (delimiterIndex != -1) {
            String deviceName = line.substring(delimiterIndex + 1);  // Extract the second column
            deviceName.trim();  // Trim any leading/trailing whitespace

            if (!firstLine) {
              jsonResponse += ",";  // Add a comma separator for all but the first entry
            }
            jsonResponse += "\"" + deviceName + "\"";  // Add the device name as a JSON string entry
            firstLine = false;  // After the first line, we need to add commas
          }
        }
      }

      file.close();  // Close the file after reading
    } else {
      Serial.println("Failed to open the file.");
      return "{\"error\":\"Failed to open the file\"}";  // Return error JSON
    }
  } else {
    Serial.println("File does not exist.");
    return "{\"error\":\"File does not exist\"}";  // Return error JSON
  }

  jsonResponse += "]}";  // End of JSON array and object
  return jsonResponse;  // Return the constructed JSON string
}


// Function to read the CSV file and print to Serial Monitor
void readCSVFile() {
  //Serial.println("Reading CSV file...");
    Serial.print("Attempting to read file:");
    Serial.println(fileName);
  if (SD.exists(fileName)) {

    File file = SD.open(fileName, FILE_READ);
    if (file) {
      Serial.println("Reading file: november_data.csv");
      while (file.available()) {
        String line = file.readStringUntil('\n');
        Serial.println(line); // Print each line to the Serial Monitor
      }
      file.close();
    } else {
      Serial.println("Failed to open file.");
    }
  } else {
    Serial.println("File november_data.csv does not exist.");
  }
}


void dataVisualisationInit() {

  initLittleFS();

  Serial.println("Checking Wi-Fi connection...");

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(g_userData.wifiSSID, g_userData.wifiPassword);
    setRGBColor(255,0,0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.print(".");
  }
  
  setRGBColor(0,0,0);
  Serial.println("\nWi-Fi is connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start mDNS service
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      setRGBColor(255,0,0);
      delay(1000);
    }
  }

  setRGBColor(0,0,0);
  Serial.println("mDNS responder started");
  Serial.println("You can now access the server using http://planmon.local");

  // Read the CSV file
  //readCSVFile();

  // Web Server setup
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("Serving index.html...");
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Request for the Slave Devices
  server.on("/slavedevices", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Received request for slave devices...");
    String json = slavedevicesjson();
    //Serial.println(json);
    request->send(200, "application/json", json);
  });

  server.on("/logdevice", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("name")) {
        String deviceName = request->getParam("name")->value();
        String filePath;
        bool deviceFound = false;

        // Check if the slave_devices file exists
        if (SD.exists(devicename)) {
            File file = SD.open(devicename, FILE_READ);
            if (file) {
                // Read the file line by line
                while (file.available()) {
                    String line = file.readStringUntil('\n');
                    line.trim(); // Remove any trailing whitespace

                    // Split the line into columns based on a delimiter (e.g., comma)
                    int delimiterIndex = line.indexOf(',');
                    if (delimiterIndex > 0) {
                        String firstColumn = line.substring(0, delimiterIndex); // First column
                        firstColumn.trim();
                        String secondColumn = line.substring(delimiterIndex + 1); // Second column
                        secondColumn.trim();

                        // Check if the second column matches the device name
                        if (secondColumn.equals(deviceName)) {
                            filePath = "/" + firstColumn + ".csv"; // Use the first column for the file name
                            deviceFound = true;
                            break;
                        }
                    }
                }
                file.close();
            } else {
                Serial.println("Failed to open slave_devices.txt");
                request->send(500, "text/plain", "Error opening slave_devices file");
                return;
            }
        } else {
            Serial.println("slave_devices.txt not found");
            request->send(500, "text/plain", "slave_devices file not found");
            return;
        }

        if (deviceFound) {
            // Use the filePath derived from the first column
            strcpy(fileName, filePath.c_str());
            Serial.println("Selected Device: " + deviceName);
            Serial.println("Filepath: " + filePath);
            request->send(200, "text/plain", "Device logged: " + deviceName);
        } else {
            request->send(404, "text/plain", "Device not found in slave_devices file");
        }
    } else {
        request->send(400, "text/plain", "Missing device name");
    }
});

server.on("/changedisplayname", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("currentName") && request->hasParam("newName")) {
        String currentName = request->getParam("currentName")->value();
        String newName = request->getParam("newName")->value();
        bool nameUpdated = false;

        // Check if the slave_devices file exists
        if (SD.exists(devicename)) {
            File file = SD.open(devicename, FILE_READ);
            String updatedContent = ""; // Store updated content

            if (file) {
                // Read file line by line
                while (file.available()) {
                    String line = file.readStringUntil('\n');
                    line.trim(); // Remove extra whitespace

                    // Split the line into columns
                    int delimiterIndex = line.indexOf(',');
                    if (delimiterIndex > 0) {
                        String firstColumn = line.substring(0, delimiterIndex);
                        firstColumn.trim();
                        String secondColumn = line.substring(delimiterIndex + 1);
                        secondColumn.trim();

                        // Update the second column if it matches the current name
                        if (secondColumn.equals(currentName)) {
                            secondColumn = newName; // Update the display name
                            nameUpdated = true;
                        }

                        // Reconstruct the line and add it to updatedContent
                        updatedContent += firstColumn + "," + secondColumn + "\n";
                    }
                }
                file.close();
            } else {
                Serial.println("Failed to open slave_devices.txt for reading");
                request->send(500, "text/plain", "Error reading file");
                return;
            }

            // Overwrite the file with updated content
            File writeFile = SD.open(devicename, FILE_WRITE);
            if (writeFile) {
                writeFile.print(updatedContent);
                writeFile.close();
            } else {
                Serial.println("Failed to open slave_devices.txt for writing");
                request->send(500, "text/plain", "Error writing file");
                return;
            }

            if (nameUpdated) {
                request->send(200, "text/plain", "Display name updated successfully");
            } else {
                request->send(404, "text/plain", "Current display name not found");
            }
        } else {
            request->send(500, "text/plain", "slave_devices.txt not found");
        }
    } else {
        request->send(400, "text/plain", "Missing current or new display name");
    }
});

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Received request for sensor readings...");
    String json = getSensorReadings();
    //Serial.println(json);
    request->send(200, "application/json", json);
  });

// Request for the Thresholds
  server.on("/thresholds", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Received request for thtesholds...");
    String json = thresholdjson();
    //Serial.println(json);
    request->send(200, "application/json", json);
  });

  // Handle historical data request and return in JSON format
 server.on("/historical-data", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Serial.println("Received request for historical data...");
  File file = SD.open(fileName, FILE_READ);
  
  if (!file) {
    Serial.println("Failed to open file!");
    request->send(500, "text/plain", "Failed to open file");
    return;
  }

  String jsonResponse = "[";  // Start of the JSON array
  bool firstLine = true;
  String lines[20];  // Buffer to store lines, assuming file has at least 15 entries
  
  // Skip the first line (header) by reading and discarding it
  String line = file.readStringUntil('\n');
  line.trim();  // Remove extra whitespace
  // Serial.println("Skipping header line: " + line);

  int lineCount = 0;
  
  // Read all lines from the file
  while (file.available()) {
    line = file.readStringUntil('\n');
    line.trim();  // Remove extra whitespace
    
    if (line.length() == 0) continue;  // Skip empty lines

    // Store the line in the buffer
    if (lineCount < 20) {
      lines[lineCount++] = line;
    } else {
      // Shift the lines to keep the buffer size at 20
      for (int i = 1; i < 20; i++) {
        lines[i - 1] = lines[i];
      }
      lines[19] = line;  // Add the new line at the end
    }
  }

  file.close();

  // Now build the JSON response with only the last 15 lines
  int startIdx = (lineCount > 15) ? lineCount - 15 : 0;
  
  for (int i = startIdx; i < lineCount; i++) {
    String line = lines[i];
    
    // Split the line by commas into fields
    String fields[5];
    int index = 0;
    
    while (line.length() > 0 && index < 5) {
      int commaIndex = line.indexOf(',');
      if (commaIndex == -1) {
        fields[index++] = line;
        break;
      } else {
        fields[index++] = line.substring(0, commaIndex);
        line = line.substring(commaIndex + 1);
      }
    }

    // Build the JSON object for each row
    if (index == 5) {  // Ensure complete row (timestamp, temperature, humidity, moisture, lux)
      if (!firstLine) {
        jsonResponse += ",";  // Add a comma before each object after the first one
      }
      jsonResponse += "{";
      jsonResponse += "\"D\":\"" + fields[0] + "\",";
      jsonResponse += "\"T\":" + fields[1] + ",";
      jsonResponse += "\"H\":" + fields[2] + ",";
      jsonResponse += "\"M\":" + fields[3] + ",";
      jsonResponse += "\"L\":" + fields[4];
      jsonResponse += "}";

      firstLine = false;  // After the first line, set the flag to false
    }
  }

  jsonResponse += "]";  // End of the JSON array

  // Send the JSON response
  request->send(200, "application/json", jsonResponse);
});

  // Handle the filter request (start date and end date)
  server.on("/setFilter", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("startDate") && request->hasParam("endDate")) {
      startDate = request->getParam("startDate")->value();
      endDate = request->getParam("endDate")->value();

      // Print the received dates for debugging
      Serial.print("Received Start Date: ");
      Serial.println(startDate);
      Serial.print("Received End Date: ");
      Serial.println(endDate);

      /*// Send a response back to the client (confirming the filter was applied)
      String response = "{\"status\":\"success\",\"startDate\":\"" + startDate + "\",\"endDate\":\"" + endDate + "\"}";
      request->send(200, "application/json", response);*/
      
      // Get filtered data from the CSV file
      String filteredData = filterDataFromFile(startDate.c_str(), endDate.c_str());
      //Serial.println("Filtered Data: " + filteredData);
      Serial.print("Done Sending data0!");
      // Send the filtered data as JSON
      request->send(200, "application/json", filteredData);
      Serial.print("Done Sending data1!");
    } 
    else {
      Serial.print("Done Sending data2!");
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing parameters\"}");
      Serial.println("Error: Missing startDate or endDate parameters!");
      Serial.print("Done Sending data3!");
    }
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