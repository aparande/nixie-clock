#include "scrambler.h"

namespace {
  // When we're scrambling, we'll do a Fischer-Yates shuffle
  // (https://en.wikipedia.org/wiki/Fisher–Yates_shuffle) on this array.
  uint8_t digit_buffer[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
}

void Scrambler::step(unsigned long now, uint8_t digits[]) {
  // Start scrambling every hour
  // TODO: Account for overflow. It'll happen after ~50 days of continuous
  // on-time
  if (now - last_scramble_time_millis_ >= 3'600'000) {
    scramble_state_ = 1;
    last_scramble_time_millis_ = now;
    memcpy(initial_condition_, digits, num_digits_);
  }

  // Marks the index of the last element available for the shuffle.
  uint8_t array_end = 10 - scramble_state_;

  if (scramble_state_ >= 1 && now - last_scramble_time_millis_ >= 10'000) {
    // Get a random number out of the remaining digits
    auto j = random(array_end);

    // Swap that number to the end of the remaining elements unless we're
    // ending the scramble.
    if (array_end != 0) {
      auto temp = digit_buffer[j];
      digit_buffer[j] = digit_buffer[array_end - 1];
      digit_buffer[array_end - 1] = temp;
      // Make sure we grab the right element when we actually apply the
      // scramble.
      array_end -= 1;
    }

    scramble_state_ = (scramble_state_ + 1) % 11;
    last_scramble_time_millis_ = now;
  }

  // In states 1 and 0, we don't want to modify the digits
  if (scramble_state_ <= 1) return;

  // Apply the scramble
  for (uint8_t k = 0; k < num_digits_; k++) {
    digits[k] = (initial_condition_[k] + digit_buffer[array_end]) % 10;
  }
}
