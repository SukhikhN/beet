/*
PWM channel wrapper bound to a GPIO pin and a configured LEDC timer.
*/

#pragma once

#include <cstdint>

#include "esp_err.h"
#include "hal/ledc_types.h"

class EspLedcTimer;

class EspLedcPwm {
 public:
  EspLedcPwm(int pin, ledc_channel_t channel, EspLedcTimer* ledc_timer);
  ~EspLedcPwm();

  esp_err_t Attach();
  esp_err_t Detach();

  esp_err_t SetDuty(std::uint32_t duty);

 private:
  int pin_;
  ledc_channel_t channel_;
  EspLedcTimer* ledc_timer_ = nullptr;

  bool attached_ = false;
};
