#include <ArduinoBLE.h>

#ifndef NIXIE_BLE
#define NIXIE_BLE

constexpr auto kTimeServiceUid = "19B10000-E8F2-537E-4F6C-D104768A1214";
constexpr auto kTimeCharacteristicUid = "19B10001-E8F2-537E-4F6C-D104768A1215";

class TimeBleService : public BLEService {
  public:
  TimeBleService() : BLEService(kTimeServiceUid) {
    addCharacteristic(time_char_);
  };

  bool hasUpdates(){
    return time_char_.written();
  }

  uint8_t getHour() const{
    return time_char_[0];
  }
  uint8_t getMinute() const{
    return time_char_[1];
  }

  private:
    BLECharacteristic time_char_{kTimeCharacteristicUid, BLERead | BLEWrite, 2};
};

class BleManager {
  public:
    bool begin();
    void step();
    TimeBleService& time_service() { return time_service_; };
    bool connected() { return device_.connected(); };

  private:
    TimeBleService time_service_;
    BLEDevice device_{};
};
#endif
