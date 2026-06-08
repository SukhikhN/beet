/*
Photoresistor Noise Visualization With Optional Smoothing

This program reads a photoresistor voltage and visualizes it on a 10-LED bar.
It also computes two filtered versions of the signal (moving average and
exponential moving average) and prints all values to Serial for comparison.
Button click toggles LED display mode between raw noisy signal and smoothed signal.
*/

#include <Arduino.h>
#include <OneButton.h>

// Photoresistor input pin.
const uint8_t photo_pin = 4;

// Mode toggle button.
const uint8_t btn_pin = 13;
OneButton btn;

// LED bar.
const uint8_t led_count = 10;
const uint8_t led_pins[led_count] = {15, 16, 17, 18, 8, 3, 46, 9, 10, 11};

// Window size for simple moving average (SMA).
const uint8_t window_size = 10;
// Exponential moving average (EMA) coefficient derived from window size.
const float smooth_alpha = 2.0 / (window_size + 1);

// Expected sensor range in millivolts for LED bar.
const uint32_t max_mv = 1100;
const uint32_t min_mv = 100;

// true: show smoothed value on LEDs, false: show injected noisy value.
bool smooth_led = true;

// Computes simple moving average using a circular buffer.
uint32_t movingAverage(uint32_t current_value)
{
  // Persistent history and running sum.
  static uint32_t ma_buffer[window_size];
  static uint8_t i = 0;
  static uint32_t sum = 0;

  // Remove the oldest value from the sum, add the new value.
  sum -= ma_buffer[i];
  ma_buffer[i] = current_value;
  sum += ma_buffer[i];

  i++;
  if (i >= window_size)
    i = 0;

  return (sum / window_size);
}

// Computes exponential moving average with previous output feedback.
uint32_t expMovingAverage(uint32_t current_value)
{
  // Previous EMA output.
  static uint32_t y_prev = 0;

  // EMA recursion: y[n] = a*x[n] + (1-a)*y[n-1].
  uint32_t y = std::lround(smooth_alpha * current_value + (1 - smooth_alpha) * y_prev);

  y_prev = y;

  return y;
}

void setup()
{
  Serial.begin(115200);
  analogReadResolution(12);

  btn.setup(btn_pin, INPUT_PULLUP, true);

  // Toggle display mode on each short click.
  btn.attachClick([]()
                  {
                    smooth_led = !smooth_led;
                    Serial.printf("LED smoothing %s\r\n", smooth_led ? "enabled" : "disabled"); });

  for (uint8_t i = 0; i < led_count; i++)
    pinMode(led_pins[i], OUTPUT);
}

void loop()
{
  btn.tick();

  // Raw photoresistor voltage in millivolts.
  uint32_t photo_v = analogReadMilliVolts(photo_pin);

  // Add synthetic random jitter for noisy visualization mode.
  uint32_t noisy = photo_v + random(-50, 50);

  uint32_t smooth_ma = movingAverage(photo_v);
  uint32_t smooth_ema = expMovingAverage(photo_v);

  Serial.printf(">input:%d,noisy:%d,ma:%d,ema:%d\r\n", photo_v, noisy, smooth_ma, smooth_ema);

  uint32_t display_value = smooth_led ? smooth_ema : noisy;

  for (uint8_t i = 0; i < led_count; i++)
  {
    // Generate evenly spaced thresholds: first LED has highest threshold and last LED has lowest
    // threshold.
    uint32_t threshold = min_mv + (max_mv - min_mv) * (led_count - i - 1) / led_count;

    // Turn on LED if display value is below threshold, so when it decreases (illuminance increases),
    // LEDs turn on from first to last.
    digitalWrite(led_pins[i], display_value < threshold ? HIGH : LOW);
  }

  delay(50);
}
