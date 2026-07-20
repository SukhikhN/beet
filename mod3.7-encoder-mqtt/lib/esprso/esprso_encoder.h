/*
Rotary Encoder

Uses ESP32 pulse counter for configuring, starting, and reading a quadrature encoder.
*/

#include <stdint.h>

#include "soc/gpio_num.h"
#include "esp_err.h"

// Encoder pins and counter range configuration.
typedef struct {
    int low_limit;
    int high_limit;
    gpio_num_t pin_a;
    gpio_num_t pin_b;
} esprso_encoder_cfg_t;

typedef struct esprso_encoder_ctx_t *esprso_encoder_handle_t;

// Creates a new encoder instance.
esp_err_t esprso_encoder_new(const esprso_encoder_cfg_t *config, esprso_encoder_handle_t *ret_encoder);

// Releases the resources owned by an encoder instance.
esp_err_t esprso_encoder_del(esprso_encoder_handle_t encoder);

// Configures the encoder input pins and pull resistors.
esp_err_t esprso_encoder_configure_pins(esprso_encoder_handle_t encoder, gpio_pull_mode_t pull_a,
                                        gpio_pull_mode_t pull_b);

// Applies a glitch filter to suppress short input pulses.
esp_err_t esprso_encoder_set_glitch_filter(esprso_encoder_handle_t encoder, uint32_t max_glitch_ns);

// Enables the underlying pulse counter unit.
esp_err_t esprso_encoder_enable(esprso_encoder_handle_t encoder);

// Disables the underlying pulse counter unit.
esp_err_t esprso_encoder_disable(esprso_encoder_handle_t encoder);

// Starts pulse counting for the encoder.
esp_err_t esprso_encoder_start(esprso_encoder_handle_t encoder);

// Stops pulse counting for the encoder.
esp_err_t esprso_encoder_stop(esprso_encoder_handle_t encoder);

// Clears the current encoder count.
esp_err_t esprso_encoder_clear_count(esprso_encoder_handle_t encoder);

// Reads the current encoder count.
esp_err_t esprso_encoder_get_count(esprso_encoder_handle_t encoder, int *count);
