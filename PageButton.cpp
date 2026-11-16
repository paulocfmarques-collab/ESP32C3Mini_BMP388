#include "PageButton.h"

#include <Arduino.h>

namespace {
constexpr uint32_t DEBOUNCE_INTERVAL_MS = 30;
}

PageButton::PageButton(uint8_t pin) : pin_(pin) {}

void PageButton::begin() {
  pinMode(pin_, INPUT_PULLUP);
  lastReadingHigh_ = digitalRead(pin_) == HIGH;
  stableStateHigh_ = lastReadingHigh_;
  lastChangeMilliseconds_ = millis();
}

bool PageButton::wasPressed() {
  const bool readingHigh = digitalRead(pin_) == HIGH;
  const uint32_t now = millis();

  if (readingHigh != lastReadingHigh_) {
    lastReadingHigh_ = readingHigh;
    lastChangeMilliseconds_ = now;
  }

  if (static_cast<uint32_t>(now - lastChangeMilliseconds_) <
      DEBOUNCE_INTERVAL_MS) {
    return false;
  }

  if (readingHigh == stableStateHigh_) {
    return false;
  }

  stableStateHigh_ = readingHigh;
  return !stableStateHigh_;
}
