
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "hal/pcnt_ll.h"
#include "driver/pulse_cnt.h"
#include "esp_check.h"
#include "esp_err.h"

#include "esprso_encoder.h"

static const char *TAG = "esprso_encoder";

typedef struct esprso_encoder_ctx_t {
    pcnt_unit_handle_t pcnt_unit;
    pcnt_channel_handle_t pcnt_channel;
    gpio_num_t pin_a;
    gpio_num_t pin_b;
} esprso_encoder_ctx_t;

// Creates a new encoder instance.
esp_err_t esprso_encoder_new(const esprso_encoder_cfg_t *config, esprso_encoder_handle_t *ret_encoder)
{
    ESP_RETURN_ON_FALSE(config && ret_encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    // Allocate the encoder context before creating the hardware resources.
    esprso_encoder_ctx_t *encoder = heap_caps_calloc(1, sizeof(esprso_encoder_ctx_t), MALLOC_CAP_DEFAULT);
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_NO_MEM, TAG, "no mem for encoder");

    encoder->pin_a = config->pin_a;
    encoder->pin_b = config->pin_b;

    esp_err_t ret;

    // Use the full hardware counter range when custom limits are not set.
    int low_limit = config->low_limit != 0 ? config->low_limit : PCNT_LL_MIN_LIM;
    int high_limit = config->high_limit != 0 ? config->high_limit : PCNT_LL_MAX_LIM;

    // Create the pulse counter unit used to accumulate encoder steps.
    pcnt_unit_config_t pcnt_config = {
        .low_limit = low_limit,
        .high_limit = high_limit,
        .flags =
            {
                .accum_count = 0,
            },
    };

    pcnt_unit_handle_t pcnt_unit = NULL;
    ret = pcnt_new_unit(&pcnt_config, &pcnt_unit);
    if (ret != ESP_OK) {
        esprso_encoder_del(encoder);
        return ret;
    }
    encoder->pcnt_unit = pcnt_unit;

    // Route channel A edges while channel B controls direction.
    pcnt_chan_config_t pcnt_channel_config = {
        .edge_gpio_num = config->pin_a,
        .level_gpio_num = config->pin_b,
        .flags =
            {
                .invert_edge_input = 0,
                .invert_level_input = 0,
            },
    };

    pcnt_channel_handle_t pcnt_channel = NULL;
    ret = pcnt_new_channel(pcnt_unit, &pcnt_channel_config, &pcnt_channel);
    if (ret != ESP_OK) {
        esprso_encoder_del(encoder);
        return ret;
    }
    encoder->pcnt_channel = pcnt_channel;

    ret = pcnt_channel_set_edge_action(pcnt_channel, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    if (ret != ESP_OK) {
        esprso_encoder_del(encoder);
        return ret;
    }

    ret =
        pcnt_channel_set_level_action(pcnt_channel, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    if (ret != ESP_OK) {
        esprso_encoder_del(encoder);
        return ret;
    }

    *ret_encoder = encoder;

    return ESP_OK;
}

// Releases the resources owned by an encoder instance.
esp_err_t esprso_encoder_del(esprso_encoder_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    esp_err_t ret;

    if (encoder->pcnt_channel) {
        ret = pcnt_del_channel(encoder->pcnt_channel);
        if (ret != ESP_OK) {
            return ret;
        }
        encoder->pcnt_channel = NULL;
    }

    if (encoder->pcnt_unit) {
        ret = pcnt_del_unit(encoder->pcnt_unit);
        if (ret != ESP_OK) {
            return ret;
        }
        encoder->pcnt_unit = NULL;
    }

    free(encoder);

    return ESP_OK;
}

// Configures the encoder input pins and pull resistors.
esp_err_t esprso_encoder_configure_pins(esprso_encoder_handle_t encoder, gpio_pull_mode_t pull_a,
                                        gpio_pull_mode_t pull_b)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    // Configure both encoder pins as inputs without interrupts.
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << encoder->pin_a) | (1ULL << encoder->pin_b),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t ret;

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = gpio_set_pull_mode(encoder->pin_a, pull_a);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = gpio_set_pull_mode(encoder->pin_b, pull_b);
    if (ret != ESP_OK) {
        return ret;
    }

    return ESP_OK;
}

// Applies a glitch filter to suppress short input pulses.
esp_err_t esprso_encoder_set_glitch_filter(esprso_encoder_handle_t encoder, uint32_t max_glitch_ns)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    pcnt_glitch_filter_config_t pcnt_filter_config = {
        .max_glitch_ns = max_glitch_ns,
    };

    return pcnt_unit_set_glitch_filter(encoder->pcnt_unit, &pcnt_filter_config);
}

// Enables the underlying pulse counter unit.
esp_err_t esprso_encoder_enable(esprso_encoder_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    return pcnt_unit_enable(encoder->pcnt_unit);
}

// Disables the underlying pulse counter unit.
esp_err_t esprso_encoder_disable(esprso_encoder_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    return pcnt_unit_disable(encoder->pcnt_unit);
}

// Starts pulse counting for the encoder.
esp_err_t esprso_encoder_start(esprso_encoder_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    return pcnt_unit_start(encoder->pcnt_unit);
}

// Stops pulse counting for the encoder.
esp_err_t esprso_encoder_stop(esprso_encoder_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    return pcnt_unit_stop(encoder->pcnt_unit);
}

// Clears the current encoder count.
esp_err_t esprso_encoder_clear_count(esprso_encoder_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    return pcnt_unit_clear_count(encoder->pcnt_unit);
}

// Reads the current encoder count.
esp_err_t esprso_encoder_get_count(esprso_encoder_handle_t encoder, int *count)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    return pcnt_unit_get_count(encoder->pcnt_unit, count);
}
