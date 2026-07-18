/*
Reference-counted LEDC timer wrapper shared by PWM channels.
*/

#pragma once

#include <cstdint>

#include "driver/ledc.h"
#include "esp_err.h"
#include "hal/ledc_types.h"
#include "soc/clk_tree_defs.h"

using std::uint32_t;

class EspLedcTimer {
 public:
  EspLedcTimer(ledc_timer_t timer_num, uint32_t frequency,
               ledc_timer_bit_t duty_resolution,
               ledc_clk_cfg_t clock_config = LEDC_AUTO_CLK);

  esp_err_t SetFrequency(uint32_t frequency);

  friend class EspLedcPwm;

 private:
  ledc_mode_t speed_mode_ = LEDC_LOW_SPEED_MODE;
  ledc_timer_t timer_num_;
  uint32_t frequency_;
  ledc_timer_bit_t duty_resolution_;
  ledc_clk_cfg_t clock_config_;

  bool initialized_ = false;
  int ref_count_ = 0;

  esp_err_t Init();
  esp_err_t Deinit();
  esp_err_t Retain();
  esp_err_t Release();
};
