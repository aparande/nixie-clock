#include "ble.h"

#ifndef NIXIE_TIMEKEEPER
#define NIXIE_TIMEKEEPER

class Timekeeper {
public:
  Timekeeper(TimeBleService &time_service)
      : time_service_(time_service){};

  void step(unsigned long now, bool connected);
  uint8_t getHour() const { return hour_; };
  uint8_t getMinute() const { return minute_; };

  void SetTime(uint8_t hour, uint8_t minute) {
    hour_ = hour;
    minute_ = minute;
  }

private:
  TimeBleService &time_service_;
  uint8_t hour_{0};
  uint8_t minute_{0};
  unsigned long last_updated_time_millis_{0};
};

#endif
