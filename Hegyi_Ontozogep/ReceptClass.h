#ifndef ReceptClass_h
#define ReceptClass_h


class Receptek
{
  private:
    bool On;
    int Time[5];
    bool Day[9];
    int Duration;
    bool R_Valve[9][36];
    int EC;
    bool R_Depressure[4];
    int R_Solution;
    int empty_cnt = 0;
    int currentOntozes;


  public:
    //Default constructor
    Receptek();
    //Constructor
    Receptek(int CurrentOntozes);

    //Setters
    void setOn(bool state);
    void setTime(int index, int time);
    void setDay(int index, bool state);
    void setDuration(int duration);
    void setValve(int index1, int index2, bool state);
    void setDepressure(int index, bool state);
    void setEC(int ec);
    void setSolution(int solution);

    //Getters
    bool getOn();
    int getTime(int index);
    bool getDay(int index);
    int getDuration();
    bool getValve(int index1, int index2);
    int getEC();
    bool getDepressure(int index);
    int getSolution();

    //Method to check time and start event
    void checkTime(int currentTime, int currentDay);

    //Check if Group is empty
    bool emptyGroup(int j);
};


#endif