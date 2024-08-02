#include "FlowSensor.h"

// Define static member variables
volatile byte FlowSensor::pulseCount = 0;
float FlowSensor::flowRate = 0.0;
unsigned int FlowSensor::flowMilliLitres = 0;
unsigned long FlowSensor::totalMilliLitres = 0;
unsigned long FlowSensor::oldTime = 0;
byte FlowSensor::sensorPin = 0;
byte FlowSensor::statusLed = 0;
byte FlowSensor::sensorInterrupt = 0;
float FlowSensor::calibrationFactor = 0.0;

FlowSensor::FlowSensor(byte _sensorPin, byte _sensorInterrupt, float _calibrationFactor) {
  sensorPin = _sensorPin;
  sensorInterrupt = _sensorInterrupt;
  calibrationFactor = _calibrationFactor;
}

void FlowSensor::begin() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  oldTime = 0;
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void FlowSensor::update() {
  
  if((millis() - oldTime) > 1000) {
    detachInterrupt(sensorInterrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    pulseCount = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}

float FlowSensor::getFlowRate() {
  return flowRate;
}

unsigned int FlowSensor::getFlowMilliLitres() {
  return flowMilliLitres;
}

unsigned long FlowSensor::getTotalMilliLitres() {
  return totalMilliLitres;
}

void FlowSensor::pulseCounter() {
  pulseCount++;
}
