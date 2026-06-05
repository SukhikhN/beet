/*
Button Press Counter With Interrupt Debounce

This program counts button presses using a hardware interrupt.
Each interrupt is filtered by a debounce time window to ignore contact bounce.
*/

#include <Arduino.h>

// Input pin for the push button.
const uint8_t btn_pin = 5;

// Number of accepted (debounced) button presses.
volatile int16_t counter = 0;

const uint16_t debounce_delay = 20;
volatile uint32_t last_interrupt_time = 0;

// Interrupt handler: counts button presses.
void IRAM_ATTR handleBtn()
{
  uint32_t interrupt_time = millis();

  // Count on button press, ignoring bounces by accepting only events outside debounce window.
  if (digitalRead(btn_pin) == LOW && interrupt_time - last_interrupt_time > debounce_delay)
  {
    counter++;
    Serial.printf("Button Pressed! Count: %d\n", counter);
  }

  // Reset debounce timer on FALLING and RISING interrupts to ignore bounces on button release too.
  last_interrupt_time = interrupt_time;
}

void setup()
{
  Serial.begin(115200);

  pinMode(btn_pin, INPUT);

  attachInterrupt(digitalPinToInterrupt(btn_pin), handleBtn, CHANGE);
}

void loop()
{
  // Main loop is intentionally empty; logic is interrupt-driven.
}
