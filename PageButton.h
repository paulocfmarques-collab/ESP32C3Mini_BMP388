#pragma once

#include <stdint.h>

class PageButton {
 public:
  explicit PageButton(uint8_t pin);

  void begin();
  bool wasPressed();

 private:
  uint8_t pin_;
  bool lastReadingHigh_ = true;
  bool stableStateHigh_ = true;
  uint32_t lastChangeMilliseconds_ = 0;
};
