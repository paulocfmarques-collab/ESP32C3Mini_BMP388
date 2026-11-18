#pragma once

#include <stddef.h>
#include <stdint.h>

#include "SensorReading.h"

class Adafruit_SSD1306;

class OledDisplay {
 public:
  explicit OledDisplay(Adafruit_SSD1306& display);

  bool begin();
  void showStartup();
  void recordReading(const SensorReading& reading);
  void nextPage();
  void render(uint64_t currentUptimeSeconds,
              uint64_t previousUptimeSeconds,
              bool hasPreviousSession,
              uint32_t batteryMillivolts,
              uint8_t batteryPercent);
  void showError(const char* message);

 private:
  enum class Page : uint8_t {
    Home,
    Pressure,
    Temperature,
    Altitude,
    Uptime,
    ResetReason,
    Battery
  };

  enum class GraphMetric : uint8_t {
    Pressure,
    Temperature,
    Altitude
  };

  static constexpr size_t HISTORY_CAPACITY = 120;

  void drawHeader(const char* title);
  void drawHomePage(const SensorReading& reading,
                    uint64_t currentUptimeSeconds);
  void drawGraphPage(const char* title,
                     const char* unit,
                     GraphMetric metric);
  void drawUptimePage(uint64_t currentUptimeSeconds,
                      uint64_t previousUptimeSeconds,
                      bool hasPreviousSession);
  void drawResetReasonPage();
  void drawBatteryPage(uint32_t millivolts, uint8_t percent);
  float valueFor(const SensorReading& reading, GraphMetric metric) const;

  Adafruit_SSD1306& display_;
  SensorReading history_[HISTORY_CAPACITY]{};
  size_t historyCount_ = 0;
  size_t nextHistoryIndex_ = 0;
  Page currentPage_ = Page::Home;
};
