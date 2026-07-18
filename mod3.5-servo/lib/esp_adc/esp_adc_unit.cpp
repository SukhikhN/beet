#include "esp_adc_unit.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "hal/adc_types.h"
#include "soc/clk_tree_defs.h"

// Keep one shared wrapper per ADC hardware unit.
AdcUnit AdcUnit::adc_units_[2] = {AdcUnit(ADC_UNIT_1), AdcUnit(ADC_UNIT_2)};

AdcUnit* AdcUnit::get(adc_unit_t id) {
  // Map ADC unit id to static storage.
  switch (id) {
    case ADC_UNIT_1:
      return &adc_units_[0];
    case ADC_UNIT_2:
      return &adc_units_[1];
    default:
      return nullptr;
  }
}

AdcUnit::AdcUnit(adc_unit_t id) : id_(id) {}

esp_err_t AdcUnit::InitOneshot() {
  // Create the ESP-IDF one-shot handle for this unit.
  adc_oneshot_unit_init_cfg_t unit_config = {
      .unit_id = id_,
      .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  const esp_err_t ret = adc_oneshot_new_unit(&unit_config, &handle_);
  if (ret != ESP_OK) {
    return ret;
  }
  return ESP_OK;
}

esp_err_t AdcUnit::DeinitOneshot() {
  // Delete the one-shot handle only when it exists.
  if (handle_) {
    const esp_err_t ret = adc_oneshot_del_unit(handle_);
    if (ret != ESP_OK) {
      return ret;
    }
    handle_ = nullptr;
  }
  return ESP_OK;
}

esp_err_t AdcUnit::RetainOneshot() {
  // Lazily initialize on first user and increase reference count.
  if (ref_count_ == 0) {
    const esp_err_t ret = InitOneshot();
    if (ret != ESP_OK) {
      return ret;
    }
  }

  ++ref_count_;
  return ESP_OK;
}

esp_err_t AdcUnit::ReleaseOneshot() {
  // Drop one reference and deinitialize when last user releases.
  if (ref_count_ == 0) {
    return ESP_OK;
  }

  --ref_count_;
  if (ref_count_ == 0) {
    return DeinitOneshot();
  }
  return ESP_OK;
}