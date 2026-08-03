/* USER CODE BEGIN Header */
/*
LED Control Over UART

This program allows control of an LED connected to a microcontroller via UART commands. The LED can be turned on, turned
off, or its status can be queried using specific commands sent over UART.
*/

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdbool.h>

#include "usbd_cdc_if.h"

#include "led.h"
#include "cmd.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

const uint32_t debounce_delay = 20; // milliseconds

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  bool last_key_state = false;
  bool current_key_state = false;
  bool key_state = false;
  uint32_t current_time = HAL_GetTick();
  uint32_t last_debounce_time = 0;

  uint8_t uart_buffer[1];
  uint8_t cmd_buffer[16]; // Buffer to store incoming command characters
  int cmd_index = 0;

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    current_key_state = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET;
    current_time = HAL_GetTick();

    if (current_key_state != last_key_state)
    {
      last_debounce_time = current_time;
    }

    last_key_state = current_key_state;

    if (current_time - last_debounce_time > debounce_delay)
    {
      if (current_key_state != key_state)
      {
        key_state = current_key_state;

        // Toggle LED state on button press
        if (key_state)
        {
          led_get_state() ? led_off() : led_on();
        }
      }
    }

    HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, uart_buffer, sizeof(uart_buffer), 10);

    switch (status)
    {
    case HAL_OK:
      // Accumulate received characters into the command buffer until a null terminator is received
      if (cmd_index <= sizeof(cmd_buffer) - 1)
      {
        cmd_buffer[cmd_index] = uart_buffer[0];
        ++cmd_index;

        if (uart_buffer[0] == 0)
        {
          // Null terminator received, process the command
          printf("Received command: %s\r\n", cmd_buffer);
          cmd_process((char *)cmd_buffer, sizeof(cmd_buffer));
          // Reset the command buffer index for the next command
          cmd_index = 0;
        }
      }
      else
      {
        // Buffer overflow, reset index
        cmd_index = 0;
        printf("Command buffer overflow. Resetting buffer.\r\n");
      }
      break;
    case HAL_TIMEOUT:
      // No data received within the timeout period; this is normal operation.
      break;
    case HAL_ERROR:
      printf("UART receive error occurred.\r\n");
      break;
    case HAL_BUSY:
      printf("UART is busy; cannot receive data at this time.\r\n");
      break;
    }
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV6;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

int _write(int file, char *ptr, int len)
{
  (void)file;

  // Check if the USB device is actually configured and ready.
  extern USBD_HandleTypeDef hUsbDeviceFS;
  if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED)
  {
    return len; // Drop bytes silently if USB cable is unplugged.
  }

  // Attempt transmission. If busy, skip this packet to avoid deadlocks.
  if (CDC_Transmit_FS((uint8_t *)ptr, len) == USBD_BUSY)
  {
    return 0;
  }

  return len;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
