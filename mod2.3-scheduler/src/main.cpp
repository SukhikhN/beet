/*
Scheduled Multi-LED Blink

This program uses TaskScheduler to blink three LEDs at different intervals.
*/

#include <Arduino.h>

#define _TASK_STD_FUNCTION
#include <TaskScheduler.h>

struct LEDConfig
{
  uint8_t pin;
  uint32_t interval;
};

// LED pin and blink period configuration.
constexpr LEDConfig led_configs[] = {
    {15, 200},
    {17, 500},
    {8, 1000}};

constexpr uint8_t led_count = sizeof(led_configs) / sizeof(LEDConfig);

// How long each LED stays on for one blink pulse.
constexpr uint8_t led_on_ms = 50;

Scheduler runner;

void ledOn(uint8_t index);
void ledOff(uint8_t index);

void setup()
{
  Serial.begin(115200);

  // Configure all LED pins and create periodic blink tasks.
  for (uint8_t i = 0; i < led_count; i++)
  {
    pinMode(led_configs[i].pin, OUTPUT);

    new Task(led_configs[i].interval, TASK_FOREVER, [i]()
             { ledOn(i); }, &runner, true);
  }
}

void loop()
{
  runner.execute();
}

// Turns one LED on and schedules a one-shot task to turn it off.
void ledOn(uint8_t index)
{
  digitalWrite(led_configs[index].pin, HIGH);
  Serial.printf("LED %d on\n", index);

  new Task(led_on_ms, 1, [index]()
           { ledOff(index); }, &runner, true);
}

// Turns one LED off.
void ledOff(uint8_t index)
{
  digitalWrite(led_configs[index].pin, LOW);
  Serial.printf("LED %d off\n", index);
}
