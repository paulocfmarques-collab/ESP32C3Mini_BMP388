#pragma once

#include <stdint.h>

class WatchdogManager {
 public:
  bool begin(uint32_t timeoutSeconds);
  bool feed();

 private:
  bool enabled_ = false;
};
