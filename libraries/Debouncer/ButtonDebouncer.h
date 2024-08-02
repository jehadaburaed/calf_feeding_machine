#ifndef ButtonDebouncer_h
#define ButtonDebouncer_h

#include <Arduino.h>

class ButtonDebouncer {
  public:
    ButtonDebouncer(int inputPin, unsigned long debounceTime);
    void begin();
    void update();
    bool getState();
    bool isPressed(); // Function to return button status

  private:
    int _inputPin;
    unsigned long _debounceTime;
    unsigned long _lastDebounceTime;
    bool _lastButtonState;
    bool _buttonState;
};

#endif
