#ifndef WebServer_h
#define WebServer_h

#include <ESPAsyncWebServer.h> //Include for AsyncWebServer and WebSocket

//Function prototypes
void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

void sendTime();
void sendAutoMode();
void sendWiFi(String color);
void sendTartaly();
void sendSolTart();


#endif