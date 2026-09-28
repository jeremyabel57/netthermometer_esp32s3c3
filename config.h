#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>            // arduino
//#include <string.h>             // strings
#include <inttypes.h>           // int types


// operating mode
enum opmode_t {
  OPMODE_DEFAULT = 0,
  OPMODE_HOST = 1,
  OPMODE_RTC = 2,
  OPMODE_THERM = 3
};


// global configuration variable.  stored in flash
class config {
public:
                                // option name          default                     description
  String name;                  // hostname             "new"                       device name.  used for web site
  bool sleep_en;                // sleep enable         false                       true if sleep enabled
  uint16_t sleep_period;        // sleep period         300                         wake every this many seconds (default 5 minutes)
  uint16_t sleep_before;        // sleep before         10                          wake this many seconds before top of wake period (wake 10 seconds before the top of the 5 minute period)
  uint16_t sleep_min_wake;      // sleep minimum wake   30                          remain awake for a minimum of this many seconds.  longer if system busy playing audio or serving web or other things
  String tz;                    // timezone             "PST8PDT,M3.2.0,M11.1.0"    timezone posix format
  String ts;                    // time server          "http://google.com"         website to use to grab date vi htpdate protocol (http headder "Date")
  String host_server;           // host server          "10.0.0.156"                ip/url of network host server to upload data onto
  String mcu;                   // mcu                  "esp32c3"                   microcontroller: esp32c3 or esp32s3
  String board;                 // board                "supermini"                 board: supermini or xiao
  String mode;                  // operating mode       "host"                      operating mode:  "host" = network data host.  "rtc" = gf clock.  "therm" = thermometer

  int index;                    //   the number at the end of the name. set and used internally
  opmode_t opmode;              //   number encoded operating mode
  int ledpin;                   //   pin number of the led pin for this board

  void setIndex();              // update index using name.  run this anytime name is changed
  void setOpmode();             // update opmode using mode.  run this anytime mode is changed
  
};

#ifndef CONFIG_C
extern class config cfg;
#endif

#endif






//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
