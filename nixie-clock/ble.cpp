#include "ble.h"

bool BleManager::begin() {
  if (!BLE.begin()) return false;

  BLE.addService(time_service_);
  BLE.setAdvertisedService(time_service_);
  BLE.setLocalName("Nixie-Clock");

  return BLE.advertise();
}

void BleManager::step() {
  device_ = BLE.central();
}

