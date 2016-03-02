#include <stdio.h>
#include <Arduino.h>
#include "Clock.h"

void Time::setDateTime(uint16_t y, uint8_t m, uint8_t d, uint8_t hr, uint8_t min, uint8_t sec) {
  TimeElements tmE;
  tmE.Year = y - 1970;
  tmE.Month = m;
  tmE.Day = d;
  tmE.Hour = hr;
  tmE.Minute = min;
  tmE.Second = sec;
  setSeconds(makeTime(tmE));
}

uint8_t Time::hourFormat12() {
  return ::hourFormat12(getSeconds());
}

uint8_t Time::hour() {
  return ::hour(getSeconds());
}

uint8_t Time::minute() {
  return ::minute(getSeconds());
}

uint8_t Time::second() {
  return ::second(getSeconds());
}

uint16_t Time::year() {
  return ::year(getSeconds());
}

uint8_t Time::month() {
  return ::month(getSeconds());
}

uint8_t Time::day() {
  return ::day(getSeconds());
}

uint8_t Time::weekday() {
  return ::weekday(getSeconds());
}

bool Time::isAM() {
  return ::isAM(getSeconds());
}

void Time::shortTime(char * timeStr) {
  sprintf(timeStr, "%d:%02d %s", hourFormat12(), minute(), isAM() ? "am":"pm");
};

void Time::longTime(char * timeStr) {
  sprintf(timeStr, "%d:%02d:%02d %s", hourFormat12(), minute(), second(), isAM() ? "am":"pm");
};

static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

// todo: internationalization & localization of names, reuse DateStrings.cpp if possible (the table is not exposed currently)
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
  if (m == 0) {
    m = month();
    if (m==2) {
      int16_t y = year();
      if ((y % 4) != 0) return 28;
      if ((y % 100) != 100) return 29;
      if ((y % 400) != 400) return 28;
      return 29;
    }
  }

  return monthDays[m-1]; // m is 1 based, the array is zero based
}

void Time::longDate(char* dateStr) {
  sprintf(dateStr, "%s, %s %d, %d", dayStrings[weekday()], monthStrings[month()], day(), year());
}

void Time::shortDate(char* dateStr) {
  sprintf(dateStr, "%d-%02d-%02d", year(), month(), day());
}

void Time::setMicros(micros_t newTime) {
  microsTime = newTime;
}

void Time::setSeconds(time_t newTime)  {
  microsTime = newTime * microsPerSec;
  // why does calling setMicros here crash on Teensy3?
  setMicros( microsTime );
  microsTime++;
}

void Time::setMillis(millis_t newTime) {
  setMicros( ((millis_t)newTime) * microsPerMilli );
}

// Use separate rollover calculations for millis() and micros() for efficiency.
// todo: Find a simpler solution

millis_t lastUptimeMillis = 0;
millis_t uptimeOffsetMillis = 0;
micros_t lastUptimeMicros = 0;
micros_t uptimeOffsetMicros = 0;

micros_t Uptime::micros() {
  micros_t nowUptime = ::micros();
  if (nowUptime < lastUptimeMicros) {
    // micros have rolled over, we add a new offset
    uptimeOffsetMillis += 0x0000000100000000;
  }
  lastUptimeMicros = nowUptime;
  return nowUptime + uptimeOffsetMicros;
}

millis_t Uptime::millis() {
  millis_t nowUptime = ::millis();
  if (nowUptime < lastUptimeMillis) {
    // millis have rolled over, we add a new offset
    uptimeOffsetMillis += 0x0000000100000000;
  }
  lastUptimeMillis = nowUptime;
  return nowUptime + uptimeOffsetMillis;
}

time_t DayTime::nextOccurance(time_t starting) {
  time_t nextup = (starting/secsPerDay)*secsPerDay + getSeconds();
  if (nextup < starting) {
    nextup+= secsPerDay;
  }
  return nextup;
};

// real time clock methods
time_t Clock::getSeconds() {

  time_t now_sec = ::now();

// todo: this is wrong, calibration should happen at a finer scale than getSeconds()
//  if (now_sec != last_sec) {
//    micros_offset = Uptime::micros();
//    last_sec = now_sec;
//  }

  return now_sec;
}

void Clock::setMicros(micros_t newTime) {
  ::setTime(newTime/microsPerSec);
  micros_offset = Uptime::micros() - newTime%microsPerSec;
  doneSet = true;
}

uint32_t Clock::fracMicros() {
  getSeconds();
  return Uptime::micros() - micros_offset;
}

Clock::Clock() {
}


#if defined(CORE_TEENSY)
time_t getTeensyRTCTime()
{
  return Teensy3Clock.get();
//  return 0;
}

TeensyRTCClock::TeensyRTCClock() {
  setSyncProvider(getTeensyRTCTime);
  if (timeStatus()!= timeSet || year() < 2015) {
    // set clock to a recent time - not needed if the RTC is set
    // a recent time seems more friendly than 1970, though 1970 was a pretty friendly year.
    // disabling this because it might just be dumb
    //::setTime(16,20,0,1,1,2015);
  } else {
    doneSet = true;
  }
}

void TeensyRTCClock::setMicros(micros_t newTime) {
    Teensy3Clock.set(newTime/microsPerSec);
    Clock::setMicros(newTime);
    // force a resync
    setSyncProvider(getTeensyRTCTime);
    doneSet = true;
}

#endif
