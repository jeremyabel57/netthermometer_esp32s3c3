#define CONFIG_C
#include "config.h"

class config cfg;


void config::setOpmode() {                                // set cfg.opmode
  opmode = OPMODE_HOST;
  if (mode == "rtc") opmode = OPMODE_RTC;
  if (mode == "therm") opmode = OPMODE_THERM;
}

void config::setIndex() {                                 // set cfg.index
  index = name[name.length()-1];
  if (isdigit((char) index))
    index = index - '0';
  else
    index = 2;
}
