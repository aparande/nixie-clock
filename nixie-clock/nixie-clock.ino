#include <hv513_bank.h>
#include <mbed_boost_converter.h>

#include "ble.h"
#include "mbed.h"
#include "nixie_driver.h"
#include "timekeeper.h"

namespace pins {
// HV513 Bank GPO
constexpr pin_size_t kBlankingPin = D2;
constexpr pin_size_t kPolarityPin = D3;
constexpr pin_size_t kClockPin = D4;
constexpr pin_size_t kLatchPin = D5;
// Leave gap for Short Detect
constexpr pin_size_t kHighZPin = D7;
constexpr pin_size_t kDataPin = D8;

// Boost Converter GPO
constexpr pin_size_t kPwmPin = D9;

// Built in LED pin
constexpr pin_size_t kIndicatorPin = D13;

// HV513 Bank GPI
constexpr pin_size_t kShortDetect = D6;

// Boost Converter GPI
constexpr auto kVoutPin = A0;
} // namespace pins

namespace constants {
constexpr unsigned int kAdcBits = 12;
constexpr float kAdcRef = 1 << kAdcBits;
constexpr float kVref = 3.3;
// Inverse of the voltage divider equation R2 / (R1 + R2)
constexpr float kDividerInv = (1'000'000 + 10'000) / 10'000;
constexpr float kFeedbackAlpha = 0.1;

// Operating point
constexpr float kVout = 140;

// Controller Settings
constexpr auto kControllerPeriodMs = 100;
constexpr auto kMeasurementPeriodMs = 1;
constexpr float kPwmPeriodUs = 25;

// Controller Bounds
constexpr float kDMin = 0.05;
constexpr float kDMax = 0.95;

MbedStepBoostConverter::Settings settings = {
    .step_size = 0.005, .tolerance_v = 10, .idle_duty = 1};

constexpr uint8_t kNumBanks = 5;
constexpr uint8_t kNumDigits = 4;
} // namespace constants

class BoostConverter : public MbedStepBoostConverter {
public:
  BoostConverter()
      : MbedStepBoostConverter(pins::kPwmPin, constants::kPwmPeriodUs,
                               constants::kVout, constants::settings){};

  void begin() {
    MbedStepBoostConverter::begin();

    pinMode(pins::kVoutPin, INPUT);
    analogReadResolution(constants::kAdcBits);
  }

  float readVoltage() final {
    float v_out_counts = analogRead(pins::kVoutPin);
    auto voltage = (v_out_counts / constants::kAdcRef) * constants::kVref;
    memory_ = constants::kFeedbackAlpha * voltage * constants::kDividerInv +
              (1 - constants::kFeedbackAlpha) * memory_;
    return memory_;
  }

private:
  float memory_;
};

BoostConverter converter;

Hv513Bank bank{
    constants::kNumBanks, Hv513Bank::Mode::kSink, pins::kDataPin,
    pins::kClockPin,      pins::kBlankingPin,     pins::kPolarityPin,
    pins::kLatchPin,      pins::kHighZPin,        pins::kShortDetect};
NixieDriver nixies{bank, constants::kNumDigits};

BleManager ble;
Timekeeper timekeeper{ble.time_service()};

uint8_t digits[constants::kNumDigits] = {0};

void setup() {
  Serial.begin(9600);
  if (!ble.begin()) {
    Serial.println("Could not set up BLE");
  }
  converter.begin();
  converter.configureBounds(constants::kDMin, constants::kDMax);

  nixies.begin();
  nixies.setDigits(digits);

  pinMode(pins::kIndicatorPin, OUTPUT);
}

uint32_t counter = 0;
unsigned long last_increment_time = 0;
unsigned long last_step_time = 0;

void loop() {
  auto curr_time = millis();

  ble.step();
  timekeeper.step(curr_time, ble.connected());

  converter.readVoltage();
  if (curr_time - last_step_time > constants::kControllerPeriodMs) {
    if (converter.state() == MbedStepBoostConverter::State::kDiverged) {
      digitalWrite(pins::kIndicatorPin, HIGH);
    } else {
      converter.step(curr_time);
      digitalWrite(pins::kIndicatorPin, LOW);
    }

    last_step_time = curr_time;
  }

  if (curr_time - last_increment_time > 1'000) {
    counter += 1;
    uint8_t temp_counter = counter;

    int divisor = 1000;
    for (int i = 0; i < constants::kNumDigits; i++) {
      digits[i] = temp_counter / divisor;
      temp_counter -= digits[i] * divisor;
      divisor /= 10;
    }

    nixies.setDigits(digits);
    last_increment_time = curr_time;
  }

  delay(constants::kMeasurementPeriodMs);
}