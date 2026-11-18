<h1 align="center">ESP32-C3 BMP388 Environmental Monitor</h1>

<p align="center">
  A modular Arduino firmware project for live environmental readings, rolling graphs, session uptime tracking, and reset diagnostics.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MCU-ESP32--C3-00979D?style=for-the-badge" alt="ESP32-C3">
  <img src="https://img.shields.io/badge/Firmware-Arduino%20C%2B%2B-00599C?style=for-the-badge" alt="Arduino C++">
  <img src="https://img.shields.io/badge/Sensor-BMP388-4D4D4D?style=for-the-badge" alt="BMP388">
  <img src="https://img.shields.io/badge/Display-SSD1306-7B2CBF?style=for-the-badge" alt="SSD1306">
</p>

<p align="center">
  <a href="./PINOUT.md">Wiring and pinout</a> |
  <a href="./changelog.md">Changelog</a>
</p>

## Overview

This project reads temperature and pressure from a BMP388, estimates altitude, and presents the data on a 128x64 SSD1306 OLED. A single push button cycles through seven screens, including a battery monitor. The firmware also records the previous session's uptime in ESP32 Preferences and uses the task watchdog to recover from a stalled main loop.

The OLED's physical yellow strip is reserved for the screen title and short status text. Sensor readings and graphs occupy the remaining display area.

## Features

- BMP388 support at I2C address `0x76`, with fallback to `0x77`.
- SSD1306 OLED support at I2C address `0x3C`.
- Seven circularly navigated screens.
- Three rolling, auto-scaled sensor graphs with 120 one-second samples.
- Current and previous session uptime, including periodic Preferences storage.
- Reset-reason diagnostics using the ESP32 reset-reason API.
- 10-second task watchdog with panic enabled.
- Separate classes for sensor access, display rendering, button input, serial logging, uptime storage, and watchdog control.

## Display screens

Press the button once to advance. After screen 7, the next press returns to screen 1.

| Screen | Content |
| --- | --- |
| 1 | Live temperature, pressure, altitude, and formatted current uptime |
| 2 | Pressure graph in hPa |
| 3 | Temperature graph in C |
| 4 | Altitude graph in m |
| 5 | Current and previous session uptime in seconds |
| 6 | Reason for the most recent ESP32 startup/reset |
| 7 | Battery charge bar, estimated percentage, and measured voltage |

Graphs use the most recent 120 successful sensor readings, sampled approximately once per second. Their vertical scale adjusts to the values in the current history window. Graph history is held in RAM and starts fresh after a reboot.

The firmware averages 16 calibrated GPIO0 readings about once per second and doubles the result for the 100 kΩ / 100 kΩ divider. The OLED smooths both voltage and percentage by blending 25% of the latest measurement with 75% of the previous displayed value. The first valid reading and a zero reading appear immediately. Serial reports each measurement before display smoothing. The displayed percentage is a linear voltage estimate from 3.20 V (0%) to 4.20 V (100%). Voltage measured while the device is running can vary with load, and the percentage does not represent remaining runtime. See [PINOUT.md](./PINOUT.md#battery-voltage-monitor) for the divider wiring.

## Hardware

- ESP32-C3 Mini development board
- BMP388 breakout
- 128x64 SSD1306 I2C OLED module
- Momentary push button
- One rechargeable 18650 Li-ion cell, a TP4056 charger module, and a voltage regulator

The assembled unit ran for approximately **33 hours on battery power** in a runtime test.

See [PINOUT.md](./PINOUT.md) for the wiring table, button connection, I2C addresses, and power notes.

## Getting started

### Arduino IDE

1. Install the ESP32 Arduino board package and select the board profile that matches your ESP32-C3 Mini.
2. Install **Adafruit BMP3XX Library**, **Adafruit SSD1306**, and **Adafruit GFX Library** using the Arduino Library Manager. Install any dependencies requested by the manager.
3. Open `ESP32C3Mini_BMP388.ino` from this sketch folder. Keep the `.cpp` and `.h` files beside the `.ino`.
4. Connect the OLED and BMP388 to the shared I2C bus, and connect the button as described in [PINOUT.md](./PINOUT.md).
5. Upload the firmware. Serial diagnostics use 115200 baud.

`Wire`, `Preferences`, the reset-reason API, and the task-watchdog API are provided by the ESP32 Arduino/ESP-IDF platform.

## Runtime behavior

### Sampling and sensor configuration

- A sensor reading is requested about once per second.
- Temperature oversampling is set to 8x and pressure oversampling to 4x.
- The BMP388 IIR filter coefficient is set to 3.
- Altitude is calculated from the same pressure sample shown on the OLED and over Serial, using a fixed sea-level reference of 1013.25 hPa.
- A drop in measured pressure raises the estimated altitude even when the sensor stays in place. Near sea level, a drop of 1 hPa corresponds to about 8 m. Compare `Pressao` and `Altitude` in the Serial Monitor at 115200 baud to check for this pattern.
- For an estimate of elevation above sea level, set `SEALEVEL_PRESSURE_HPA` in `Bmp388Sensor.cpp` to the current local pressure reduced to sea level. As weather changes, the estimate can drift again if the reference stays fixed.
- Sensor initialization or read failures are reported over Serial; read failures are also shown on the OLED.

### Uptime storage

The current session uses the ESP32-C3 internal timer (`esp_timer_get_time()`) to measure elapsed time since boot. The home screen shows days, hours, and minutes (for example, `15d09:15`). The uptime screen shows the current session and last saved session in seconds. The firmware checkpoints session milliseconds every 60 seconds in the `bmp388` Preferences namespace under the `session_ms` key. A checkpoint saved by firmware with the earlier double-counting bug can still show an inflated value until a new checkpoint is saved.

An abrupt power loss can leave up to roughly one minute of the current session unrecorded. Preferences initialization or write failures are reported over Serial. If Preferences cannot be opened, the current-session counter still runs but cannot be preserved.

For a battery runtime test, charge the battery and start a fresh session with the ESP32 powered only by the battery. After it shuts down, power the ESP32 again and check `SESSAO ANTERIOR (s)` on the uptime screen. This is the last saved elapsed time. If a session ends before its first successful checkpoint, the display can still show a checkpoint from an older session.

### Watchdog

The task watchdog is configured for a 10-second timeout with panic enabled. The main loop feeds it as it runs; if the loop stops making progress long enough, the ESP32 should reset. Expected error-wait loops for a missing display or sensor feed the watchdog intentionally so the firmware does not repeatedly reboot while reporting a hardware error.

If watchdog configuration fails, the firmware reports the failure over Serial and continues without watchdog protection.

## Serial output

The firmware prints temperature, pressure, and altitude at 115200 baud. `Bateria GPIO0` reports the raw ADC count, the calibrated voltage at the pin, and the calculated battery voltage. If the OLED shows `SEM SINAL GPIO0`, compare `pino` with a DC multimeter measurement between GPIO0 and GND. With a 3.70 V cell, expect about 1850 mV at the pin. If USB power is unstable, or an external USB-to-UART adapter is used, see the power guidance in [PINOUT.md](./PINOUT.md).

## Project structure

| File | Responsibility |
| --- | --- |
| `ESP32C3Mini_BMP388.ino` | Hardware setup, initialization, and main loop |
| `Bmp388Sensor.h/.cpp` | BMP388 initialization and readings |
| `OledDisplay.h/.cpp` | Home screen, graphs, uptime screen, and reset diagnostics |
| `PageButton.h/.cpp` | Debounced, active-low screen navigation |
| `SerialLogger.h/.cpp` | Sensor output over Serial |
| `SensorReading.h` | Shared sensor-reading data structure |
| `UptimeCounter.h/.cpp` | Current and previous session uptime with Preferences storage |
| `WatchdogManager.h/.cpp` | ESP-IDF task-watchdog setup and feeding |

## Troubleshooting

| Symptom | Checks |
| --- | --- |
| OLED is not detected | Confirm the module uses I2C address `0x3C`, and check SDA/SCL and common ground. |
| BMP388 is not detected | Check the shared I2C bus and confirm the breakout responds at `0x76` or `0x77`. |
| Display stops when powered from USB | Try a known-good USB cable and 5 V source. Do not power the whole board from a USB-to-UART adapter's 3.3 V output. |
| Button does not change screens | Connect the momentary switch between GPIO 4 and GND; the input uses `INPUT_PULLUP`. |
| Previous uptime is missing or stale | The previous value is checkpointed every 60 seconds. Check Serial output for Preferences errors. |
| Watchdog is not active | Check Serial output for `Falha ao configurar watchdog` and verify the selected ESP32 board package. |

## Documentation

- [Pinout and wiring](./PINOUT.md)
- [Changelog](./changelog.md)
