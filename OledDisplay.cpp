#include "OledDisplay.h"

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>
#include <stdio.h>

namespace {
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr int16_t GRAPH_LEFT = 4;
constexpr int16_t GRAPH_RIGHT = 123;
constexpr int16_t GRAPH_TOP = 20;
constexpr int16_t GRAPH_BOTTOM = 60;

void formatDuration(uint64_t totalSeconds, char* output, size_t outputSize) {
  const uint64_t totalMinutes = totalSeconds / 60;
  const uint64_t days = totalMinutes / (24 * 60);
  const uint64_t hours = (totalMinutes / 60) % 24;
  const uint64_t minutes = totalMinutes % 60;

  if (days > 999) {
    snprintf(output, outputSize, "999d+%02llu:%02llu",
             static_cast<unsigned long long>(hours),
             static_cast<unsigned long long>(minutes));
    return;
  }

  snprintf(output, outputSize, "%llud%02llu:%02llu",
           static_cast<unsigned long long>(days),
           static_cast<unsigned long long>(hours),
           static_cast<unsigned long long>(minutes));
}

void formatSeconds(uint64_t seconds, char* output, size_t outputSize) {
  snprintf(output, outputSize, "%llu",
           static_cast<unsigned long long>(seconds));
}

const char* resetReasonText(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_UNKNOWN:
      return "Desconhecido";
    case ESP_RST_POWERON:
      return "Ligacao inicial";
    case ESP_RST_EXT:
      return "Pino de reset";
    case ESP_RST_SW:
      return "Software";
    case ESP_RST_PANIC:
      return "Panic / falha";
    case ESP_RST_INT_WDT:
      return "WDT interrupcao";
    case ESP_RST_TASK_WDT:
      return "WDT tarefa";
    case ESP_RST_WDT:
      return "WDT hardware";
    case ESP_RST_DEEPSLEEP:
      return "Deep sleep";
    case ESP_RST_BROWNOUT:
      return "Tensao baixa";
    case ESP_RST_SDIO:
      return "Reset SDIO";
  }
  return "Outro";
}
}

constexpr size_t OledDisplay::HISTORY_CAPACITY;

OledDisplay::OledDisplay(Adafruit_SSD1306& display) : display_(display) {}

bool OledDisplay::begin() {
  if (!display_.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    return false;
  }

  display_.setTextColor(SSD1306_WHITE);
  display_.clearDisplay();
  display_.display();
  return true;
}

void OledDisplay::showStartup() {
  display_.clearDisplay();
  drawHeader("BMP388 | AMBIENTE");
  display_.setTextSize(1);
  display_.setCursor(4, 29);
  display_.print("Iniciando sensor...");
  display_.display();
}

void OledDisplay::recordReading(const SensorReading& reading) {
  history_[nextHistoryIndex_] = reading;
  nextHistoryIndex_ = (nextHistoryIndex_ + 1) % HISTORY_CAPACITY;
  if (historyCount_ < HISTORY_CAPACITY) {
    ++historyCount_;
  }
}

void OledDisplay::nextPage() {
  switch (currentPage_) {
    case Page::Home:
      currentPage_ = Page::Pressure;
      break;
    case Page::Pressure:
      currentPage_ = Page::Temperature;
      break;
    case Page::Temperature:
      currentPage_ = Page::Altitude;
      break;
    case Page::Altitude:
      currentPage_ = Page::Uptime;
      break;
    case Page::Uptime:
      currentPage_ = Page::ResetReason;
      break;
    case Page::ResetReason:
      currentPage_ = Page::Home;
      break;
  }
}

void OledDisplay::render(uint64_t currentUptimeSeconds,
                         uint64_t previousUptimeSeconds,
                         bool hasPreviousSession) {
  if (historyCount_ == 0) {
    showStartup();
    return;
  }

  switch (currentPage_) {
    case Page::Home: {
      const size_t latestIndex =
          (nextHistoryIndex_ + HISTORY_CAPACITY - 1) % HISTORY_CAPACITY;
      drawHomePage(history_[latestIndex], currentUptimeSeconds);
      break;
    }
    case Page::Pressure:
      drawGraphPage("PRESSAO (hPa)", "hPa", GraphMetric::Pressure);
      break;
    case Page::Temperature:
      drawGraphPage("TEMPERATURA (C)", "C", GraphMetric::Temperature);
      break;
    case Page::Altitude:
      drawGraphPage("ALTITUDE (m)", "m", GraphMetric::Altitude);
      break;
    case Page::Uptime:
      drawUptimePage(currentUptimeSeconds, previousUptimeSeconds,
                     hasPreviousSession);
      break;
    case Page::ResetReason:
      drawResetReasonPage();
      break;
  }
}

void OledDisplay::drawHomePage(const SensorReading& reading,
                               uint64_t currentUptimeSeconds) {
  display_.clearDisplay();
  drawHeader("BMP388 | AMBIENTE");

  display_.setTextSize(1);
  display_.setCursor(4, 18);
  display_.print("TEMP");
  display_.setCursor(68, 18);
  display_.print("PRESSAO");
  display_.drawLine(63, 17, 63, 42, SSD1306_WHITE);

  display_.setCursor(4, 28);
  display_.print(reading.temperature, 1);
  display_.print(" C");
  display_.setCursor(68, 28);
  display_.print(reading.pressureHpa, 1);
  display_.print(" hPa");

  display_.drawLine(0, 44, SCREEN_WIDTH - 1, 44, SSD1306_WHITE);
  display_.setCursor(4, 46);
  display_.print("ALT");
  display_.setCursor(68, 46);
  display_.print("UPTIME");

  display_.setCursor(4, 55);
  display_.print(reading.altitude, 0);
  display_.print(" m");
  char currentDuration[16];
  formatDuration(currentUptimeSeconds, currentDuration,
                 sizeof(currentDuration));
  display_.setCursor(68, 55);
  display_.print(currentDuration);
  display_.display();
}

void OledDisplay::drawGraphPage(const char* title,
                                const char* unit,
                                GraphMetric metric) {
  const size_t latestIndex =
      (nextHistoryIndex_ + HISTORY_CAPACITY - 1) % HISTORY_CAPACITY;
  const SensorReading& latestReading = history_[latestIndex];
  display_.clearDisplay();
  drawHeader(title);

  display_.setTextSize(1);
  display_.setCursor(4, 8);
  display_.print("Atual: ");
  display_.print(valueFor(latestReading, metric),
                 metric == GraphMetric::Altitude ? 0 : 1);
  display_.print(" ");
  display_.print(unit);

  const size_t firstIndex =
      (nextHistoryIndex_ + HISTORY_CAPACITY - historyCount_) %
      HISTORY_CAPACITY;
  float minimum = valueFor(history_[firstIndex], metric);
  float maximum = minimum;
  for (size_t i = 1; i < historyCount_; ++i) {
    const size_t index = (firstIndex + i) % HISTORY_CAPACITY;
    const float value = valueFor(history_[index], metric);
    if (value < minimum) {
      minimum = value;
    }
    if (value > maximum) {
      maximum = value;
    }
  }

  const float minimumRange =
      metric == GraphMetric::Pressure ? 1.0F
      : metric == GraphMetric::Temperature ? 0.2F
                                           : 1.0F;
  float range = maximum - minimum;
  if (range < minimumRange) {
    const float midpoint = (minimum + maximum) / 2.0F;
    minimum = midpoint - minimumRange / 2.0F;
    maximum = midpoint + minimumRange / 2.0F;
  } else {
    const float padding = range * 0.1F;
    minimum -= padding;
    maximum += padding;
  }
  range = maximum - minimum;

  display_.drawLine(GRAPH_LEFT, GRAPH_TOP, GRAPH_LEFT, GRAPH_BOTTOM,
                    SSD1306_WHITE);
  display_.drawLine(GRAPH_LEFT, GRAPH_BOTTOM, GRAPH_RIGHT, GRAPH_BOTTOM,
                    SSD1306_WHITE);
  for (int16_t x = GRAPH_LEFT + 2; x < GRAPH_RIGHT; x += 4) {
    display_.drawPixel(x, (GRAPH_TOP + GRAPH_BOTTOM) / 2, SSD1306_WHITE);
  }

  int16_t previousX = GRAPH_LEFT;
  int16_t previousY = GRAPH_BOTTOM;
  for (size_t i = 0; i < historyCount_; ++i) {
    const size_t index = (firstIndex + i) % HISTORY_CAPACITY;
    const float value = valueFor(history_[index], metric);
    const float normalizedValue = (value - minimum) / range;
    const int16_t x = GRAPH_LEFT + static_cast<int16_t>(i);
    const int16_t y =
        GRAPH_BOTTOM -
        static_cast<int16_t>(normalizedValue * (GRAPH_BOTTOM - GRAPH_TOP) +
                             0.5F);

    if (i == 0) {
      display_.drawPixel(x, y, SSD1306_WHITE);
    } else {
      display_.drawLine(previousX, previousY, x, y, SSD1306_WHITE);
    }
    previousX = x;
    previousY = y;
  }

  display_.display();
}

void OledDisplay::drawUptimePage(uint64_t currentUptimeSeconds,
                                 uint64_t previousUptimeSeconds,
                                 bool hasPreviousSession) {
  display_.clearDisplay();
  drawHeader("UPTIME | SEGUNDOS");

  display_.setTextSize(1);
  display_.setCursor(4, 8);
  display_.print("ATUAL / ANTERIOR");
  display_.setCursor(4, 22);
  display_.print("SESSAO ATUAL (s)");
  display_.setCursor(4, 31);
  char currentSeconds[24];
  formatSeconds(currentUptimeSeconds, currentSeconds, sizeof(currentSeconds));
  display_.print(currentSeconds);

  display_.setCursor(4, 44);
  display_.print("SESSAO ANTERIOR (s)");
  display_.setCursor(4, 53);
  if (hasPreviousSession) {
    char previousSeconds[24];
    formatSeconds(previousUptimeSeconds, previousSeconds,
                  sizeof(previousSeconds));
    display_.print(previousSeconds);
  } else {
    display_.print("--");
  }

  display_.display();
}

void OledDisplay::drawResetReasonPage() {
  display_.clearDisplay();
  drawHeader("MOTIVO DO RESET");

  display_.setTextSize(1);
  display_.setCursor(4, 8);
  display_.print("ULTIMA INICIALIZACAO");
  display_.setCursor(4, 26);
  display_.print("Causa:");
  display_.setCursor(4, 38);
  display_.print(resetReasonText(esp_reset_reason()));
  display_.display();
}

void OledDisplay::showError(const char* message) {
  display_.clearDisplay();
  drawHeader("ERRO BMP388");
  display_.setTextSize(1);
  display_.setCursor(4, 30);
  display_.print(message);
  display_.display();
}

float OledDisplay::valueFor(const SensorReading& reading,
                            GraphMetric metric) const {
  switch (metric) {
    case GraphMetric::Pressure:
      return reading.pressureHpa;
    case GraphMetric::Temperature:
      return reading.temperature;
    case GraphMetric::Altitude:
      return reading.altitude;
  }
  return 0.0F;
}

void OledDisplay::drawHeader(const char* title) {
  display_.setTextSize(1);
  display_.setCursor(4, 0);
  display_.print(title);
  display_.drawLine(0, 15, SCREEN_WIDTH - 1, 15, SSD1306_WHITE);
}
