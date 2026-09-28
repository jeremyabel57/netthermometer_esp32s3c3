#define I2SAUDIO_C
#include "i2saudio.h"

// I2S audio

#include "general.h"
#include "rtctime.h"          // what time is it


// Define I2S pin connections (adjust as needed for your board)
#define I2S_BCLK 4
#define I2S_LRC 5
#define I2S_DOUT 1
#define I2S_EN 3



#include "serialq.h"        // queue buffer for serial output
#ifndef SERIALQ             //
#define SERIALQ Serial      //
#endif                      //



void i2saudioc::begin() {
  decoder = new EncodedAudioStream(&out, &dec);   // decode audio file and send to output

  //AudioLogger::instance().begin(Serial, AudioLogger::Info); // Optional: enable logging

  // 1. Setup Output (e.g., I2S for MAX98357A)
  auto config = out.defaultConfig(TX_MODE);
  config.pin_bck = I2S_BCLK;
  config.pin_ws = I2S_LRC;
  config.pin_data = I2S_DOUT;   //
  config.channels = 1;          // convert mono to stereo output
  // config.pin_mck = -1; // -1 indicates not used
  out.begin(config);

  //auto volconfig = out.defaultConfig();
  //out.begin(volconfig);
  //out.setVolume(1.0);   // volume 100%

  enable(false);
}



void i2saudioc::enable(bool en) {
  if (en) {
    pinMode(I2S_EN, INPUT);   // hi-z / float output to enable I2S DAC circuit
  } else {
    pinMode(I2S_EN, OUTPUT);
    digitalWrite(I2S_EN, 0);  // shutdown I2S DAC circuit  
  }
}


void i2saudioc::loop() {
  if (audioFile) {
    if (!copier.copy()) { // Keep copying data until file ends
      endbell();
    }
  }
}



void i2saudioc::loop_1s() {
  if ((rtc.dt.tm_min == 0) && (rtc.dt.tm_sec == 4)) {             // start bell sequence at top of hour
    bellcount = rtc.hour12(rtc.dt.tm_hour);
    SERIALQ.printf("Playing %d bells at ",bellcount);
    rtc.print();
    SERIALQ.println();
    enable(true);                                           // power up external max98357b I2S DAC circuit
    udelay(11);                                             // minimum 10 us delay for max98357 power up requirement
  }
  if (!audioFile) {                                         // if not playing audio file (file closed) then
    if (bellcount > 0) {
      SERIALQ.printf("Ringing Bell %i\r\n",bellcount);
      bellcount--;
      beginbell();
    }
  }
}



void i2saudioc::beginbell() {
#ifdef MP3
  audioFile = LittleFS.open("/bell.mp3", "r");    // 2. Open file from LittleFS
#else
  audioFile = LittleFS.open("/bell.wav", "r");    // 2. Open file from LittleFS
#endif
  decoder->begin();                               // 3. Start decoder
  copier.begin(*decoder, audioFile);              //    Start copier
}



void i2saudioc::endbell() {
  copier.end();
  decoder->end();
  if (bellcount==0)                               // if last bell, then power down external I2S DAC circuit
    enable(false);
  audioFile.close();                              // if finished, then close the file
}


bool i2saudioc::isbusy() {                        // return true if in the middle of playing bells
  return ((bellcount>0) || (audioFile));
}

//
