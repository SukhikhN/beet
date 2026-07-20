/*
Encoder Count Publisher

This program reads a quadrature encoder through the ESP32 pulse counter, connects to Wi-Fi and MQTT broker, and
publishes the current encoder count whenever it changes.
*/

#include <stdint.h>

#include "soc/gpio_num.h"
#include "hal/gpio_types.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"

#include "esprso_encoder.h"

#include "wifi/wifi_sta.h"
#include "mqtt/client.h"

// Encoder pins configuration.
const gpio_num_t encoder_pin_a = GPIO_NUM_12;
const gpio_num_t encoder_pin_b = GPIO_NUM_13;

// Glitch filter duration in nanoseconds.
const uint32_t encoder_glitch_timeout_ns = 12787;

// MQTT topic for encoder count updates.
const char *mqtt_encoder_count_topic = "beet/mk/encoder/count";

static bool s_wifi_connected = false;
static bool s_mqtt_connected = false;

void app_main()
{
    // Configure and start the quadrature encoder reader.
    esprso_encoder_cfg_t cfg = {
        .pin_a = encoder_pin_a,
        .pin_b = encoder_pin_b,
    };
    esprso_encoder_handle_t encoder = NULL;
    ESP_ERROR_CHECK(esprso_encoder_new(&cfg, &encoder));
    ESP_ERROR_CHECK(esprso_encoder_configure_pins(encoder, GPIO_PULLUP_ONLY, GPIO_PULLUP_ONLY));
    ESP_ERROR_CHECK(esprso_encoder_set_glitch_filter(encoder, encoder_glitch_timeout_ns));
    ESP_ERROR_CHECK(esprso_encoder_enable(encoder));
    ESP_ERROR_CHECK(esprso_encoder_start(encoder));

    // Initialize NVS storage required by Wi-Fi.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Connect network services before entering the publish loop.
    s_wifi_connected = (ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_sta_init()) == ESP_OK);
    s_mqtt_connected = (ESP_ERROR_CHECK_WITHOUT_ABORT(mqtt_client_init()) == ESP_OK);

    // Disable the task watchdog because the main loop runs continuously.
    esp_task_wdt_deinit();

    int old_count = 0;
    while (true) {
        int count = 0;
        ESP_ERROR_CHECK(esprso_encoder_get_count(encoder, &count));
        if (count != old_count) {
            ESP_LOGI("main", "Encoder count: %d", count);

            if (s_wifi_connected && s_mqtt_connected) {
                // Publish each new encoder position as a decimal string.
                char count_str[12];
                snprintf(count_str, sizeof(count_str), "%d", count);
                mqtt_publish(mqtt_encoder_count_topic, count_str);
            }

            old_count = count;
        }
    }
}
