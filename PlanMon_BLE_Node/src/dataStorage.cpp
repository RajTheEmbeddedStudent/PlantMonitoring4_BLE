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
  //File Name on the Master SD card storage depending on Slave Unique ID
  nameSlave.concat("Clientdatalog");
  nameSlave.concat(".csv");
  Serial.print(F("File name is"));
  Serial.println(nameSlave);
  nameSlave.toCharArray(FileName, 100);

  File file = SD.open(nameSlave);
  if(!file) {
    Serial.println(F("File doens't exist"));
    Serial.println(F("Creating file..."));
    writeFile(SD, FileName, "Date,Time,Temperature,Humidity,Soil Moisture,Light Intensity \r\n"); 
  }
  else {
    Serial.println(F("File already exists")); 
  }
  file.close();

  dataMessage = String(RxdData);
/*---------------------------------------------------PROBLEM: What if appending of data fails below ? Retry method ?----------------------------------------------------------*/
  appendFile(SD, FileName, dataMessage.c_str());
  //Once appending is successful reset the Filename string.
  nameSlave = "/";
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println(F("Failed to open file for writing"));
    return;
  }
  if(file.print(message)) {
    Serial.println(F("File written"));
  } else {
    Serial.println(F("Write failed"));
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println(F("Failed to open file for appending"));
    return;
  }
  if(file.print(message)) {
    Serial.println(F("Message appended"));
  } else {
    Serial.println(F("Append failed"));
  }
  file.close();
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("Data pending to be sent from file: ");
  while (file.available()) {
      String line = file.readStringUntil('\n'); 
      if (line.length() > 0) {
        // Split the line by comma
        int commaIndex = 0;
        int prevCommaIndex = -1;
        int columnCount = 0;
        String column[6]; 

        while (commaIndex >= 0) {
          commaIndex = line.indexOf(',', prevCommaIndex + 1);
          if (commaIndex >= 0) {
            column[columnCount] = line.substring(prevCommaIndex + 1, commaIndex);
            columnCount++;
          } else {
            column[columnCount] = line.substring(prevCommaIndex + 1);
            columnCount++;
          }
          prevCommaIndex = commaIndex;
        }

        // Check if the 6th column value is "no"
        if (column[5] == "no") {
          // Append the row to the dataString
          for (int i = 0; i < 5; i++) {
            dataMessage += column[i];
            if (i < 4) { 
              dataMessage += ","; 
            }
          }
          dataMessage += "\n";
        }
      }
    }
  Serial.println(dataMessage);
  file.close();
}