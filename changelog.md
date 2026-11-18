# Changelog

All notable project changes are documented here.

## Unreleased

### Added

- Modular BMP388 sensor, OLED display, page-button, serial logger, uptime, and watchdog components.
- Seven OLED screens: live readings, pressure graph, temperature graph, altitude graph, uptime counters, reset reason, and battery status.
- Battery charge bar, estimated percentage, and voltage from the GPIO0 divider with two 100 kΩ resistors.
- GPIO0 battery diagnostics over Serial and a no-signal indication when the calibrated ADC reading is zero.
- Circular single-button navigation with input debouncing.
- A rolling history of up to 120 sensor readings for graph screens.
- Current and previous session uptime, with a Preferences checkpoint every 60 seconds.
- A 10-second ESP32 task watchdog with panic enabled.
- GitHub-oriented project documentation and a dedicated wiring/pinout guide.

### Changed

- Smooth battery voltage and percentage on the OLED while keeping the Serial readings available before display smoothing.
- Measure session uptime directly with the ESP32-C3 internal timer and retain the 60-second Preferences checkpoints for battery runtime measurements.

### Fixed

- Calculate altitude from the same pressure sample reported on the OLED and over Serial, avoiding a second sensor read.
- Prevent uptime from counting elapsed time again after a successful Preferences checkpoint. Checkpoints saved by the earlier firmware may still contain inflated durations.

### Notes

- Documented the 18650 cell, TP4056 charger, voltage regulator, and observed battery runtime of about 33 hours.

- Graph history is volatile and is cleared on reboot.
- A sudden power loss can discard up to roughly 60 seconds of the current session uptime.
- The firmware reports watchdog and Preferences initialization/write failures over Serial.
