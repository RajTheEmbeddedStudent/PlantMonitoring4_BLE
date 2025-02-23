#include "dataStorage.h"

// Define CS pin for the SD card module
#define SD_CS                  5

String nameSlave = "/";
char FileName[50] = {};
String dataMessage;

//Declare the functions
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);

void dataStorageInit()
{
  // Initialize SD card
  SD.begin(SD_CS);  
  if(!SD.begin(SD_CS)) {
    Serial.println(F("Card Mount Failed"));
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println(F("No SD card attached"));
    return;
  }
  Serial.println(F("Initializing SD card..."));
  if (!SD.begin(SD_CS)) {
    Serial.println(F("ERROR - SD card initialization failed!"));
    return;    // init failed
  }
}

// Write the sensor readings on the SD card
void logSDCard(char *RxdData) {
  //Extract the device_ID from the received string.
  bool deviceIDExists = false;
  // Extract first 5 characters
  char deviceID[5]; // Allocate space for 4 characters + null terminator
  strncpy(deviceID, RxdData, 4); 
  deviceID[4] = '\0'; // Ensure null termination

  // Delete first 5 characters from the original string
  memmove(RxdData, RxdData + 5, strlen(RxdData + 5) + 1);

//--------------------------------------------------DEVICE ID ENTRY BEGIN -----------------------------------------//
  //File Number (1) -> To store the details of the device ID into a File
  nameSlave.concat("slave_devices");
  nameSlave.concat(".csv");
  Serial.print(F("File name is"));
  Serial.println(nameSlave);
  nameSlave.toCharArray(FileName, 100);

  dataMessage = "PlanMon_" + String(deviceID) + "," +"PlanMon_" + String(deviceID) + "\r\n";
  Serial.print(F("    - Device ID to be written written into SD card is: "));
  Serial.print(dataMessage);

  File file = SD.open(nameSlave);
  if(!file) {
    Serial.println(F("    - File doesn't exist, Creating File..."));
  }
  else {
    Serial.println(F("    - File already exists"));
    // Open the file for reading
    File file = SD.open(FileName, FILE_READ);
    if (!file) {
      Serial.println(F("    - Failed to open file for reading!"));
      return;
    }

    // Check if the full string already exists in the file
    String compareStringMes = "PlanMon_" + String(deviceID);
    while (file.available())
    {
      String line = file.readStringUntil(','); // Read a line from the file
      Serial.println(line);
      // Remove any trailing whitespace (e.g., newline or carriage return)
      line.trim();

      // Skip empty lines
      if (line.length() == 0)
      {
        break;
      }

      // Check if the line matches the target device ID (case-insensitive)
      if (line.equalsIgnoreCase(compareStringMes))
      {
        deviceIDExists = true;
        break; // Exit the loop if the device ID is found
      }
      line = file.readStringUntil('\n');
      line.trim();
    }
  }
  file.close();

  if (deviceIDExists) {
    Serial.println(F("    - Device ID already exists in the file. Skipping write."));
  }
  else {
    Serial.println(F("    - Adding the new device ID into the file..."));
    appendFile(SD, FileName, dataMessage.c_str());
  }
/*---------------------------------------------------PROBLEM: What if appending of data fails below ? Retry method ?----------------------------------------------------------*/

  //Once appending is successful reset the Filename string.
  nameSlave = "/";
  dataMessage = "";
//--------------------------------------------------DEVICE ID ENTRY END -----------------------------------------//


//--------------------------------------------------DATA COPY INTO BEGIN -----------------------------------------//

  //File Number (2) -> To store the data received from client on Master SD card storage depending on Client device ID
  nameSlave.concat("PlanMon_" + String(deviceID));
  nameSlave.concat(".csv");
  Serial.print(F("File name is"));
  Serial.println(nameSlave);
  nameSlave.toCharArray(FileName, 100);

  file = SD.open(nameSlave);
  if(!file) {
    Serial.println(F("    - File doesn't exist, Creating File..."));
    writeFile(SD, FileName, "Date,Time,Temperature,Humidity,Soil Moisture,Light Intensity \r\n"); 
  }
  else {
    Serial.println(F("    - File already exists")); 
  }
  file.close();

  dataMessage = String(RxdData); //Updated data without Device ID and comma
  //Serial.print(F("Client data to be written into SD card is: "));
  //Serial.println(dataMessage);
/*---------------------------------------------------PROBLEM: What if appending of data fails below ? Retry method ?----------------------------------------------------------*/
  appendFile(SD, FileName, dataMessage.c_str());
  //Once appending is successful reset the Filename string.
  nameSlave = "/";
  dataMessage = "";
//--------------------------------------------------DATA COPY INTO END -----------------------------------------//

//--------------------------------------------------AVERAGE DATA FILE CREATION BEGIN -----------------------------------------//

  //File Number (2) -> To store the data received from client on Master SD card storage depending on Client device ID
  nameSlave.concat("AvgPlanMon_" + String(deviceID));
  nameSlave.concat(".csv");
  Serial.print(F("File name is"));
  Serial.println(nameSlave);
  nameSlave.toCharArray(FileName, 100);

  file = SD.open(nameSlave);
  if(!file) {
    Serial.println(F("    - File doesn't exist, Creating File..."));
    writeFile(SD, FileName, "Date,Time,Temperature,Humidity,Soil Moisture,Light Intensity \r\n"); 
  }
  else {
    Serial.println(F("    - File already exists")); 
  }
  file.close();

  dataMessage = String(RxdData); //Updated data without Device ID and comma
  //Serial.print(F("Client data to be written into SD card is: "));
  //Serial.println(dataMessage);
/*---------------------------------------------------PROBLEM: What if appending of data fails below ? Retry method ?----------------------------------------------------------*/
  appendFile(SD, FileName, dataMessage.c_str());
  //Once appending is successful reset the Filename string.
  nameSlave = "/";
  dataMessage = "";
//--------------------------------------------------AVERAGE DATA FILE CREATION END -----------------------------------------//
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("    - Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println(F("    - Failed to open file for writing"));
    return;
  }
  if(file.print(message)) {
    Serial.println(F("    - File written"));
  } else {
    Serial.println(F("    - Write failed"));
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("    - Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println(F("    - Failed to open file for appending"));
    return;
  }
  if(file.print(message)) {
    Serial.println(F("    - Message appended"));
  } else {
    Serial.println(F("    - Append failed"));
  }
  file.close();
}

/*
void getUniqueIDs(const char * path, String *deviceid) {
  int rowCount = 0;
  File myFile = SD.open(path); 
  if (myFile) {
    //Serial.println("    - File opened");
    while (myFile.available()) {
      String line = myFile.readStringUntil('\n'); 
      Serial.println(line);
      if (line.length() > 0) {
        // Find the position of the first comma
        int commaIndex = line.indexOf(',');
        String firstColumn;
        if (commaIndex != -1) {
          // Extract the first column (device ID) before the comma
          firstColumn = line.substring(0, commaIndex);
        } else {
          // If there is no comma, take the entire line as the first column
          firstColumn = line;
        }
        // Remove any leading or trailing whitespace
        firstColumn.trim();
        // Extract the first 12 characters (assuming the format is consistent)
        if (firstColumn.length() >= 12) {
          deviceid[rowCount] = firstColumn.substring(0, 12);
          rowCount++;
        }
      }
    }
  } else {
    Serial.print("    - Failed to open file (File not available?) ");
    Serial.println(String(path));
  }
  Serial.print("    - No. of devices to process data of: ");
  Serial.println(rowCount);
  myFile.close();
  //Serial.println("    - File closed");
}
*/

void getUniqueIDs(const char * path, String *deviceid) {
  int rowCount = 0;
  File myFile = SD.open(path); 
  if (myFile) {
    //Serial.println("    - File opened");
    while (myFile.available()) {
      String line = myFile.readStringUntil('\n'); 
      Serial.println(line);
      if (line.length() > 0) {
        // Find the position of the first comma
        int commaIndex = line.indexOf(',');
        String firstColumn;
        String secondColumn;
        if (commaIndex != -1) {
          // Extract the first column (device ID) before the comma
          firstColumn = line.substring(0, commaIndex);
          // Extract the second column (data after the comma)
          secondColumn = line.substring(commaIndex + 1);
        } else {
          // If there is no comma, take the entire line as the first column
          firstColumn = line;
          // Set the second column to an empty string
          secondColumn = "";
        }
        // Remove any leading or trailing whitespace
        //firstColumn.trim();
        secondColumn.trim();
        // Extract the first 12 characters of the device ID (assuming the format is consistent)
        if (secondColumn.length() >= 12) {
          deviceid[rowCount]  = secondColumn;
          rowCount++;
        }
      }
    }
  } else {
    Serial.print("    - Failed to open file (File not available?) ");
    Serial.println(String(path));
  }
  Serial.print("    - No. of devices to process data of: ");
  Serial.println(rowCount);
  myFile.close();
  //Serial.println("    - File closed");
}

// Function to read the last row and first column from the CSV file
String getLastRowFirstColumn(const char * path) {
  File myFile = SD.open("/" + String(path) + ".csv");
  String lastRowDate; 
  String lastLine;
  String currentLine;

  if (myFile) {
    Serial.println("    - File opened to check last row");
    // Read the file line by line
    while (myFile.available()) {
      currentLine = myFile.readStringUntil('\n'); 
      lastLine = currentLine; 
    }
  }
  else {
    Serial.println("    - File not found.");
  }

  // Extract the first column from the last row
  int commaIndex = lastLine.indexOf(',');
  if (commaIndex > 0) {
    lastRowDate = lastLine.substring(0, commaIndex); 
  }
  else {
    Serial.print("    - Comma not found for Line (in getLastRowFirstColumn): ");
    Serial.println(lastLine);
  }
  myFile.close();
  Serial.println("    - File closed after checking last row");
  return lastRowDate;
}