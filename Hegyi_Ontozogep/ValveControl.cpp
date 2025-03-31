#include "ValveControl.h"
#include "Globals.h"
#include "MenuSystem.h"


Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;
Adafruit_MCP23X17 mcp3;


bool AutoMode;

bool Valve[36];
bool lastValve[36];

int Solution;
int lastSolution;

bool Depressure[4];
bool lastDepressure[4];

bool Motor = false;
bool lastMotor = false;

bool Tartaly = true;
bool lastTartaly = true;
bool SolTart[4] = {true, true, true, true};
bool lastSolTart[4] = {true, true, true, true};

int istEC;
int sollEC;
float cTemp;


void ValveChange()
{
  bool change = false;
  for (int i = 0; i < 36; i++)
  {
    if (Valve[i] != lastValve[i])
    {
      lastValve[i] = Valve[i];
      if(i < 16) mcp1.digitalWrite(i, Valve[i]);
      else if(i < 32) mcp2.digitalWrite(i-16, Valve[i]);
      else mcp3.digitalWrite(i-32, Valve[i]);
      change = true;
    }
  }

  bool AllOff = true;
  for (int i = 0; i < 36; i++) if(Valve[i]) { AllOff = false; break; }
  Motor = !AllOff;

  if (Solution != lastSolution)
  {
    lastSolution = Solution;
    if (!Solution)
      for (int i = 0; i < 4; i++) digitalWrite(SolutionPins[i], LOW);
    change = true;
  }

  for (int i = 0; i < 4; i++)
  {
    if (Depressure[i] != lastDepressure[i])
    {
      lastDepressure[i] = Depressure[i];
      mcp3.digitalWrite(i+8, Depressure[i]);
      change = true;
    }
  }

  Tartaly = !digitalRead(TartalyPin);
  for(int i = 0; i < 4; i++) SolTart[i] = !mcp3.digitalRead(i+4);

  if (Motor != lastMotor && Tartaly)
  {
    lastMotor = Motor;
    mcp3.digitalWrite(15, Motor);
    if (!Motor)
      for (int i = 0; i < 4; i++) digitalWrite(SolutionPins[i], LOW);
  }

  if (Tartaly != lastTartaly)
  {
    lastTartaly = Tartaly;
    TartalyStatus();
    if (!Tartaly)
    {
      mcp3.digitalWrite(15, LOW);
      for (int i = 0; i < 4; i++) digitalWrite(SolutionPins[i], LOW);
    }
    else mcp3.digitalWrite(15, Motor);
  }

  for(int i = 0; i < 4; i++) if(SolTart[i] != lastSolTart[i])
  {
    lastSolTart[i] = SolTart[i];
    SolTartStatus();
    if(!SolTart[i]) digitalWrite(SolutionPins[i], LOW);
  }

  if (Solution && Motor && Tartaly && SolTart[Solution]) EC_Regler();

  if (change) ValveIndicators();
}


void allSTOP()
{
  for (int i = 0; i < 36; i++) Valve[i] = 0;
  sollEC = 0;
  for (int i = 0; i < 4; i++) Solution = 0;
  for (int i = 0; i < 4; i++) Depressure[i] = 0;
}


void EC_Regler()
{
  //Timing with millis()?
  for (int i = 0; i < 4; i++)
  {
    if (i == Solution) digitalWrite(SolutionPins[i], HIGH);
    else digitalWrite(SolutionPins[i], LOW);
  }
}