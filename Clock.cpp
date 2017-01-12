#include <stdio.h>
#include <Arduino.h>
#include "Clock.h"

/////////////////////
// From TimeLib:
typedef struct  {
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;   // offset from 1970;
} 	tmElements_t, TimeElements;

static tmElements_t tm;          // a cache of time elements
static time_t cacheTime;   // the time the cache was updated

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void breakTime(time_t timeInput, tmElements_t &tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.Month = month + 1;  // jan is month 1
  tm.Day = time + 1;     // day of month
}

time_t makeTime(tmElements_t &tm){
// assemble time elements into time_t
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.Year*(Time::secsPerDay * 365);
  for (i = 0; i < tm.Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  Time::secsPerDay;   // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.Year)) {
      seconds += Time::secsPerDay * 29;
    } else {
      seconds += Time::secsPerDay * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.Day-1) * Time::secsPerDay;
  seconds+= tm.Hour * Time::secsPerHour;
  seconds+= tm.Minute * Time::secsPerMin;
  seconds+= tm.Second;
  return (time_t)seconds;
}

/*============================================================================*/
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */
void refreshCache(time_t t) {
  if (t != cacheTime) {
    breakTime(t, tm);
    cacheTime = t;
  }
}

int hour(time_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;
}

int hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if( tm.Hour == 0 )
    return 12; // 12 midnight
  else if( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

uint8_t isPM(time_t t) { // returns true if PM
  return (hour(t) >= 12);
}

uint8_t isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);
}

int minute(time_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;
}

int second(time_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}

int day(time_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return tm.Day;
}

int weekday(time_t t) {
  refreshCache(t);
  return tm.Wday;
}

int month(time_t t) {  // the month for the given time
  refreshCache(t);
  return tm.Month;
}

int year(time_t t) { // the year for the given time
  refreshCache(t);
  return tm.Year + 1970;
}


///////////////////////////
micros_t Time::getMicros() {
  return _micros_time;
}

void Time::setMicros(micros_t newTime) {
  _micros_time = newTime;
}

void Time::setDateTime(uint16_t y, uint8_t m, uint8_t d, uint8_t hr, uint8_t min, uint8_t sec) {
//todo: clean this up
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
  uint8_t h = getSeconds() % (secsPerDay/2) / secsPerHour;
  if (h == 0) { h = 12; };
  return h;
}

uint8_t Time::hour() {
  return getSeconds() % secsPerDay / secsPerHour;
}

bool Time::isAM() {
    return hour() < 12;
}

uint8_t Time::minute() {
    return getSeconds() % secsPerHour / secsPerMin;
}

uint8_t Time::second() {
  return getSeconds() % secsPerMin;
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

void Time::shortTime(char * timeStr) {
  sprintf(timeStr, "%d:%02d %s", hourFormat12(), minute(), isAM() ? "am":"pm");
};

void Time::longTime(char * timeStr) {
  sprintf(timeStr, "%d:%02d:%02d %s", hourFormat12(), minute(), second(), isAM() ? "am":"pm");
};

void Time::longDate(Print& p) {
  p.printf("%s, %s %d, %d", dayStrings[weekday()], monthStrings[month()], day(), year());
}

void Time::shortDate(Print& p) {
  p.printf("%d-%02d-%02d", year(), month(), day());
}

void Time::shortTime(Print& p) {
  p.printf("%d:%02d %s", hourFormat12(), minute(), isAM() ? "am":"pm");
};

void Time::longTime(Print& p) {
  p.printf("%d:%02d:%02d %s", hourFormat12(), minute(), second(), isAM() ? "am":"pm");
};


//////////////////////////////////////////////////////////////////////////////
// Uptime Methods
//
micros_t Uptime::lastUptimeMicros = 0;
micros_t Uptime::uptimeOffsetMicros = 0;

micros_t Uptime::micros() {
  micros_t nowUptime = ::micros();
  if (nowUptime < lastUptimeMicros) {
    // micros have rolled over, we add a new offset
    uptimeOffsetMicros += 0x0000000100000000;
  }
  lastUptimeMicros = nowUptime;
  return nowUptime + uptimeOffsetMicros;
}

//////////////////////////////////////////////////////////////////////////////
// DayTime Methods
//
time_t DayTime::nextOccurance(time_t starting) {
  time_t intervalSecs = interval()/microsPerSec;
  time_t nextup = (starting/intervalSecs)*intervalSecs + getSeconds();
  if (nextup < starting) {
    nextup+= intervalSecs;
  }
  return nextup;
};

//////////////////////////////////////////////////////////////////////////////
// BaseClock Methods
//

micros_t BaseClock::_update_interval = 0;
micros_t BaseClock::_last_update = 0;
micros_t BaseClock::_micros_offset = 0;
micros_t BaseClock::_utc_micros_time = 0;
bool BaseClock::_is_setting = false;

void BaseClock::setMicros(micros_t newTime) {
  _micros_offset = Uptime::micros();
  _utc_micros_time = newTime - + _zone_offset;
}

micros_t BaseClock::getMicros() {
  micros_t now = Uptime::micros();
  if (_update_interval && (now - _last_update) > _update_interval) {
    _last_update = now;
    updateTime();
  }

  return _utc_micros_time + now - _micros_offset + _zone_offset;
}

bool BaseClock::hasBeenSet() {
  return (_micros_offset != 0) && !_is_setting;
}

//////////////////////////////////////////////////////////////////////////////
//  TeensyClock Methods
//
#if defined(CORE_TEENSY)

TeensyClock::TeensyClock() {
  updateTime();
  setUpdateInterval(10);
}

void TeensyClock::updateTime() {
  setSeconds(Teensy3Clock.get());
}

void TeensyClock::setMicros(micros_t newTime) {
  time_t newSecs = newTime/microsPerSec;
  BaseClock::setMicros(newTime);
  Teensy3Clock.set(newSecs);
  _last_update = Uptime::micros();
}
#endif
