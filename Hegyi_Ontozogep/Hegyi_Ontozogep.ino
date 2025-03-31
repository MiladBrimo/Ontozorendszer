#include "Globals.h" //Global Variables, Conastants, Libraries


bool isTouched = false;
int lastMinute = -1;
int lastSecond = -1;


void setup()
{
  Serial.begin(9600); Serial2.end(); //Close UART2

  Initialization();

  IntroScreen();
  HomeScreen();
}


void loop()
{
  ///When Screen touched
  if(tp.touched()) isTouched = true;
  if(!tp.touched() && isTouched)
  {
    Touched();
    isTouched = false;
  }

  //Set display brightness
  if( millis()-FenyeroCounter > FenyeroTime && Fenyero != minFenyero) { Fenyero = minFenyero; analogWrite(TFT_LED, Fenyero); }

  //When Second changes
  if(second() != lastSecond)
  {
    lastSecond = second();
    if(onWiFi) checkWiFi();
    ValueIndicators();
    if(dataPrint) BeallitInfoScreenData();
  }

  //When Minute changes
  if(minute() != lastMinute)
  {
    lastMinute = minute();
    updateTime();
    //Check if it's time to start an event
    if(AutoMode) for(int i = 0; i < 25; i++) Ontozes[i].checkTime( hour()*60 + minute(), weekday());
  }
  
  //Check for Valves change
  ValveChange();

  delay(10);
}