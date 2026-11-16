#include "UptimeCounter.h"

#include <Arduino.h>

namespace {
constexpr char PREFERENCES_NAMESPACE[] = "bmp388";
constexpr char SESSION_KEY[] = "session_ms";
constexpr uint32_t SAVE_INTERVAL_MS = 60UL * 1000UL;
}

bool UptimeCounter::begin() {
  lastUpdateMilliseconds_ = millis();
  lastSaveAttemptMilliseconds_ = lastUpdateMilliseconds_;

  storageReady_ = preferences_.begin(PREFERENCES_NAMESPACE, false);
  if (!storageReady_) {
    return false;
  }

  hasPreviousSession_ = preferences_.isKey(SESSION_KEY);
  if (hasPreviousSession_) {
    previousSessionMilliseconds_ = preferences_.getULong64(SESSION_KEY, 0);
  }

  return true;
}

bool UptimeCounter::update() {
  uint32_t now = millis();
  elapsedSessionMilliseconds_ +=
      static_cast<uint32_t>(now - lastUpdateMilliseconds_);
  lastUpdateMilliseconds_ = now;

  if (!storageReady_ ||
      static_cast<uint32_t>(now - lastSaveAttemptMilliseconds_) <
          SAVE_INTERVAL_MS) {
    return true;
  }

  lastSaveAttemptMilliseconds_ = now;
  const uint64_t sessionMilliseconds =
      savedSessionMilliseconds_ + elapsedSessionMilliseconds_;
  if (preferences_.putULong64(SESSION_KEY, sessionMilliseconds) !=
      sizeof(sessionMilliseconds)) {
    return false;
  }

  const uint32_t savedAtMilliseconds = millis();
  savedSessionMilliseconds_ = sessionMilliseconds;
  elapsedSessionMilliseconds_ +=
      static_cast<uint32_t>(savedAtMilliseconds - now);
  lastUpdateMilliseconds_ = savedAtMilliseconds;
  return true;
}

uint64_t UptimeCounter::currentSessionSeconds() const {
  return (savedSessionMilliseconds_ + elapsedSessionMilliseconds_) / 1000;
}

uint64_t UptimeCounter::previousSessionSeconds() const {
  return previousSessionMilliseconds_ / 1000;
}

bool UptimeCounter::hasPreviousSession() const {
  return hasPreviousSession_;
}
