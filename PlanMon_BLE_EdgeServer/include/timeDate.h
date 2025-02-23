#ifndef TIMEDATE_H
#define TIMEDATE_H

//External libraries
#include <WiFi.h>
#include <ESP32Time.h>
#include <time.h>

//Dependency libraries


#define RED_PIN           2
#define GREEN_PIN         22
#define BLUE_PIN          21

extern ESP32Time rtc; // offset in seconds GMT+1

extern int Adv_period; //How long to advertise once advertisement has begun?

void setRGBColor(int red, int green, int blue);

#define SEC_60 60
enum FrequencyVal {
  FREQ_1 = 1U,
  FREQ_2 = 2U,
  FREQ_3 = 3U,
  FREQ_4 = 4U
};

enum Minutes {
  MIN_00 = 0U,
  MIN_01 = 1U,
  MIN_02 = 2U,
  MIN_03 = 3U,
  MIN_04 = 4U,
  MIN_05 = 5U,
  MIN_06 = 6U,
  MIN_07 = 7U,
  MIN_08 = 8U,
  MIN_09 = 9U,
  MIN_10 = 10U,
  MIN_11 = 11U,
  MIN_12 = 12U,
  MIN_13 = 13U,
  MIN_14 = 14U,
  MIN_15 = 15U,
  MIN_16 = 16U,
  MIN_17 = 17U,
  MIN_18 = 18U,
  MIN_19 = 19U,
  MIN_20 = 20U,
  MIN_21 = 21U,
  MIN_22 = 22U,
  MIN_23 = 23U,
  MIN_24 = 24U,
  MIN_25 = 25U,
  MIN_26 = 26U,
  MIN_27 = 27U,
  MIN_28 = 28U,
  MIN_29 = 29U,
  MIN_30 = 30U,
  MIN_31 = 31U,
  MIN_32 = 32U,
  MIN_33 = 33U,
  MIN_34 = 34U,
  MIN_35 = 35U,
  MIN_36 = 36U,
  MIN_37 = 37U,
  MIN_38 = 38U,
  MIN_39 = 39U,
  MIN_40 = 40U,
  MIN_41 = 41U,
  MIN_42 = 42U,
  MIN_43 = 43U,
  MIN_44 = 44U,
  MIN_45 = 45U,
  MIN_46 = 46U,
  MIN_47 = 47U,
  MIN_48 = 48U,
  MIN_49 = 49U,
  MIN_50 = 50U,
  MIN_51 = 51U,
  MIN_52 = 52U,
  MIN_53 = 53U,
  MIN_54 = 54U,
  MIN_55 = 55U,
  MIN_56 = 56U,
  MIN_57 = 57U,
  MIN_58 = 58U,
  MIN_59 = 59U,
  MIN_60 = 60U
};

// your declarations (and certain types of definitions) here
void timeDateInit();
void getLocalTime();
void getTimeoftheday(char *);












/************************************** INFORMATION SECTION BEGIN ****************************************/
//  Serial.println(rtc.getTime());          //  (String) 15:24:38
//  Serial.println(rtc.getDate());          //  (String) Sun, Jan 17 2021
//  Serial.println(rtc.getDate(true));      //  (String) Sunday, January 17 2021
//  Serial.println(rtc.getDateTime());      //  (String) Sun, Jan 17 2021 15:24:38
//  Serial.println(rtc.getDateTime(true));  //  (String) Sunday, January 17 2021 15:24:38
//  Serial.println(rtc.getTimeDate());      //  (String) 15:24:38 Sun, Jan 17 2021
//  Serial.println(rtc.getTimeDate(true));  //  (String) 15:24:38 Sunday, January 17 2021
//
//  Serial.println(rtc.getMicros());        //  (long)    723546
//  Serial.println(rtc.getMillis());        //  (long)    723
//  Serial.println(rtc.getEpoch());         //  (long)    1609459200
//  Serial.println(rtc.getSecond());        //  (int)     38    (0-59)
//  Serial.println(rtc.getMinute());        //  (int)     24    (0-59)
//  Serial.println(rtc.getHour());          //  (int)     3     (1-12)
//  Serial.println(rtc.getHour(true));      //  (int)     15    (0-23)
//  Serial.println(rtc.getAmPm());          //  (String)  pm
//  Serial.println(rtc.getAmPm(true));      //  (String)  PM
//  Serial.println(rtc.getDay());           //  (int)     17    (1-31)
//  Serial.println(rtc.getDayofWeek());     //  (int)     0     (0-6)
//  Serial.println(rtc.getDayofYear());     //  (int)     16    (0-365)
//  Serial.println(rtc.getMonth());         //  (int)     0     (0-11)
//  Serial.println(rtc.getYear());          //  (int)     2021
 
//  Serial.println(rtc.getLocalEpoch());         //  (long)    1609459200 epoch without offset
//Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // (String) returns time with specified format 
// formating options  http://www.cplusplus.com/reference/ctime/strftime/
/************************************** INFORMATION SECTION END ****************************************/

#endif