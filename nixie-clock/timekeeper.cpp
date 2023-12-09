#include "timekeeper.h"

void Timekeeper::step(unsigned long now, bool connected) {
  // TODO: Account for rollover
  if (now - last_updated_time_millis_ >= 60'000) {
    minute_ += 1;
    hour_ = (hour_ + minute_ / 60) % 24;
    minute_ = minute_ % 60;
    last_updated_time_millis_ = now;
  }

  if (connected && time_service_.hasUpdates()) {
    hour_ = time_service_.getHour();
    minute_ = time_service_.getMinute();
    last_updated_time_millis_ = now;
  }
}
