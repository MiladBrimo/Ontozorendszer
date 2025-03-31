#include "WebServer.h"
#include "Globals.h"

enum SiteState {  Home, Ontozesek, Recept, KeziSzelep, KeziEc, KeziOldat, KeziNyomas, 
                    AutoIdoIdopont, AutoIdoOra, AutoIdoPerc, AutoIdoNapok, AutoIdoHossz, AutoSzelep, AutoSzelepCsoport, AutoEC, AutoOldat, AutoNyomas,
                    BeallitOraIdo, BeallitOraIdo2, BeallitOraDatum, BeallitOraDatum2, BeallitOraDatum3, BeallitOraZona, BeallitOraTNY, 
                    BeallitMax, BeallitInfo };
SiteState currentSite = Home;


//Map out site states
void getSite(const char* site)
{
  if(strcmp(site, "Home") == 0) currentSite = Home;
  if(strcmp(site, "Ontozesek") == 0) currentSite = Ontozesek;
}

//Command received from Client
void receivedData(String cmd_message)
{
  
  StaticJsonDocument<200> cmd_json;
  deserializeJson(cmd_json, cmd_message);
  const char* site = cmd_json["site"];
  const char* value = cmd_json["value"];

  StaticJsonDocument<200> json;
  json["type"] = site;
  String message = "";

  getSite(site);
  switch (currentSite)
  {
      
    case Home:
      json["Home"] = 0;
      serializeJson(json, message);
      ws.textAll(message);
    break;

    case Ontozesek:
      //A
    break;
    
  }

}


//WebSocket event handler
void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{ 
  //If a new client connects
  if(type == WS_EVT_CONNECT)
  {
    WiFiStatus(true, MAGENTA);

    StaticJsonDocument<200> json;
    String message;

    json["type"] = "Time";
    json["Time"] = (hour() < 10 ? "0" : "") + String( hour() ) + ":" + (minute() < 10 ? "0" : "") + String( minute() );
    serializeJson(json, message);
    client->text(message);

    json["type"] = "AutoMode";
    json["AutoMode"] = AutoMode;
    serializeJson(json, message);
    client->text(message);

    json["type"] = "WiFiStatus";
    json["WiFiStatus"] = "magenta";
    serializeJson(json, message);
    client->text(message);

    json["type"] = "Tartaly";
    json["Tartaly"] = Tartaly;
    serializeJson(json, message);
    client->text(message);

    json["type"] = "SolTart";
    JsonArray solTartArray = json.createNestedArray("SolTart");
    for(int i = 0; i < 4; i++) solTartArray.add(SolTart[i]);
    serializeJson(json, message);
    client->text(message);
  }

  if(type == WS_EVT_DATA)
  {
    //Handle incoming data from the client
    String cmd_message = String((char*)data);
    Serial.printf("Received data from client: %s\n", cmd_message.c_str());
    receivedData(cmd_message);
  }

}


void sendTime()
{
  StaticJsonDocument<200> json;
  json["type"] = "Time";
  json["Time"] = (hour() < 10 ? "0" : "") + String( hour() ) + ":" + (minute() < 10 ? "0" : "") + String( minute() );
  String message;
  serializeJson(json, message);
  ws.textAll(message);
}

void sendAutoMode()
{
  StaticJsonDocument<200> json;
  json["type"] = "AutoMode";
  json["AutoMode"] = AutoMode;
  String message;
  serializeJson(json, message);
  ws.textAll(message);
}

void sendWiFi(String color)
{
  StaticJsonDocument<200> json;
  json["type"] = "WiFiStatus";
  json["WiFiStatus"] = color;
  String message;
  serializeJson(json, message);
  ws.textAll(message);
}

void sendTartaly()
{
  StaticJsonDocument<200> json;
  json["type"] = "Tartaly";
  json["Tartaly"] = Tartaly;
  String message;
  serializeJson(json, message);
  ws.textAll(message);
}

void sendSolTart()
{
  StaticJsonDocument<200> json;
  json["type"] = "SolTart";
  JsonArray solTartArray = json.createNestedArray("SolTart");
  for(int i = 0; i < 4; i++) solTartArray.add(SolTart[i]);
  String message;
  serializeJson(json, message);
  ws.textAll(message);
}