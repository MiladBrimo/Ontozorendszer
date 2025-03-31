#include "MenuSystem.h"
#include "Globals.h"
#include "ValveControl.h"
#include "ReceptClass.h"


Arduino_DataBus* bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX* gfx = new Arduino_ILI9488_18bit(bus, TFT_RESET, 3 /* rotation */, false /* IPS */);
XPT2046_Touchscreen tp(TOUCH_CS);

File logFile;


Receptek Ontozes[25]; //Array of 25 Ontozes events

enum ScreenState {  Home, Menu, Ontozesek, KeziMod, Naplo, Beallitasok, Recept, KeziSzelep, KeziEc, KeziOldat, KeziNyomas, 
                    AutoIdo, AutoIdoIdopont, AutoIdoOra, AutoIdoPerc, AutoIdoNapok, AutoIdoHossz, AutoSzelep, AutoSzelepCsoport, AutoEC, AutoOldat, AutoNyomas,
                    BeallitOra, BeallitOraIdo, BeallitOraIdo2, BeallitOraDatum, BeallitOraDatum2, BeallitOraDatum3, BeallitOraZona, BeallitOraTNY, 
                    BeallitWiFi, BeallitWiFiSSID, BeallitWiFiPW, BeallitEC, BeallitMax, BeallitInfo };
ScreenState currentScreen = Home;


String inputValue = "";
int inputValueTMP = 0;
int inputValueTMP2 = 0;

int currentRecept = 0;
int currentCsoport = 0;
int currentIdopont = 0;

int lastTouched = 0;
bool firstTouch = false;

int MaxValveNumb;
bool dataPrint = false;

byte Fenyero = 255;
unsigned long FenyeroCounter = 0;


int currentLayout = 0;
//Keyboard layouts
const char layouts[3][4][10] =
{
  { //Capital letters
    {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'},
    {'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't'},
    {'u', 'v', 'w', 'x', 'y', 'z', '\0', '.', '?', '!'},
    {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}
  },
  { //Lowercase letters
    {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'},
    {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'},
    {'U', 'V', 'W', 'X', 'Y', 'Z', '\0', '.', '?', '!'},
    {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}
  },
  { //Symbols and Numbers
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'},
    {'"', '#', '$', '%', '&', '\'', '(', ')', '*', '+'},
    {',', '-', '/', ':', ';', '<', '=', '>', '@', '['},
    {'\\', ']', '^', '_', '`', '{', '|', '}', '~', '\0'}
  }
};

const char Napok[9][4] = { "He", "Ke", "Sze", "Csu", "Pe", "Szo", "Va", "", "H-V" };


//Hiba uzenet
void ErrorMessage(const char* label, int sor)
{
  gfx->setTextSize(2); gfx->setTextColor(RED);
  gfx->setCursor(0, 20 + sor*50);
  gfx->print(label);
  delay(1500);
}
//Intro kepernyo
void IntroScreen()
{
  //Draw Credentials
  gfx->fillScreen(BLACK);
  gfx->setTextSize(3); gfx->setTextColor(YELLOW);
  gfx->setCursor(20, 120); gfx->print("Ontozorendszer-");
  gfx->setCursor(160, 170); gfx->print("vezerles");
  gfx->setCursor(20, 270); gfx->print("Keszitette:");
  gfx->setCursor(100, 320); gfx->print("Brimo Milad");

  delay(3000);

  //Draw StatusBar and Home
  gfx->fillScreen(BLACK);
  TimeStatus();
  AutoStatus();
  if(!onWiFi) WiFiStatus(false, RED);
  TartalyStatus();
  SolTartStatus();
}
//Kezdo kepernyo
void HomeScreen()
{
  currentScreen = Home;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  //Display Temp and EC indicators
  gfx->setTextSize(2); gfx->setTextColor(WHITE);
  gfx->setCursor(5, 45); gfx->print("EC:");
  gfx->setCursor(82, 45); gfx->print("-->");
  gfx->setCursor(219, 45); gfx->print("VH:");


  //Draw state indicators for 36 valves with labels
  gfx->setTextSize(2); gfx->setTextColor(CYAN);
  gfx->setCursor(5, 77);
  gfx->print("Magnes-szelepek:");
  gfx->setTextSize(1); gfx->setTextColor(WHITE);
  int x = 20; int y = 107;
  for (int i = 0; i < 36; i++)
  {
    if(i < 9) gfx->setCursor(x - 2, y + 13);
    else gfx->setCursor(x - 5, y + 13);
    gfx->print(i + 1); //Label each circle 1-36
    gfx->drawCircle(x, y, 9, WHITE); //Outer circle for background

    x += 35; //Increase spacing for larger circles and labels
    if ((i + 1) % 9 == 0)
    {
      x = 20;
      y += 40;  //Move to next row after 8 valves
    }
  }

  //Draw state indicators for 4 Solutions with labels
  gfx->setTextSize(2); gfx->setTextColor(CYAN);
  gfx->setCursor(5, 260);
  gfx->print("Oldat-parok:");
  gfx->setTextColor(WHITE);
  x = 15; y = 290;
  for (int i = 0; i < 4; i++)
  {
    gfx->setCursor(x - 5, y + 15); //Position label to the left of the circle
    gfx->print(i+1); //Label each circle 1-4
    gfx->print(":A+B");
    gfx->drawCircle(x+24, y, 9, WHITE); gfx->drawCircle(x+47, y, 9, WHITE);  //Outer circle for background

    x += 80; //Increase spacing for larger circles and labels
  }

  //Draw state indicators for 4 Depressurizer with labels
  gfx->setTextSize(2); gfx->setTextColor(CYAN);
  gfx->setCursor(5, 330);
  gfx->print("Nyomas-csokkentok:");
  gfx->setTextColor(WHITE);
  x = 40; y = 360;
  for (int i = 0; i < 4; i++)
  {
    gfx->setCursor(x - 5, y + 15); //Position label to the left of the circle
    gfx->print(i+1); //Label each circle 1-4
    gfx->drawCircle(x, y, 9, WHITE); //Outer circle for background
    
    x += 80; //Increase spacing for larger circles and labels
  }

  ValueIndicators();
  ValveIndicators();

  redButton("STOP", 15); //Draw Menu button
  blueButton("Menu", 20); //Draw STOP button
}
//Menu kepernyo
void MenuScreen()
{
  currentScreen = Menu;
  gfx->fillRect(0, 30, 320, 450, BLACK);
  
  //Draw Ontozesek icon
  gfx->fillRoundRect(35, 60, 100, 100, 20, RED); //Icon background with rounded corners
  gfx->setTextSize(2); gfx->setTextColor(WHITE);
  gfx->setCursor(33, 170); //Position text below icon
  gfx->print("Ontozesek");
  //Draw a clock
  gfx->fillCircle(85, 110, 35, WHITE); //Clock face
  gfx->drawCircle(85, 110, 35, BLACK); //Clock outline
  gfx->drawLine(85, 110, 85, 80, BLACK); //Minute hand
  gfx->drawLine(85, 110, 105, 110, BLACK); //Hour hand
  //Draw a water drop
  gfx->fillCircle(115, 140, 12, BLUE); //Water drop base
  gfx->fillTriangle(115, 120, 103, 136, 127, 136, BLUE); //Drop tip

  //Draw Kezi mod icon
  gfx->fillRoundRect(185, 60, 100, 100, 20, GREEN); //Icon background with rounded corners
  gfx->setTextSize(2); gfx->setTextColor(WHITE);
  gfx->setCursor(188, 170); //Position text below icon
  gfx->print("Kezi mod");
  //Draw a hand
  gfx->fillRoundRect(210, 110, 50, 35, 15, WHITE); //Palm
  gfx->fillRoundRect(251, 80, 9, 45, 10, WHITE); //Pinky
  gfx->fillRoundRect(241, 75, 9, 50, 10, WHITE); //Ring finger
  gfx->fillRoundRect(231, 75, 9, 50, 10, WHITE); //Middle finger
  gfx->fillRoundRect(221, 80, 9, 45, 10, WHITE); //Index finger
  gfx->fillRoundRect(210, 95, 10, 35, 10, WHITE); //Thumb
  //Draw a water drop
  gfx->fillCircle(265, 140, 12, BLUE); //Water drop base
  gfx->fillTriangle(265, 120, 253, 136, 277, 136, BLUE); //Drop tip

  //Draw Naplo icon
  gfx->fillRoundRect(35, 215, 100, 100, 20, YELLOW); //Icon background with rounded corners
  gfx->setTextSize(2); gfx->setTextColor(WHITE);
  gfx->setCursor(56, 325); //Position text below icon
  gfx->print("Naplo");
  //Draw a book
  gfx->fillRect(50, 230, 70, 70, WHITE); //Book body
  gfx->drawRect(50, 230, 71, 71, BLACK); //Outline
  gfx->drawLine(60, 230, 60, 300, BLACK); //Divider
  gfx->drawLine(60, 248, 120, 248, BLACK); //Divider
  gfx->drawLine(60, 282, 120, 282, BLACK); //Divider

  //Draw Beallitasok icon
  gfx->fillRoundRect(185, 215, 100, 100, 20, BLUE); //Icon background with rounded corners
  gfx->setTextSize(2); gfx->setTextColor(WHITE);
  gfx->setCursor(173, 325); //Position text below icon
  gfx->print("Beallitasok");
  //Draw a gear
  for (int i = 0; i < 8; i++)
  {
    float angle = i*(PI/4) + (PI/8); //45 degrees per tooth
    gfx->fillCircle(235 + 25*cos(angle), 265 + 25*sin(angle), 8, WHITE);
    gfx->drawCircle(235 + 25*cos(angle), 265 + 25*sin(angle), 8, BLACK);
  }
  gfx->fillCircle(235, 265, 27, WHITE); //Outer circle
  gfx->drawCircle(235, 265, 20, BLACK); //Draw the inner circle
  gfx->fillCircle(235, 265, 7, BLACK); //Draw the inner dot

  //Draw Fokepernyo icon
  gfx->fillRoundRect(85, 370, 150, 100, 20, CYAN); //Icon background with rounded corners
  gfx->setTextSize(2); gfx->setTextColor(BLACK);
  gfx->setCursor(102, 440); //Position text below icon
  gfx->print("Fokepernyo");
  //Draw a house
  gfx->fillTriangle(125, 405, 195, 405, 160, 380, WHITE); //Roof
  gfx->drawTriangle(125, 405, 195, 405, 160, 380, BLACK); //Roof
  gfx->fillRect(135, 405, 50, 30, WHITE); //House body
  gfx->drawRect(135, 405, 50, 30, BLACK); //House body
  gfx->drawRect(155, 420, 10, 15, BLACK); //Door
}


//Draw functions
void ValueIndicators()
{
  if(currentScreen == Home)
  {
    gfx->setTextSize(2); gfx->setTextColor(CYAN, BLACK);
    gfx->setCursor(41, 45);
    if(istEC <= 10) gfx->print("0");
    if(istEC <= 100) gfx->print("0");
    gfx->print(istEC);

    gfx->setCursor(123, 45);
    if(sollEC <= 10) gfx->print("0");
    if(sollEC <= 100) gfx->print("0");
    gfx->print(sollEC);

    gfx->setCursor(255, 45);
    if(cTemp <= 10) gfx->print("0");
    gfx->print(cTemp);
  }
}

void ValveIndicators()
{
  if(currentScreen == Home)
  {

    //Draw state indicators for 36 valves with labels
    int x = 20; int y = 107;
    for (int i = 0; i < 36; i++)
    {
      if (Valve[i]) gfx->fillCircle(x, y, 7, RED); //Valve on - red circle
      else gfx->fillCircle(x, y, 7, BLACK); //Valve off - black circle

      x += 35; //Increase spacing for larger circles and labels
      if ((i + 1) % 9 == 0)
      {
        x = 20;
        y += 40;  //Move to next row after 8 valves
      }
    }

    //Draw state indicators for 4 Solutions with labels
    x = 15; y = 290;
    for (int i = 0; i < 4; i++)
    {   
      if (i == Solution-1) { gfx->fillCircle(x + 24, y, 7, RED); gfx->fillCircle(x + 47, y, 7, RED); }
      else { gfx->fillCircle(x + 24, y, 7, BLACK); gfx->fillCircle(x + 47, y, 7, BLACK); }

      x += 80; //Increase spacing for larger circles and labels
    }

    //Draw state indicators for 4 Depressurizer with labels
    x = 40; y = 360;
    for (int i = 0; i < 4; i++)
    {   
      if (Depressure[i]) gfx->fillCircle(x, y, 7, RED);
      else gfx->fillCircle(x, y, 7, BLACK);

      x += 80; //Increase spacing for larger circles and labels
    }

  }
}

void TimeStatus()
{
  gfx->setTextSize(3); gfx->setTextColor(CYAN, BLACK);
  gfx->setCursor(5, 5);
  if(hour() < 10) gfx->print("0");
  gfx->print(hour());
  gfx->print(":");
  if(minute() < 10) gfx->print("0");
  gfx->print(minute());

  if( ws.count() ) sendTime();
}

void AutoStatus()
{
  gfx->fillRect(160, 5, 21, 21, BLACK);
  gfx->setTextSize(3);
  gfx->setCursor(163, 5);

  prefs.begin("Settings", false); //Open Preferences in write mode
  prefs.putBool( "AutoMode", AutoMode );
  prefs.end();

  if(AutoMode)
  {
    //Draw the letter "A" in white to indicate connected status
    gfx->setTextColor(BLUE);
    gfx->print("A");
  }
  else
  {
    //Draw the letter "A" in a dimmed color to indicate disconnected status
    gfx->setTextColor(RED);
    gfx->print("A");
    //Draw an "X" over the "A" to indicate disconnection
    gfx->drawLine(160, 5, 180, 25, RED); // "X" line 1
    gfx->drawLine(180, 5, 160, 25, RED); // "X" line 2
  }

  if( ws.count() ) sendAutoMode();
}

void WiFiStatus(bool isConnected, const uint16_t COLOR)
{
  gfx->fillRect(190, 5, 21, 21, BLACK);
  if(isConnected)
  {
    //Draw WiFi connected icon with 3 arcs and a dot
    gfx->drawCircle(200, 21, 2, COLOR);  // Small dot at base
    gfx->drawArc(200, 21, 8, 8, 225, 315, COLOR);  // Smallest arc
    gfx->drawArc(200, 21, 12, 12, 225, 315, COLOR);  // Middle arc
    gfx->drawArc(200, 21, 16, 16, 225, 315, COLOR);  // Largest arc

    if( ws.count() )
    {
      if(COLOR == YELLOW) sendWiFi("yellow");
      if(COLOR == MAGENTA) sendWiFi("magenta");
    }
  }
  else
  {
    //Draw WiFi disconnected icon (with an "X" over the symbol)
    gfx->drawCircle(200, 21, 2, COLOR); //Small dot at base
    gfx->drawArc(200, 21, 8, 8, 225, 315, COLOR); //Smallest arc
    gfx->drawArc(200, 21, 12, 12, 225, 315, COLOR); //Middle arc
    gfx->drawArc(200, 21, 16, 16, 225, 315, COLOR); //Largest arc
    gfx->drawLine(190, 5, 210, 25, COLOR); //"X" line 1
    gfx->drawLine(210, 5, 190, 25, COLOR); //"X" line 2
  }

}

void TartalyStatus()
{ 
  gfx->fillRect(220, 5, 21, 21, BLACK);
  gfx->setTextSize(3);
  gfx->setCursor(223, 5);
  if(Tartaly)
  {
    //Draw the letter "V" in white to indicate connected status
    gfx->setTextColor(BLUE);
    gfx->print("V");
  }
  else
  {
    //Draw the letter "V" in a dimmed color to indicate disconnected status
    gfx->setTextColor(RED);
    gfx->print("V");
    //Draw an "X" over the "V" to indicate disconnection
    gfx->drawLine(220, 5, 240, 25, RED); //"X" line 1
    gfx->drawLine(240, 5, 220, 25, RED); //"X" line 2
  }

  if( ws.count() ) sendTartaly();
}

void SolTartStatus()
{ 
  gfx->fillRect(241, 5, 75, 21, BLACK);
  gfx->setTextSize(3);

  for(int i = 0; i < 4; i++)
  {
    gfx->setCursor(244 + i*18, 5);
    if(SolTart[i])
    {
      //Draw the letter "i+1" in white to indicate connected status
      gfx->setTextColor(BLUE);
      gfx->print(i+1);
    }
    else
    {
    //Draw the letter "i+1" in a dimmed color to indicate disconnected status
      gfx->setTextColor(RED);
      gfx->print(i+1);
      //Draw an "X" over the "i+1" to indicate disconnection
      gfx->drawLine(243 + i*18, 5, 259 + i*18, 25, RED); //"X" line 1
      gfx->drawLine(259 + i*18, 5, 243 + i*18, 25, RED); //"X" line 2
    }
  }

  if( ws.count() ) sendSolTart();
}


void blueButton(const char* label, int posX)
{
  gfx->fillRoundRect(170, 400, 150, 80, 15, BLUE);
  gfx->setTextSize(3); gfx->setTextColor(WHITE);
  gfx->setCursor(190+posX, 430);
  gfx->print(label);
}

void redButton(const char* label, int posX)
{
  gfx->fillRoundRect(0, 400, 150, 80, 15, RED);
  gfx->setTextSize(3); gfx->setTextColor(WHITE);
  gfx->setCursor(25+posX, 430);
  gfx->print(label);
}

void drawTitle(const char* Title, const uint16_t COLOR, int posX)
{
  gfx->setTextColor(COLOR);
  gfx->setTextSize(4);
  gfx->setCursor(50+posX, 45);
  gfx->print(Title);
}

void drawXbyX(const int i, const int XbyX, const uint16_t COLOR, const bool Source)
{
  gfx->setTextSize(3);
  gfx->fillRoundRect(10 + (i%XbyX)*(310/XbyX), 90 + (i/XbyX)*(310/XbyX), (310/XbyX)-10, (310/XbyX)-10, 15, Source ? COLOR : BLACK);
  gfx->drawRoundRect(10 + (i%XbyX)*(310/XbyX), 90 + (i/XbyX)*(310/XbyX), (310/XbyX)-10, (310/XbyX)-10, 15, COLOR);
  if(i < 9) gfx->setCursor(10 + (i%XbyX)*(310/XbyX) + (310/XbyX)/2-12, 90 + (i/XbyX)*(310/XbyX) + (310/XbyX)/2-15);
  else gfx->setCursor(10 + (i%XbyX)*(310/XbyX) + (310/XbyX)/2-21, 90 + (i/XbyX)*(310/XbyX) + (310/XbyX)/2-15);
  gfx->setTextColor(Source ? BLACK : WHITE);
  gfx->print(i+1);
}

void drawXbyX(const int i, const int XbyX, const uint16_t COLOR, const int Source)
{
  gfx->setTextSize(3);
  gfx->fillRoundRect(10 + (i%XbyX)*(310/XbyX), 90 + (i/XbyX)*(310/XbyX), (310/XbyX)-10, (310/XbyX)-10, 15, Source==(i+1) ? COLOR : BLACK);
  gfx->drawRoundRect(10 + (i%XbyX)*(310/XbyX), 90 + (i/XbyX)*(310/XbyX), (310/XbyX)-10, (310/XbyX)-10, 15, COLOR);
  if(i < 9) gfx->setCursor(10 + (i%XbyX)*(310/XbyX) + (310/XbyX)/2-12, 90 + (i/XbyX)*(310/XbyX) + (310/XbyX)/2-15);
  else gfx->setCursor(10 + (i%XbyX)*(310/XbyX) + (310/XbyX)/2-21, 90 + (i/XbyX)*(310/XbyX) + (310/XbyX)/2-15);
  gfx->setTextColor(Source==(i+1) ? BLACK : WHITE);
  gfx->print(i+1);
}

void drawXbyX(const int i, const int XbyX, const uint16_t COLOR, const bool Source, const char Felirat[][4])
{
  gfx->setTextSize(3);
  gfx->fillRoundRect(10 + (i%XbyX)*(310/XbyX), 90 + (i/XbyX)*(310/XbyX), (310/XbyX)-10, (310/XbyX)-10, 15, Source ? COLOR : BLACK);
  gfx->drawRoundRect(10 + (i%XbyX)*(310/XbyX), 90 + (i/XbyX)*(310/XbyX), (310/XbyX)-10, (310/XbyX)-10, 15, COLOR);
  if( strlen(Felirat[i]) < 3) gfx->setCursor(10 + (i%XbyX)*(310/XbyX) + (310/XbyX)/2-21, 90 + (i/XbyX)*(310/XbyX) + (310/XbyX)/2-15);
  else gfx->setCursor(10 + (i%XbyX)*(310/XbyX) + (310/XbyX)/2-30, 90 + (i/XbyX)*(310/XbyX) + (310/XbyX)/2-15);
  gfx->setTextColor(Source ? BLACK : WHITE);
  gfx->print(Felirat[i]);
}

int touchXbyX(const int x, const int y, const int XbyX)
{
  for (int i = 0; i < XbyX*XbyX; i++)
  {
    if(x > 10 + (i%XbyX)*(310/XbyX) && x < 10+(i%XbyX)*(310/XbyX) + (310/XbyX)-10 && y > 90 + (i/XbyX)*(310/XbyX) && y < 90+(i/XbyX)*(310/XbyX) + (310/XbyX)-10) return i;
  }
  return -1;
}

void drawXbutton(const int X, const uint16_t COLOR, const char Felirat[][18])
{
  gfx->setTextSize(3); gfx->setTextColor(WHITE);
  for (int i = 0; i < X; i++)
  {
    gfx->drawRoundRect(10, 90 + i*(310/X), 300, (310/X)-10, 15, COLOR);
    gfx->setCursor(10 + 10, 90 + i*(310/X) + (310/X)/2-15);
    gfx->print(Felirat[i]);
  }
}

int touchXbutton(const int x, const int y, const int X)
{
  for (int i = 0; i < X; i++)
  {
    if(x > 10 && x < 310 && y > 90 + i*(310/X) && y < 90+i*(310/X) + (310/X)-10) return i;
  }
  return -1;
}

void drawKeypad(const int startValue, const uint16_t COLOR)
{
  const char* keys[12] = {"0", "1", "2", "3", "M", "4", "5", "6", "T", "7", "8", "9"};
  gfx->setTextSize(3);
  
  gfx->drawRoundRect(10, 90, 300, 67.5, 15, COLOR);
  for(int i = 0; i < 12; i++)
  {
    gfx->drawRoundRect(10 + (i%4)*77.5, 165 + (i/4)*77.5, 67.5, 67.5, 15, COLOR);
    gfx->setCursor(10 + (i%4)*77.5 + 26, 165 + (i/4)*77.5 + 23);
    gfx->print(keys[i]);
  }

  //Display the current input value
  gfx->setCursor(20, 113); gfx->setTextColor(WHITE);
  gfx->print("Ertek: ");
  gfx->setCursor(130, 113); gfx->setTextColor(CYAN);
  gfx->print(startValue);
  inputValue = startValue;
  firstTouch = true;
}

int touchKeypad(int tx, int ty, const int border)
{ 
  int touchReturn = touchXbyX(tx, ty, 4);
  if(touchReturn >= 4)
  {
    if(firstTouch && touchReturn!=12) { inputValue = ""; firstTouch = false; }

    if(touchReturn == 8) inputValue = ""; //Clear all button

    if(touchReturn == 12) //Clear button
    {
      if( inputValue.length() ) inputValue.remove( inputValue.length()-1, 1 );
      firstTouch = false;
    }

    String tmp = inputValue;
    if(touchReturn >= 4 && touchReturn <= 7) if( (tmp += String(touchReturn-4)).toInt() < border) inputValue += String(touchReturn-4);
    if(touchReturn >= 9 && touchReturn <= 11) if( (tmp += String(touchReturn-5)).toInt() < border) inputValue += String(touchReturn-5);
    if(touchReturn >= 13 && touchReturn <= 15) if( (tmp += String(touchReturn-6)).toInt() < border) inputValue += String(touchReturn-6);

    gfx->fillRect(130, 113, ( (int)log10(border) + 1) * 18, 21, BLACK);
    gfx->setCursor(130, 113); gfx->setTextColor(CYAN);
    gfx->print(inputValue.toInt());
  }

  return inputValue.toInt();
}

void drawKeyboardLayout(const uint16_t COLOR)
{
  gfx->fillRect(0, 150, 320, 250, BLACK);

  int x = 3, y = 150;
  gfx->setTextSize(2);
  for(int row = 0; row < (currentLayout<2 ? 3 : 4); row++)
  {
    for(int col = 0; col < 10; col++)
    {
      gfx->drawRoundRect(x, y, 26, 40, 5, COLOR); //Button border
      gfx->setCursor(x + 8, y + 13);
      gfx->setTextColor(WHITE);
      gfx->print(layouts[currentLayout][row][col]);
      x += 32; //Move to the next button
    }

    x = 3; //Reset x position for the next row
    y += 48; //Move to the next row
  }

   gfx->setTextSize(3);
  //Draw Szokoz button
  if(currentLayout<2)
  {
    gfx->drawRoundRect(68, 296, 186, 40, 5, COLOR);
    gfx->setCursor(108, 304); gfx->setTextColor(WHITE);
    gfx->print("Szokoz");
  }

  //Draw Clear button
  gfx->drawRoundRect(0, 342, 100, 50, 5, COLOR);
  gfx->setCursor(14, 355);
  gfx->setTextColor(WHITE);
  gfx->print("M.T.");
  //Draw Clear all button
  gfx->drawRoundRect(110, 342, 100, 50, 5, COLOR);
  gfx->setCursor(142, 355);
  gfx->print("T.");
  //Draw Layout button
  gfx->drawRoundRect(220, 342, 100, 50, 5, COLOR);
  gfx->setCursor(243, 355);
  gfx->print(currentLayout == 0 ? "ABC" : currentLayout == 1 ? "3@#" : "abc");
}

void drawKeyboard(String startValue, const uint16_t COLOR)
{ 
  //Display the current input value
  gfx->drawRoundRect(5, 90, 310, 50, 15, COLOR);
  gfx->setTextSize(2);
  gfx->setCursor(10, 100); gfx->setTextColor(WHITE);
  gfx->print("Ertek: ");
  gfx->setCursor(82, 100); gfx->setTextColor(CYAN);

  if(startValue.length() < 20) for(int i = 0; i < startValue.length(); i++) gfx->print(startValue[i]);
  else
  {
    for(int i = 0; i < 19; i++) gfx->print(startValue[i]);
    gfx->setCursor(82, 116);
    for(int i = 19; i < startValue.length(); i++) gfx->print(startValue[i]);
  }
  
  currentLayout = 0;
  inputValue = startValue;
  firstTouch = true;

  //Draw keys
  drawKeyboardLayout(COLOR);
}

String touchKeyboard(int tx, int ty)
{

  char touchReturn = '\0';

  int x = 3, y = 150;
  for(int row = 0; row < (currentLayout<2 ? 3 : 4); row++) if( ty >= y + row*48 && ty <= y + row*48 + 40 )
  {
    for(int col = 0; col < 10; col++) if( tx >= x + col*32 && tx <= x + col*32 + 26 )
    {
      touchReturn = layouts[currentLayout][row][col];
      break;
    }
    break;
  }

  if(firstTouch && touchReturn != '\0')
  {
    inputValue = "";
    gfx->fillRect(82, 100, 228, 35, BLACK);
    firstTouch = false;
  }

  String tmp = inputValue + String(touchReturn);
  if(touchReturn != '\0' && tmp.length() <= 38) inputValue += String(touchReturn);
  
  //Szokoz button
  if( tx >= 68 && tx <= 254 && ty >= 296 && ty <= 336 && currentLayout < 2)
  { 
    if(firstTouch)
    {
      inputValue = " ";
      gfx->fillRect(82, 100, 228, 35, BLACK);
      firstTouch = false;
    }
    else inputValue += " ";
  }

  //Clear all button
  if( tx >= 0 && tx <= 100 && ty >= 342 && ty <= 392 )
  {
    inputValue = "";
    gfx->fillRect(82, 100, 228, 35, BLACK);
  }
  //Clear button
  if( tx >= 110 && tx <= 210 && ty >= 342 && ty <= 392 )
  {
    if( inputValue.length() ) inputValue.remove( inputValue.length()-1, 1 );
    gfx->fillRect(82, 100, 228, 35, BLACK);
    firstTouch = false;
  }
  
  gfx->setTextSize(2);
  gfx->setCursor(82, 100); gfx->setTextColor(CYAN, BLACK);
  //Print inputValue
  if(inputValue.length() < 20) for(int i = 0; i < inputValue.length(); i++) gfx->print(inputValue[i]);
  else
  {
    for(int i = 0; i < 19; i++) gfx->print(inputValue[i]);
    gfx->setCursor(82, 116);
    for(int i = 19; i < inputValue.length(); i++) gfx->print(inputValue[i]);
  }

  return inputValue;
}

void drawXtime(const int i, const int X, const uint16_t COLOR, int Hour, int Minute, int posX)
{
  gfx->setTextSize(3); gfx->setTextColor(COLOR, BLACK);
  gfx->setCursor(posX + 150, 90 + i*(310/X) + (310/X)/2-15);
  if(Hour < 10) gfx->print("0");
  gfx->print(Hour);
  gfx->print(":");
  if(Minute < 10) gfx->print("0");
  gfx->print(Minute);
}


//Menu Screen
void OntozesekScreen()
{
  currentScreen = Ontozesek;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Ontozesek", RED, 5); //Draw title
  for (int i = 0; i < 25; i++) drawXbyX(i, 5, RED, Ontozes[i].getOn() ); //Draw 5x5 items

  redButton("Be/Ki", 8); //Draw Be/Ki button
  blueButton("Vissza", 3);//Draw back button
}

void KeziModScreen()
{
  currentScreen = KeziMod;
  gfx->fillRect(0, 30, 320, 450, BLACK);
  drawTitle("Kezi mod", GREEN, 10);

  const char Felirat[4][18] = { "Magnes-szelep", "EC", "Oldat-parok", "Nyomas-csokk." };
  drawXbutton(4, GREEN, Felirat); //Draw list items

  redButton("STOP", 15); //Draw STOP button
  blueButton("Vissza", 3);//Draw back button
}

void NaploScreen()
{
  currentScreen = Naplo;
  gfx->fillRect(0, 30, 320, 450, BLACK);
  gfx->fillRoundRect(0, 400, 150, 80, 15, BLACK);
  drawTitle("Naplo", YELLOW, 50);

  const char Felirat[5][18] = { "Kimaradt", "2", "3", "4", "5" };
  drawXbutton(5, YELLOW, Felirat); //Draw list items

  blueButton("Vissza", 3);//Draw back button
}

void BeallitasokScreen()
{
  currentScreen = Beallitasok;
  gfx->fillRect(0, 30, 320, 450, BLACK);
  drawTitle("Beallitasok", BLUE, -20);

  const char Felirat[5][18] = { "Ora", "WiFi", "EC kalibralas", "Max szelep szam", "Rendszer info" };
  drawXbutton(5, BLUE, Felirat); //Draw list items

  redButton("Ujraind.", -18); //Draw Urjaindit button
  blueButton("Vissza", 3);//Draw back button
}


//Ontozesek Screen
void ReceptScreen()
{
  currentScreen = Recept;
  gfx->fillRect(0, 30, 320, 365, BLACK);
  drawTitle("Ontozes:", RED, -3);
  gfx->setTextColor(Ontozes[currentRecept].getOn() ? RED : WHITE, BLACK );
  gfx->print(currentRecept+1);

  const char Felirat[5][18] = { "Ido", "Magnes-szelep", "EC", "Oldat-parok", "Nyomas-csokk." };
  drawXbutton(5, RED, Felirat); //Draw list items
  redButton("Be/Ki", 8); //Draw Be/Ki button
}

void AutoIdoScreen()
{
  currentScreen = AutoIdo;
  gfx->fillRect(0, 30, 320, 450, BLACK);
  gfx->fillRoundRect(0, 400, 150, 80, 15, BLACK);
  drawTitle("Ido", RED, 74);

  const char Felirat[3][18] = { "Idopontok", "Napok", "Hossz(p)" };
  drawXbutton(3, RED, Felirat); //Draw list items

  blueButton("Vissza", 3);//Draw back button
}

void AutoIdoIdopontScreen()
{
  currentScreen = AutoIdoIdopont;
  gfx->fillRect(0, 30, 320, 450, BLACK);
  gfx->fillRoundRect(0, 400, 150, 80, 15, BLACK);
  drawTitle("Idopontok", RED, 5);

  const char Felirat[5][18] = { "1.Idopont:", "2.Idopont:", "3.Idopont:", "4.Idopont:", "5.Idopont:" };
  drawXbutton(5, RED, Felirat); //Draw list items

  for(int i = 0; i < 5; i++) drawXtime(i, 5, CYAN, Ontozes[currentRecept].getTime(i) / 60, Ontozes[currentRecept].getTime(i) % 60, 50);

  blueButton("Vissza", 3);//Draw back button
}

void AutoIdoOraScreen()
{
  currentScreen = AutoIdoOra;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Ora", RED, 78); //Draw title
  drawKeypad(Ontozes[currentRecept].getTime(currentIdopont)/60, RED); //Draw keypad

  redButton("Tovabb", -4); //Draw Tovabb button
  blueButton("Vissza", 3); //Draw back button
}

void AutoIdoPercScreen()
{
  currentScreen = AutoIdoPerc;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Perc", RED, 66); //Draw title
  drawKeypad(Ontozes[currentRecept].getTime(currentIdopont)%60, RED); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void AutoIdoNapokScreen()
{
  currentScreen = AutoIdoNapok;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Napok", RED, 52); //Draw title
  for(int i = 0; i < 9; i++) drawXbyX(i, 3, RED, Ontozes[currentRecept].getDay(i), Napok); //Draw 3x3

  blueButton("Vissza", 3); //Draw back button
}

void AutoIdoHosszScreen()
{
  currentScreen = AutoIdoHossz;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Hossz(p)", RED, 20); //Draw title
  drawKeypad(Ontozes[currentRecept].getDuration(), RED); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void AutoSzelepScreen()
{
  currentScreen = AutoSzelep;
  gfx->fillRect(0, 30, 320, 365, BLACK);
  gfx->fillRoundRect(0, 400, 150, 80, 15, BLACK);

  drawTitle("Csoportok", RED, 5); //Draw title
  for (int i = 0; i < 9; i++) drawXbyX(i, 3, RED, !Ontozes[currentRecept].emptyGroup(i)); //Draw 3x3 items
}

void AutoSzelepCsoportScreen()
{
  currentScreen = AutoSzelepCsoport;
  gfx->fillRect(0, 30, 320, 365, BLACK);

  drawTitle("Csoport:", RED, -3); //Draw title
  gfx->setTextColor( Ontozes[currentRecept].emptyGroup(currentCsoport) ? WHITE : RED, BLACK );
  gfx->print(currentCsoport+1);

  for (int i = 0; i < 36; i++) drawXbyX(i, 6, RED, Ontozes[currentRecept].getValve(currentCsoport, i) ); //Draw 6x6 items
}

void AutoEcScreen()
{
  currentScreen = AutoEC;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("EC", RED, 90); //Draw title
  drawKeypad(Ontozes[currentRecept].getEC(), RED); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void AutoOldatScreen()
{
  currentScreen = AutoOldat;
  gfx->fillRect(0, 30, 320, 365, BLACK);
  gfx->fillRoundRect(0, 400, 150, 80, 15, BLACK);

  drawTitle("Oldat-parok", RED, -20); //Draw title
  for(int i = 0; i < 4; i++) drawXbyX( i, 2, RED, Ontozes[currentRecept].getSolution() ); //Draw 2x2 items

  lastTouched = Ontozes[currentRecept].getSolution();
}

void AutoNyomasScreen()
{
  currentScreen = AutoNyomas;
  gfx->fillRect(0, 30, 320, 365, BLACK);
  gfx->fillRoundRect(0, 400, 150, 80, 15, BLACK);

  drawTitle("Nyomas-csokk.", RED, -45); //Draw title
  for (int i = 0; i < 4; i++) drawXbyX(i, 2, RED, Ontozes[currentRecept].getDepressure(i)); //Draw 2x2 items
}


//Kezi mod Screen
void KeziSzelepScreen()
{
  currentScreen = KeziSzelep;
  gfx->fillRect(0, 30, 320, 365, BLACK);

  drawTitle("Magnes-szelep", GREEN, -45); //Draw title
  for (int i = 0; i < 36; i++) drawXbyX(i, 6, GREEN, Valve[i]); //Draw 6x6 items
}

void KeziEcScreen()
{
  currentScreen = KeziEc;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("EC", GREEN, 90); //Draw title
  drawKeypad(sollEC, GREEN); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void KeziOldatScreen()
{
  currentScreen = KeziOldat;
  gfx->fillRect(0, 30, 320, 365, BLACK);

  drawTitle("Oldat-parok", GREEN, -20); //Draw title
  for(int i = 0; i < 4; i++) drawXbyX( i, 2, GREEN, Solution); //Draw 2x2 items
}

void KeziNyomasScreen()
{
  currentScreen = KeziNyomas;
  gfx->fillRect(0, 30, 320, 365, BLACK);

  drawTitle("Nyomas-csokk.", GREEN, -45); //Draw title
  for (int i = 0; i < 4; i++) drawXbyX(i, 2, GREEN, Depressure[i]); //Draw 2x2 items
}


//Beallitasok Screen
void BeallitOraScreen()
{
  currentScreen = BeallitOra;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Ora", BLUE, 74); //Draw title

  const char Felirat[4][18] = { "Ido", "Datum", "Idozona", "Teli/Nyari" };
  drawXbutton(4, BLUE, Felirat); //Draw list items

  blueButton("Vissza", 3); //Draw back button
}

void BeallitOraIdoScreen()
{
  currentScreen = BeallitOraIdo;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Ora", BLUE, 74); //Draw title
  drawKeypad(hour(), BLUE); //Draw keypad

  redButton("Tovabb", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitOraIdo2Screen()
{
  currentScreen = BeallitOraIdo2;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Perc", BLUE, 62); //Draw title
  drawKeypad(minute(), BLUE); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitOraDatumScreen()
{
  currentScreen = BeallitOraDatum;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Ev", BLUE, 86); //Draw title
  drawKeypad(year(), BLUE); //Draw keypad

  redButton("Tovabb", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitOraDatum2Screen()
{
  currentScreen = BeallitOraDatum2;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Honap", BLUE, 50); //Draw title
  drawKeypad(month(), BLUE); //Draw keypad

  redButton("Tovabb", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitOraDatum3Screen()
{
  currentScreen = BeallitOraDatum3;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Nap", BLUE, 74); //Draw title
  drawKeypad(day(), BLUE); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitOraZonaScreen()
{
  currentScreen = BeallitOraZona;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Idozona GMT+", BLUE, -30); //Draw title
  drawKeypad(gmtOffset, BLUE); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitOraTNYScreen()
{
  currentScreen = BeallitOraTNY;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Teli0/Nyari1", BLUE, -34); //Draw title
  drawKeypad(daylightOffset, BLUE); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitWiFiScreen()
{
  currentScreen = BeallitWiFi;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("WiFi", BLUE, 62); //Draw title
  
  if(onWiFi) gfx->fillRoundRect(10, 90, 300, 93, 15, BLUE);
  const char Felirat[3][18] = { "WiFi Be/Ki", "SSID", "Jelszo" };
  drawXbutton(3, BLUE, Felirat); //Draw list items

  blueButton("Vissza", 3); //Draw back button
}

void BeallitWiFiSSIDScreen()
{
  currentScreen = BeallitWiFiSSID;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("SSID", BLUE, 62); //Draw title

  drawKeyboard(ssid, BLUE);

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitWiFiPWScreen()
{
  currentScreen = BeallitWiFiPW;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Jelszo", BLUE, 38); //Draw title
 
  drawKeyboard(password, BLUE);

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitECScreen()
{
  currentScreen = BeallitEC;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("EC kalibralas", BLUE, -44); //Draw title
  //drawWhat?

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitMaxScreen()
{
  currentScreen = BeallitMax;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Max sz. szam", BLUE, -32); //Draw title
  drawKeypad(MaxValveNumb, BLUE); //Draw keypad

  redButton("Mentes", -4); //Draw Mentes button
  blueButton("Vissza", 3); //Draw back button
}

void BeallitInfoScreen()
{
  currentScreen = BeallitInfo;
  dataPrint = true;
  gfx->fillRect(0, 30, 320, 450, BLACK);

  drawTitle("Rendszer info", BLUE, -44); //Draw title
  
  BeallitInfoScreenData();

  blueButton("Vissza", 3); //Draw back button
}

void BeallitInfoScreenData()
{
  gfx->setTextSize(2); gfx->setTextColor(BLUE, BLACK);

  gfx->setCursor(5, 100); gfx->print("Futasi ido: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( millis()/86400000 ); //Napok
  gfx->print(":"); gfx->print( (millis()%86400000) / 3600000 ); //Orak
  gfx->print(":"); gfx->print( (millis()%3600000) / 60000 );  //Percek
  gfx->print(":"); gfx->print( (millis()%60000) / 1000 ); //Masodpercek

  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 120); gfx->print("RAM:"); //RAM
  gfx->setTextColor(CYAN, BLACK); gfx->print( ESP.getFreeHeap()/1000 );
  gfx->print( "/" ); gfx->print( ESP.getHeapSize()/1000 );
  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(149, 120); gfx->print("ROM:"); //ROM
  gfx->setTextColor(CYAN, BLACK); gfx->print( ESP.getSketchSize()/1000 );
  gfx->print( "/" ); gfx->print( ESP.getFlashChipSize()/1000 );


  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 160); gfx->print("Rendszer e.h.n: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( year() );
  gfx->print("."); gfx->print( month() );
  gfx->print("."); gfx->print( day() );

  gfx->setTextColor(BLUE, BLACK);gfx->setCursor(5, 180); gfx->print("Rendszer o:p:mp: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( hour() ); 
  gfx->print(":"); gfx->print( minute() );
  gfx->print(":"); gfx->print( second() );

  DateTime now = rtc.now();
  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 220); gfx->print("RTC homerseklet: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( rtc.getTemperature() );

  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 240); gfx->print("RTC e.h.n: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( now.year() );
  gfx->print("."); gfx->print( now.month() );
  gfx->print("."); gfx->print( now.day() );

  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 260); gfx->print("RTC o:p:mp: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( now.hour() ); 
  gfx->print(":"); gfx->print( now.minute() );
  gfx->print(":"); gfx->print( now.second() );

  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 300); gfx->print("MAC-cim: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( WiFi.macAddress() );

  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 320); gfx->print("IP cim: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( WiFi.localIP() );

  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 340); gfx->print("WiFi jel erosseg: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( WiFi.RSSI() );

  gfx->setTextColor(BLUE, BLACK); gfx->setCursor(5, 360); gfx->print("WiFi SSID: ");
  gfx->setTextColor(CYAN, BLACK); gfx->print( WiFi.SSID() );
}


//Touch return
void Touched()
{
  TS_Point p = tp.getPoint();
  int tx = 320 - (p.x - 260) / 11.25;
  int ty = 480 - (p.y - 190) / 7.7;

  if(Fenyero < maxFenyero) { Fenyero = maxFenyero; analogWrite(TFT_LED, Fenyero); }
  FenyeroCounter = millis();
  
  int touchReturn = -1;
  int input = 0;
  String input_S = "";
  bool AllOn = true;


  switch (currentScreen)
  {
    case Home:
      if(tx > 170 && tx < 320 && ty > 400 && ty < 480) MenuScreen(); //Menu button 
      if(tx > 0 && tx < 150 && ty > 400 && ty < 480) { allSTOP(); ValueIndicators(); AutoMode = false; AutoStatus(); }  //STOP button
    break;

    case Menu:
      if (tx > 40 && tx < 140 && ty > 60 && ty < 160) OntozesekScreen();
      if (tx > 180 && tx < 280 && ty > 60 && ty < 160) KeziModScreen();
      if (tx > 40 && tx < 140 && ty > 210 && ty < 310) NaploScreen();
      if (tx > 180 && tx < 280 && ty > 210 && ty < 310) BeallitasokScreen();
      if (tx > 90 && tx < 230 && ty > 360 && ty < 460) HomeScreen();
    break;

  //Menu Screen
    case Ontozesek:      
    {
      touchReturn = touchXbyX(tx, ty, 5);
      if(touchReturn >= 0)
      {
        currentRecept = touchReturn;
        ReceptScreen(); //Kivalasztott recept
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) MenuScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { AutoMode = !AutoMode; AutoStatus(); } //Be/Ki button
    }
    break;

    case KeziMod:
      touchReturn = touchXbutton(tx, ty, 4);
      if (touchReturn == 0) KeziSzelepScreen(); //Kezi-agak button
      if (touchReturn == 1) KeziEcScreen(); //EC button
      if (touchReturn == 2) KeziOldatScreen(); //Oldat-parok button
      if (touchReturn == 3) KeziNyomasScreen(); //Nyomas-csokk. button

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) MenuScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) allSTOP(); //STOP button
    break;

    case Naplo:
      touchReturn = touchXbutton(tx, ty, 5);
      //if (touchReturn == 0) //1 button
      //if (touchReturn == 1) //2 button
      //if (touchReturn == 2) //3 button
      //if (touchReturn == 3) //4 button
      //if (touchReturn == 4) //5 button

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) MenuScreen(); //Back button
    break;

    case Beallitasok:
      touchReturn = touchXbutton(tx, ty, 5);
      if (touchReturn == 0) BeallitOraScreen(); //Ora button
      if (touchReturn == 1) BeallitWiFiScreen(); //WiFi button
      if (touchReturn == 2) BeallitECScreen(); //EC kalibralas button
      if (touchReturn == 3) BeallitMaxScreen(); //Max szelep szam button
      if (touchReturn == 4) BeallitInfoScreen(); //Info button

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) MenuScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) ESP.restart(); //Ujraindit button
    break;


  //Ontozesek Screen
    case Recept:
      touchReturn = touchXbutton(tx, ty, 5);
      if (touchReturn == 0) AutoIdoScreen(); //Ido button
      if (touchReturn == 1) AutoSzelepScreen(); //Magnes-szelep button
      if (touchReturn == 2) AutoEcScreen(); //EC button
      if (touchReturn == 3) AutoOldatScreen(); //Oldat-parok button
      if (touchReturn == 4) AutoNyomasScreen(); //Nyomas-csokk. button

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) OntozesekScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) //Be/Ki button
      {
        Ontozes[currentRecept].setOn( !Ontozes[currentRecept].getOn() );
        gfx->setTextSize(4);
        gfx->setCursor(239, 45);
        gfx->setTextColor(Ontozes[currentRecept].getOn() ? RED : WHITE, BLACK );
        gfx->print(currentRecept+1);
      }
    break;

    case AutoIdo:
      touchReturn = touchXbutton(tx, ty, 3);
      if (touchReturn == 0) AutoIdoIdopontScreen(); //Idopontok button
      if (touchReturn == 1) AutoIdoNapokScreen(); //Napok button
      if (touchReturn == 2) AutoIdoHosszScreen(); //Hossz button

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) ReceptScreen(); //Back button
    break;

    case AutoIdoIdopont:
      touchReturn = touchXbutton(tx, ty, 5);
      if(touchReturn >= 0)
      {
        currentIdopont = touchReturn;
        AutoIdoOraScreen();
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) AutoIdoScreen(); //Back button
    break;

    case AutoIdoOra:
      input = touchKeypad(tx, ty, 24);
      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) AutoIdoIdopontScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { inputValueTMP = input; AutoIdoPercScreen(); } //Tovabb button
    break;

    case AutoIdoPerc:
      input = touchKeypad(tx, ty, 60);
      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) AutoIdoIdopontScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) //Mentes button
      {
        if( (60*inputValueTMP+input) != 0 && !Ontozes[currentRecept].getOn() ) Ontozes[currentRecept].setOn(true);
        Ontozes[currentRecept].setTime( currentIdopont, 60*inputValueTMP + input );
        AutoIdoIdopontScreen();
      }
    break;

    case AutoIdoNapok:
      touchReturn = touchXbyX(tx, ty, 3);
      if(touchReturn >= 0 && touchReturn < 7)
      {
        Ontozes[currentRecept].setDay( touchReturn, !Ontozes[currentRecept].getDay(touchReturn) );
        drawXbyX(touchReturn, 3, RED, Ontozes[currentRecept].getDay(touchReturn), Napok);
      }
      
      AllOn = true;
      for(int i = 0; i < 7; i++) if( !Ontozes[currentRecept].getDay(i) ) { AllOn = false; break; }
      Ontozes[currentRecept].setDay(8, AllOn);
      drawXbyX(8, 3, RED, Ontozes[currentRecept].getDay(8), Napok);

      if(touchReturn == 8) //H-V button
      {
        bool hvState = Ontozes[currentRecept].getDay(8);
        for(int i = 0; i < 7; i++) Ontozes[currentRecept].setDay(i, !hvState);
        Ontozes[currentRecept].setDay(8, !hvState);
        AutoIdoNapokScreen();
      }

      if(tx > 170 && tx < 320 && ty > 400 && ty < 480) AutoIdoScreen(); //Back button
    break;

    case AutoIdoHossz:
      input = touchKeypad(tx, ty, 1000);
      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) AutoIdoScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { Ontozes[currentRecept].setDuration(input); AutoIdoScreen(); } //Mentes button
    break;

    case AutoSzelep:
      touchReturn = touchXbyX(tx, ty, 3);
      if(touchReturn >= 0)
      {
        currentCsoport = touchReturn;
        AutoSzelepCsoportScreen();
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) ReceptScreen(); //Back button
    break;

    case AutoSzelepCsoport:
      touchReturn = touchXbyX(tx, ty, 6);
      if(touchReturn >= 0)
      {
        if(Ontozes[currentRecept].getValve(currentCsoport, touchReturn) )
        {
          Ontozes[currentRecept].setValve(currentCsoport, touchReturn, false);
          drawXbyX(touchReturn, 6, RED, Ontozes[currentRecept].getValve(currentCsoport, touchReturn));
        }
        else
        {
          int cnt = 0;
          for(int i = 0; i < 36; i++) if( Ontozes[currentRecept].getValve(currentCsoport, i) ) cnt++;
          if(cnt < MaxValveNumb)
          {
            Ontozes[currentRecept].setValve(currentCsoport, touchReturn, true);
            drawXbyX(touchReturn, 6, RED, Ontozes[currentRecept].getValve(currentCsoport, touchReturn));
          }
        }

        gfx->setTextSize(4);
        gfx->setCursor(239, 45);
        gfx->setTextColor( Ontozes[currentRecept].emptyGroup(currentCsoport) ? WHITE : RED, BLACK );
        gfx->print(currentCsoport+1);
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) AutoSzelepScreen(); //Back button
    break;

    case AutoEC:
      input = touchKeypad(tx, ty, 1000);
      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) ReceptScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { Ontozes[currentRecept].setEC(input); ReceptScreen(); } //Mentes button
    break;

    case AutoOldat:
      touchReturn = touchXbyX(tx, ty, 2);
      if(touchReturn >= 0)
      {
        if(touchReturn+1 == Ontozes[currentRecept].getSolution()) Ontozes[currentRecept].setSolution(0);
        else Ontozes[currentRecept].setSolution(touchReturn+1);
        drawXbyX(touchReturn, 2, RED, Ontozes[currentRecept].getSolution() );
        if(lastTouched) drawXbyX(lastTouched-1, 2, RED, Ontozes[currentRecept].getSolution() );
        lastTouched = touchReturn+1;
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) { lastTouched = 0; ReceptScreen();} //Back button
    break;

    case AutoNyomas:
      touchReturn = touchXbyX(tx, ty, 2);
      if(touchReturn >= 0)
      {
        Ontozes[currentRecept].setDepressure( touchReturn, !Ontozes[currentRecept].getDepressure(touchReturn) );
        drawXbyX(touchReturn, 2, RED, Ontozes[currentRecept].getDepressure(touchReturn));
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) ReceptScreen(); //Back button
    break;


  //Kezi Screen
    case KeziSzelep:
      touchReturn = touchXbyX(tx, ty, 6);
      if(touchReturn >= 0)
      {
        if( Valve[touchReturn] )
        {
          Valve[touchReturn] = false;
          drawXbyX(touchReturn, 6, GREEN, Valve[touchReturn]);
        }
        else
        {
          int cnt = 0;
          for(int i = 0; i < 36; i++) if( Valve[i] ) cnt++;
          if(cnt < MaxValveNumb)
          {
            Valve[touchReturn] = true;
            drawXbyX(touchReturn, 6, GREEN, Valve[touchReturn]);
          }
        }
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) KeziModScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { allSTOP(); KeziSzelepScreen(); } //STOP button
    break;

    case KeziEc:
      input = touchKeypad(tx, ty, 1000);
      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) KeziModScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { sollEC = input; KeziModScreen(); } //Mentes button
    break;

    case KeziOldat:
      touchReturn = touchXbyX(tx, ty, 2);
      if(touchReturn >= 0)
      {
        if(touchReturn+1 == Solution) Solution = 0;
        else Solution = touchReturn + 1; 
        drawXbyX(touchReturn, 2, GREEN, Solution);
        if(lastSolution) drawXbyX(lastSolution-1, 2, GREEN, Solution);
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) KeziModScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { allSTOP(); KeziOldatScreen(); } //STOP button
    break;

    case KeziNyomas:
      touchReturn = touchXbyX(tx, ty, 2);
      if(touchReturn >= 0)
      {
        Depressure[touchReturn] = !Depressure[touchReturn];
        drawXbyX(touchReturn, 2, GREEN, Depressure[touchReturn]);
      }

      if (tx > 170 && tx < 320 && ty > 400 && ty < 480) KeziModScreen(); //Back button
      if (tx > 0 && tx < 150 && ty > 400 && ty < 480) { allSTOP(); KeziNyomasScreen(); } //STOP button
    break;


  //Beallitasok Screen
  case BeallitOra:
    touchReturn = touchXbutton(tx, ty, 4);
    if(touchReturn == 0) BeallitOraIdoScreen(); //Ido button
    if(touchReturn == 1) BeallitOraDatumScreen(); //Datum button
    if(touchReturn == 2) BeallitOraZonaScreen(); //Idozona button
    if(touchReturn == 3) BeallitOraTNYScreen(); //Teli/Nyari button
    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitasokScreen(); //Back button
  break;

  case BeallitOraIdo:
    input = touchKeypad(tx, ty, 24);
    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitOraScreen(); //Back button
    if(tx > 0 && tx < 150 && ty > 400 && ty < 480) { inputValueTMP = input; BeallitOraIdo2Screen(); } //Tovabb button
  break;

  case BeallitOraIdo2:
    input = touchKeypad(tx, ty, 60);
    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitOraScreen(); //Back button
    if(tx > 0 && tx < 150 && ty > 400 && ty < 480)  //Mentes button
    {
      DateTime now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), inputValueTMP, input, 0));
      setTime( inputValueTMP, input, 0, day(), month(), year() );

      updateTime();
      BeallitOraScreen();
    }
  break;

  case BeallitOraDatum:
    input = touchKeypad(tx, ty, 2039);
    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitOraScreen(); //Back button
    if(tx > 0 && tx < 150 && ty > 400 && ty < 480) { inputValueTMP = input; BeallitOraDatum2Screen(); } //Tovabb button
  break;

  case BeallitOraDatum2:
    input = touchKeypad(tx, ty, 13);
    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitOraScreen(); //Back button
    if(tx > 0 && tx < 150 && ty > 400 && ty < 480) { inputValueTMP2 = input; BeallitOraDatum3Screen(); } //Tovabb button
  break;

  case BeallitOraDatum3:
    input = touchKeypad(tx, ty, 32);
    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitOraScreen(); //Back button
    if(tx > 0 && tx < 150 && ty > 400 && ty < 480) //Tovabb button
    {
      DateTime now = rtc.now();
      rtc.adjust(DateTime( inputValueTMP, inputValueTMP2, input, now.hour(), now.minute(), now.second() ));
      setTime( hour(), minute(), second(), input, inputValueTMP2, inputValueTMP );

      updateTime();
      BeallitOraScreen();
    }
  break;

  case BeallitOraZona:
    input = touchKeypad(tx, ty, 24);
    if (tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitOraScreen(); //Back button
    if (tx > 0 && tx < 150 && ty > 400 && ty < 480) //Mentes button
    {
      DateTime now = rtc.now();

      int newHour = now.hour() + input-gmtOffset;
      if(newHour >= 24) newHour -= 24;
      if(newHour < 0) newHour += 24;
      rtc.adjust(DateTime( now.year(), now.month(), now.day(), newHour , now.minute(), now.second() ));

      newHour = hour() + input-gmtOffset;
      if(newHour >= 24) newHour -= 24;
      if(newHour < 0) newHour += 24;
      setTime( newHour , minute(), second(), day(), month(), year() );

      gmtOffset = input;

      prefs.begin("Settings", false); //Open Preferences in write mode
      prefs.putInt( "gmtOffset", input );
      prefs.end();

      updateTime();
      BeallitOraScreen();
    }
  break;

  case BeallitOraTNY:
    input = touchKeypad(tx, ty, 2);
    if (tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitOraScreen(); //Back button
    if (tx > 0 && tx < 150 && ty > 400 && ty < 480) //Mentes button
    {
      DateTime now = rtc.now();

      int newHour = now.hour() + input-daylightOffset;
      if(newHour >= 24) newHour -= 24;
      if(newHour < 0) newHour += 24;
      rtc.adjust(DateTime( now.year(), now.month(), now.day(), newHour , now.minute(), now.second() ));

      newHour = hour() + input-daylightOffset;
      if(newHour >= 24) newHour -= 24;
      if(newHour < 0) newHour += 24;
      setTime( newHour , minute(), second(), day(), month(), year() );

      daylightOffset = input;

      prefs.begin("Settings", false); //Open Preferences in write mode
      prefs.putInt( "daylightOffset", input );
      prefs.end();

      updateTime();
      BeallitOraScreen();
    }
  break;

  case BeallitWiFi:
    touchReturn = touchXbutton(tx, ty, 3);
    if (touchReturn == 0) //WiFi Be/Ki button
    {
      onWiFi = !onWiFi;

      prefs.begin("Settings", false); //Open Preferences in write mode
      prefs.putBool( "onWiFi", onWiFi );
      prefs.end();

      if(onWiFi)
      {
        WiFi.begin( ssid.c_str(), password.c_str() );
        unsigned long currentMillis = millis();
        while(WiFi.status() != WL_CONNECTED) { delay(500); if( millis() - currentMillis > 5000 ) break; }
        WiFiStatus(true, YELLOW);
        configTime(3600*gmtOffset, 3600*daylightOffset, ntpServer); //Initialize NTP
        currentMillis = millis();
        while(time(nullptr) < 946684800) { delay(500); if( millis() - currentMillis > 5000 ) break; }
        updateTime();
      }

      else
      {
        WiFi.disconnect();
        delay(100);
        WiFiStatus(false, RED);
      }

      BeallitWiFiScreen();
    }
    if (touchReturn == 1) BeallitWiFiSSIDScreen(); //SSID button
    if (touchReturn == 2) BeallitWiFiPWScreen(); //Jelszo button

    if (tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitasokScreen(); //Back button
  break;

  case BeallitWiFiSSID:
    input_S = touchKeyboard(tx, ty);

    if( tx >= 220 && tx <= 320 && ty >= 342 && ty <= 392 ) //Layout button
    {
      currentLayout++;
      if(currentLayout >= 3) currentLayout = 0;
      drawKeyboardLayout(BLUE);
    }

    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitWiFiScreen(); //Back button
    if(tx > 0 && tx < 150 && ty > 400 && ty < 480) //Mentes button
    {
      ssid = input_S;

      prefs.begin("Settings", false); //Open Preferences in write mode
      prefs.putString( "ssid", ssid );
      prefs.end();

      BeallitWiFiScreen();
    }
  break;

  case BeallitWiFiPW:
    input_S = touchKeyboard(tx, ty);

    if( tx >= 220 && tx <= 320 && ty >= 342 && ty <= 392 ) //Layout button
    {
      currentLayout++;
      if(currentLayout >= 3) currentLayout = 0;
      drawKeyboardLayout(BLUE);
    }

    if(tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitWiFiScreen(); //Back button
    if(tx > 0 && tx < 150 && ty > 400 && ty < 480) //Mentes button
    {
      password = input_S;

      prefs.begin("Settings", false); //Open Preferences in write mode
      prefs.putString( "password", password );
      prefs.end();

      BeallitWiFiScreen();
    }
  break;

  case BeallitEC:
    //input = touchWhat?(tx, ty, ??);
    if (tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitasokScreen(); //Back button
    if (tx > 0 && tx < 150 && ty > 400 && ty < 480) BeallitasokScreen(); //Mentes button
  break;

  case BeallitMax:
    input = touchKeypad(tx, ty, 37);
    if (tx > 170 && tx < 320 && ty > 400 && ty < 480) BeallitasokScreen(); //Back button
    if (tx > 0 && tx < 150 && ty > 400 && ty < 480) //Mentes button
    {
      MaxValveNumb = input;

      prefs.begin("Settings", false); //Open Preferences in write mode
      prefs.putInt( "MaxValveNumb", input );
      prefs.end();

      BeallitasokScreen();
    }
  break;

  case BeallitInfo:
    if (tx > 170 && tx < 320 && ty > 400 && ty < 480) { dataPrint = false; BeallitasokScreen(); } //Back button
  break;

  }

}