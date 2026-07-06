/*
ADC One-Shot Read With Calibration Check

This program reads a potentiometer input on button press and prints the
raw ADC-based voltage estimate, calibrated voltage, and the percentage
error between them.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/adc_channel.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "driver/gpio.h"

// ADC input at pin 5.
const adc_unit_t adc_unit = ADC_UNIT_1;
const adc_channel_t adc_channel = ADC1_GPIO5_CHANNEL;

const uint8_t adc_bitwidth = ADC_BITWIDTH_12;

// Active-low button input pin.
const gpio_num_t btn_gpio = GPIO_NUM_13;

// Configures ADC one-shot unit, channel, and calibration handle.
void setup_adc(adc_oneshot_unit_handle_t *p_adc_handle, adc_cali_handle_t *p_cali_handle)
{
    // ADC instance initial configuration
    adc_oneshot_unit_init_cfg_t adc_unit_config = {
        .unit_id = adc_unit};

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_unit_config, p_adc_handle));

    // ADC IO configuration
    adc_oneshot_chan_cfg_t adc_channel_config = {
        .bitwidth = adc_bitwidth,
        .atten = ADC_ATTEN_DB_0,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(*p_adc_handle, adc_channel, &adc_channel_config));

    // Calibration scheme setup
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = adc_unit,
        .chan = adc_channel,
        .atten = ADC_ATTEN_DB_0,
        .bitwidth = adc_bitwidth,
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, p_cali_handle));
}

// Configures the button GPIO as input with internal pull-up.
void setup_btn()
{
    gpio_config_t btn_conf = {
        .pin_bit_mask = 1ULL << btn_gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&btn_conf));
}

// Prints table header for ADC output values.
void print_header()
{
    printf("Raw value | Voltage | Calibrated voltage | Error \n");
    printf("----------|---------|--------------------|------\n");
}

// Reads ADC once and prints table row.
void print_adc_data(adc_oneshot_unit_handle_t adc_handle, adc_cali_handle_t cali_handle)
{
    int raw_value;
    float voltage;
    int calibrated_mv;
    float calibrated_voltage;
    float error_percentage;

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channel, &raw_value));

    voltage = (float)raw_value / ((1 << adc_bitwidth) - 1) * 1.1;

    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw_value, &calibrated_mv));
    calibrated_voltage = (float)calibrated_mv / 1000.0;

    error_percentage = calibrated_voltage != 0 ? ((voltage - calibrated_voltage) / calibrated_voltage) * 100.0 : 0;

    printf("%9d | %7.3f | %18.3f | %5.2f%%\n", raw_value, voltage, calibrated_voltage, error_percentage);
}

void app_main()
{
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;
    setup_adc(&adc_handle, &cali_handle);

    setup_btn();

    print_header();

    uint8_t btn_state;
    uint8_t last_btn_state = 1;

    while (true)
    {
        btn_state = gpio_get_level(btn_gpio);

        // Trigger ADC read when button is released (active low).
        if (last_btn_state == 0 && btn_state == 1)
        {
            print_adc_data(adc_handle, cali_handle);
        }

        last_btn_state = btn_state;

        // Yield to other tasks to avoid watchdog timeout.
        // Also prevents button bounce from causing multiple reads.
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
