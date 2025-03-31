#ifndef MenuSystem_h
#define MenuSystem_h

#include <Arduino.h> //Arduino core library, includes uint16_t and more

//Function prototypes
void ErrorMessage(const char* label, int sor);
void IntroScreen();
void HomeScreen();

void ValueIndicators();
void ValveIndicators();
void TimeStatus();
void AutoStatus();
void WiFiStatus(bool isConnected, const uint16_t COLOR);
void TartalyStatus();
void SolTartStatus();
void blueButton(const char* label, int posX);
void redButton(const char* label, int posX);
void BeallitInfoScreenData();

void Touched();


#endif