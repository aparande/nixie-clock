#include <hv513_bank.h>
#include <mbed_boost_converter.h>

#include "ble.h"
#include "mbed.h"
#include "nixie_driver.h"
#include "timekeeper.h"
#include "scrambler.h"

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
constexpr pin_size_t kIndicatorPin = D10;
constexpr pin_size_t kHighVoltagePin = D11;

// HV513 Bank GPI
constexpr pin_size_t kShortDetect = D6;

// Boost Converter GPI
constexpr auto kVoutPin = A0;
} // namespace pins

namespace constants {
constexpr auto kSafeVoltage = 15;

constexpr unsigned int kAdcBits = 12;
constexpr float kAdcRef = 1 << kAdcBits;
constexpr float kVref = 3.3;
// Inverse of the voltage divider equation R2 / (R1 + R2)
constexpr float kDividerInv = (1'000'000 + 10'000) / 10'000;
constexpr float kFeedbackAlpha = 0.1;

// Operating point
constexpr float kVout = 170;
constexpr float kVin = 45;

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

uint8_t ic_buffer_[constants::kNumDigits] = {0};
Scrambler scrambler{constants::kNumDigits, ic_buffer_};

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

unsigned long last_step_time = 0;

void loop() {
  auto curr_time = millis();

  ble.step();
  timekeeper.step(curr_time, ble.connected());

  // Read the voltage every step to keep the converter memory up to date.
  auto latest_voltage = converter.readVoltage();
  if (curr_time - last_step_time > constants::kControllerPeriodMs) {
    if (latest_voltage < constants::kSafeVoltage) {
      digitalWrite(pins::kHighVoltagePin, LOW);
    } else {
      digitalWrite(pins::kHighVoltagePin, HIGH);
    }

    if (converter.state() == MbedStepBoostConverter::State::kIdle) {
      digitalWrite(pins::kIndicatorPin, HIGH);
      
      // When idle, start the converter when the input voltage is present at
      // the output and enough time has passed.
      if (latest_voltage > constants::kVin) {
        converter.step(curr_time);
        last_step_time = curr_time;
      }
    } else if (converter.state() == MbedStepBoostConverter::State::kDiverged) {
      digitalWrite(pins::kIndicatorPin, HIGH);

      // Idle the converter once the capacitor has discharged beyond the input
      // voltage. Once the input voltage is again present at the output, the
      // converter will automatically restart.
      if (latest_voltage < constants::kVin) {
        converter.reset();
      }
    } else {
      converter.step(curr_time);
      digitalWrite(pins::kIndicatorPin, LOW);
      last_step_time = curr_time;
    }
  }

  digits[0] = timekeeper.getHour() / 10;
  digits[1] = timekeeper.getHour() % 10;
  digits[2] = timekeeper.getMinute() / 10;
  digits[3] = timekeeper.getMinute() % 10;

  scrambler.step(curr_time, digits);

  nixies.setDigits(digits);

  delay(constants::kMeasurementPeriodMs);
}
