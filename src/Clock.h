#ifndef _Clock_
#define _Clock_

#include "inttypes.h"
#include "Print.h"
#include "Timezone.h"


#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef uint32_t time_t;
#endif
// note that on some platforms time_t is predefined to be 64 bits in length

// signed time (in seconds) for relative time, deltas, adjustments, etc.
typedef int32_t stime_t;

// milliseconds are always expressed as 64-bit numbers, to avoid rollover
typedef int64_t millis_t;

// microseconds are always expressed as 64-bit numbers, to avoid rollover
typedef int64_t micros_t;

// Time is a base class that represents a point in time and provides utility functions for getting information about that time
// Time does not change unless you set() it.
// Use Clock (or one of its descendents) for a real-time clock.
// Use Uptime for a clock that keeps track of time since boot.
class Time {
  public:
    Time() {};
    Time(Time* t) { setMicros(t->getMicros()); }

    virtual micros_t getMicros();// microseconds since 1970-01-01
    virtual void setMicros(micros_t newTime);
    virtual void adjustMicros(micros_t adjustment) {    setMicros(getMicros()+adjustment); }  // signed delta micros

    inline time_t getSeconds() { return getMicros()/microsPerSec; } // seconds since 1970-01-01
    inline millis_t getMillis() { return getMicros()/microsPerMilli; }  // milliseconds since 1970-01-01

    inline millis_t millis() { return getMillis(); } // alias
    inline millis_t micros() { return getMicros(); } // alias

    uint32_t fracMicros() { return getMicros()%microsPerSec; } // microseconds since the last second expired
    uint16_t fracMillis() { return getMillis()%millisPerSec; } // milliseconds since the last second expired

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

    void shortTime(Print& p);
    void shortDate(Print& p);
    void longTime(Print& p);
    void longDate(Print& p);

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
    static const time_t secsPerMinute = secsPerMin;
    static const time_t secsPerHour = 60L*60;
    static const time_t secsPerDay = 60L*60*24;
    static const time_t secsPerYear = 60L*60*24*365;  // approximately

    static const millis_t millisPerSec = 1000;
    static const millis_t millisPerMin = secsPerMin*millisPerSec;
    static const millis_t millisPerMinute = millisPerMin;
    static const millis_t millisPerHour = secsPerHour*millisPerSec;
    static const millis_t millisPerDay = secsPerDay*millisPerSec;
    static const millis_t millisPerYear = secsPerYear*millisPerSec;

    static const micros_t microsPerMilli = 1000;
    static const micros_t microsPerSec = 1000000;
    static const micros_t microsPerMin = secsPerMin*microsPerSec;
    static const micros_t microsPerMinute = microsPerMin;
    static const micros_t microsPerHour = secsPerHour*microsPerSec;
    static const micros_t microsPerDay = secsPerDay*microsPerSec;
    static const micros_t microsPerYear = secsPerYear*microsPerSec;
  protected:
    micros_t _micros_time = 0;
};

// DayTime provides a time of day for a single day, it's time_t value can be in the range 0 to secsPerDay.  Useful for daily recurring alarms.
class DayTime : public Time {
  public:
    virtual void setMicros(micros_t newTime) {  Time::setMicros(newTime % interval()); }
    virtual void adjustMicros(micros_t adjustment)  { if (adjustment < 0) { adjustment = interval()-((-adjustment)%interval()); } setMicros(adjustment+getMicros()); };
    virtual bool isTime(time_t newTime) { return getSeconds() == (newTime % (interval()/microsPerSec)); }
    virtual time_t nextOccurance(time_t starting);
  protected:
    virtual micros_t interval() { return microsPerDay; }
};

// YearTime provides a time of day for a single year, it's time_t value can be in the range 0 to secsPerYear.  Useful for yearly events (birthdays, DST, etc...)
class YearTime : public DayTime {
  protected:
    virtual micros_t interval() { return microsPerYear; }
};


// Uptime provides a Time that is tied to the micros() since the system started.  Easiest access is by Uptime::micros() or Uptime::millis()
// Setting has no effect.
class Uptime : public Time {
  public:
    static micros_t micros();
    static inline millis_t millis() { return micros()/microsPerMilli; }
    static inline time_t seconds() { return millis()/millisPerSec; }
    static void longTime(Print& p);


    micros_t getMicros() { return micros(); }

  private:
    static micros_t lastUptimeMicros;
    static micros_t uptimeOffsetMicros;
};

class LocalTime : public Time {
  // LocalTime's internal time is in the base (typically UTC) time, then the offset is applied to it
  public:
    LocalTime() {};

    virtual micros_t getMicros() { return Time::getMicros() + getZoneOffset(); };
    virtual void setMicros(micros_t newTime) {
      micros_t off = 0;
      if (_zone) {
        time_t newSecs = newTime / microsPerSec;
        off = microsPerSec * _zone->offset(_zone->toUTC(newSecs));
      }
      Time::setMicros(newTime - off);
    };

    void setZone(Timezone* zone) { _zone = zone; }
    inline Timezone* getZone(void) { return _zone; }

    stime_t getZoneOffset() { return _zone ? _zone->offset(getSeconds()) : 0; }
    TimeChangeRule* getZoneRule() { return _zone ? _zone->rule(getSeconds()) : nullptr; }

  protected:
      Timezone* _zone = nullptr;
};

// RTCClock is an abstract class that provides a Time
// that progresses in real time, for use with an RTC
//
// Clock should be a platform-specific RTC-backed clock, derived from this class, when available.
//
// note that the UTC time is shared across all Clocks, which means that there should be only
// one RTC per system (for now)

class RTCClock : public LocalTime {

  public:
    virtual micros_t getMicros();
    virtual void setMicros(micros_t newTime);

    virtual bool hasBeenSet();
    virtual void beginSetTime() { _is_setting = true;}
    virtual void endSetTime() { _is_setting = false; }

    virtual micros_t getRTCMicros() = 0;
    virtual void setRTCMicros(micros_t newTime) = 0;
    virtual void updateTime() = 0;

    time_t lastUpdate() { return _last_update/microsPerSec; }

    void setUpdateInterval(time_t i) { _update_interval = i * microsPerSec; }
    time_t getUpdateInterval() { return _update_interval / microsPerSec; }

  protected:
    static micros_t _micros_offset;
    static bool _is_setting;
    static micros_t _update_interval;
    static micros_t _last_update;
    static micros_t _utc_micros_time;
};

#if defined(CORE_TEENSY)
// On Teensy, the Clock is tied to the Teensy3 RTC
class TeensyClock : public RTCClock {
  public:
    TeensyClock();
    TeensyClock(Timezone* zone) { setZone(zone); }
    virtual void updateTime();
    virtual void setMicros(micros_t newTime);

    micros_t getRTCMicros();
    void setRTCMicros(micros_t newTime);
};

class Clock : public TeensyClock {
};

#else
// we have no RTC, therefore updateTime doesn't do anything
class Clock : public RTCClock {
  public:
    Clock(Timezone* zone) { setZone(zone); }
    Clock() {};

    virtual void updateTime() { };
    virtual micros_t getRTCMicros() { return getMicros(); };
    virtual void setRTCMicros(micros_t newTime) { setMicros(newTime); };
};
#endif

#endif
