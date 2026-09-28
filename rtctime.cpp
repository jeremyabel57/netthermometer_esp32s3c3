#define RTCTIME_C
#include "rtctime.h"

#include <esp_sleep.h>
#include "config.h"

//#define DEBUG1



/*
 * Open issues:
 * \ DS3231 RTC code (use the Ezeee...RTC) 
 * / Timezone code
 *   0 ESP core not working, only did 1 hour offset for DST.  
 *   0 TinyTZ caused a crash - only for AVR?
 *   / try ezTime, but copy source code into project and set #define for no network support (eliminates NTP client code)
 *   \ store UTC epoch in ESP RTC and DS RTC
 *     
 * . test time sync between clients and host
 * . test http server host page, it is reverting to client page instead
 *   - test data uploads to make sure data file is getting written
 *     
 *   
 * . future ideas:
 *   \ migrate all datastore variables into AsyncFsWebServer options.  then remove datastore to shrink code size.
 *   . might have functions in AsyncFsWebServer that could replace sto.h functions.  We would just need to verify sdmicro card, then initialize FsWeb with sd instead of littlefs.
 *     . this would put all configurations on the sd card
 */


/////////
/*
 Posix TZ: PST8PDT,M3.2.0,M11.1.0
           PST
              8=offset for PST ( negative for west of GMT -> GMT-8 )
               PDT (minus 1 hour)
                    3=Mar
                      2=second week
                        0=Sunday
                           11=Nov
                              1=first week
                                0=sunday
*/ 


// Variable stored in RTC memory
RTC_DATA_ATTR time_t rtcTimeUpdDue; 



void rtcc::begin() {
  
  char *s;

  // time sync / update:  Since UDP is not working, using htpdate instead 

  s = (char*) cfg.ts.c_str();                                                     // set time server
  if (!*s) s = "http://google.com";                                               // default time server
  strncpy(timeUrl,s,50);
  timeUrl[49]=0;
  Serial.print("Time sync URL is ");
  Serial.println(timeUrl);

  timeUpdInterval = dspresent ? 30*24*60*60 : 24*60*60;                           // once per day if only using ESP RTC.  If using DS3231, once per month.  DS3231 may drift 10 seconds per month.

  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER) {                   // if not woke by timer (came back from sleep period) then
    timeUpdDue = now();                                                           //   initially set to syncronize with NTP now.
    Serial.println("Time sync due now.");
  } else {                                                                        // else
    timeUpdDue = rtcTimeUpdDue;                                                   //   recover next ntp due time from RTC memory
    Serial.print("Woke from sleep, next time sync at ");
    rtc.print(timeUpdDue);
    SERIALQ.flushout();
    Serial.println();
  }                                                                               // end if


  // TimeZone
  s=(char*) cfg.tz.c_str();                                                       // set timezone
  if (!*s) s = "PST8PDT,M3.2.0,M11.1.0";                                          // default TZ = PST
  strncpy(tz,s,30);
  tz[29]=0;
  Serial.print("TimeZone is ");
  Serial.println(tz);
  tzLocal.setPosix(tz);                                                           // set timezone in tzlocal object

  // DS3231 external RTC
  dspresent = dsrtc.begin();                                                      // initialize DS3231
  if (dspresent) {                                                                // if DS3231 found then
    SERIALQ.println("Found DS3231 external RTC.");                                //   messsage serial
    if (dsrtc.isRunning()) {                                                      //   if rtc oscillator is not enabled then
      dsrtc.clockEnable(true);                                                    //     enable it
    } else {                                                                      //   else
      readds();                                                                   //     sync internal RTC with external RTC
    }                                                                             //   end if
  } else                                                                          // else
    SERIALQ.println("DS3231 not found, using internal RTC only.");                //   message serial

  delay(100);
}



void rtcc::end() {
  rtcTimeUpdDue = timeUpdDue;       // save ntpdue to ESP RTC memory so it can be restored after sleep
}



void rtcc::loop() {
}


void rtcc::loop_1s() {
  if (utc() >= timeUpdDue) timeSync();        // if due for time sync then send time sync request
  if (htpdateReq.received()) { htpdate(); }   // if time sync received then process response

  time_t epoch = now();                       // get local time
  struct tm *t = gmtime(&epoch);              // break local time epoch into struct tm
  memcpy(&dt,t,sizeof(struct tm));            // copy local time to dt
}



void rtcc::loop_1m() {                        // execute this every minute
}



void rtcc::loop_1h() {                        // execute this every hour
  readds();                                   // keep ESP32 RTC updates with external DS RTC if available
}



////////////////////// time sync functions

void rtcc::update(time_t epoch) {
  if (epoch>946684800UL)                        // if timestamp is valid then
    if (utc() >= (timeUpdDue-900))              //   if within 15 minutes of next time sync then
      set(epoch);                               //     use this timestamp to update the clock and push out next time update due
}

void rtcc::timeSync() {                         // start htpdate by sending request to time server
  timeUpdDue = utc() + timeUpdInterval;         // push schedule to next time sync period to avoid race condition where we keep sending time sync requests
  htpdateReq.send(timeUrl);                     // send request to time server
}

void rtcc::htpdate() {                        // process received response to time sync request
  String serverDate = htpdateReq.respHeaderValue("Date"); // get "Date" header value from http response
  char *s;
  char ts[30];
  memset(ts,0,30);
  strncpy(ts,serverDate.c_str(),29);
  ts[29]=0;
  SERIALQ.printf("htpdate received: %s\r\n",ts);
  ts[3]=ts[7]=ts[11]=ts[16]=ts[19]=ts[22]=ts[25]=0;       // parse the date items
  struct tm t;                                            // parse date into tm struct
  t.tm_sec   = atoi(ts+23);
  t.tm_min   = atoi(ts+20);
  t.tm_hour  = atoi(ts+17);
  t.tm_mday  = atoi(ts+5);
  char moy[] ="JanFebMarAprMayJunJulAugSepOctNovDec";
  s=strstr(moy,ts+8);
  t.tm_mon   = s ? (s-moy)/3 : 0;
  int year   = atoi(ts+12);
  t.tm_year  = year - 1900;
  char dow[] ="SunMonTueWedThuFriSat";
  s=strstr(dow,ts);
  t.tm_wday  = s ? (s-dow)/3 : 0;
  int yd[12] = { 0,31,59,90,120,151,181,212,243,273,304,334 };
  t.tm_yday  = yd[t.tm_mon]+t.tm_mday;
  if (((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0)) && (t.tm_mon > 1)) t.tm_yday++;
  t.tm_isdst = 0;
  time_t epoch=mktime(&t);                                // convert date to epoch
  if (epoch>946684800UL) set(epoch);                      // if valid then set internal RTC and schedule next time sync 
}



//////////////////////////////////////



// print time to serial port

void rtcc::print()              { print(&dt); }                   // print current time to serial
void rtcc::print(time_t t)      { SERIALQ.print(sprint(t)); }     // print a given time epoch to serial
void rtcc::print(struct tm *t)  { SERIALQ.print(sprint(t)); }     // print a given time struct tm to serial


// print time to string

char *rtcc::sprint() { return sprint(&dt); }
char *rtcc::sprint(time_t t) {
  gmtime_r(&t, &tms);  
  return sprint(&tms); 
}
char *rtcc::sprint(struct tm *t) {
  snprintf(tstr,23,"%d/%d/%d %d:%02d:%02d %s",t->tm_mon+1,t->tm_mday,t->tm_year+1900,hour12(t->tm_hour),t->tm_min,t->tm_sec,ampm(t->tm_hour) ? "pm" : "am");
  tstr[22]=0;       // ensure null termination
  return tstr;
}


// 24 hour to 12 hour functions

uint8_t rtcc::hour12(uint8_t hour) { return (hour>12) ? hour-12 : (hour ? hour : 12); }     // convert hour from 24-hour to 12-hour format.  0 -> 12 am.  13-23 -> 1-12 pm.
bool rtcc::ampm(uint8_t hour) { return (hour>=12); }                                        // return true if 24-hour is in pm, otherwise it is in am



// get current time

time_t rtcc::utc() { return esprtc.getLocalEpoch(); }                                       // get utc/gmt time epoch
time_t rtcc::gmt() { return esprtc.getLocalEpoch(); }                                       // get utc/gmt time epoch
time_t rtcc::now() {                                                                        // get local time epoch
  return tzLocal.tzTime(utc(),UTC_TIME);                                                    // convert UTC time to localtime and return epoch
}



// set current time, including updating external RTC DS3231 and rescheduling next time sync

void rtcc::set(time_t epoch) {
  esprtc.setTime(epoch);                                    // update ESP32 RTC
  writeds();                                                // update DS RTC
//  tzLocal.setTime(epoch);                                   // set time in Timezone object
  timeUpdDue = epoch + timeUpdInterval;                     // set next ntp sync due
}
void rtcc::set(struct tm *tmtime) {
  set(mktime(tmtime));
}



void rtcc::set(char *timestr) {
  // The string hould be in the order YYMMDDHHMMSS.
  uint8_t j = strlen(timestr);
  tms.tm_year=26;  tms.tm_mon=0;  tms.tm_mday=1;  tms.tm_hour=0;  tms.tm_min=0;  tms.tm_sec=0;  tms.tm_isdst=0;
  if (j >= 2)  tms.tm_year=(uint8_t)(timestr[ 0]-'0') * 10 + (uint8_t)(timestr[ 1]-'0');      // year
  if (j >= 4)  tms.tm_mon= (uint8_t)(timestr[ 2]-'0') * 10 + (uint8_t)(timestr[ 3]-'0');      // month
  if (j >= 6)  tms.tm_mday=(uint8_t)(timestr[ 4]-'0') * 10 + (uint8_t)(timestr[ 5]-'0');      // day
  if (j >= 8)  tms.tm_hour=(uint8_t)(timestr[ 6]-'0') * 10 + (uint8_t)(timestr[ 7]-'0');      // hour
  if (j >= 10) tms.tm_min= (uint8_t)(timestr[ 8]-'0') * 10 + (uint8_t)(timestr[ 9]-'0');      // minute
  if (j >= 12) tms.tm_sec= (uint8_t)(timestr[10]-'0') * 10 + (uint8_t)(timestr[11]-'0');      // second
  // if at least the date was entered, then update RTC
  if (j >= 6) {
    set(&tms);
    SERIALQ.println();
    SERIALQ.print("RTC is now set to: ");
  }
  print();
  SERIALQ.println();
}




// DS3231 external RTC read/write

bool rtcc::readds() {                                                 // read ds rtc device and sync internal rtc with it
  bool r;
  if (dspresent) {
    time_t t;
    r=dsrtc.getEpoch();
    if (r) esprtc.setTime(t);
  } else r = false;
  return r;                                                           // return true if successful
}



bool rtcc::writeds() {                                                // write current time from internal RTC to external DS3231 RTC
  bool r;
  if (dspresent) {
    r = dsrtc.setEpoch(utc());
  } else r = false;
  return r;                                                           // return true if successful
}








/*
 * Todo:  
 * / use ezTime library for timezone and DST
 * / possibly switch to a DS RTC library without DST or TZ features
 * / change dt from DateTime to more standard struct tm
 * / ezTime does not use internal ESP32 RTC, it uses millis and a timestamp of when you set the time
 *   / loop_1s function - update dt
 *   / if ds present then use it to update esprtc every hour and once at setup/begin
 *   / setup/begin - set eztime from esp rtc.  this will cover sleep periods
 *   / set function will set esp rtc and ds rtc
 * / continue to use timedue variable to track when to sync with time server
 * / eztime
 *   / use set posix timezone function from datastore variable
 *   / use its features to convert from utc to localtime
 * / utc function
 *   / read time from esprtc
 * / now function
 *   / use eztime library to convert to local time
 * / set function
 *   / call writeds and setesptime to set each of the places time is tracked
 *   / setesptime - update esp rtc clock
 *
 * ntp server did not work, switch to the other one that has basic.ino
 *   look into its code to see if we need to put anything in the loop function.  not sure why basic.ino does not have anything there
 */

 //
