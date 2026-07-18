#include "esp_ledc_timer.h"

#include <cstdint>

#include "driver/ledc.h"
#include "esp_err.h"
#include "hal/ledc_types.h"

using std::uint32_t;

EspLedcTimer::EspLedcTimer(ledc_timer_t timer_num, uint32_t frequency,
                           ledc_timer_bit_t duty_resolution,
                           ledc_clk_cfg_t clock_config)
    : timer_num_(timer_num),
      frequency_(frequency),
      duty_resolution_(duty_resolution),
      clock_config_(clock_config) {}

esp_err_t EspLedcTimer::Init() {
  // Configure and start LEDC timer with requested frequency/resolution.
  ledc_timer_config_t timer_config = {
      .speed_mode = speed_mode_,
      .duty_resolution = duty_resolution_,
      .timer_num = timer_num_,
      .freq_hz = frequency_,
      .clk_cfg = clock_config_,
      .deconfigure = false,
  };

  const esp_err_t result = ledc_timer_config(&timer_config);
  if (result != ESP_OK) {
    return result;
  }

  initialized_ = true;
  return ESP_OK;
}

esp_err_t EspLedcTimer::Deinit() {
  // Pause and deconfigure timer only if it was initialized.
  if (!initialized_) {
    return ESP_OK;
  }

  esp_err_t ret;

  ret = ledc_timer_pause(speed_mode_, timer_num_);
  if (ret != ESP_OK) {
    return ret;
  }

  ledc_timer_config_t timer_config = {
      .speed_mode = speed_mode_,
      .duty_resolution = duty_resolution_,
      .timer_num = timer_num_,
      .freq_hz = frequency_,
      .clk_cfg = clock_config_,
      .deconfigure = true,
  };

  ret = ledc_timer_config(&timer_config);
  if (ret != ESP_OK) {
    const esp_err_t resume_ret = ledc_timer_resume(speed_mode_, timer_num_);
    if (resume_ret != ESP_OK) {
      return resume_ret;
    }
    return ret;
  }

  initialized_ = false;
  return ESP_OK;
}

esp_err_t EspLedcTimer::Retain() {
  // Initialize lazily on first owner.
  if (ref_count_ == 0) {
    const esp_err_t ret = Init();
    if (ret != ESP_OK) {
      return ret;
    }
  }

  ++ref_count_;
  return ESP_OK;
}

esp_err_t EspLedcTimer::Release() {
  // Deinitialize once the last owner releases the timer.
  if (ref_count_ == 0) {
    return ESP_OK;
  }

  --ref_count_;
  if (ref_count_ == 0) {
    return Deinit();
  }

  return ESP_OK;
}

esp_err_t EspLedcTimer::SetFrequency(uint32_t frequency) {
  // Runtime frequency updates require an initialized timer.
  if (!initialized_) {
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t ret = ledc_set_freq(speed_mode_, timer_num_, frequency);
  if (ret != ESP_OK) {
    return ret;
  }

  frequency_ = frequency;
  return ESP_OK;
}
