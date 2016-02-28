#ifndef _Clock_
#define _Clock_

#include "TimeLib.h"

// milliseconds are always expressed as 64-bit numbers, to avoid rollover
typedef int64_t millis_t;

// signed time for relative time, deltas, adjustments, etc.
typedef int32_t stime_t;


// Time is a base class that is abased time and has utility functions for getting information about that time
// Time does not change unless you set() it.   Use Clock (or one of its descendents) for a real-time clock.
class Time {
  public:
    Time() {};

    virtual time_t get() { return millisTime/1000; } // seconds since 1970-01-01
    virtual millis_t getMillis() { return millisTime; }  // milliseconds since 1970-01-01
    virtual uint16_t frac() { return millisTime%1000; } // milliseconds since the last second expired

    virtual void set(time_t newTime) { millisTime = (millis_t)newTime * 1000; }
    virtual void setMillis(millis_t newTime) { millisTime = newTime; }

    virtual void set(uint16_t y, uint8_t m = 1, uint8_t d = 1, uint8_t hr = 0, uint8_t min = 0, uint8_t sec = 0);
    virtual void adjust(stime_t adjustment) { set(get() + adjustment); } // signed time
    virtual void adjustMillis(millis_t adjustment) {  setMillis(getMillis()+adjustment); }  // signed delta millis

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

    const time_t secsPerMin = 60L;
    const time_t secsPerHour = 60L*60;
    const time_t secsPerDay = 60L*60*24;
    const time_t secsPerYear = 60L*60*24*365;

  protected:
    millis_t millisTime = 0;

};

// DayTime provides a time of day for a single day, it's value can be in the range 0 to secsPerDay.  Useful for daily recurring alarms.
class DayTime : public Time {
  public:
    virtual void set(time_t newTime) {  Time::set(newTime % secsPerDay); }
    virtual void adjust(stime_t adjustment)  { if (adjustment < 0) { adjustment = secsPerDay-((-adjustment)%secsPerDay); } set(adjustment+get()); };
    virtual bool isTime(time_t newTime) { return get() == (newTime % secsPerDay); }
    virtual time_t nextOccurance(time_t starting);
};

// Uptime provides a Time that is tied to the millis() since the system started.  Easiest access is by Uptime::millis()
class Uptime : public Time {
  public:
    static millis_t millis();
    time_t get() { return millis()/1000; }
    millis_t getMillis() { return millis(); }
};

// Clock provides a Time that progresses in real time, without a separate RTC
class Clock : public Time {

  public:
    Clock();

    time_t get();
    millis_t getMillis() { return get()*1000+frac(); }
    uint16_t frac();  // fractional seconds in millis

    // convenience for the old syntax
    time_t now() { return get(); }
    millis_t nowMillis() { return getMillis(); }

    void set(time_t newTime);
    void setMillis(millis_t newTime) { set(newTime/1000);  millis_offset = Uptime::millis() - newTime%1000;  }

    virtual void adjust(stime_t adjustment); // signed delta seconds

    virtual bool hasBeenSet() { return doneSet && !setting; }
    virtual void beginSetTime() { setting = true;};
    virtual void endSetTime() { setting = false; } ;

  protected:
    millis_t millis_offset = 0;

    bool doneSet = false;
    bool setting = false;
};

#if defined(CORE_TEENSY)
// TeensyRTCClock provides a Clock that is tied to the Teensy3 RTC. (Todo: Verify if this will work with the Teensy 2.0++)
class TeensyRTCClock : public Clock {
  public:
    TeensyRTCClock();
    void set(time_t newTime) { Teensy3Clock.set(newTime);  Clock::set(newTime); }
    void adjust(stime_t adjustment); // signed time
};
#endif

#endif
