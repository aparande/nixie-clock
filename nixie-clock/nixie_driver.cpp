#include "nixie_driver.h"

namespace {
uint8_t digitToPin(uint8_t digit) {
  // 0 is pin 4
  if (digit == 0)
    return 4;
  // 6-9 are pins 0-3
  else if (digit >= 6)
    return digit - 6;
  // 1-5 are pins 5-9
  else
    return digit + 4;
}
} // namespace

void NixieDriver::begin() const {
  reg_bank_.begin();
  reg_bank_.unblank();
  reg_bank_.enableOutputs();
}

void NixieDriver::setDigits(const uint8_t digits[]) const {
  uint8_t bank_vals[reg_bank_.getBankSize()] = {0};
  // Pretend the registers are a 8 * reg_bank_.getBankSize bit number
  uint8_t pin_offset = 0;
  for (uint8_t dig_idx = 0; dig_idx < num_digits_; dig_idx++) {
    uint8_t global_pin_num = pin_offset + digitToPin(digits[dig_idx]);
    uint8_t bank_num = global_pin_num / 8;
    uint8_t bank_bit = global_pin_num % 8;
    bank_vals[bank_num] |= 1 << bank_bit;

    pin_offset += 10;
  }
  reg_bank_.write(bank_vals);
}
