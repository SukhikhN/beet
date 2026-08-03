#pragma once

#include "soc/gpio_num.h"
#include "hal/uart_types.h"
#include "driver/uart.h"
#include "esp_err.h"

typedef enum { UART_CMD_LED_ON = 0, UART_CMD_LED_OFF, UART_CMD_LED_STATUS, UART_CMD_MAX } uart_cmd_t;

esp_err_t uart_init(uart_port_t uart_num, const gpio_num_t tx_pin, const gpio_num_t rx_pin,
                    const uart_config_t *uart_config);

int uart_send_command(uart_cmd_t cmd);

int uart_read(uart_port_t uart_num, uint8_t *data, uint32_t data_length, TickType_t ticks_to_wait);
