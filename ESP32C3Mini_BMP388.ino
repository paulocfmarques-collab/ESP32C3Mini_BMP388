#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP3XX.h>

#include "Bmp388Sensor.h"
#include "OledDisplay.h"
#include "PageButton.h"
#include "SerialLogger.h"
#include "UptimeCounter.h"
#include "WatchdogManager.h"

namespace {
constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;
constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t BATTERY_PIN = 0;
constexpr uint8_t BATTERY_SAMPLE_COUNT = 16;
constexpr uint32_t BATTERY_EMPTY_MV = 3200;
constexpr uint32_t BATTERY_FULL_MV = 4200;
constexpr uint32_t UPDATE_INTERVAL_MS = 1000;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Adafruit_BMP3XX bmp;
OledDisplay display(oled);
PageButton pageButton(BUTTON_PIN);
Bmp388Sensor sensor(bmp);
SerialLogger logger;
UptimeCounter uptime;
WatchdogManager watchdog;
bool watchdogReady = false;
uint32_t lastReadingMilliseconds = 0;
bool hasReadings = false;
uint32_t displayedBatteryMillivolts = 0;

uint32_t readBatteryMillivolts() {
  uint32_t sum = 0;
  for (uint8_t sample = 0; sample < BATTERY_SAMPLE_COUNT; ++sample) {
    sum += analogReadMilliVolts(BATTERY_PIN);
  }
  // The two 100 kOhm resistors halve the battery voltage at GPIO0.
  return (2 * sum + BATTERY_SAMPLE_COUNT / 2) / BATTERY_SAMPLE_COUNT;
}

uint8_t batteryPercent(uint32_t millivolts) {
  const uint32_t voltage = constrain(millivolts, BATTERY_EMPTY_MV, BATTERY_FULL_MV);
  const uint32_t span = BATTERY_FULL_MV - BATTERY_EMPTY_MV;
  return ((voltage - BATTERY_EMPTY_MV) * 100 + span / 2) / span;
}
}  // namespace

void setup() {
  Serial.begin(115200);

  watchdogReady = watchdog.begin(10);
  if (!watchdogReady) {
    Serial.println("Falha ao configurar watchdog");
  }

  if (!uptime.begin()) {
    Serial.println("Falha ao abrir Preferences; uptime nao sera preservado");
  }

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin()) {
    Serial.println("OLED nao encontrado");
    while (true) {
      if (watchdogReady) {
        watchdog.feed();
      }
      delay(UPDATE_INTERVAL_MS);
    }
  }

  pageButton.begin();
  pinMode(BATTERY_PIN, INPUT);
  analogSetPinAttenuation(BATTERY_PIN, ADC_11db);
  display.showStartup();

  if (!sensor.begin()) {
    Serial.println("BMP388 nao encontrado");
    display.showError("BMP388 ausente");
    while (true) {
      if (watchdogReady) {
        watchdog.feed();
      }
      delay(UPDATE_INTERVAL_MS);
    }
  }
}

void loop() {
  if (watchdogReady && !watchdog.feed()) {
    Serial.println("Falha ao alimentar watchdog");
    watchdogReady = false;
  }

  if (!uptime.update()) {
    Serial.println("Falha ao gravar uptime nas Preferences");
  }

  bool shouldRender = false;
  if (pageButton.wasPressed()) {
    display.nextPage();
    shouldRender = true;
  }

  const uint32_t now = millis();
  if (!hasReadings ||
      static_cast<uint32_t>(now - lastReadingMilliseconds) >=
          UPDATE_INTERVAL_MS) {
    lastReadingMilliseconds = now;
    const uint32_t batteryMillivolts = readBatteryMillivolts();
    logger.logBattery(analogRead(BATTERY_PIN), batteryMillivolts);
    if (batteryMillivolts == 0 || displayedBatteryMillivolts == 0) {
      displayedBatteryMillivolts = batteryMillivolts;
    } else {
      // Give the latest measurement one quarter of the displayed value.
      displayedBatteryMillivolts = (3 * displayedBatteryMillivolts + batteryMillivolts + 2) / 4;
    }
    SensorReading reading;
    if (!sensor.read(reading)) {
      Serial.println("Erro na leitura do BMP388");
      display.showError("Falha na leitura");
      delay(5);
      return;
    }

    display.recordReading(reading);
    logger.log(reading);
    hasReadings = true;
    shouldRender = true;
  }

  if (shouldRender) {
    display.render(uptime.currentSessionSeconds(),
                   uptime.previousSessionSeconds(),
                   uptime.hasPreviousSession(),
                   displayedBatteryMillivolts,
                   batteryPercent(displayedBatteryMillivolts));
  }

  delay(5);
}
