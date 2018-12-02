#include <stdio.h>
#include <Arduino.h>
#include "Clock.h"
#include "Time.h"

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0


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

void Uptime::longTime(Print& p) {
  time_t s = seconds();
  int t = s/secsPerDay;
  if (t) p.printf( "%d days, ", t);

  t = (s%secsPerDay)/secsPerHour;
  if (t) p.printf( "%d hours, ", t);

  t = (s%secsPerHour)/secsPerMinute;
  if (t) p.printf( "%d minutes, ", t);

  t = s%secsPerMinute;
  p.printf("%d seconds", t);
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
// RTCClock Methods
//

micros_t RTCClock::_update_interval = 0;
micros_t RTCClock::_last_update = 0;
micros_t RTCClock::_micros_offset = 0;
micros_t RTCClock::_utc_micros_time = 0;
bool RTCClock::_is_setting = false;

void RTCClock::setMicros(micros_t newTime) {
  _micros_offset = Uptime::micros();
  micros_t zone_offset = 0;
  if (_zone) {
    zone_offset = microsPerSec * _zone->offset(_zone->toUTC(newTime/microsPerSec));
  }
  _utc_micros_time = newTime - zone_offset;
}

micros_t RTCClock::getMicros() {

  micros_t up = Uptime::micros();
  if (_update_interval && (up - _last_update) > _update_interval) {
    _last_update = up;
    updateTime();
  }

  micros_t zone_offset = 0;
  micros_t utc_now = _utc_micros_time + up - _micros_offset;

  if (_zone) {
    zone_offset = microsPerSec * _zone->offset(utc_now/microsPerSec);
  }

  return utc_now + zone_offset;
}

bool RTCClock::hasBeenSet() {
  return (_micros_offset != 0) && !_is_setting && (year() > 2016);
}

//////////////////////////////////////////////////////////////////////////////
//  Teensy Clock Methods
//
#if defined(CORE_TEENSY)

TeensyClock::TeensyClock() {
  updateTime();
  setUpdateInterval(10);
}

void TeensyClock::updateTime() {
  _last_update = Uptime::micros();
  Timezone* savedZone = _zone;
  _zone = nullptr;

  RTCClock::setMicros(getRTCMicros());

  _zone = savedZone;

}

void TeensyClock::setMicros(micros_t newTime) {
  _last_update = Uptime::micros();

  RTCClock::setMicros(newTime);

  Timezone* savedZone = _zone;
  _zone = nullptr;

  setRTCMicros(getMicros());

  _zone = savedZone;

}

millis_t TeensyClock::getRTCMicros() {
  uint32_t read1, read2, secs, us = 0;
    do {
      read1 = RTC_TSR;
      read2 = RTC_TSR;
    } while (read1 != read2);       //insure the same read twice to avoid 'glitches'

    secs = read1;

    do {
      read1 = RTC_TPR;
      read2 = RTC_TPR;
    } while (read1 != read2);       //insure the same read twice to avoid 'glitches'

    //Scale 32.768KHz to microseconds
    us = ((micros_t)read1 * microsPerSec) / 32768;

    //if prescaler just rolled over from zero, might have just incremented seconds -- refetch
    if (us < 1000) {
      do {
        read1 = RTC_TSR;
        read2 = RTC_TSR;
      } while (read1 != read2);       //insure the same read twice to avoid 'glitches'
      secs = read1;
    }

  return(secs*microsPerSec + us);
}

void TeensyClock::setRTCMicros(micros_t newTime) {

  uint32_t secs = newTime / microsPerSec;
  uint32_t tics = ((newTime % microsPerSec) * 32768) / (1000000); // a teensy tic is 1/32768

  RTC_SR = 0;
  RTC_TPR = tics;
  RTC_TSR = secs;
  RTC_SR = RTC_SR_TCE;
}

#endif // defined(CORE_TEENSY)
