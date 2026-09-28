#ifndef I2SAUDIO_H
#define I2SAUDIO_H

/*

  i2saudio.begin();  // setup
  i2saudio.loop();   // interface with host thru usb/primary uart


*/

//#define WAV       // use wav file
#define MP3         // use mp3 file

#include <Arduino.h>                                //
#include <inttypes.h>                               //
#include <FS.h>                                     //
#include <LittleFS.h>                               // little fs
#include "AudioTools.h"                             // Arduino Audio Tools
#ifdef MP3
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"   // MP3 decoder
#endif


class i2saudioc {
  protected:

    I2SStream out;                  // output to DAC
    //VolumeStream out(i2sout);     // adjust volume
#ifdef MP3
    MP3DecoderHelix dec;            // MP3 Decoder   
#else
    WAVDecoder dec;                 // WAV Decoder
#endif
    EncodedAudioStream *decoder;    // Decodes and sends to output
    StreamCopy copier;              // Helper to copy data
    File audioFile;                 // wav file to play to speaker
    uint8_t bellcount = 0;          // number of bells to play in speaker

    void beginbell();           // begin playing bell sound
    void endbell();             // end playing bell sound
    void enable(bool);          // enable/disable i2s audio circuit/amp

  public:

    void begin();               // initialize (setup)
    void loop();                // run in loop to respond to serial input
    void loop_1s();             // execute every 1 second
    bool isbusy();              // return true if busy playing sound

};


#ifdef I2SAUDIO_C
class i2saudioc i2saudio;
#else
extern class i2saudioc i2saudio;
#endif


#endif

//
