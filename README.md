Arduino C++ code for ESP32

This includes AsyncWebFS server.  
Compiled on Arduino legacy UI.  The code is large, so use the following UI settings under tools:
  USB CDC On boot: enable
  Board: ESP32S3 Dev Module, or ESP32C3 Dev Module - I was using ESP32S3 and ESP32C3 Supermini board
  Partition Scheme:  Minimal SPIFFS (1.9MB App, 128k SPIFFS)

The goal is to have 3 operating modes for 3 different units
1. a server unit that sits inside the house and is connected to the internet.  This unit collects the data and can upload it to weatherunderground
2. one or more thermometer units.  These have a small solar panel and li-ion or lipo battery to keep them running around the clock.
   They will enter deep sleep and wake every 5 minutes to take a temperature reading and upload it to the data server.  This should keep the power
   draw to a minimum so the battery can be small.
3. a koo-koo clock.  This unit was to go into a clock and provide hourly chimes.  it has an RTC chip and i2s to audio output to drive a speaker.
