/*#include "ButtonDebouncer.h"

ButtonDebouncer::ButtonDebouncer(int inputPin, int outputPin, unsigned long debounceTime) {
  _inputPin = inputPin;
  _outputPin = outputPin;
  _debounceTime = debounceTime;
}

void ButtonDebouncer::begin() {
  pinMode(_inputPin, INPUT);
  pinMode(_outputPin, OUTPUT);
}

void ButtonDebouncer::update() {
  bool reading = digitalRead(_inputPin);

  if (reading != _lastButtonState) {
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > _debounceTime) {
    if (reading != _buttonState) {
      _buttonState = reading;
      digitalWrite(_outputPin, _buttonState);
    }
  }

  _lastButtonState = reading;
}

bool ButtonDebouncer::getState() {
  return _buttonState;
}

bool ButtonDebouncer::isPressed() {
  return _buttonState == HIGH; // Assuming the button is active HIGH
}
*/
#include "ButtonDebouncer.h"

ButtonDebouncer::ButtonDebouncer(int inputPin, unsigned long debounceTime) {
  _inputPin = inputPin;
  _debounceTime = debounceTime;
}

void ButtonDebouncer::begin() {
  pinMode(_inputPin, INPUT_PULLUP); // Use INPUT_PULLUP to enable internal pull-up resistor
}

void ButtonDebouncer::update() {
  bool reading = digitalRead(_inputPin); // Invert reading for active low button

  if (reading != _lastButtonState) {
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > _debounceTime) {
    if (reading != _buttonState) {
      _buttonState = reading;
    }
  }

  _lastButtonState = reading;
}

bool ButtonDebouncer::getState() {
  return _buttonState;
}

bool ButtonDebouncer::isPressed() {
  return _buttonState == LOW; // Check for LOW state for active low button
}
