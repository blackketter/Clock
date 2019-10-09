#include "Timer.h"
#include "Clock.h"

extern Clock clock;

Timer* Timer::_first = nullptr;

Timer::~Timer() {
  remove();
};

// fixme: handle other clocks besides "clock"
void Timer::setClockTime(time_t clockTimeSet) {
  cancel();
  _clockTime = clockTimeSet;
  _millisDur = (clockTimeSet-clock.now())*1000;
  insert();
}

void Timer::setMillis(millis_t millisDuration, bool repeat) {
  cancel();
  _clockTime = 0;
  _repeatTimer = repeat;
  _millisDur = millisDuration;
  _millisTime = Uptime::millis() + _millisDur;
  insert();
}

void Timer::setSecs(time_t secs, bool repeat) {
  setMillis(secs*1000, repeat);
}

millis_t Timer::remainingMillis() {
  millis_t remainingMillis = 0;
  if (_millisTime) {
    if (isPaused()) {
      return -_millisTime;
    } else {
      return (_millisTime - Uptime::millis());
    }
  } else if (_clockTime) {
    // todo make this more accurate using the difference between millis() and now()
    return remainingSecs() * 1000;
  }
  return remainingMillis;
}

time_t Timer::remainingSecs() {
  millis_t remainingSecs = 0;
  if (_millisTime) {
    return remainingMillis()/1000;
  } else if (_clockTime) {
    if (isPaused()) {
      return -_clockTime;
    } else {
      return (_clockTime - clock.now());
    }
  }
  return remainingSecs;
}

time_t Timer::timeInSecs() {
  return clock.now()+remainingSecs();
}

millis_t Timer::timeInMillis() {
  return Uptime::millis()+remainingMillis();
}

time_t Timer::durationSecs() {
  return _millisDur/1000;
}

millis_t Timer::durationMillis() {
    return _millisDur;
}

bool Timer::hasPassed() {

  if (isPaused()) { return false; }

  if (_millisTime) {
    if (Uptime::millis() >= _millisTime) {
      return true;
    }
  } else if (_clockTime) {
    if (clock.now() >= _clockTime) {
      return true;
    }
  }
  return false;
}

bool Timer::isRunning() {
  return (_millisTime > 0) || (_clockTime > 0);
}

void Timer::cancel() {
  _millisTime = 0;
  _clockTime = 0;
  _millisDur = 0;
  _repeatTimer = 0;
  remove();
}

void Timer::pause() {

  if (isPaused()) {
    return;
  }

  if (_millisTime) {
    _millisTime = -(_millisTime - Uptime::millis());
  } else if (_clockTime) {
    _clockTime = -(_clockTime - clock.now());
  }
}

void Timer::resume() {
  if (!isPaused()) {
    return;
  }
  if (_millisTime) {
    _millisTime = Uptime::millis() - _millisTime;
  } else if (_clockTime) {
    _clockTime = clock.now() - _clockTime;
  }
}

bool Timer::isPaused() {
  return (_millisTime < 0) || (_clockTime < 0);
}

void Timer::idle() {
  Timer* t = _first;
  while (t) {
    if (t->hasPassed()) {
      //pu.debugf("timer has passed %d\n",t);
      Timer* nextup = t->_next;
      t->remove();
      t->callback();
      if (t->_repeatTimer) {
        //console.debugf("reinserting repeat timer %d\n", t);
        t->_millisTime += t->_millisDur;
        t->insert();
      }
      t = nextup;
    } else {
      t = t->_next;
    }
  }
}

void Timer::insert() {
   //console.debugf("Inserting timer %d\n",this);

  if (_first != nullptr) {
    _first->_prev = this;
  }

  _next = _first;
  _prev = nullptr;
  _first = this;
}

void Timer::remove() {
  //console.debugf("removing timer %d\n",this);

  if (_next) {
    _next->_prev = _prev;
  }

  if (_prev) {
    _prev->_next = _next;
  } else {
    if (_first == this) {
      _first = _next;
    }
  }

  _next = nullptr;
  _prev = nullptr;
}

void Timer::printInfo(Print* p) {
  int i = 0;
  Timer* t = _first;
  while (t) {
    i++;
    p->printf(" Timer: %d :\n",(int)t);
    p->printf("  Remaining: %d\n", (int)t->remainingMillis());
    p->printf("  Millis: %d\n", (int)t->_millisTime);
    p->printf("  Clocktime: %d\n", (int)t->_clockTime);
    p->println(t->_repeatTimer ? "  Repeating" : "  Not repeating");
    p->println(t->isPaused() ? "  Paused" : "  Not paused");
    t = t->_next;
  }
  p->printf("Timer count:%d\n", i);
}

////////////////////////////////////////////////////////////////////////////////
void CallbackTimer::setClockTime(time_t clockTimeSet, timerCallback_t callback, void* callbackData) {
  _cb = callback;
  setData(callbackData);
  Timer::setClockTime(clockTimeSet);
}

void CallbackTimer::setSecs(time_t secs, timerCallback_t callback, void* callbackData, bool repeat) {
  _cb = callback;
  setData(callbackData);
  Timer::setSecs(secs,repeat);
}

void CallbackTimer::setMillis(millis_t millisDuration, timerCallback_t callback, void* callbackData, bool repeat) {
  _cb = callback;
  setData(callbackData);
  Timer::setMillis(millisDuration,repeat);
}


