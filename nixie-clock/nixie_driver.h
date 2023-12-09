#ifndef NIXIE_DRIVER
#define NIXIE_DRIVER

#include <hv513_bank.h>

class NixieDriver {
  public:
  NixieDriver(const Hv513Bank& reg_bank, uint8_t num_digits): reg_bank_{reg_bank}, num_digits_{num_digits} {};

  void begin() const;
  // Write Digits to the Nixie Tubes. The left-most tube is index 0
  void setDigits(const uint8_t digits[]) const;

  private:
  const Hv513Bank& reg_bank_;
  const uint8_t num_digits_;
};
#endif
