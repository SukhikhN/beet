/*
Servo Position Control From Potentiometer

This program reads potentiometer input through ADC one-shot mode and maps the
measured range to servo PWM duty cycle.
*/

#include <cstdint>

#include "esp_err.h"
#include "esp_ledc_pwm.h"
#include "esp_ledc_timer.h"
#include "esp_numeric_utils.h"
#include "esp_oneshot_adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "hal/ledc_types.h"

using std::uint8_t, std::uint16_t, std::uint32_t;

// Potentiometer and servo pins configuration
constexpr int kPotentiometerPin = 5;
constexpr int kServoPin = 8;

// Potentiometer ADC configuration
constexpr adc_bitwidth_t kPotentiometerResolution = ADC_BITWIDTH_12;

constexpr uint8_t kPotentiometerBits =
    static_cast<uint8_t>(kPotentiometerResolution);
constexpr uint16_t kMinPotentiometer = 0;
constexpr uint16_t kMaxPotentiometer =
    static_cast<uint16_t>(esp_utils::max_for_bits(kPotentiometerBits));

// Servo PWM configuration
constexpr uint32_t kServoFrequency = 50;  // 50 Hz for standard servo motors.
constexpr ledc_timer_bit_t kServoResolution = LEDC_TIMER_9_BIT;

constexpr uint8_t kServoBits = static_cast<uint8_t>(kServoResolution);
constexpr uint16_t kMaxServo =
    static_cast<uint16_t>(esp_utils::max_for_bits(kServoBits));
constexpr uint16_t kServoPeriod = 1000000 / kServoFrequency;  // in microseconds
// Servo duty range for 0.5ms to 2.5ms pulse width.
constexpr uint16_t kMinServoDuty = static_cast<uint16_t>(
    esp_utils::map_range(500, 0, kServoPeriod, 0, kMaxServo));
constexpr uint16_t kMaxServoDuty = static_cast<uint16_t>(
    esp_utils::map_range(2500, 0, kServoPeriod, 0, kMaxServo));

extern "C" void app_main() {
  // Configure ADC one-shot reader for potentiometer input.
  EspOneshotAdc potentiometer_adc = EspOneshotAdc(
      kPotentiometerPin, kPotentiometerResolution, ADC_ATTEN_DB_0);

  ESP_ERROR_CHECK(potentiometer_adc.Attach());

  // Configure LEDC timer and PWM channel for servo output.
  EspLedcTimer ledc_timer =
      EspLedcTimer(LEDC_TIMER_0, kServoFrequency, kServoResolution);
  EspLedcPwm servo_pwm = EspLedcPwm(kServoPin, LEDC_CHANNEL_0, &ledc_timer);

  ESP_ERROR_CHECK(servo_pwm.Attach());

  while (true) {
    // Read ADC and convert potentiometer position to servo duty.
    int potentiometer_value = 0;
    ESP_ERROR_CHECK(potentiometer_adc.Read(&potentiometer_value));

    uint32_t servo_duty = static_cast<uint32_t>(
        esp_utils::map_range(potentiometer_value, kMinPotentiometer,
                             kMaxPotentiometer, kMinServoDuty, kMaxServoDuty));

    ESP_ERROR_CHECK(servo_pwm.SetDuty(servo_duty));

    // Small delay and yields CPU time to avoid watchdog timeout.
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}