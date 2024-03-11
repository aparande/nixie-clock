#ifndef NIXIE_SCRAMBLER
#define NIXIE_SCRAMBLER

#include <Arduino.h>

class Scrambler {
  public:
    Scrambler(uint8_t num_digits, uint8_t digit_buffer[]): num_digits_(num_digits), initial_condition_(digit_buffer) {};
    // Potentially modify digits based on whether we're scrambling or not
    void step(unsigned long now, uint8_t digits[]);
  private:
    const uint8_t num_digits_;
    // Last time we scrambled the digits
    unsigned long last_scramble_time_millis_{0};
    // Scramble State 0 means we're not scrambling. During the scramble, this
    // serves as the iterate number of the Fischer-Yates shuffle.
    uint8_t scramble_state_{0};
    
    // Initial condition for scrambling
    uint8_t* initial_condition_;
    
};
#endif
