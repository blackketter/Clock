#ifndef _Clock_
#define _Clock_

#include "TimeLib.h"

typedef int64_t millis_t;
typedef int32_t stime_t; // signed time for relative time, deltas, adjustments, etc.

class Time {
  public:
    Time() {};
    virtual time_t get() { return curTime; };
    virtual void set(time_t newTime) { curTime = newTime; }

    virtual void set(uint16_t y, uint8_t m = 1, uint8_t d = 1, uint8_t hr = 0, uint8_t min = 0, uint8_t sec = 0);
    virtual void adjust(stime_t adjustment) { set(get() + adjustment); } // signed time
    virtual bool isTime(time_t newTime) { return newTime == get(); }

    virtual void beginSetTime() {};
    virtual void endSetTime() {};

    void shortTime(char* timeStr);
    void longTime(char* timeStr);
    void longDate(char* dateStr);
    void shortDate(char* dateStr);

    bool isAM();
    uint8_t hourFormat12();
    uint8_t hour();
    uint8_t minute();
    uint8_t second();
    uint16_t year();
    uint8_t month();
    uint8_t day();
    uint8_t weekday();
    const char* weekdayString(uint8_t d = 0 );  // zero means current day
    const char* monthString(uint8_t m = 0); // zero means current month

    uint8_t daysInMonth(uint8_t m = 0);  // zero means current month, months are 1 based
    const time_t secsPerMin = 60;
    const time_t secsPerHour = 60*60;
    const time_t secsPerDay = 60*60*24;
    const time_t secsPerYear = 60*60*24*365;

  protected:
    time_t curTime = 0;

};

// can only be set to a time of day, i.e. [0, secsPerDay)
class DayTime : public Time {
  public:
    virtual void set(time_t newTime) {  Time::set(newTime % secsPerDay); }
    virtual void adjust(stime_t adjustment)  { if (adjustment < 0) { adjustment = secsPerDay-((-adjustment)%secsPerDay); } set(adjustment+get()); };
    virtual bool isTime(time_t newTime) { return get() == (newTime % secsPerDay); }
    virtual time_t nextOccurance();
};

class Uptime : public Time {
  public:
    static millis_t millis();
    time_t get() { return millis()/1000; }
};

class Clock : public Time {

  public:
    Clock();
    time_t now();

    virtual void set(time_t newTime);
    virtual time_t get() { return now(); };
    virtual void adjust(stime_t adjustment); // signed time

    uint16_t frac();  // fractional seconds in millis  TODO: Make base Time class support fractional seconds too

    virtual bool hasBeenSet() { return doneSet && !setting; }
    virtual void beginSetTime() { setting = true;};
    virtual void endSetTime() { setting = false; } ;

  private:
    millis_t millis_offset = 0;

    bool doneSet = false;
    bool setting = false;
};

extern Clock clock;

#endif
