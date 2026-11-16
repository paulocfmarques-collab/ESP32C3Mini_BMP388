#include "Bmp388Sensor.h"

#include <Adafruit_BMP3XX.h>

namespace {
constexpr float SEALEVEL_PRESSURE_HPA = 1013.25F;
}

Bmp388Sensor::Bmp388Sensor(Adafruit_BMP3XX& sensor) : sensor_(sensor) {}

bool Bmp388Sensor::begin() {
  if (!sensor_.begin_I2C(0x76) && !sensor_.begin_I2C(0x77)) {
    return false;
  }

  sensor_.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  sensor_.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  sensor_.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  return true;
}

bool Bmp388Sensor::read(SensorReading& reading) {
  if (!sensor_.performReading()) {
    return false;
  }

  reading.temperature = sensor_.temperature;
  reading.pressureHpa = sensor_.pressure / 100.0F;
  reading.altitude = sensor_.readAltitude(SEALEVEL_PRESSURE_HPA);
  return true;
}
