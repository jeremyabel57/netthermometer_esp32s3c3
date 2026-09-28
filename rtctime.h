#ifndef RTCTIME_H
#define RTCTIME_H


/*
   RTC functions
   Handles internal ESP RTC, external DS3231 RTC and provides time tracking functions
   provides NTP client and server

   Time sync client - update RTC at regular intervals, unless updated via GFClock/RTC host on the network
     - RTC/GFClock host will need to do its own regular sync with time server on internet
   RTC syncs - anytime set() is called, set ESP RTC, then if present set DS RTC
     - if ESP core code NTP client is used, then when it updates then set DS RTC if present.
   Every second, update the dt variable to the current time (broken out into struct tm)
*/



///////////////////





#include <Arduino.h>              // arduino
#include <inttypes.h>             // int types
#include <string.h>               // strings
#include "general.h"              // general
#include "serialq.h"              // queue buffer for serial output
#ifndef SERIALQ                   //
#define SERIALQ Serial            //
#endif                            //

#include <ESP32Time.h>            // ESP32 Time functions and internal RTC
#include <time.h>                 // time functions
#include "ezTime_min.h"               // timezone operations
#include "web.h"                  // web get function for htpdate
#include <Wire.h>                 // external RTC
#include <ErriezDS3231.h>         // DS3231 I2C external RTC



/* RTC notes
 *  UDP is not working with AsyncTDP (AsyncFsWebServer), switching to HTPdate protocol to get latest time sync 
 */



class rtcc {
  protected:

    ESP32Time esprtc;                             // internal rtc

    char tstr[23];                                // time in readable format "mm/dd/yyyy hh:mm:ss am"
    struct tm tms;                                // time tm struct

    // time sync
    time_t timeUpdInterval;                       // time update interval (seconds)
    time_t timeUpdDue;                            // when the next ntp sync update is due
    ////

    // timezone
    Timezone tzLocal;                             // local timezone object    
	  ////

    // DS3231 external RTC
    bool dspresent;                               // true if ds3231 rtc is available
    ErriezDS3231 dsrtc;                           // Create RTC object
    ////

    class webreq htpdateReq;                      // http request object for htpdate


  public:

    char timeUrl[50];                             // time server URL or IP address
    char tz[30];                                  // timezone

    struct tm dt;                                 // time broken out, as local time zone.  UTC epoch is stored in ESP RTC.

    time_t utc();                                 // get current time in utc timezone (stored on internal rtc)
    time_t gmt();                                 // same as utc()
    time_t now();                                 // get current time in local time zone and DST adjusted

    void begin();                                 // initialize (setup)
    void end();                                   // save data to RTC memory for use after sleep
    void loop();                                  // run in loop
    void loop_1s();                               // run every 1 second in loop
    void loop_1m();                               // run every 1 minute in loop
    void loop_1h();                               // run every 1 hour in loop

    void print();                                 // print time to serial
    void print(time_t);                           // print time to serial
    void print(struct tm*);                       // print time to serial
    char *sprint();                               // get current local time as string
    char *sprint(time_t);                         // get date time as a string
    char *sprint(struct tm *);                    // get date time as a string

    uint8_t hour12(uint8_t);                      // convert 24 hour to a 12 hour number (0 >> 12, 1-12 >> 1-12, 13-23 >> 1-12)
    bool ampm(uint8_t);                           // from 24 hour number, return true if pm, false if am

    void set(char *);                             // take serial port input to set the clock, using GMT time zone
    void set(time_t);                             // set clock using unix epoch (number of seconds since 1970)
    void set(struct tm *);                        // set clock using tm struct

    void update(time_t);                          // update clock only if time update is due
    void timeSync();                              // sync the clock with the time server from the internet
    void htpdate();                               // handle received htpdate

    // DS3231 RTC
    bool readds();                                // set internal ESP RTC with time from DS3231
    bool writeds();                               // set DS3231 using time from internal ESP RTC
    
};


#ifdef RTCTIME_C
class rtcc rtc;
#else
extern class rtcc rtc;
#endif


#endif

//
