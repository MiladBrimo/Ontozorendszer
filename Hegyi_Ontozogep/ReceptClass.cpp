#include "ReceptClass.h"
#include "Globals.h"


bool EventOn[25];
Preferences prefs;


//Default constructor
Receptek::Receptek() {}

//Constructor
Receptek::Receptek(int CurrentOntozes)
{
  currentOntozes = CurrentOntozes;
  prefs.begin("Recept", true); //Open Preferences in read-only mode

  On = prefs.getBool( (String(currentOntozes) + "-On").c_str(), false );
  for(int i = 0; i < 5; i++) Time[i] = prefs.getInt( (String(currentOntozes) + "-Time_" + String(i)).c_str(), 0 );
  for(int i = 0; i < 9; i++) Day[i] = prefs.getBool( (String(currentOntozes) + "-Day_" + String(i)).c_str(), false);
  Duration = prefs.getInt( (String(currentOntozes) + "-Duration").c_str(), 0 );
  for(int i = 0; i < 9; i++) for(int j = 0; j < 36; j++) R_Valve[i][j] = prefs.getBool( (String(currentOntozes) + "-R_Valve_" + String(i) + "_" + String(j)).c_str(), false);
  EC = prefs.getInt( (String(currentOntozes) + "-EC").c_str(), 0 );
  for(int i = 0; i < 4; i++) R_Depressure[i] = prefs.getBool( (String(currentOntozes) + "-R_Depress_" + String(i)).c_str(), false);
  R_Solution = prefs.getInt( (String(currentOntozes) + "-R_Solution").c_str(), 0 );

  prefs.end();
  delay(20);
}


//Setter methods
void Receptek::setOn(bool state)
{
  On = state;
  prefs.begin("Recept", false); //Open Preferences in write mode
  prefs.putBool( (String(currentOntozes) + "-On").c_str(), state );
  prefs.end();
}
void Receptek::setTime(int index, int time)
{
  if(index >= 0 && index < 5)
  {
    Time[index] = time;
    prefs.begin("Recept", false); //Open Preferences in write mode
    prefs.putInt( (String(currentOntozes) + "-Time_" + String(index)).c_str(), time );
    prefs.end();
  }
}
void Receptek::setDay(int index, bool state)
{
  if(index >= 0 && index < 9)
  {
    Day[index] = state;
    prefs.begin("Recept", false); //Open Preferences in write mode
    prefs.putBool( (String(currentOntozes) + "-Day_" + String(index)).c_str(), state);
    prefs.end();
  }
}
void Receptek::setDuration(int duration)
{
  Duration = duration;
  prefs.begin("Recept", false); //Open Preferences in write mode
  prefs.putInt( (String(currentOntozes) + "-Duration").c_str(), duration );
  prefs.end();
}
void Receptek::setValve(int index1, int index2, bool state)
{
  if(index1>=0 && index1<9 && index2>=0 && index2<36)
  {
    R_Valve[index1][index2] = state;
    prefs.begin("Recept", false); //Open Preferences in write mode
    prefs.putBool( (String(currentOntozes) + "-R_Valve_" + String(index1) + "_" + String(index2)).c_str(), state);
    prefs.end();
  }
}
void Receptek::setEC(int ec)
{
  EC = ec;
  prefs.begin("Recept", false); //Open Preferences in write mode
  prefs.putInt( (String(currentOntozes) + "-EC").c_str(), ec );
  prefs.end();
}
void Receptek::setDepressure(int index, bool state)
{
  if(index >= 0 && index < 4)
  {
    R_Depressure[index] = state;
    prefs.begin("Recept", false); //Open Preferences in write mode
    prefs.putBool( (String(currentOntozes) + "-R_Depress_" + String(index)).c_str(), state);
    prefs.end();
  }
}
void Receptek::setSolution(int solution)
{
  R_Solution = solution;
  prefs.begin("Recept", false); //Open Preferences in write mode
  prefs.putInt( (String(currentOntozes) + "-R_Solution").c_str(), solution );
  prefs.end();
}

//Getter methods
bool Receptek::getOn() { return On; }
int Receptek::getTime(int index) { if(index >= 0 && index < 5) return Time[index]; else return 0; }
bool Receptek::getDay(int index) { if(index >= 0 && index < 9) return Day[index]; else return 0; }
int Receptek::getDuration() { return Duration; }
bool Receptek::getValve(int index1, int index2) { if(index1>=0 && index1<9 && index2>=0 && index2<36) return R_Valve[index1][index2]; else return 0; }
int Receptek::getEC() { return EC; }
bool Receptek::getDepressure(int index) { if(index >= 0 && index < 4) return R_Depressure[index]; else return 0; }
int Receptek::getSolution() { return R_Solution; }


//Check if it's time to start an event
void Receptek::checkTime(int currentTime, int currentDay)
{
  if(currentDay >= 2 && currentDay <= 7) currentDay -= 2;
  else currentDay = 6;
  
  if(On && Day[currentDay]) for(int i = 0; i < 5; i++)
  {
    for(int j = 0; j < 10; j++)
    {
      if(j >= 9 && currentTime == Time[i] + (j-empty_cnt)*Duration && EventOn[currentOntozes])
      {
        EventOn[currentOntozes] = false;

        bool Stop = true;
        for(int k = 0; k < 25; k++) if(EventOn[k]) { Stop = false; break; }
        if(Stop) allSTOP();
      }

      if(j >= 9) empty_cnt = 0;    
      else
      {
        if( emptyGroup(j) ) empty_cnt++;

        if(!emptyGroup(j) && currentTime == Time[i] + (j-empty_cnt)*Duration)
        {
          sollEC = EC;
          for(int k = 0; k < 4; k++) Depressure[k] = R_Depressure[k];
          Solution = R_Solution;
          for(int k = 0; k < 36; k++) Valve[k] = R_Valve[j][k];

          for(int k = 0; k < 25; k++) if(EventOn[k]) { EventOn[k] = false; break; }
          EventOn[currentOntozes] = true; 
        }
      }

    }
  }

}


//Check if Group is empty
bool Receptek::emptyGroup(int j)
{
  for(int k = 0; k < 36; k++) if(R_Valve[j][k]) return false;
  return true;
}