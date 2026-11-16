# Pinout and Wiring

This guide documents the GPIO assignments configured in `ESP32C3Mini_BMP388.ino`. The BMP388 and OLED share the same I2C bus.

## ESP32-C3 Mini connections

| ESP32-C3 Mini pin | Connect to | Notes |
| --- | --- | --- |
| GPIO 8 | OLED SDA and BMP388 SDA | I2C data (`Wire.begin(8, 9)`) |
| GPIO 9 | OLED SCL and BMP388 SCL | I2C clock (`Wire.begin(8, 9)`) |
| GPIO 4 | One terminal of the push button | Active-low input with internal pull-up |
| GND | OLED GND, BMP388 GND, and the other button terminal | All devices must share ground |
| 3V3 | 3.3 V-compatible sensor/display power inputs | Check each breakout's voltage rating before connecting |

```text
GPIO 8 (SDA) ----+---- OLED SDA
                 +---- BMP388 SDA
GPIO 9 (SCL) ----+---- OLED SCL
                 +---- BMP388 SCL
3V3 ------------+---- OLED VCC/VIN*
                 +---- BMP388 VCC/VIN*
GND ------------+---- OLED GND
                 +---- BMP388 GND

GPIO 4 --------- momentary push button --------- GND
```

`*` Use the supply input and voltage specified by the breakout manufacturer. Keep I2C logic levels at 3.3 V; ESP32-C3 GPIO pins are not 5 V tolerant.

## I2C devices

| Device | Configured address |
| --- | --- |
| SSD1306 OLED | `0x3C` |
| BMP388 | `0x76`, then `0x77` fallback |

The OLED reset pin is configured as `-1`, so the sketch does not assign a GPIO to display reset.

GPIO 8 and GPIO 9 are used by the firmware for I2C. They are also strapping pins on ESP32-C3 variants; verify the exact board schematic and ensure attached hardware does not hold either line at an invalid boot level.

## Button behavior

The button is connected between GPIO 4 and GND. The firmware configures GPIO 4 with `INPUT_PULLUP`, so:

- Released: HIGH
- Pressed: LOW
- One debounced press advances one screen
- The sixth screen wraps back to the home screen

No external pull-up resistor is required for this wiring.

## Serial connection and power

Serial diagnostics run at 115200 baud. The exact USB/UART pins depend on the selected ESP32-C3 board and its USB configuration; the sketch does not assign separate UART GPIO pins.

Use the board's USB connector or a stable supply connected to the board's supported 5 V/VIN input. A USB-to-UART adapter's 3.3 V output may not provide enough current to power the ESP32-C3, OLED, and sensor together. If using a separate supply with a USB-to-UART adapter, connect grounds and use 3.3 V-safe UART signals. Do not connect independent power sources in parallel unless the board explicitly supports it.
