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

void SerialLogger::logBattery(uint16_t rawAdc, uint32_t batteryMillivolts) const {
  Serial.print("Bateria GPIO0: ADC=");
  Serial.print(rawAdc);
  Serial.print("; pino=");
  Serial.print(batteryMillivolts / 2.0, 1);
  Serial.print(" mV; bateria=");
  Serial.print(batteryMillivolts);
  Serial.println(" mV");
}
