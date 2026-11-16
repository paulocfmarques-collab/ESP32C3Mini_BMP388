#include "SerialLogger.h"

#include <Arduino.h>

void SerialLogger::log(const SensorReading& reading) const {
  Serial.println("-------------");
  Serial.print("Temperatura: ");
  Serial.print(reading.temperature);
  Serial.println(" C");
  Serial.print("Pressao: ");
  Serial.print(reading.pressureHpa);
  Serial.println(" hPa");
  Serial.print("Altitude: ");
  Serial.print(reading.altitude);
  Serial.println(" m");
}
