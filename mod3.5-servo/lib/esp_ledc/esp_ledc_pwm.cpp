#include "esp_ledc_pwm.h"

#include <cstdint>

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_ledc_timer.h"
#include "hal/ledc_types.h"

EspLedcPwm::EspLedcPwm(int pin, ledc_channel_t channel,
                       EspLedcTimer* ledc_timer)
    : pin_(pin), channel_(channel), ledc_timer_(ledc_timer) {}

EspLedcPwm::~EspLedcPwm() { Detach(); }

esp_err_t EspLedcPwm::Attach() {
  // Skip setup when channel is already attached.
  if (attached_) {
    return ESP_OK;
  }

  // A valid timer is required to bind PWM channel configuration.
  if (!ledc_timer_) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret;

  // Retain shared timer before channel configuration.
  ret = ledc_timer_->Retain();
  if (ret != ESP_OK) {
    return ret;
  }

  // Configure channel output on the selected GPIO and timer.
  ledc_channel_config_t channel_config = {
      .gpio_num = pin_,
      .speed_mode = ledc_timer_->speed_mode_,
      .channel = channel_,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = ledc_timer_->timer_num_,
      .duty = 0,
      .hpoint = 0,
      .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
      .flags =
          {
              .output_invert = 0,
          },
      .deconfigure = false,
  };

  ret = ledc_channel_config(&channel_config);
  if (ret != ESP_OK) {
    ledc_timer_->Release();
    return ret;
  }

  attached_ = true;
  return ESP_OK;
}

esp_err_t EspLedcPwm::Detach() {
  if (!attached_) {
    return ESP_OK;
  }

  // Stop output and release timer ownership.
  esp_err_t ret;

  ret = ledc_stop(ledc_timer_->speed_mode_, channel_, 0);
  if (ret != ESP_OK) {
    return ret;
  }

  ledc_channel_config_t channel_config = {
      .gpio_num = pin_,
      .speed_mode = ledc_timer_->speed_mode_,
      .channel = channel_,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = ledc_timer_->timer_num_,
      .duty = 0,
      .hpoint = 0,
      .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
      .flags =
          {
              .output_invert = 0,
          },
      .deconfigure = true,
  };
  ret = ledc_channel_config(&channel_config);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = ledc_timer_->Release();
  attached_ = false;
  return ret;
}

esp_err_t EspLedcPwm::SetDuty(std::uint32_t duty) {
  // Duty updates require an attached PWM channel.
  if (!attached_) {
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t ret = ledc_set_duty(ledc_timer_->speed_mode_, channel_, duty);
  if (ret != ESP_OK) {
    return ret;
  }

  return ledc_update_duty(ledc_timer_->speed_mode_, channel_);
}
