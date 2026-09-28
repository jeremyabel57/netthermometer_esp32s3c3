#define WEB_C
#include "web.h"                // http server hooks to web pages

#include "serialq.h"            // queue buffer for serial output
#ifndef SERIALQ                 //
#define SERIALQ Serial          //
#endif                          //

#include "config.h"             //
#include "general.h"            // misc functions
#include "rtctime.h"            //
#include "bme280my.h"           //
#include "ina226my.h"           //
#include "espsleep.h"           //
#include <AsyncHTTPRequest_Generic.h>                           // Async web requests   https://github.com/khoih-prog/AsyncHTTPRequest_Generic




//const char websrvc::datahtml[] = "/tdata.html";                 // name of temperature data file containing data uploaded from the net thermometers



void websrvc::begin() {

  wakeme = String("");                                          // initialize to blank so this feature is inert until called upon

  for (int i=0; i<MAX_DATA_ITEMS; i++) tdata[i]=String("");     //
  
  server = new AsyncFsWebServer(LittleFS, 80, "");              // AsyncFsWebServer object

  // set configuration defaults
  cfg.name = String("new");
  cfg.sleep_en = false;
  cfg.sleep_period = 300;
  cfg.sleep_before = 10;
  cfg.sleep_min_wake = 30;
  cfg.tz = String("PST8PDT,M3.2.0,M11.1.0");
  cfg.ts = String("http://google.com");
  cfg.host_server = String("10.0.0.156");
  cfg.mcu = String("esp32c3");
  cfg.board = String("supermini");
  cfg.mode = String("host");

  if (LittleFS.exists(server->getConfiFileName())) {                      // load option variables from config
    // Test "options" values
    server->getOptionValue("hostname", cfg.name);
    server->getOptionValue("sleep enable", cfg.sleep_en);
    server->getOptionValue("sleep period", cfg.sleep_period);
    server->getOptionValue("sleep before", cfg.sleep_before);
    server->getOptionValue("sleep minimum wake", cfg.sleep_min_wake);
    server->getOptionValue("timezone", cfg.tz);
    server->getOptionValue("time server", cfg.ts);
    server->getOptionValue("host server", cfg.host_server);
    server->getOptionValue("mcu", cfg.mcu);
    server->getOptionValue("board", cfg.board);
    server->getOptionValue("operating mode", cfg.mode);
  }

  server->setHostname(cfg.name.c_str());        // set hostname
  
  cfg.setOpmode();                              // set cfg.opmode
  cfg.setIndex();                               // set cfg.index

  captivePortal = !server->startWiFi(10000);                              // Try to connect to WiFi (will start AP if not connected after timeout)
  if (captivePortal) {                                                    // If not able to configure wifi connection then
    Serial.println("\r\nWiFi not connected! Starting AP mode...");        //   start captive wifi setup portal
    String t = String(cfg.name) + "_Setup";                               //   set Wifi SSID name to be devname + Setup
    server->startCaptivePortal(t.c_str(), "", "/setup");                  //   start captive setup portal
    Serial.print("AP mode, Captive Portal at SSID: ");
    Serial.println(t.c_str());
  }                                                                       // end if

  // custom options
  server->addOptionBox("Custom options");
  server->addOption("hostname", cfg.name);
  server->addOption("sleep enable", cfg.sleep_en, "enable/disable sleep every so often");
  server->addOption("sleep period", cfg.sleep_period, "number of seconds for each period to wake up");
  server->addOption("sleep before", cfg.sleep_before, "number of seconds before top of period to wake up");
  server->addOption("sleep minimum wake", cfg.sleep_min_wake, "minimum time to remain awake");
  server->addOption("timezone", cfg.tz, "posix string for local timezone");
  server->addOption("time server", cfg.ts, "server to use for htpdate sync");
  server->addOption("host server", cfg.host_server, "target network host to upload periodic data");
  server->addOption("mcu", cfg.mcu, "esp32c3 or esp32s3");
  server->addOption("board", cfg.board, "supermini or xiao");
  server->addOption("operating mode", cfg.mode, "host, rtc, or therm");
  //server->addOption("name", testInt);
  //server->addOption("Test float variable", (double)testFloat, 0.0, 100.0, 0.001);
  //server->addComment("Test float variable", "Use this to adjust the intensity (0-100)");     // add a helpful comment beneath the float slider
  //server->addOption("Test bool variable", testBool, "Enable/disable feature");               // add a helpful comment direclty (boolean comment appears inline by default)

  server->on("/led", HTTP_GET, web.handleLed);                                          // Custom endpoint handler

  server->setSetupPageTitle("Async ESP FS WebServer");               // set main webpage title


  //////////////////////////////////
  // our HTTP server hooks
  server->on("/resetwifi", HTTP_GET, web.clearWifi);
  server->on("/reset", HTTP_GET, web.reset);
  server->on("/timesync", HTTP_GET, web.timeSync);
  server->on("/ds", HTTP_GET, web.dstore);
  if (cfg.opmode == OPMODE_HOST)
    server->on("/dupload", HTTP_GET, web.dupload);
  server->on("/wake", HTTP_GET, web.wake);

  //server->on("/d", HTTP_GET, web.data);
  //server->on("/t", HTTP_GET, web.data);

  server->on("/", HTTP_GET, web.root);
  //////////////////////////////////


  server->enableFsCodeEditor();                                      // Enable ACE FS file web editor and add FS info callback function
  server->init();                                                    // Start AsyncFsWebServer

  Serial.print(F("\r\nAsync ESP Fs Web Server started on IP Address: "));
  Serial.println(server->getServerIP());
  Serial.println(F(
                   "Open /setup page to configure optional parameters.\r\n"
                   "Open /edit page to view, edit or upload example or your custom webserver source files."));

}



void websrvc::end() {
  //LittleFS.flush();
}



void websrvc::handleLed(AsyncWebServerRequest* request) {
  static int value = false;
  // http://xxx.xxx.xxx.xxx/led?val=1
  if (request->hasParam("val")) {
    value = request->arg("val").toInt();
  } else {
    value = !digitalRead(cfg.ledpin);
  }
  digitalWrite(cfg.ledpin, value);
  String reply = "LED is now ";
  reply += (!value) ? "ON" : "OFF";
  request->send(200, "text/plain", reply);
}



void websrvc::wake(AsyncWebServerRequest* request) {
  if (request->hasParam("wake"))
    web.wakeme = request->arg("wake");
  request->send(200, "text/plain", web.wakeme);
}



void websrvc::reset(AsyncWebServerRequest* request) {
  request->send(200, "text/html",
                F(""
                  "<html><head><meta http-equiv='refresh' content='10; url=/' /></head>"
                  "<body>Rebooting Microcontroller.  Waiting 10 sec until refresh...</body></html>"));
#ifdef RTCTIME_H
  rtc.end();                                          // save data before sleep
#endif
  mdelay(1000);
  ESP.restart();  // reset mcu
}



void websrvc::printFileList(const char *directory, uint8_t levels) {
  server->printFileList(LittleFS, directory, levels);
}



void websrvc::timeSync(AsyncWebServerRequest* request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server", "ESP Async Web Server");
  response->printf("<!DOCTYPE html><html><head><title>%s %s</title></head><body>", cfg.name.c_str(), request->url().c_str());
  response->printf(F("<center><br>Old: %s<br>UTC: %s"), rtc.sprint(), rtc.sprint(rtc.utc()));  // current local and GMT date/time

  response->printf(F("<br><br>TimeZone: %s<br>NTP Server: %s<br>"), cfg.tz.c_str(), rtc.timeUrl);
  rtc.timeSync();

  response->printf(F("<br>New: %s<br>UTC: %s<br>"), rtc.sprint(), rtc.sprint(rtc.utc()));  // current local and GMT date/time
  response->print(F("<br><br></body></html>"));

  request->send(response);    //send the response last
}



void websrvc::clearWifi(AsyncWebServerRequest* request) {
  request->send(200, "text/html", F("<html><body><center>Wifi settings have been reset.</center></body></html>"));
  delay(500);
  WiFi.disconnect(true, true);      // first true = turn off wifi radio, second true = erase stored credentials
  delay(1000);
  ESP.restart();  // reset mcu
}



bool websrvc::isbusy() {    // return true if busy uploading data
  return false;
}



void websrvc::root(AsyncWebServerRequest* request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server", "ESP Async Web Server");
  response->print(F(R"(
<html>
<head>
  <title>)"));
  response->print(cfg.name.c_str());
  response->print(F(R"(</title>
  <style>
    table {
      border-collapse: separate;
      border-spacing: 15px 5px;
    }
  </style>
  <meta http-equiv="refresh" content="5">
</head>
<body>
  <center>
  <br>
)"));
  response->print(rtc.sprint());       // current date/time
  response->print(F(R"(
  <br><br>
  <table>
    <tr>
      <th>Device</th>
      <th>Time</th>
      <th>Temperature</th>
      <th>Humidity</th>
      <th>Pressure</th>
      <th>Voltage</th>
      <th>Current</th>
      <th>Power</th>
      <th>Shunt (0.1ohm)</th>
    </tr>
)"));

  if (cfg.opmode == OPMODE_HOST) {
    for (int i=0; i<MAX_DATA_ITEMS; i++) {
      if (web.tdata[i] != "") {
        response->print(web.tdata[i].c_str());
      }
    }
    /*
    File f;
    uint8_t buf[129];
    int n;
    if (f=LittleFS.open(web.datahtml,"r")) {
      while ((n=f.read(buf,128))>0) {
        buf[n]=0;                       // null terminate read file data (now treating like a string)
        response->print((char*)buf);    // send data to web response
      }
      f.close();
    }
    */
  }

  char dataline[210];
  web.dline(dataline,cfg.name.c_str(),rtc.now(),bme280.temp,bme280.hum,bme280.pres,ina226.voltage,ina226.current,ina226.power,ina226.shuntv);
  response->print(dataline);    

  response->print(R"(
  </table>
  <br>
  </center>
</body>
)");
  
  request->send(response);   // send response
}



void websrvc::dstore(AsyncWebServerRequest* request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");
  int n = request->params();                                        // number of arguments in url QUERY string
  String var, val;
  for (uint8_t i=0; i<n; i++) {
    AsyncWebParameter* p = (AsyncWebParameter*) (request->getParam(i));
    var=p->name();
    val=p->value();
    if (val != "")                                                  // if value not empty then
      web.setOpt(var.c_str(),(char*)val.c_str());                   //   set new value in storage
    val=web.getOpt(var.c_str());
    response->print(String(var + "=" + val + "\n").c_str());        // send variable and value in http respose
  }
  if (n == 0) {
    response->printf("name=%s\r\n",cfg.name);
    response->printf("sleep_en=%i\r\n",cfg.sleep_en);
    response->printf("sleep_period=%i\r\n",cfg.sleep_period);
    response->printf("sleep_before=%i\r\n",cfg.sleep_before);
    response->printf("sleep_min_wake=%i\r\n",cfg.sleep_min_wake);
    response->printf("tz=%s\r\n",cfg.tz);
    response->printf("ts=%s\r\n",cfg.ts);
    response->printf("host_server=%s\r\n",cfg.host_server);
    response->printf("mcu=%s\r\n",cfg.mcu);
    response->printf("board=%s\r\n",cfg.board);
    response->printf("operating mode=%s\r\n",cfg.mode);
  }
  request->send(response);                                          // close http response
}



void websrvc::dupload(AsyncWebServerRequest* request) {
  // time, name, tempf, humidity, baromin, voltage, current, power, shuntv
  char *a;
  String name;
  float temp,hum,pres,voltage,current,power,shuntv;
  time_t timestamp = rtc.now();                                     // timestamp when data was received
  temp=hum=pres=voltage=current=power=shuntv=0;
  if (request->hasParam(a="time"))        rtc.set((time_t) atoll(request->getParam(a)->value().c_str()));
  if (request->hasParam(a="name"))        name=request->getParam(a)->value();
  if (request->hasParam(a="tempf"))       temp=atof(request->getParam(a)->value().c_str());
  if (request->hasParam(a="hum"))         hum=atof(request->getParam(a)->value().c_str());
  if (request->hasParam(a="baromin"))     pres=atof(request->getParam(a)->value().c_str());
  if (request->hasParam(a="voltage"))     voltage=atof(request->getParam(a)->value().c_str());
  if (request->hasParam(a="current"))     current=atof(request->getParam(a)->value().c_str());
  if (request->hasParam(a="power"))       power=atof(request->getParam(a)->value().c_str());
  if (request->hasParam(a="shuntv"))      shuntv=atof(request->getParam(a)->value().c_str());

  char rb[27];
  snprintf(rb,23,"%lld",rtc.utc());                                                       // line 1 of response is the time epoch for time syncing
  strcpy(rb+strlen(rb),"\n0");
  if (web.wakeme==name) {rb[strlen(rb)-1]='1'; web.wakeme=String(""); }                   // line 2 of response is "1" to toggle sleep_en or "0" to do nothing
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", rb);       // Create response
  
  //char dtbuf[40];                                                                         // include date/time in header and time epoch in body
  //strftime(dtbuf, sizeof(dtbuf), "%a, %d %b %Y %H:%M:%S GMT", &rtc.dt);                   // format time for HTTP header (RFC 7231)
  //response->addHeader("Date", dtbuf);                                                     // add Date to header
  request->send(response);

  // process temperature data file and update data using received sample
  const char *datatemp = "/tdata_.html";
  File dfile, dfilet;                      // read dfile, write to dfilet
  char dataline[210];   // roughly 200+ bytes for each line
  web.dline(dataline,name.c_str(),timestamp,temp,hum,pres,voltage,current,power,shuntv);   // build html line from received data
  
  bool found = false;
  int i;
  for (i=0; i<MAX_DATA_ITEMS; i++) {
    if (*strfind((char*)web.tdata[i].c_str(),name.c_str())) {
      web.tdata[i] = String(dataline);
      found = true;
    }
  }
  if (!found) {
    for (i=0; (i<MAX_DATA_ITEMS) && (web.tdata[i]!=""); i++) ;
    if (web.tdata[i]=="") {
      web.tdata[i] = String(dataline);
    }
  }
  /*
  if (dfile=LittleFS.open(web.datahtml,"r")) {
    if (dfilet=LittleFS.open(datatemp,"w+")) {

      // alternate option is to make every line padded to take up exactly 200+ bytes, then use the seek function to move to the begining of the line and overwrite it with the new line
      const int BUFN = 250;
      uint8_t buf[251];
      const char *sname = name.c_str();
      bool found=false;
      char *sline, *st;
      int n;
      while (dfile.readBytesUntil('\n',buf,BUFN)!=0) {
        if (*strfind((char*)buf,sname)) {            // if line with device name found then use dataline instead
          sline = dataline;
          found = true;
        } else {
          st = (char*)buf + strlen((const char*)buf);
          *st='\n';
          st++;
          *st=0;
          sline = (char*)buf;
        }
        dfilet.write((const uint8_t*)sline, strlen(sline));
      }
      dfile.close();
      if (!found)     // if line with name not found, then append line to file
        dfilet.write((const uint8_t*)dataline, strlen(dataline));
      dfilet.close();
      LittleFS.remove((const char*)web.datahtml);                           // delete old file
      LittleFS.rename((const char*)datatemp,(const char*)web.datahtml);     // rename new file to old file's name 
    } else {
      dfile.close();
    }
  } else {      // else if cannot open web.datahtml for reading, then attempt to append the dataline to it
    if (dfile=LittleFS.open(web.datahtml,"w+")) {
      dfile.write((const uint8_t*)dataline, strlen(dataline));
      dfile.close();
    }
  }
  */
}


void websrvc::dline(char *dataline, const char *name, time_t timestamp, float temp, float hum, float pres, float voltage, float current, float power, float shuntv) {  // generate dataline, store in first parameter
  sprintf(dataline,"<tr><td>%s</td><td>%s</td><td>%.2f F</td><td>%.2f %%</td><td>%.2f inHg</td><td>%.2f V</td><td>%.2f mA</td><td>%.2f mW</td><td>%.2f mV</td></tr>\r\n",
    name,rtc.sprint(timestamp),temp,hum,pres,voltage,current,power,shuntv);   // build html line from received data
}



//        get and set options / configuration

String websrvc::getOpt(const char *lbl) {                                   // get option by name
  if (strcmp(lbl,"hostname")==0)              { server->getOptionValue(lbl, cfg.name);              return cfg.name;                    }
  if (strcmp(lbl,"sleep enable")==0)          { server->getOptionValue(lbl, cfg.sleep_en);          return String(cfg.sleep_en);        }
  if (strcmp(lbl,"sleep period")==0)          { server->getOptionValue(lbl, cfg.sleep_period);      return String(cfg.sleep_period);    }
  if (strcmp(lbl,"sleep before")==0)          { server->getOptionValue(lbl, cfg.sleep_before);      return String(cfg.sleep_before);    }
  if (strcmp(lbl,"sleep minimum wake")==0)    { server->getOptionValue(lbl, cfg.sleep_min_wake);    return String(cfg.sleep_min_wake);  }
  if (strcmp(lbl,"timezone")==0)              { server->getOptionValue(lbl, cfg.tz);                return cfg.tz;                      }
  if (strcmp(lbl,"time server")==0)           { server->getOptionValue(lbl, cfg.ts);                return cfg.ts;                      }
  if (strcmp(lbl,"host server")==0)           { server->getOptionValue(lbl, cfg.host_server);       return cfg.host_server;             }
  if (strcmp(lbl,"mcu")==0)                   { server->getOptionValue(lbl, cfg.mcu);               return cfg.mcu;                     }
  if (strcmp(lbl,"board")==0)                 { server->getOptionValue(lbl, cfg.board);             return cfg.board;                   }
  if (strcmp(lbl,"operating mode")==0)        { server->getOptionValue(lbl, cfg.mode);              return cfg.mode;                    }
  return String("");
}

void websrvc::setOpt(const char *lbl, char *val) {                          // set option by name
  if (strcmp(lbl,"hostname")==0)              { cfg.name = String(val);           server->saveOptionValue(lbl, cfg.name);               cfg.setIndex(); }
  if (strcmp(lbl,"sleep enable")==0)          { cfg.sleep_en = atoll(val);        server->saveOptionValue(lbl, cfg.sleep_en);           }
  if (strcmp(lbl,"sleep period")==0)          { cfg.sleep_period = atoll(val);    server->saveOptionValue(lbl, cfg.sleep_period);       }
  if (strcmp(lbl,"sleep before")==0)          { cfg.sleep_before = atoll(val);    server->saveOptionValue(lbl, cfg.sleep_before);       }
  if (strcmp(lbl,"sleep minimum wake")==0)    { cfg.sleep_min_wake = atoll(val);  server->saveOptionValue(lbl, cfg.sleep_min_wake);     }
  if (strcmp(lbl,"timezone")==0)              { cfg.tz = String(val);             server->saveOptionValue(lbl, cfg.tz);                 }
  if (strcmp(lbl,"time server")==0)           { cfg.ts = String(val);             server->saveOptionValue(lbl, cfg.ts);                 }
  if (strcmp(lbl,"host server")==0)           { cfg.host_server = String(val);    server->saveOptionValue(lbl, cfg.host_server);        }
  if (strcmp(lbl,"mcu")==0)                   { cfg.mcu = String(val);            server->saveOptionValue(lbl, cfg.mcu);                }
  if (strcmp(lbl,"board")==0)                 { cfg.board = String(val);          server->saveOptionValue(lbl, cfg.board);              }
  if (strcmp(lbl,"operating mode")==0)        { cfg.mode = String(val);           server->saveOptionValue(lbl, cfg.mode);               cfg.setOpmode(); }
}
  
////






void websrvc::loop_1s() {
  if (((cfg.opmode == OPMODE_RTC) || (cfg.opmode == OPMODE_THERM)) && ((rtc.dt.tm_min % 5) == 0) && (rtc.dt.tm_sec == cfg.index)) { // upload temperature data at the top of the 5 minute mark
    updateHostServer();
  }
  if (updateHostReq.received()) {
    time_t epoch=0;
    char *e=(char*)updateHostReq.responseText().c_str();
    char *f=strfind(e,'\n');
    if (*f) { *f='\0'; f++; }
    sscanf(e,"%lld",&epoch);
    rtc.update(epoch);
    if (*f=='1') cfg.sleep_en = !cfg.sleep_en;                      // if host sends flag then toggle cfg.sleep_en so we stay awake for maintenance
  }
}



void websrvc::updateHostServer() {       // send temperature, humidity, temperature data to host server
  String s;
  char t[100];
  sprintf(t,"http://%s/dupload?name=%s",cfg.host_server.c_str(),cfg.name.c_str());
  s = String(t);
#ifdef BME280MY_H
  if (bme280.present) {
    sprintf(t,"&tempf=%f&humidity=%f&baromin=%f",bme280.temp, bme280.hum, bme280.pres);
    s += t;
  }
#endif
  if ((cfg.opmode == OPMODE_RTC) && (rtc.dt.tm_hour==22) && (rtc.dt.tm_min==0)) {        // upload date/time if RTC mode and only once per day
    sprintf(t,"&time=%lld",rtc.utc());
    s += t;
  }
#ifdef INA226MY_H
  if (ina226.present) {
    sprintf(t,"&voltage=%f&current=%f&power=%f&shuntv=%f",ina226.voltage, ina226.current, ina226.power, ina226.shuntv);
    s += t;
  }
#endif
  updateHostReq.send(s.c_str());     // upload data to host server
}





////////////////////////////////////////////////////////////////////////////////////////////////////////
// http client functions





/*
void reqCB(void *optParm, AsyncHTTPRequest *request, int readyState) {
  (void) optParm;
  if (readyState == readyStateDone) {
    if (request.responseHTTPcode() == 200) {
      SERIALQ.println(F("\n**************************************"));
      SERIALQ.println(request.responseText());
      SERIALQ.println(F("**************************************"));
    }
  }
}
*/


//        send - start a http request
bool webreq::send(const char *url) {
  if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone) {
    if (request.open("GET", url)) {
      request.send();
      processed=false;
      return true;
    }
  }
  return false;
}

bool webreq::received() { if (!processed) if (request.readyState()==readyStateDone) { processed=true; return true;}  return false; }          // true if http request complete - response received
bool webreq::pending() { return (request.readyState() != readyStateUnsent) && (request.readyState() != readyStateDone); }                     // true if http request is pending
String webreq::responseText() { return request.responseText(); }                                                                              // text of http response
char *webreq::respHeaderValue(const char *var) { return request.respHeaderValue(var); }                                                       // return value of a given header from response





//
