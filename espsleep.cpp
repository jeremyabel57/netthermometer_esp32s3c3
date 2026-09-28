
#define ESPSLEEP_C
#include "espsleep.h"

#include "config.h"
#include <Arduino.h>
#include <inttypes.h>
#include "espsleep.h"
#include "i2saudio.h"
#include "web.h"
#include "rtctime.h"
#include "ina226my.h"       // ina226 end function (shutdown)


// ESP sleep

#include "esp_sleep.h"
#define S_TO_uS_FACTOR 1000000
#define TIME_AWAKE 30             // minimal time to remain awake each period (seconds)
#define TIME_WAKE_PERIOD 300      // wake at top of every 5 minutes
#define TIME_WAKE_BEFORE 10       // wake 10 seconds before mark
#define ENABLE_SLEEP1

//RTC_DATA_ATTR int bootCount = 0;


#include "serialq.h"        // queue buffer for serial output
#ifndef SERIALQ             //
#define SERIALQ Serial      //
#endif                      //




void espsleepc::begin() {
  print_wakeup_reason();  //Print the wakeup reason for ESP32
  sleeppause = 1;
  if (!cfg.sleep_period) cfg.sleep_period = 300;
  if (!cfg.sleep_before) cfg.sleep_before = 10;
  if (!cfg.sleep_min_wake) cfg.sleep_min_wake = 30;  
}



void espsleepc::loop() {
#ifdef ENABLE_SLEEP1
  if (cfg.sleep_en)
    if ((!i2saudio.isbusy()) && (!web.isbusy()))
      if (millis() > (sleeppause * cfg.sleep_min_wake * 1000)) {   //
#ifdef SERIALQ_H              // flush serial port
        SerialQ.flushout();
#else
        Serial.flush();
#endif

        Serial.print("Current time: ");
        rtc.print();
        SERIALQ.println();
#ifdef SERIALQ_H
        SerialQ.flushout();
#endif

        time_t t = rtc.utc();
        // configure timer to wake up at the top of the next 5 minute interval
        t = cfg.sleep_period * ((time_t)(t / cfg.sleep_period) + 1) - t - cfg.sleep_before - 1; // calculate number of seconds to wait until 15 seconds before next top of 5 minute mark
        if (t > 10) {                                         // if at least 10 seconds before next wake period then sleep
#ifdef RTCTIME_H
          rtc.end();                                          // save data before sleep
#endif
#ifdef INA226MY_H
          ina226.end();                                       // send shutdown to ina226 device for power savings until next
#endif
#ifdef WEB_H
          web.end();                                          // flush fs data to flash
#endif
#ifdef SERIALQ_H
          SerialQ.end();
#endif
          // Configure timer wake-up
          esp_sleep_enable_timer_wakeup(t * S_TO_uS_FACTOR);  // number of us to wait until next wake period
          delay(1000);                                        // allow some time for the rtc wake setup

          Serial.print("Going to sleep for ");
          Serial.print(t);
          Serial.println(" seconds.");
          Serial.flush();
          delay(100);
          esp_deep_sleep_start();                             // Enter deep sleep
        } else {
          Serial.println("Within 10 seconds of next wake period, skipping sleep.");
          sleeppause++;                                       // increase backoff time for sleep checks
        }
      }
#endif
}



/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void espsleepc::print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\r\n", wakeup_reason); break;
  }
}

//
