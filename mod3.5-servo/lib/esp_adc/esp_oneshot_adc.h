/*
One-shot ADC channel wrapper bound to a GPIO pin.
*/

#pragma once

#include "esp_err.h"
#include "hal/adc_types.h"

class AdcUnit;

class EspOneshotAdc {
 public:
  EspOneshotAdc(int pin, adc_bitwidth_t bitwidth,
                adc_atten_t attenuation = ADC_ATTEN_DB_0);
  ~EspOneshotAdc();

  esp_err_t Attach();
  esp_err_t Detach();

  esp_err_t Read(int* raw_value);

 private:
  int pin_;
  adc_bitwidth_t bitwidth_;
  adc_atten_t attenuation_;
  AdcUnit* unit_ = nullptr;
  adc_channel_t channel_;
};