#include "timeDate.h"
#include <WiFiClient.h>

ESP32Time rtc(0);

int frequency = 4U;

bool setLocalClientTime(char *RxdTimebuf) {
  struct tm timeinfo;
  int year,month,day,hour,minute,second;
  String rxdData = String(RxdTimebuf).c_str();
  //Rxd format: 2025-01-12,13:06:16,freq,
  // Find the position of the first comma (separator between date and time)
  int comma1 = rxdData.indexOf(',');
  int comma2 = rxdData.indexOf(',', comma1 + 1);
  int comma3 = rxdData.indexOf(',', comma2 + 1);

  // Extract date part
  String dateStr = rxdData.substring(0, comma1);
  //Serial.print("dateStr is: ");
  //Serial.println(dateStr);
  // Extract time part
  String timeStr = rxdData.substring(comma1 + 1, comma2);
  //Serial.print("timeStr is: ");
  //Serial.println(timeStr);
  // Extract frequency part
  frequency = rxdData.substring(comma2 + 1, comma3).toInt();
  //.print("frequency is in setLocalClientTime: ");
  //Serial.println(frequency);

  // Split date into year, month, and day
  int dash1 = dateStr.indexOf('-');
  year = dateStr.substring(0, dash1).toInt();
  int dash2 = dateStr.indexOf('-', dash1 + 1);
  month = dateStr.substring(dash1 + 1, dash2).toInt();
  day = dateStr.substring(dash2 + 1).toInt();

  // Split time into hour, minute, and second
  int colon1 = timeStr.indexOf(':');
  hour = timeStr.substring(0, colon1).toInt();
  int colon2 = timeStr.indexOf(':', colon1 + 1);
  minute = timeStr.substring(colon1 + 1, colon2).toInt();
  second = timeStr.substring(colon2 + 1).toInt();
  
  if(year>2024) {
    //Set the RTC clock
    rtc.setTime(second, minute, hour, day, month, year);
    return true;
  }
  else {
    return false;
  }
} //end of function



void getTimeoftheday(char *buffer) {
  time_t rawtime;
  struct tm * timeinfo;
    
  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,25,"%e-%m-%G %R,",timeinfo); //23-01-2001 14:02,
} //end of function
