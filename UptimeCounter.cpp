#include "UptimeCounter.h"

#include <esp_timer.h>

namespace {
constexpr char PREFERENCES_NAMESPACE[] = "bmp388";
constexpr char SESSION_KEY[] = "session_ms";
constexpr uint64_t SAVE_INTERVAL_MS = 60ULL * 1000ULL;

uint64_t uptimeMilliseconds() {
  return static_cast<uint64_t>(esp_timer_get_time()) / 1000;
}
}

bool UptimeCounter::begin() {
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
  const uint64_t sessionMilliseconds = uptimeMilliseconds();
  if (!storageReady_ ||
      sessionMilliseconds - lastSaveAttemptMilliseconds_ < SAVE_INTERVAL_MS) {
    return true;
  }

  lastSaveAttemptMilliseconds_ = sessionMilliseconds;
  return preferences_.putULong64(SESSION_KEY, sessionMilliseconds) ==
         sizeof(sessionMilliseconds);
}

uint64_t UptimeCounter::currentSessionSeconds() const {
  return uptimeMilliseconds() / 1000;
}

uint64_t UptimeCounter::previousSessionSeconds() const {
  return previousSessionMilliseconds_ / 1000;
}

bool UptimeCounter::hasPreviousSession() const {
  return hasPreviousSession_;
}
