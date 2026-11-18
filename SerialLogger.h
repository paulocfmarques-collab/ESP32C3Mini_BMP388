#pragma once

#include <stdint.h>

#include "SensorReading.h"

class SerialLogger {
 public:
  void log(const SensorReading& reading) const;
  void logBattery(uint16_t rawAdc, uint32_t batteryMillivolts) const;
};
