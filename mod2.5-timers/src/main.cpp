/*
Timed Relay Pulse

This program uses two hardware timers to alternate a relay between on and off states.
*/

#include <Arduino.h>

// Relay pin and pulse duration configuration (in seconds).
constexpr uint8_t relay_pin = 4;
constexpr uint16_t on_time = 2;
constexpr uint16_t off_time = 5;

hw_timer_t *turn_on_timer;
hw_timer_t *turn_off_timer;

// Turns the relay on and arms the timer that will turn it back off.
void IRAM_ATTR relayOn()
{
  digitalWrite(relay_pin, LOW);
  timerStop(turn_on_timer);
  timerStart(turn_off_timer);
}

// Turns the relay off and arms the timer that will turn it back on.
void IRAM_ATTR relayOff()
{
  digitalWrite(relay_pin, HIGH);
  timerStop(turn_off_timer);
  timerStart(turn_on_timer);
}

// Creates and configures one hardware timer for periodic callbacks.
hw_timer_t *setupTimer(uint8_t num, uint16_t time, void (*callback)())
{
  hw_timer_t *timer = timerBegin(num, 80, true); // 80 prescaler for 1 MHz timer frequency (1 tick = 1 microsecond).
  timerStop(timer);
  timerAlarmWrite(timer, time * 1000000, true); // Convert seconds to microseconds for the alarm value.
  timerAttachInterrupt(timer, callback, false);
  timerAlarmEnable(timer);
  return timer;
}

void setup()
{
  Serial.begin(115200);

  pinMode(relay_pin, OUTPUT);

  // Configure timers for alternating off and on phases.
  turn_on_timer = setupTimer(0, off_time, &relayOn);
  turn_off_timer = setupTimer(1, on_time, &relayOff);

  relayOn();
}

void loop()
{
  // Main loop is intentionally empty; logic is interrupt-driven.
}
