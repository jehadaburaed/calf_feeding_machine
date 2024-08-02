#ifndef FlowSensor_h
#define FlowSensor_h

#include <Arduino.h>

class FlowSensor {
  public:
    FlowSensor(byte sensorPin, byte sensorInterrupt, float calibrationFactor);
    void begin();
    void update();
    float getFlowRate();
    unsigned int getFlowMilliLitres();
    unsigned long getTotalMilliLitres();
    
  private:
    static volatile byte pulseCount;
    static float flowRate;
    static unsigned int flowMilliLitres;
    static unsigned long totalMilliLitres;
    static unsigned long oldTime;
    static byte sensorPin;
    static byte sensorInterrupt;
    static float calibrationFactor;
    static void pulseCounter();
};

#endif
