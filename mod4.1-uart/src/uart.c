#include <string.h>

#include "soc/gpio_num.h"
#include "driver/uart.h"
#include "esp_err.h"

#include "esprso_err.h"

#include "uart.h"

// Command strings to be sent over UART for each command
const char *uart_cmd_strs[] = {
    [UART_CMD_LED_ON] = "LED ON", [UART_CMD_LED_OFF] = "LED OFF", [UART_CMD_LED_STATUS] = "LED STATUS"};

esp_err_t uart_init(uart_port_t uart_num, const gpio_num_t tx_pin, const gpio_num_t rx_pin,
                    const uart_config_t *uart_config)
{
    ESPRSO_RETURN_ON_ERROR(uart_driver_install(uart_num, 1024, 0, 0, NULL, 0));

    ESPRSO_RETURN_ON_ERROR(uart_param_config(uart_num, uart_config));

    return uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int uart_send_command(uart_cmd_t cmd)
{
    if (cmd < 0 || cmd >= UART_CMD_MAX) {
        return -1; // Invalid command
    }

    const char *uart_cmd_str = uart_cmd_strs[cmd];
    return uart_write_bytes(UART_NUM_1, uart_cmd_str, strlen(uart_cmd_str) + 1); // +1 to include the null terminator
}

int uart_read(uart_port_t uart_num, uint8_t *data, uint32_t data_length, TickType_t ticks_to_wait)
{
    // Subtract 1 from data_length to leave space for the null terminator.
    int bytes_read = uart_read_bytes(uart_num, data, data_length - 1, ticks_to_wait);

    if (bytes_read > 0) {
        data[bytes_read] = '\0'; // Null-terminate the received data
    }

    return bytes_read;
}
