#ifndef Globals_h
#define Globals_h


//Libraries
#include <TimeLib.h> //Ora
#include <Arduino_GFX_Library.h> //Kijelzo grafika
#include <XPT2046_Touchscreen.h> //Kijelzo erintes
#include <SD.h> //SD kartya
#include <SPI.h> // Kijelzo kommunikacio
#include <Adafruit_MCP23X17.h> //I2C portbovito
#include <RTClib.h> //DS3231 RTC
#include <Preferences.h> //EEPROM
#include <WiFi.h> //WiFi
#include <WiFiUdp.h> //NTP Server
#include <ESP32Ping.h> //Test live internet connection
#include <ESPAsyncWebServer.h> //WebServer
#include <ArduinoJson.h> //JSON data
#include <ESPmDNS.h> //mDNS

#include "Initializations.h" //Initialization Functions
#include "MenuSystem.h" //Menu System Funktions
#include "ValveControl.h" //Valve Control Functions
#include "ReceptClass.h" //Receptek Class
#include "WebServer.h" //WebServer Functions


//Constants
#define TFT_CS    15
#define TFT_RESET 17
#define TFT_DC    4
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_LED   2
#define TFT_MISO  19
#define TOUCH_CS  16
#define SD_CS     5

#define NUM_PROG 25

const byte SolutionPins[4] = {12, 14, 27, 26};
#define TartalyPin 13

#define FenyeroTime 60000
#define minFenyero 25
#define maxFenyero 255


//Global variables
extern Arduino_GFX* gfx;
extern XPT2046_Touchscreen tp;
extern File logFile;

extern Receptek Ontozes[25];
extern Preferences prefs;

extern Adafruit_MCP23X17 mcp1;
extern Adafruit_MCP23X17 mcp2;
extern Adafruit_MCP23X17 mcp3;

extern RTC_DS3231 rtc;

extern String ssid;
extern String password;
extern const char* ntpServer;
extern AsyncWebServer server;
extern AsyncWebSocket ws;

extern int gmtOffset;
extern bool daylightOffset;

extern byte Fenyero;
extern unsigned long FenyeroCounter;

extern bool AutoMode;
extern bool onWiFi;
extern bool ProgOn[25];
extern bool Valve[36];
extern int Solution;
extern int lastSolution;
extern bool Depressure[4];
extern bool Tartaly;
extern bool SolTart[4];
extern int istEC;
extern int sollEC;
extern float cTemp;
extern int MaxValveNumb;
extern bool dataPrint;


#endif