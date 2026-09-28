#ifndef WEB_H
#define WEB_H


/*
 * Todo:
 * . add web page to set wakeme and monitor wakeme variable.  wakeme="" for inert.  wakeme=hostname of thermo or rtc host to keep awake.  the next time the host uploads data, it is told to toggle sleep_en, and wakeme is erased.
 * . validate time sync
 * . validate /dstore web interface
 * . validate web pages
 * . test host mode data storage and reporting /tdata.html file / web page report
 */


/*

web.begin();    // setup including AsyncFsWebServer
web.loop_1s();  // run every 1 second inside loop

*/



#include <Arduino.h>                            // arduino
#include "serialq.h"                            // queue buffer for serial i/o
#ifndef SERIALQ                                 // SerialQ
#define SERIALQ Serial                          //
#endif                                          //
#include <string.h>                             // strings

#include <FS.h>                                 // AsyncFsWebServer (in place of ESP Essentials)
#include <LittleFS.h>                           // LittleFS
#include <AsyncFsWebServer.h>                   // AsyncFsWebServer (in place of ESP Essentials), this uses Async background operations
#include <AsyncHTTPRequest_Generic.h>           // Async web requests   https://github.com/khoih-prog/AsyncHTTPRequest_Generic



// client functions
class webreq {
  public:
    
    AsyncHTTPRequest request;                         // async request object
    bool processed=true;                              // use this to flag that even though readyStateDone, it has been processed, and ready for next request
    
    bool send(const char*);                           // send http request.  return true if successful
    bool received();                                  // returns true if response received from request.  Only returns true once, then sets processed flag so subsequent calls do not return true until next request is made.
    bool pending();                                   // returns true if web request in process
    String responseText();                            // return response text (if available)
    char *respHeaderValue(const char *);              // return given header value
} ;



#define MAX_DATA_ITEMS 20                                             // max number of clients sending data to server


// server and AsyncFsWebServer features
class websrvc {
  protected:

//    static const char datahtml[];                                     // name of temperature data file containing data uploaded from the net thermometers
    String tdata[MAX_DATA_ITEMS];                                     // data received from clients to display in web page

    AsyncFsWebServer *server;                                         // AsyncFsWebServer object pointer

    // web pages
    static void reset(AsyncWebServerRequest*);                        // reset mcu
    static void clearWifi(AsyncWebServerRequest*);                    // clear Wifi settings and reset mcu
    static void root(AsyncWebServerRequest*);                         // root web page
    static void dstore(AsyncWebServerRequest*);                       // handle web option requests
    static void dupload(AsyncWebServerRequest*);                      // receive upload data from client thermometers
    static void timeSync(AsyncWebServerRequest*);                     // sync time with ntp server
    static void handleLed(AsyncWebServerRequest*);                    // switch LED on/off
    static void wake(AsyncWebServerRequest*);                         // handle wake requests

    String wakeme;                                                    // if we are the host server then this is the name of any client to toggle sleep_en as soon as it uploads data.  If blank, then do nothing.
    void dline(char *dataline, const char *name, time_t timestamp, float temp, float hum, float pres, float voltage, float current, float power, float shuntv);   // generate dataline, store in first parameter

  public:

    // FS, Server functions
    
    bool captivePortal;                                               // true if wifi setup captive portal active because wifi is not connecting or not configured

    void begin();                                                     // initialize (setup)
    void end();                                                       // flush LittleFS

    String getOpt(const char *lbl);                                   // get option by name
    void setOpt(const char *lbl, char *val);                          // set option by name

    bool isbusy();

    void printFileList(const char*,uint8_t);                          // print list of files to serial

    void loop_1s();                                                   // run every second

    // client functions
    
    class webreq updateHostReq;                                       // async http request object for sending data updates to the host server
    void updateHostServer();                                          // send http request to update host server

} ;





#ifdef WEB_C
class websrvc web;
#else
extern class websrvc web;
#endif


#endif

//
