#include "Initializations.h"
#include "Globals.h"
#include "MenuSystem.h"


RTC_DS3231 rtc;

String ssid;
String password;
const char* ntpServer = "pool.ntp.org";
bool reconnect = true;
bool onWiFi;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int gmtOffset; //Adjust for your timezone (e.g., 1 for GMT+1)
bool daylightOffset; //Daylight Saving Time offset (1 for summer)


void setupWiFiTime()
{
  if(onWiFi)
  {

    WiFi.begin( ssid.c_str(), password.c_str() );

    //Wait for WiFi connection
    unsigned long currentMillis = millis();
    while(WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      if( millis() - currentMillis > 5000 ) break;
    }

    if( WiFi.status() == WL_CONNECTED)
    {
      WiFiStatus(true, YELLOW);
      configTime(3600*gmtOffset, 3600*daylightOffset, ntpServer); //Initialize NTP
    
      //Wait for time to be set
      currentMillis = millis();
      while(time(nullptr) < 946684800) //Check if the time is before 2000-01-01 00:00:00 UTC
      {
        delay(500);
        if( millis() - currentMillis > 5000 ) break;
      }

      if(time(nullptr) > 946684800)
      {
        time_t now = time(nullptr); //Get current times
        setTime(now + 3600*gmtOffset + 3600*daylightOffset); //Update TimeLib with NTP time
        rtc.adjust(DateTime(now)); //Set RTC time
        WiFiStatus(true, BLUE);
        delay(500);
      }
      else
      {
        ErrorMessage("HIBA: Nem sikerult csatlakozni az NTP szerverre!", 4);
        ErrorMessage("Ido beolvasasa RTC-rol...", 5);
      
        if(rtc.begin())
        {
          setTime( rtc.now().unixtime() );
        }
        else
        {
          ErrorMessage("HIBA: RTC nem mukodik!", 6);
          setTime(0, 0, 0, 1, 1, 2025);
        }
      }

    }

    else
    {
      WiFiStatus(false, RED);
      ErrorMessage("HIBA: Nem sikerult csatlakozni a WiFi-re!", 4);
      ErrorMessage("Ido beolvasasa RTC-rol...", 5);
    
      if(rtc.begin())
      {
        setTime( rtc.now().unixtime() );
      }
      else
      {
        ErrorMessage("HIBA: RTC nem mukodik!", 6);
        setTime(0, 0, 0, 1, 1, 2025);
      }
    }

  }

  else
  {
    WiFiStatus(false, RED);
    ErrorMessage("WiFi kikapcsolva.", 4);
    ErrorMessage("Ido beolvasasa RTC-rol...", 5);

    if(rtc.begin())
    {
      setTime( rtc.now().unixtime() );
    }
    else
    {
      ErrorMessage("HIBA: RTC nem mukodik!", 6);
      setTime(0, 0, 0, 1, 1, 2025);
    }
  }

}

void setupWebServer()
{
  //Serve static HTML from the SD card
  server.serveStatic("/", SD, "/").setDefaultFile("index.html");

  //Configure WebSocket server
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  //Start the mDNS and the server
  MDNS.begin("ontozogep");
  server.begin();
}


void Initialization()
{

  if (gfx->begin())
  {
    gfx->fillScreen(BLACK);
    gfx->setRotation(2);
    pinMode(TFT_LED, OUTPUT);
    analogWrite(TFT_LED, maxFenyero);
  }
  else
  {
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, LOW);
  }


  if (tp.begin()) tp.setRotation(2);
  else ErrorMessage("HIBA: Erintes nem mukodik!", 0);


  if(SD.begin(SD_CS))
  {
    logFile = SD.open("/touch_log.txt", FILE_WRITE);
    if (!logFile) ErrorMessage("HIBA: SD kartya log fajl nem nyithato meg!", 1);
  }
  else ErrorMessage("HIBA: SD kartya vagy olvaso nem mukodik!", 1);


  if (mcp1.begin_I2C(0x20) && mcp2.begin_I2C(0x21) && mcp3.begin_I2C(0x22)) {
    for (int i = 0; i < 16; i++) {
      mcp1.pinMode(i, OUTPUT);
      mcp1.digitalWrite(i, LOW);
      mcp2.pinMode(i, OUTPUT);
      mcp2.digitalWrite(i, LOW);
      if(i < 4 || i > 7)
      {
        mcp3.pinMode(i, OUTPUT);
        mcp3.digitalWrite(i, LOW);
      }
      else mcp3.pinMode(i, INPUT);
    }
  } else ErrorMessage("HIBA: I2C portbovito nem mukodik!", 2);


  for (int i = 0; i < 4; i++) {
    pinMode(SolutionPins[i], OUTPUT);
    digitalWrite(SolutionPins[i], LOW);
  }
  pinMode(TartalyPin, INPUT);

  //Load data from Preferences (EEPROM)
  for(int i = 0; i < 25; i++) Ontozes[i] = Receptek(i);
  prefs.begin("Settings", true);  //Open Preferences in read-only mode
  AutoMode = prefs.getBool("AutoMode", true);
  onWiFi = prefs.getBool("onWiFi", true);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");
  MaxValveNumb = prefs.getInt("MaxValveNumb", 36);
  gmtOffset = prefs.getInt("gmtOffset", 1);
  daylightOffset = prefs.getBool("gmtOffset", false);
  prefs.end();


  if(!rtc.begin()) ErrorMessage("HIBA: RTC nem mukodik!", 3);
  if(rtc.lostPower()) ErrorMessage("HIBA: RTC elem lemerult!", 3);

  //Setup Wifi and Time
  setupWiFiTime();
  //Setup WebServer
  setupWebServer();

}


void checkWiFi()
{
  
  if( WiFi.status() != WL_CONNECTED )
  {
    WiFiStatus(false, RED);
    ws.cleanupClients();
    WiFi.reconnect();
    reconnect = true;
  }

  if ( WiFi.status() == WL_CONNECTED && reconnect)
  {
    WiFiStatus(true, YELLOW);
    updateTime();
    reconnect = false;
  }

  if( !ws.count() && WiFi.status() == WL_CONNECTED) WiFiStatus(true, BLUE);

}


void updateTime()
{
  if( WiFi.status() == WL_CONNECTED )
  {
    bool pingResult = Ping.ping("www.pool.ntp.org", 1);
    if( ws.count() ) WiFiStatus(true, pingResult ? MAGENTA : YELLOW);
    else WiFiStatus(true, pingResult ? BLUE : YELLOW);

    if(pingResult)
    {
      time_t now = time(nullptr); //Get current times
      setTime(now + 3600*gmtOffset + 3600*daylightOffset); //Update TimeLib with NTP time
      rtc.adjust(DateTime(now)); //Set RTC time
    }
    else if(rtc.begin()) setTime( rtc.now().unixtime() );

  }
  else if(rtc.begin()) setTime( rtc.now().unixtime() );

  ws.cleanupClients();
  TimeStatus();
}