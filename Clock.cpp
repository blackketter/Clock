#include <stdio.h>
#include <Arduino.h>

#include "Clock.h"

void Time::set(uint16_t y, uint8_t m, uint8_t d, uint8_t hr, uint8_t min, uint8_t sec) {
  TimeElements tmE;
  tmE.Year = y - 1970;
  tmE.Month = m;
  tmE.Day = d;
  tmE.Hour = hr;
  tmE.Minute = min;
  tmE.Second = sec;
  set(makeTime(tmE));
}

uint8_t Time::hourFormat12() {
  return ::hourFormat12(get());
}

uint8_t Time::hour() {
  return ::hour(get());
}

uint8_t Time::minute() {
  return ::minute(get());
}

uint8_t Time::second() {
  return ::second(get());
}

uint16_t Time::year() {
  return ::year(get());
}

uint8_t Time::month() {
  return ::month(get());
}

uint8_t Time::day() {
  return ::day(get());
}

uint8_t Time::weekday() {
  return ::weekday(get());
}

bool Time::isAM() {
  return ::isAM(get());
}

void Time::shortTime(char * timeStr) {
  sprintf(timeStr, "%d:%02d %s", hourFormat12(), minute(), isAM() ? "am":"pm");
};

void Time::longTime(char * timeStr) {
  sprintf(timeStr, "%d:%02d:%02d %s", hourFormat12(), minute(), second(), isAM() ? "am":"pm");
};

static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
static const char* dayStrings[] = { "", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char* monthStrings[] = { "", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

const char* Time::weekdayString(uint8_t d) {
  if (d == 0) { d = weekday(); }
  return dayStrings[d];
}

const char* Time::monthString(uint8_t m) {
  if (m == 0) { m = month();}
  return monthStrings[m];
}

uint8_t Time::daysInMonth(uint8_t m) {
  // todo: make this work for leap years
  if (m == 0) { m = month(); }
  return monthDays[m-1]; // m is 1 based, the array is zero based
}

void Time::longDate(char* dateStr) {
  sprintf(dateStr, "%s, %s %d, %d", dayStrings[weekday()], monthStrings[month()], day(), year());
}

void Time::shortDate(char* dateStr) {
  sprintf(dateStr, "%d-%02d-%02d", year(), month(), day());
}

millis_t Uptime::millis() {
  static millis_t lastUptime = 0;
  static millis_t uptimeOffset = 0;

  millis_t nowUptime = ::millis();
  if (nowUptime < lastUptime) {
    // millis have rolled over, we add a new offset
    uptimeOffset += 0x0000000100000000;
  }
  lastUptime = nowUptime;
  return nowUptime + uptimeOffset;
}

time_t DayTime::nextOccurance() {
  time_t nextup = (clock.now()/secsPerDay)*secsPerDay + get();
  if (nextup < clock.now()) {
    nextup+= secsPerDay;
  }
  return nextup;
};

// real time clock methods
time_t Clock::now() {
  static time_t last_sec = 0;

  time_t now_sec = ::now();

  if (now_sec != last_sec) {
    millis_offset = Uptime::millis();
    last_sec = now_sec;
  }

  return now_sec;
}

uint16_t Clock::frac() {
  now();

  return Uptime::millis() - millis_offset;
}

time_t getRTCTime()
{
  return Teensy3Clock.get();
}

Clock::Clock() {
    setSyncProvider(getRTCTime);
    if (timeStatus()!= timeSet || year() < 2015) {
//      DEBUG_LN("Unable to sync with the RTC");
      // set clock to a recent time - not needed with RTC
      ::setTime(16,20,0,1,1,2015);
    } else {
//      DEBUG_LN("RTC has set the system time");
      doneSet = true;
    }
}

void Clock::set(time_t newTime) {
  Teensy3Clock.set(newTime);
  ::setTime(newTime);
}


void Clock::adjust(stime_t adjustment) {
  Teensy3Clock.set(now() + adjustment);

  // force a resync
  setSyncProvider(getRTCTime);

  doneSet = true;

}

