/*

Pinouts

gfclock: esp32s3 supermini
LED/RGB LED = 48
I2S        I2C
LR/WS=5    SDA=8
BCLK=4     SCL=9
DIN=1      BME280
SD/EN=3	
PCA98357

nethost/server: esp32c3 mini
LED (blue)=8 low=on
SD/TF card
SCK=4
MISO=5
MOSI=6
CS=7

thermometer: esp32c3 xiao 
LED (blue)=8 low=on
I2C
scl=7=d5
sda=6=d4
BME280  Temperature Humidity Pressure sensor
INA226  Current, Voltage sensor to monitor solar power

*/



#if 0

/*


to do
/ rename project to something like lanthermometer or something
\ add serial port function to read/write data store variables
  v var=val
/ change gfclock bell frequency to once per hour and use hour mod 12 for the bell count
/ change esp sleep to a period of 5 minutes with a wake at 10 seconds before the
  top of the 5 minute mark
- add sd card functions, probably sdfat
\ add web get function for both http and https
  \ add weather underground upload function
  - may need to download wunderground https/ssl certs, or have the code ignore the certs
/ rtc date/time
  / use ds3231 chip if available
  / if ds3231 not available, then use onboard rtc circuit
  / add ESP32Time library functions to keep internal rtc updated
  / if in server or default mode, then use internet ntp (once per day) to keep rtc in sync
\ add web based function to read/write variables in datastore 
  \ if sleep variable updated, then update espsleep class instance variable as well
/ more config for datastore
  / host_server= ip address of host server
  / board= board type (esp32c3 xiao, esp32s3 supermini, esp32c3 supermini)
\ ina226 module code
. add mode variable to dsstore
  - default mode is to stay awake and just sit on the network and be available
  \ gfclock mode (10.0.0.74)
    \ bells on the hour
	\ upload t/h/p data to server every 5 minutes, http_get
	\ upload clock epoch every 1 day, http_get
	\ once per month, download date/time epoch from internet
	\ sleep between t/h/p readings
  - thermometer mode (multiple instances)
	\ upload t/h/p data and solar voltage/current readings to server every 5 minutes
	\ sleep between t/h/p readings
	. every 5 minutes send update to weather underground (only if devname is first thermometer)
	  - datastore variables:
	    . enable weather underground updates switch (bool)
		. wunderground station id
		. wunderground login passwod
  - server mode (10.0.0.156)
    \ keep internal rtc clock updated via gfclock uploads
	  \ if no updates in last day, then use internet ntp server to sync
	\ serve web page of thermometer data
	\ serve time for local network
	\ when thermometer data comes in, attach timestamp using rtc for log
	\ log time and weather data to sd card
	. fw and little fs file upload feature for sleeping mcu clients
	  - fw update feature
	    . store esp32c3 and esp32s3 fw update on local sd flash
		. watch for data uploads from sleeping mcus, then when they are awake, transfer fw update
		  - verify esp32 c3 vs s3 FW before sending to client mcu
		  - use webget function to check variable in client mcu datastore
		\ need web based function to disable sleep during update
	  - little fs file upload feature
	    . store file in sd flash
		. upload when client mcu is awake
		. dont forget to restore sleep mode of client mcu
	
	

/ change ds3231.h file to rtctime.h  and add functions to use internal rtc.  use a flag to 
    follow ds3231 if available or use internal.  either way, keep both up to date



#

*/




#endif
