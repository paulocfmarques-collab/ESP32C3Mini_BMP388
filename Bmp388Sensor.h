#pragma once

#include "SensorReading.h"

class Adafruit_BMP3XX;

class Bmp388Sensor {
 public:
  explicit Bmp388Sensor(Adafruit_BMP3XX& sensor);

  bool begin();
  bool read(SensorReading& reading);

 private:
  Adafruit_BMP3XX& sensor_;
};
