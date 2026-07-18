/*
Shared ADC unit holder with reference-counted one-shot lifecycle.
*/

#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "hal/adc_types.h"

class AdcUnit {
 public:
  static AdcUnit* get(adc_unit_t id);

  adc_oneshot_unit_handle_t handle() const { return handle_; }

  friend class EspOneshotAdc;

 private:
  adc_unit_t id_;
  adc_oneshot_unit_handle_t handle_ = nullptr;
  int ref_count_ = 0;

  static AdcUnit adc_units_[2];

  AdcUnit(adc_unit_t id);

  esp_err_t InitOneshot();
  esp_err_t DeinitOneshot();
  esp_err_t RetainOneshot();
  esp_err_t ReleaseOneshot();
};