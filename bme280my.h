#ifndef BME280MY_H
#define BME280MY_H


//  BME280 temperature, humidity, pressure sensor

#include <Arduino.h>          //
#include <Wire.h>             // Needed for legacy versions of Arduino.
#include <BME280I2C.h>        // I2C communication to BME280 module
#include "web.h"              // 

#define INHGTOKPA  3.38639    // convert InHg to kPa



class BME280c {
  protected:

    BME280I2C *bme;

  public:

    bool present;
    BME280::TempUnit tempUnit;
    BME280::PresUnit presUnit;
    float temp;
    float hum;
    float pres;

    void begin();                               // initialize (setup)
    void loop_1s();                             // run every 1 second in loop

    void read();                                // read BME280 sensor
    void print();                               // print temperature, humidity, pressure to serial port


    class webreq wuReq;                         // http request object
    void updateWUnderground(const char*);       // update Weather Underground
};


#ifdef BME280MY_C
class BME280c bme280;
#else
extern class BME280c bme280;
#endif


#endif


/* Recommended Modes -
   Based on Bosch BME280I2C environmental sensor data sheet.

  Weather Monitoring :
   forced mode, 1 sample/minute
   pressure ×1, temperature ×1, humidity ×1, filter off
   Current Consumption =  0.16 μA
   RMS Noise = 3.3 Pa/30 cm, 0.07 %RH
   Data Output Rate 1/60 Hz

  Humidity Sensing :
   forced mode, 1 sample/second
   pressure ×0, temperature ×1, humidity ×1, filter off
   Current Consumption = 2.9 μA
   RMS Noise = 0.07 %RH
   Data Output Rate =  1 Hz

  Indoor Navigation :
   normal mode, standby time = 0.5ms
   pressure ×16, temperature ×2, humidity ×1, filter = x16
   Current Consumption = 633 μA
   RMS Noise = 0.2 Pa/1.7 cm
   Data Output Rate = 25Hz
   Filter B
   andwidth = 0.53 Hz
   Response Time (75%) = 0.9 s


  Gaming :
   normal mode, standby time = 0.5ms
   pressure ×4, temperature ×1, humidity ×0, filter = x16
   Current Consumption = 581 μA
   RMS Noise = 0.3 Pa/2.5 cm
   Data Output Rate = 83 Hz
   Filter Bandwidth = 1.75 Hz
   Response Time (75%) = 0.3 s

*/

//
