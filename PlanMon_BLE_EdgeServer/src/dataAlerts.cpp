#include "dataAlerts.h"

String alertMessage;

// SMTPSession
SMTPSession smtp;

// SMTP Server
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// Email credentials
#define AUTHOR_EMAIL "planmonproject@gmail.com"
#define AUTHOR_PASSWORD "dhmjubaheawfeftj"

const char* fileNameda = "/slave_devices.csv";

struct deviceinfo {
    String deviceID[30];
    bool deviceprocess[30];
}deviceobj;

void dataAlertsInit() {
  // Enable SMTP debug
  smtp.debug(1);
}


void dataAlertsRun() {
  processLastDayData(); // Process data once a day
}

// Function to process only the last day's data from the SD card
void processLastDayData() {
  Serial.println("Processing last day's data...");
  File dataFile = SD.open(fileNameda);
  if (!dataFile) {
    Serial.println("Error opening file!");
    return;
  }

  // Find the latest date in the file
  int lastDay = 0, lastMonth = 0, currentday = 0, currentmonth = 0;
  currentday = rtc.getDay();
  currentmonth = rtc.getMonth()+1;

  // Get all unique IDs for the last day's data
  int idCount = 0;
  getUniqueIDs(fileNameda,deviceobj.deviceID);

  for(int i=0;i<30;i++) {
    if((deviceobj.deviceID[i]).isEmpty()) {
        break;
    }
    else {
        idCount = idCount+1;
        continue;
    }
  }
  //Serial.println("idCount:"+ idCount);
  String alertMessage = "Plants have something to complain, hear them below! Date: " + String(currentday) + "/" + String(currentmonth) + "\n\n";
  
  char buffer[15];

  // Process data for each ID and add to the alert message
  for (int i = 0; i < idCount; i++) {
    (deviceobj.deviceID[i]).toCharArray(buffer,15);
    processDaytimeDataForID(buffer, currentday, alertMessage);
  }

  
  dataFile.close();

  alertMessage += String("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-");
  alertMessage += String("\n \n Visit - http://planmon.local for more detailed view!");
  alertMessage += String("\n \n \n Regards,\n Team Plant Monitoring :)");
  Serial.println(alertMessage);
  // Send email if the alert message has content
  if (alertMessage.length() > 0) {
    sendEmail(alertMessage);
  } else {
    Serial.println("No alerts triggered today.");
  }
}

// Parse a single line from the file
SensorData parseLine(const String& line) {
  SensorData data;
  int values[9];
  int index = 0;
  int start = 0;

  for (int i = 0; i < line.length(); i++) {
    if (line[i] == ',' || i == line.length() - 1) {
      values[index++] = line.substring(start, i + (i == line.length() - 1 ? 1 : 0)).toInt();
      start = i + 1;
      if (index >= 9) break;
    }
  }

  data.temperature = values[1];
  data.humidity = values[2];
  data.soilMoisture = values[3];
  data.lightIntensity = values[4];

/*  Serial.print(data.temperature);
  Serial.print(",");
  Serial.print(data.humidity);
  Serial.print(",");
  Serial.print(data.soilMoisture);
  Serial.print(",");
  Serial.print(data.lightIntensity);
  Serial.print("\n");  */

  return data;
}

// Process daytime data for a specific ID and the last day's data
void processDaytimeDataForID(const char *fileNameda, int currentdate, String &alertMessage)
{

  String filePath = "/" + String(fileNameda) + ".csv";
  Serial.println("Attempting to open file: " + filePath);

  String timedata = getLastRowFirstColumn(fileNameda);
  int year, month, day, hour, min;
  extractTime(timedata, year, month, day, hour, min);
  
  Serial.print("     - Last data entry date,time: ");
  Serial.print(year);
  Serial.print("-");
  Serial.print(month);
  Serial.print("-");
  Serial.print(day);
  Serial.print("-");
  Serial.print(hour);
  Serial.print("-");
  Serial.print(min);
  Serial.print("\n");
  
  alertMessage += String("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-");
  alertMessage += String("\n") + String("Please pay attention to plant with Node ID :: ")  + String(fileNameda) + String("\n");

  if (currentdate == day)
  {
    // Process the data
    String line;
    SensorData data;
    //String filePath = "/" + String(fileNameda) + ".csv";
    //Serial.println("Attempting to open file: " + filePath);

    File dataFile = SD.open(filePath);
    // Serial.print("Attempting to open file:");
    // Serial.print(String(fileNameda) + ".csv");
    if (!dataFile)
    {
      Serial.println("Error opening file!");
      return;
    }

    // Define 3-hour daytime intervals
    const int intervals[5][2] = {{6, 9}, {9, 12}, {12, 15}, {15, 18}, {18, 21}};
    bool allIntervalsHaveData = true; // Assume all intervals have data initially
    
    for (int interval = 0; interval < 5; interval++)
    {
      float tempSum = 0, humiditySum = 0, soilMoistureSum = 0, lightSum = 0;
      int readingCount = 0;

      dataFile.seek(0); // Reset file pointer
      while (dataFile.available())
      {
        line = dataFile.readStringUntil('\n');
        int commaIndex = line.indexOf(',');
        String Timeinfo;
        if (commaIndex > 0)
        {
          Timeinfo = line.substring(0, commaIndex);
        }
        else
        {
          Serial.print("Comma not found for Line (in processDaytimeDataForID):");
          Serial.println(line);
        }

        int year, month, day, hour, min;
        extractTime(Timeinfo, year, month, day, hour, min);

        // Check if data matches the target ID, date, and falls within the interval
        if ((day == currentdate) &&
            (month == (rtc.getMonth() + 1)) &&
            (hour >= intervals[interval][0]) &&
            (hour < intervals[interval][1]))
        {

          data = parseLine(line);
          // Check for garbage or invalid readings and increment reading count only for valid data

          tempSum += data.temperature;
          humiditySum += data.humidity;
          soilMoistureSum += data.soilMoisture;
          lightSum += data.lightIntensity;
          readingCount++;
        }
      }

      // If no data is found for this interval, set allIntervalsHaveData to false
      if (readingCount == 0)
      {
        allIntervalsHaveData = false;
      }
      else
      {
        // Calculate averages for this interval
        float avgTemp = tempSum / readingCount;
        float avgHumidity = humiditySum / readingCount;
        float avgSoilMoisture = soilMoistureSum / readingCount;
        float avgLightIntensity = lightSum / readingCount;

        String alertForInterval = "";
        // Check if the averages exceed threshold values
        if (avgTemp < g_thresholdData.TempThreshMin || avgTemp > g_thresholdData.TempThreshMax)
        {
          alertForInterval += "    - Temperature out of threshold range, Value: " + String(avgTemp) + " deg C\n";
        }
        if (avgHumidity < g_thresholdData.HumThreshMin || avgHumidity > g_thresholdData.HumThreshMax)
        {
          alertForInterval += "    - Humidity out of threshold range, Value: " + String(avgHumidity) + " %\n";
        }
        if (avgSoilMoisture < g_thresholdData.SoilMoistThreshMin || avgSoilMoisture > g_thresholdData.SoilMoistThreshMax)
        {
          alertForInterval += "    - Soil moisture out of threshold range, Value: " + String(avgSoilMoisture) + " %\n";
        }
        if (avgLightIntensity < g_thresholdData.LightThreshMin || avgLightIntensity > g_thresholdData.LightThreshMax)
        {
          alertForInterval += "    - Light intensity out of threshold range, Value: " + String(avgLightIntensity) + " lux\n";
        }

        // Append to alert message if thresholds are exceeded
        if (alertForInterval.length() > 0)
        {
          alertMessage += "\n ** Alert for time interval " + 
                          String(intervals[interval][0]) + ":00 to " +
                          String(intervals[interval][1]) + ":00\n" + alertForInterval;
        }
      }
    }

    dataFile.close();

    // If any interval is missing data, send an alert
    //if (!allIntervalsHaveData)
    //{
    //  alertMessage += "\nNo data for Node ID " + String(fileNameda) + " for today (some intervals missing).\n";
    //}
  }
  else
  {
    // No data for today
    alertMessage += "\n ** No data was recorded for today! Please check the Node!\n";
  }
}

// Function to send email
void sendEmail(const String& message) {
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;

  SMTP_Message emailMessage;
  emailMessage.sender.name = F("Team PlanMon");
  emailMessage.sender.email = AUTHOR_EMAIL;
  emailMessage.subject = F("Your plants need you!!");
  emailMessage.addRecipient(F("Recipient"), g_userData.emailid);
  Serial.print("Sending email to:");
  Serial.println(g_userData.emailid);
  emailMessage.text.content = message.c_str();
  emailMessage.text.charSet = "us-ascii";
  emailMessage.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&config)) {
    Serial.printf("Error connecting to SMTP: %s\n", smtp.errorReason().c_str());
    return;
  }

  if (!MailClient.sendMail(&smtp, &emailMessage)) {
    Serial.printf("Error sending email: %s\n", smtp.errorReason().c_str());
  } else {
    Serial.println("Email sent successfully!");
  }
}

// Callback function for SMTP status
void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("Email sent successfully.");
  } else {
    Serial.println("Email failed to send.");
  }
}

// Function to extract time from a string in "YYYY-MM-DD HH:MM" format
void extractTime(const String& timeString, int& year, int& month, int& day, int& hour, int& minute) {
  // Extract year
  year = timeString.substring(6, 10).toInt();
  //Serial.println(year);

  // Extract month
  month = timeString.substring(3, 5).toInt();
  //Serial.println(month);

  // Extract day
  day = timeString.substring(0, 2).toInt();
  //Serial.println(day);

  // Extract hour
  hour = timeString.substring(11, 13).toInt();
  //Serial.println(hour);

  // Extract minute
  minute = timeString.substring(14, 16).toInt();
  //Serial.println(minute);
}