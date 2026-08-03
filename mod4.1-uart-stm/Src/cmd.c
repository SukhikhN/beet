#include <string.h>
#include <stdio.h>

#include "cmd.h"

#include "usart.h"
#include "led.h"

typedef enum
{
  CMD_LED_ON = 0,
  CMD_LED_OFF,
  CMD_LED_STATUS,
  CMD_MAX
} cmd_t;

static const char *cmd_strs[] = {[CMD_LED_ON] = "LED ON", [CMD_LED_OFF] = "LED OFF", [CMD_LED_STATUS] = "LED STATUS"};

bool cmd_process(const char *cmd, size_t sz)
{
  // Find the command in the list of known commands
  for (int i = 0; i < CMD_MAX; ++i)
  {
    if (strncmp(cmd, cmd_strs[i], sz) == 0)
    {
      switch (i)
      {
      case CMD_LED_ON:
        led_on();
        return true;
      case CMD_LED_OFF:
        led_off();
        return true;
      case CMD_LED_STATUS:
      {
        bool state = led_get_state();
        uint8_t *message = (uint8_t *)(state ? "ON" : "OFF");
        HAL_UART_Transmit(&huart1, message, strlen((char *)message), HAL_MAX_DELAY);
        return true;
      }
      default:
        printf("Unknown command: %s\r\n", cmd);
        return false; // Command not recognized
      }
    }
  }

  printf("Unknown command: %s\r\n", cmd);
  return false; // Command not recognized
}