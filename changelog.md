# Changelog

All notable project changes are documented here.

## Unreleased

### Added

- Modular BMP388 sensor, OLED display, page-button, serial logger, uptime, and watchdog components.
- Six OLED screens: live readings, pressure graph, temperature graph, altitude graph, uptime counters, and reset reason.
- Circular single-button navigation with input debouncing.
- A rolling history of up to 120 sensor readings for graph screens.
- Current and previous session uptime, with a Preferences checkpoint every 60 seconds.
- A 10-second ESP32 task watchdog with panic enabled.
- GitHub-oriented project documentation and a dedicated wiring/pinout guide.

### Notes

- Graph history is volatile and is cleared on reboot.
- A sudden power loss can discard up to roughly 60 seconds of the current session uptime.
- The firmware reports watchdog and Preferences initialization/write failures over Serial.
