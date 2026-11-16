#pragma once

#include <Preferences.h>
#include <stdint.h>

class UptimeCounter {
 public:
  bool begin();
  bool update();

  uint64_t currentSessionSeconds() const;
  uint64_t previousSessionSeconds() const;
  bool hasPreviousSession() const;

 private:
  Preferences preferences_;
  uint64_t savedSessionMilliseconds_ = 0;
  uint64_t elapsedSessionMilliseconds_ = 0;
  uint64_t previousSessionMilliseconds_ = 0;
  uint32_t lastUpdateMilliseconds_ = 0;
  uint32_t lastSaveAttemptMilliseconds_ = 0;
  bool storageReady_ = false;
  bool hasPreviousSession_ = false;
};
