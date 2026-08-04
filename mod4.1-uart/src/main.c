/*
UART Commander

Provides a simple command interface over UART to control a LED. Reads commands from the monitoring console and sends
them over UART to another device. Includes commands to turn the LED on, off, and check its status.
*/

#include <string.h>
#include <stdio.h>

#include "hal/uart_types.h"
#include "soc/gpio_num.h"
#include "esp_task_wdt.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"

#include "uart.h"

const gpio_num_t uart_tx_pin = GPIO_NUM_17;
const gpio_num_t uart_rx_pin = GPIO_NUM_18;
const int uart_baud_rate = 9600;

typedef enum { CMD_LED_ON = '1', CMD_LED_OFF = '0', CMD_LED_STATUS = 's' } cmd_t;

void led_on()
{
    uart_send_command(UART_CMD_LED_ON);
}

void led_off()
{
    uart_send_command(UART_CMD_LED_OFF);
}

void led_status()
{
    uart_clean_rx_buffer(UART_NUM_1); // Clear any previous data in the RX buffer
    uart_send_command(UART_CMD_LED_STATUS);

    uint8_t uart_data[16];
    int length = uart_read(UART_NUM_1, uart_data, sizeof(uart_data), pdMS_TO_TICKS(100));
    if (length > 0) {
        printf("LED status: %s\n", uart_data);
    }
}

void app_main()
{
    uart_config_t uart_config = {
        .baud_rate = uart_baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_init(UART_NUM_1, uart_tx_pin, uart_rx_pin, &uart_config));

    // Disable the task watchdog because the main loop runs continuously.
    esp_task_wdt_deinit();

    while (true) {
        int cmd = getchar();

        if (cmd != EOF) {
            printf("Received command: '%c' (%d)\n", cmd, cmd);

            switch (cmd) {
            case CMD_LED_ON:
                printf("Turning LED ON\n");
                led_on();
                break;
            case CMD_LED_OFF:
                printf("Turning LED OFF\n");
                led_off();
                break;
            case CMD_LED_STATUS:
                printf("Checking LED status\n");
                led_status();
                break;
            default:
                printf("Unknown command: '%c' (%d)\n", cmd, cmd);
                break;
            }
        }
    }
}