#ifndef _Clock_
#define _Clock_

#include "TimeLib.h"

// signed time for relative time, deltas, adjustments, etc.
typedef int32_t stime_t;

// milliseconds are always expressed as 64-bit numbers, to avoid rollover
typedef int64_t millis_t;

// microseconds are always expressed as 64-bit numbers, to avoid rollover
typedef int64_t micros_t;

// Time is a base class that represents a time and provides utility functions for getting information about that time
// Time does not change unless you set() it.
// Use Clock (or one of its descendents) for a real-time clock.
// Use Uptime for a clock that keeps track of time since boot.
class Time {

  public:
    Time() {};

    virtual micros_t getMicros();// microseconds since 1970-01-01
    virtual void setMicros(micros_t newTime);
    virtual void adjustMicros(micros_t adjustment) {    setMicros(getMicros()+adjustment); }  // signed delta micros

    time_t getSeconds() { return getMicros()/microsPerSec; } // seconds since 1970-01-01
    millis_t getMillis() { return getMicros()/microsPerMilli; }  // milliseconds since 1970-01-01
    uint32_t fracMicros() { return getMicros()%microsPerSec; } // microseconds since the last second expired
    uint16_t fracMillis() { return fracMicros()/microsPerMilli; } // microseconds since the last second expired

    virtual void setMillis(millis_t newTime) { setMicros(newTime * microsPerMilli); }
    virtual void setSeconds(time_t newTime) { setMicros(newTime * microsPerSec); }

    // convenience for the old syntax
    time_t now() { return getSeconds(); }

    virtual void setDateTime(uint16_t y, uint8_t m = 1, uint8_t d = 1, uint8_t hr = 0, uint8_t min = 0, uint8_t sec = 0);
    virtual void adjustSeconds(stime_t adjustment) { adjustMicros(microsPerSec * adjustment); } // signed time
    virtual void adjustMillis(millis_t adjustment) { adjustMicros(millisPerSec * adjustment); }  // signed delta millis

    virtual bool isTime(time_t newTime) { return newTime == getSeconds(); }

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

    static const time_t secsPerMin = 60L;
    static const time_t secsPerHour = 60L*60;
    static const time_t secsPerDay = 60L*60*24;
    static const time_t secsPerYear = 60L*60*24*365;  // approximately

    static const millis_t millisPerSec = 1000;
    static const millis_t millisPerMin = secsPerMin*millisPerSec;
    static const millis_t millisPerHour = secsPerHour*millisPerSec;
    static const millis_t millisPerDay = secsPerDay*millisPerSec;
    static const millis_t millisPerYear = secsPerYear*millisPerSec;

    static const micros_t microsPerMilli = 1000;
    static const micros_t microsPerSec = 1000000;
    static const micros_t microsPerMin = secsPerMin*microsPerSec;
    static const micros_t microsPerHour = secsPerHour*microsPerSec;
    static const micros_t microsPerDay = secsPerDay*microsPerSec;
    static const micros_t microsPerYear = secsPerYear*microsPerSec;
  protected:
    micros_t microsTime = 0;
};

// DayTime provides a time of day for a single day, it's value can be in the range 0 to secsPerDay.  Useful for daily recurring alarms.
class DayTime : public Time {
  public:
    virtual void setMicros(micros_t newTime) {  Time::setMicros(newTime % microsPerDay); }
    virtual void adjustMicros(micros_t adjustment)  { if (adjustment < 0) { adjustment = microsPerDay-((-adjustment)%microsPerDay); } setMicros(adjustment+getMicros()); };
    virtual bool isTime(time_t newTime) { return getSeconds() == (newTime % secsPerDay); }
    virtual time_t nextOccurance(time_t starting);
};

// Uptime provides a Time that is tied to the micros() since the system started.  Easiest access is by Uptime::millis() or Uptime::micros()
// Setting has no effect.
class Uptime : public Time {
  public:
    millis_t getMillis() { return millis(); }
    micros_t getMicros() { return micros(); }

    static millis_t millis();
    static micros_t micros();
};

// Clock provides a Time that progresses in real time, without a separate RTC, assumes the RTC only has second resolution
class Clock : public Time {

  public:
    virtual micros_t getMicros();
    virtual void setMicros(micros_t newTime);

    virtual bool hasBeenSet() { return doneSet && !setting; }
    virtual void beginSetTime() { setting = true;};
    virtual void endSetTime() { setting = false; } ;

  protected:
    micros_t micros_offset = 0;
    time_t last_sec = 0;

    bool doneSet = false;
    bool setting = false;
};

#if defined(CORE_TEENSY)
// TeensyRTCClock provides a Clock that is tied to the Teensy3 RTC. (Todo: Verify if this will work with the Teensy 2.0++)
class TeensyRTCClock : public Clock {
  public:
    TeensyRTCClock();
    virtual void setMicros(micros_t newTime);
};
#endif

#endif
