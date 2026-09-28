
/*
   init reorder
   serial.begin(115200)
   initialize AsyncFsWebServer
   get device name from options
   set hostname in AsyncFsWebServer using device name
   finish initializing AsyncFsWebServer

   get device program type (outdoor thermometer, grandfather clock, network host) from datastore
   initialize and execute accordingly
      outdoor thermometer will have multiple instances, each instance reports its name and the data to the network host
      one outdoor thermometer will upload its data to weather underground, use a special project name to identify this one (i.e. outdoor thermon 1)
      grandfather clock reports its name, current date/time stamp, and temperature/enviornment data to the network host
   network host
      will also support distributing OTA to each client by continusouly ping them until they wake up and come online, then quickly upload OTA
        if necessary, first disable client sleep mode, then OTA update, then reenable sleep
      when receiving data, add date/time stamp to the data so when reported we can see the last update.
      sync current date time stamp from GF clock.  if not available, then sync from internet

*/




#include <Arduino.h>            // Arduino, including String
#include "serialq.h"            // queue buffer for serial i/o
#ifndef SERIALQ                 //
#define SERIALQ Serial          //
#endif                          //


#include "general.h"            // misc functions
#include "config.h"
#include "web.h"                // WiFi web setup, FS, HTTP server
#include "espsleep.h"    			  // esp deep sleep mode
#include <Wire.h>               // I2C
#include "rtctime.h"    				// DS3231 RTC clock
#include "bme280my.h"	    			// BME280 temperature pressure humidity sensor
#include "i2saudio.h"           // i2s audio
#include "serif.h"              // serial command processor
#include "ina226my.h"           // INA226 voltage, current sensor






// timers for loop function
unsigned int loopTimer;
unsigned int loopTimer1s;
unsigned int loopTimer1m;
unsigned int loopTimer1h;






void setup() {
  char *s;

  Serial.begin(115200);   // UART / Serial
#ifdef SERIALQ_H
  SerialQ.begin(&Serial, 1024, 1024);  // initialize the serial queue/buffer with 1024 byte input and 1024 byte output buffers
#endif


  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  //} else {
  //  Serial.println("LittleFS mounted successfully.");
  }

#ifdef WEB_H
  web.begin();      // among other things, cfg is loaded from here
#endif

  // I2C
  Wire.begin();                                                                 // I2C
  Wire.setClock(100000);                                                        // I2C speed/clock 100kHz  
  Serial.println("I2C clock: 100kHz");
  if (cfg.board == "xiao") {                                                    // set I2C pins based on board
    Wire.setPins(4, 5);                                                         //   sda,scl for ESP32-C3 XIAO
    Serial.println("I2C pins: sda=4, scl=5");
  } else if ((cfg.board == "supermini") || (cfg.board == "mini")) {
    Wire.setPins(8, 9);                                                         //   sda,scl for ESP32-S3 supermini or ESP32-C3 supermini
    Serial.println("I2C pins: sda=8, scl=9");
  }

  // LED pin
  cfg.ledpin = (cfg.mcu == "esp32s3") ? 48 : 8;                                 // get led pin based on mcu configured
  Serial.print("LED pin: ");
  Serial.println(cfg.ledpin);

  pinMode(cfg.ledpin, OUTPUT);
  digitalWrite(cfg.ledpin, !web.captivePortal);                                 // set LED to show status (off = wifi configured, on = captive portal)
  

#ifdef RTCTIME_H
  rtc.begin();
#endif
#ifdef BME280MY_H
  bme280.begin();
#endif
#ifdef INA226MY_H
  ina226.begin();
#endif
#ifdef I2SAUDIO_H
  if (cfg.opmode == OPMODE_RTC)  i2saudio.begin();
#endif
#ifdef ESPSLEEP_H
  espsleep.begin();
#endif
#ifdef SERIF_H
  serif.begin();
#endif

  loopTimer1h = loopTimer1m = loopTimer1s = millis();
}







void loop() {
#ifdef SERIALQ_H
  SerialQ.loop();             // serial Q handler
#endif
#ifdef SERIF_H
  serif.loop();               // serial interface handler
#endif
#ifdef RTCTIME_H
  rtc.loop();
#endif

  loopTimer = millis();

#ifdef I2SAUDIO_H
  if (cfg.opmode == OPMODE_RTC) i2saudio.loop();
#endif

  // execute once per second
  if (elapsems(loopTimer1s, loopTimer) > 1000) {    // execute once per second
    loopTimer1s = millis();

#ifdef RTCTIME_H
    rtc.loop_1s();
#endif
#ifdef BME280MY_H
    bme280.loop_1s();
#endif
#ifdef INA226MY_H
    ina226.loop_1s();
#endif
#ifdef I2SAUDIO_H
    if (cfg.opmode == OPMODE_RTC) i2saudio.loop_1s();
#endif
#ifdef WEB_H
    web.loop_1s();
#endif
  }

  if (elapsems(loopTimer1m, loopTimer) > 60000) {     // execute once per minute
    loopTimer1m = millis();
#ifdef RTCTIME_H
    rtc.loop_1m();
#endif
  }

  if (elapsems(loopTimer1h, loopTimer) > 3600000) {   // execute once per hour
    loopTimer1h = millis();
#ifdef RTCTIME_H
    rtc.loop_1h();
#endif
  }



#ifdef ESPSLEEP_H
  espsleep.loop();
#endif

  delay(10);    // yield to RTOS and background processes
}

//
