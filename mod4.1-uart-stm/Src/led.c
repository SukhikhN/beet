#include <stdbool.h>
#include <stdio.h>

#include "led.h"

#include "gpio.h"
#include "main.h"

static bool led_state = false;

void led_on()
{
  led_state = true;
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  printf("LED state: ON\r\n");
}

void led_off()
{
  led_state = false;
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  printf("LED state: OFF\r\n");
}

bool led_get_state()
{
  return led_state;
}
