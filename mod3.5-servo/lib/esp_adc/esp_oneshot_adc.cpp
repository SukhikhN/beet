#include "esp_oneshot_adc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc_unit.h"
#include "esp_err.h"
#include "hal/adc_types.h"

EspOneshotAdc::EspOneshotAdc(int pin, adc_bitwidth_t bitwidth,
                             adc_atten_t attenuation)
    : pin_(pin), bitwidth_(bitwidth), attenuation_(attenuation) {}

EspOneshotAdc::~EspOneshotAdc() { Detach(); }

esp_err_t EspOneshotAdc::Attach() {
  // Skip reattachment when already configured.
  if (unit_) {
    return ESP_OK;
  }

  esp_err_t ret;

  // Resolve GPIO pin to ADC unit and channel.
  adc_unit_t unit_id;
  ret = adc_oneshot_io_to_channel(pin_, &unit_id, &channel_);
  if (ret != ESP_OK) {
    return ret;
  }

  // Retain shared unit handle before configuring channel.
  AdcUnit* unit = AdcUnit::get(unit_id);
  ret = unit->RetainOneshot();
  if (ret != ESP_OK) {
    return ret;
  }

  // Apply per-channel attenuation and bit width settings.
  adc_oneshot_unit_handle_t adc_handle = unit->handle();
  adc_oneshot_chan_cfg_t channel_config = {
      .atten = attenuation_,
      .bitwidth = bitwidth_,
  };
  ret = adc_oneshot_config_channel(adc_handle, channel_, &channel_config);
  if (ret != ESP_OK) {
    unit->ReleaseOneshot();
    return ret;
  }

  unit_ = unit;
  return ESP_OK;
}

esp_err_t EspOneshotAdc::Detach() {
  if (!unit_) {
    return ESP_OK;
  }

  // Release shared ADC unit ownership.
  const esp_err_t ret = unit_->ReleaseOneshot();
  unit_ = nullptr;
  return ret;
}

esp_err_t EspOneshotAdc::Read(int* raw_value) {
  if (!unit_) {
    return ESP_ERR_INVALID_STATE;
  }

  // Read raw sample from configured one-shot channel.
  adc_oneshot_unit_handle_t adc_handle_ = unit_->handle();
  return adc_oneshot_read(adc_handle_, channel_, raw_value);
}