//#define DEBUG3
//#define DEBUG2
//#define DEBUG1





#define SERIF_C
#include "serif.h"


#include "config.h"
#include "general.h"      //
#include "serif.h"        //
#include "web.h"          //

#include <WiFi.h>         // WiFi

#include "bme280my.h"     // BME280 temperature, humidity, pressure sensor
#include "rtctime.h"        // date/time functions




#define BLOCKSTART 0x1E  // character that starts a block of text on the serial interface
#define BLOCKEND 0x1D    // character that ends a block of text on the serial interface




/*    process commands sent thru serial port
in:   serinbuf - first char is command char, following characters are the parameter (BLOCKSTART and BLOCKEND chars stripped, or \r \n stripped)
out:  Serial
*/
void serifc::serinproc() {
  char *arg = serinbuf + 1;                // pointer to string after first letter
  while ((*arg == ' ') || (*arg == '\t')) arg++;  // skip past white spaces

  switch (serinbuf[0]) {
    case 'z':  // ack response from reboot/reset request, now time for this mcu to reset
      delay(200);
#ifdef WEB_H
      web.end();
#endif
      ESP.restart();  // reset mcu
      break;
    case 'w':  // get/set wifi state
      SERIALQ.printf("wifi ");  // respond with current wifi state
      SERIALQ.print(WiFi.localIP().toString().c_str());
      SERIALQ.printf("\r\n");
      break;
    case 'v':
      if (*arg) {
        char *val = strfind(arg,'=');
        if (*val) {
          *val=0;
          val++;
          web.setOpt(arg, val);
        }
        SERIALQ.printf("Options: %s=%s\r\n",arg,web.getOpt(arg).c_str());
      } else {
        SERIALQ.printf("Options: hostname=%s\r\n",cfg.name.c_str());
        SERIALQ.printf("Options: sleep enable=%i\r\n",cfg.sleep_en);
        SERIALQ.printf("Options: sleep period=%i\r\n",cfg.sleep_period);
        SERIALQ.printf("Options: sleep before=%i\r\n",cfg.sleep_before);
        SERIALQ.printf("Options: sleep minimum wake=%i\r\n",cfg.sleep_min_wake);
        SERIALQ.printf("Options: tz=%s\r\n",cfg.tz.c_str());
        SERIALQ.printf("Options: ts=%s\r\n",cfg.ts.c_str());
        SERIALQ.printf("Options: host server=%s\r\n",cfg.host_server.c_str());
        SERIALQ.printf("Options: mcu=%s\r\n",cfg.mcu.c_str());
        SERIALQ.printf("Options: board=%s\r\n",cfg.board.c_str());
        SERIALQ.printf("Options: operating mode=%s\r\n",cfg.mode.c_str());
      }
      break;
    case 'd':  // send/receiving date/time
      //while ((*arg == ' ') || (*arg == '\t')) arg++;  // skip past white spaces
      rtc.set(arg);
      break;
    case 'D':  // sync date/time with NTP server
      SERIALQ.print("Old time: ");
      rtc.print();
      SERIALQ.println();
      rtc.timeSync();
      SERIALQ.print("New time: ");
      rtc.print();
      SERIALQ.println();
      break;
    case 't':  // request for temperature data
      bme280.print();
      break;
    case 'l':  // list littlefs files
      web.printFileList("/", 0);
      break;
    case '\0': // help page
    case '?':  // ?, h, or blank line
    case 'h':
      SERIALQ.printf("\r\nhelp:\r\n"
                     "t                 get temperature/humidity/pressure\r\n"
                     "d [YYMMDDHHMMSS]  get/set date/time\r\n"
                     "D                 sync date/time with NTP server\r\n"
                     "l                 list files in littlefs\r\n"
                     "v [var[=value]]   get/set option variables, or list all options\r\n"
                     "w                 show ip address.  w=1 (on) or w=0 (off) to switch wifi on/off\r\n"
                     "z                 reset this mcu\r\n"
                     );
      break;
  }
}





/*      serin
task:   process input from serial port
        process format1: c[arg]\n         (arg cannot include \n)
                format2: c\x1E[arg]\x1D   (arg can include \n)
        c = command character
        arg = argument string
in:     char read from serial port
out:    serinproc() called where serinbuf[0]=c and serinbuf+1=arg (null terminated, \x1E \x1D stripped, or \r \n stripped)
*/
void serifc::serin(char c) {
  //Serial.print(c);                  // echo input back to sender
  //if (c == 13) Serial.print('\n');  //
  //if (c == 26) Serial.print('\r');  //

  if (((serinmode == 0) && (c != 10) && (c != 13)) ||          // if   (serinmode==line and input not eol/cr)
      ((serinmode == 1) && (c != BLOCKEND))) {                 //   or (serinmode==text and input not GS)
    if ((serinmode == 0) && (c == BLOCKSTART)) serinmode = 1;  //   if RS, then switch to text input mode (input ends with RS 0x1E)
    else {                                                     //   else
      if (c == 8) serinbuflen--;                               //     if backspace then erase last character
      else if (c == 26) serinbuflen = 0;                       //     else if escape, then erase input
      else if (serinbuflen < SERINSZ)                          //     else if not buffer overflow then
        serinbuf[serinbuflen++] = c;                           //       append input with new character
    }                                                          //
  } else {                                                     // else end of input signaled, process input as a full record
    serinbuf[serinbuflen] = 0;                                 // null terminate string
    if ((serinbuflen > 0) || (!serinateol)) {                  // ignore double eol
      serinproc();
      serinbuflen = 0;
      serinateol=true;
    } else {
      serinateol=false;
    }
    serinmode = 0;  // reset input mode back to line
  }
}



void serifc::begin() {
  // flush serial port input
#ifdef SERIALQ_H
  SERIALQ.flushin();
#else
  for (uint8_t i = 20; (i > 0) && SERIALQ.available(); i--)
    Serial.read();
#endif
  serinmode = 0;
  serinbuflen = 0;
}



void serifc::loop() {
  if (SERIALQ.available()) serin(SERIALQ.read());  // process incomming serial data
}

//
