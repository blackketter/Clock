#ifndef _Timer_
#define _Timer_

#include "Arduino.h"
#include "Clock.h"

class Timer {
  public:
    virtual ~Timer();

    void setSecs(time_t setTime, bool repeat = false);
    void setMillis(millis_t millisDur, bool repeat = false);
    void setClockTime(time_t clockTimeSet);

    time_t remainingSecs();  // seconds from now (works on both kinds of timer)
    millis_t remainingMillis();  // millis from now (works on both kinds of timer)

    time_t timeInSecs();   // clock time of timer (works on both kinds of timer)
    millis_t timeInMillis();   // millis() time of timer (works on both kinds of timer)

    time_t durationSecs();  // how far out was the timer when it was initially set?
    millis_t durationMillis();

    void cancel();         // cancel timer including callback

    void pause();          // pause timer
    void resume();         // resume timer
    bool isPaused();       // is paused
    bool isRunning();        // is the timer running?  (i.e. is not paused and is set)
    bool hasPassed();         // is the timer in the past
    bool isReset() { return ((_millisTime == 0) && (_clockTime == 0)); }
    void setData(void* data) { _data = data; }
    void* getData() { return _data; };

    static void idle();    // idle so callbacks get a chance to run
    static Timer* first() { return _first; }
    Timer* next() { return _next; }
    static void printInfo(Print* p);
  protected:
    void insert();
    void remove();
    virtual void callback() = 0;

    Timer* _next = nullptr;
    Timer* _prev = nullptr;

    time_t _clockTime = 0;

    millis_t _millisTime = 0;
    millis_t _millisDur = 0;
    bool _repeatTimer;
    void* _data = nullptr;

    static Timer* _first;

};

typedef void (*timerCallback_t)(void*);

class CallbackTimer : public Timer {

  public:
    void setSecs(time_t setTime, timerCallback_t callback, void* callbackData, bool repeat = false);
    void setMillis(millis_t millisDur, timerCallback_t callback, void* callbackData, bool repeat = false);
    void setClockTime(time_t clockTimeSet, timerCallback_t callback, void* callbackData);

  protected:
    virtual void callback() { if (_cb) { (_cb)(_data); }; }
    timerCallback_t _cb = nullptr;
};

#endif
