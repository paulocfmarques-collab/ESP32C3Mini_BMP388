#pragma once

#include "SensorReading.h"

class SerialLogger {
 public:
  void log(const SensorReading& reading) const;
};
