/*
LED Binary Counter with Variable Blink Speed

Three LEDs represent the binary count from 0 to 7. The blinking speed changes dynamically, going from the minimum
LED glow time to the maximum, and then back down.
*/

#include <Arduino.h>

// LED pins.
const uint8_t led_1_pin = 15;
const uint8_t led_2_pin = 17;
const uint8_t led_3_pin = 8;

// Blink delay parameters: min/max delay and step size for increasing/decreasing the delay.
const uint16_t min_delay_ms = 200;
const uint16_t max_delay_ms = 1000;
const uint16_t delay_step_ms = 20;

void setup()
{
  Serial.begin(115200);

  pinMode(led_1_pin, OUTPUT);
  pinMode(led_2_pin, OUTPUT);
  pinMode(led_3_pin, OUTPUT);
}

void blinkNum(uint8_t num)
{
  // Lower 3 bits of num map directly to LED states: bit0->LED1, bit1->LED2, bit2->LED3.
  bool is_led_1_on = num & 0x01;
  bool is_led_2_on = num & 0x02;
  bool is_led_3_on = num & 0x04;

  digitalWrite(led_1_pin, is_led_1_on ? HIGH : LOW);
  digitalWrite(led_2_pin, is_led_2_on ? HIGH : LOW);
  digitalWrite(led_3_pin, is_led_3_on ? HIGH : LOW);
}

void loop()
{
  // Blink delay state.
  uint16_t delay_ms = min_delay_ms; // Current delay.
  bool is_delay_inc = true;         // Direction of delay change: true for increasing, false for decreasing.

  // Timing accumulator.
  uint16_t delay_acc_ms = 0;          // Accumulated delay since last update.
  uint32_t prev_update_ms = millis(); // Timestamp of last delay update.
  uint32_t update_dt_ms;              // Time between delay updates.

  while (true)
  {
    for (uint8_t num = 0; num <= 7; num++)
    {
      blinkNum(num);
      delay(delay_ms);
      delay_acc_ms += delay_ms;

      // Update delay after every ~max_delay_ms of accumulated blink time.
      if (delay_acc_ms >= max_delay_ms)
      {
        // Keep overflow so timing does not drift when we cross the boundary.
        delay_acc_ms = delay_acc_ms % max_delay_ms;

        if (is_delay_inc)
        {
          delay_ms += delay_step_ms;
          // Reverse direction at the upper bound.
          if (delay_ms >= max_delay_ms)
          {
            is_delay_inc = false;
          }
        }
        else
        {
          delay_ms -= delay_step_ms;
          // Reverse direction at the lower bound.
          if (delay_ms <= min_delay_ms)
          {
            is_delay_inc = true;
          }
        }

        update_dt_ms = millis() - prev_update_ms;
        prev_update_ms = millis();

        Serial.printf("Current delay %dms, changed after %lums.\n", delay_ms, update_dt_ms);
      }
    }
  }
}
